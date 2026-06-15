# 🃏 Uni-Uno

Un'implementazione multigiocatore in tempo reale del gioco di carte **UNO**, sicura
(HTTPS/WSS), con **backend in C++23** (uWebSockets), persistenza su **SQLite** e
**frontend in Svelte 5**.

I giocatori possono registrarsi, creare o unirsi a lobby (anche tramite codice di
invito a 6 caratteri), configurare le regole della partita e giocare contro altri
utenti o contro dei bot.

### ✨ Funzionalità principali

- **Autenticazione** con email/password, sessione via cookie HttpOnly e token JWT (HS256).
- **Lobby** pubbliche/private, gestione dei membri (host, promozione, espulsione),
  riconnessione automatica dopo una disconnessione.
- **Motore di gioco** estendibile basato su una coda di _effetti_ e _regole_, con
  supporto a mod opzionali: _Draw Stacking_, _Progressive_, _Force Play_, _Jump In_,
  _No Bluffing_, _Seven-Zero_.
- **Bot** con euristiche, usati anche per gestire i turni in caso di inattività (AFK).
- **Animazioni** delle carte (volo dal mazzo alla mano e agli scarti, contatore `+N`
  per le penalità accumulate).
- **Salvataggio e ripresa** di partite interrotte, **statistiche** personali e
  **classifica** globale.

---

## 🛠️ Prerequisiti di Sistema

### Backend (C++)

- Compilatore C++ con supporto a **C++23** (GCC o Clang)
- **Python 3** (con `pip`) — usato per installare Conan e Ninja
- **CMake**
- **OpenSSL** — per generare i certificati TLS

### Frontend (Svelte)

- **Node.js** e **npm**

_Installazione dei tool di base:_

#### 🐧 Linux

- **Fedora/RHEL**: `sudo dnf install gcc-c++ python3 pip cmake openssl nodejs npm`
- **Ubuntu/Debian**: `sudo apt install g++ python3 python3-pip cmake openssl nodejs npm`

#### 🪟 Windows

1. **Compilatore**: [Visual Studio 2022](https://visualstudio.microsoft.com/) con il
   carico di lavoro _"Sviluppo di applicazioni desktop con C++"_.
2. **Python**: da [python.org](https://www.python.org/downloads/) o dal Microsoft Store.
3. **CMake**: dal [sito ufficiale](https://cmake.org/download/).
4. **Node.js** (include npm): da [nodejs.org](https://nodejs.org/).
5. **OpenSSL**: incluso in **Git Bash**, oppure installabile con
   `winget install ShiningLight.OpenSSL.Light` (o `choco install openssl`).

---

## 🚀 Setup e Build

### Clona il repository

```bash
git clone https://github.com/Eldyn/uni-uno.git
cd uni-uno
```

### Setup automatico (consigliato)

Una volta installati i [prerequisiti](#️-prerequisiti-di-sistema), un singolo script
porta il progetto da una checkout pulita a un eseguibile pronto all'uso. Lo script:

1. verifica i prerequisiti di sistema e segnala con chiarezza ciò che manca;
2. genera certificati TLS self-signed per `localhost` (`cert.pem` / `key.pem`) se assenti;
3. genera un file `.env` con segreti casuali (`JWT_SECRET`, `PASSWORD_PEPPER`) se assente;
4. compila il frontend Svelte nella cartella `public/`;
5. installa **Conan** + **Ninja** e scarica le dipendenze del backend;
6. configura e compila il backend;
7. lascia che CMake copi `cert.pem`, `key.pem`, `.env` e `public/` **accanto all'eseguibile**.

```bash
# Linux / macOS
chmod +x setup.sh
./setup.sh
```

```powershell
# Windows (PowerShell)
./setup.ps1
# Se PowerShell blocca lo script per le policy di esecuzione:
#   powershell -ExecutionPolicy Bypass -File .\setup.ps1
```

A fine esecuzione lo script stampa il comando per avviare il server (vedi
[Esecuzione](#-esecuzione)).

---

### Build manuale (passo per passo)

Se preferisci controllare ogni fase — o lo script automatico fallisce — puoi
riprodurne i passaggi manualmente.

#### 1. Certificati TLS

Il server usa HTTPS/WSS, quindi servono `key.pem` e `cert.pem` nella root del progetto.
Per lo sviluppo locale basta un certificato self-signed per `localhost`:

```bash
# Linux / macOS / Git Bash
openssl req -newkey rsa:2048 -nodes -x509 -days 365 \
  -keyout key.pem -out cert.pem \
  -subj "/C=IT/ST=Italy/L=Uniba/O=uni-uno/CN=localhost"
```

```powershell
# Windows (PowerShell, con OpenSSL nel PATH)
openssl req -newkey rsa:2048 -nodes -x509 -days 365 `
  -keyout key.pem -out cert.pem `
  -subj "/C=IT/ST=Italy/L=Uniba/O=uni-uno/CN=localhost"
```

#### 2. File `.env`

Il backend richiede `JWT_SECRET` e `PASSWORD_PEPPER`. Crea un file `.env` nella root:

```bash
printf 'JWT_SECRET=%s\nPASSWORD_PEPPER=%s\n' "$(openssl rand -hex 32)" "$(openssl rand -hex 32)" > .env
```

#### 3. Frontend

Compila il frontend Svelte e copialo nella cartella `public/` servita dal server:

```bash
./deploy_frontend.sh
```

> Lo script esegue `npm install`/`npm run build` nella cartella `frontend/`. Vedi
> [`frontend/README.md`](frontend/README.md) per i dettagli e per la modalità di sviluppo.
> **Importante:** `public/` va generata _prima_ di configurare CMake, perché la copia
> automatica degli asset (punto 5) ne rileva la presenza in fase di configurazione.

#### 4. Dipendenze del backend (Conan + Ninja)

```bash
pip install --upgrade --user conan ninja
conan profile detect --force
conan install . --build=missing -s build_type=Release -s compiler.cppstd=20 \
  -c tools.cmake.cmaketoolchain:generator=Ninja
```

#### 5. Build del backend

Usa i preset CMake generati automaticamente da Conan:

```bash
cmake --preset conan-release
cmake --build --preset conan-release
```

In fase di build CMake copia `cert.pem`, `key.pem`, `.env` e la cartella `public/`
accanto all'eseguibile, così il binario può essere avviato anche direttamente dalla
sua cartella di build.

> **Nota per gli sviluppatori (clangd / LSP):** Conan genera `compile_commands.json`
> in `build/Release/generators/`. Se il tuo editor lo richiede nella root del progetto:
>
> - Linux: `ln -s build/Release/generators/compile_commands.json`
> - Windows: `New-Item -ItemType SymbolicLink -Path "compile_commands.json" -Target "build\Release\generators\compile_commands.json"`

---

## 🏃 Esecuzione

Il server cerca certificati, `.env`, il database `game.db` e la cartella `public/`
nella **directory di lavoro corrente**. Poiché la build copia gli asset di runtime
accanto all'eseguibile, puoi avviarlo in due modi equivalenti:

```bash
# Linux / macOS — dalla root del progetto
./build/Release/uno_server

# …oppure dalla cartella di build (asset già copiati accanto al binario)
cd build/Release && ./uno_server
```

```powershell
# Windows — dalla root del progetto
build\Release\uno_server.exe

# …oppure dalla cartella di build
cd build\Release; .\uno_server.exe
```

Il database SQLite (`game.db`) viene creato automaticamente al primo avvio se non
esiste. Per impostazione predefinita il server è raggiungibile su
**https://localhost:9999** (accetta l'avviso del browser sul certificato self-signed).

---

## 📚 Documentazione

La documentazione completa è già allegata come **`documentazione-completa.pdf`** (analisi,
casi d'uso e test del frontend + manuale del backend Doxygen + API del frontend TypeDoc).

Per **rigenerarla** servono `doxygen`, `xelatex` (TeX Live), `mutool` (MuPDF) e Node.js;
quindi:

```bash
./docs/build-full-docs.sh
```
