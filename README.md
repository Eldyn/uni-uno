# 🃏 Uni-Uno

Un'implementazione sicura (HTTPS/WSS) del gioco UNO, con backend in C++, database SQLite per la persistenza e frontend in Svelte.

---

## Dipendenze del Sistema

### 🐧 Linux (Fedora/Ubuntu)

Assicurati di avere installato il gruppo di sviluppo C++ e le librerie SSL:

- **Fedora**: `sudo dnf install cmake openssl-devel sqlite-devel zlib-devel g++`
- **Ubuntu/Debian**: `sudo apt install cmake libssl-dev libsqlite3-dev zlib1g-dev g++`

### 🪟 Windows

1. **Compilatore**: [Visual Studio 2022](https://visualstudio.microsoft.com/) (con l'estensione "Sviluppo di applicazioni desktop con C++") oppure **MinGW-w64**.
2. **CMake**: Scaricabile dal [sito ufficiale](https://cmake.org/download/).
3. [vcpkg](https://github.com/microsoft/vcpkg) (`vcpkg install sqlite3 openssl zlib`) o gli installer binari.
4. **Git Bash**: Consigliato per eseguire i comandi di generazione certificati.

---

## 🚀 Setup e Build (Backend)

Clona la repo e le dipendenze:

```bash
git clone --recursive https://github.com/Eldyn/uni-uno.git
cd uni-uno
```

Genera i certificati SSL:

```bash
openssl req -newkey rsa:2048 -new -nodes -x509 -days 365 -keyout key.pem -out cert.pem -subj "/C=IT/ST=Italy/L=Uniba/O=uni-uno/CN=localhost"
```

Compila (e genera il `compile_commands.json` per clangd)

```bash
./deploy_frontend.sh
mkdir build
cd build
cmake ..
ln -s ./compile_commands.json .. # collega compile_commands.json alla root del progetto
cmake --build .
```

Attenzione ad eseguire il server nella cartella dove sono presenti i certificati, il frontend, e il database, e non nella cartella build.
