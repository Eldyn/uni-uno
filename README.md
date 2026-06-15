# 🃏 Uni-Uno

A secure (HTTPS/WSS), real-time multiplayer implementation of the **UNO** card game,
with a **C++23 backend** (uWebSockets), **SQLite** persistence and a **Svelte 5
frontend**.

Players can register, create or join lobbies (including via a 6-character invite
code), configure the match rules and play against other users or against bots.

### ✨ Key features

- **Authentication** with email/password, session via HttpOnly cookie and JWT token (HS256).
- **Public/private lobbies**, member management (host, promotion, kicking),
  automatic reconnection after a disconnection.
- **Extensible game engine** based on a queue of _effects_ and _rules_, with support
  for optional mods: _Draw Stacking_, _Progressive_, _Force Play_, _Jump In_,
  _No Bluffing_, _Seven-Zero_.
- **Bots** with heuristics, also used to handle turns in case of inactivity (AFK).
- **Card animations** (flight from the deck to the hand and to the discard pile, `+N`
  counter for accumulated penalties).
- **Saving and resuming** interrupted matches, personal **statistics** and a global
  **leaderboard**.

---

## 🛠️ System prerequisites

### Backend (C++)

- A C++ compiler with **C++23** support (GCC or Clang)
- **Python 3** (with `pip`) — used to install Conan and Ninja
- **CMake**
- **OpenSSL** — to generate the TLS certificates

### Frontend (Svelte)

- **Node.js** and **npm**

_Installing the base tools:_

#### 🐧 Linux

- **Fedora/RHEL**: `sudo dnf install gcc-c++ python3 pip cmake openssl nodejs npm`
- **Ubuntu/Debian**: `sudo apt install g++ python3 python3-pip cmake openssl nodejs npm`

#### 🪟 Windows

1. **Compiler**: [Visual Studio 2022](https://visualstudio.microsoft.com/) with the
   _"Desktop development with C++"_ workload.
2. **Python**: from [python.org](https://www.python.org/downloads/) or the Microsoft Store.
3. **CMake**: from the [official site](https://cmake.org/download/).
4. **Node.js** (includes npm): from [nodejs.org](https://nodejs.org/).
5. **OpenSSL**: included in **Git Bash**, or installable with
   `winget install ShiningLight.OpenSSL.Light` (or `choco install openssl`).

---

## 🚀 Setup and Build

### Clone the repository

```bash
git clone https://github.com/Eldyn/uni-uno.git
cd uni-uno
```

### Automatic setup (recommended)

Once the [prerequisites](#️-system-prerequisites) are installed, a single script takes
the project from a clean checkout to a ready-to-use executable. The script:

1. checks the system prerequisites and clearly reports what is missing;
2. generates self-signed TLS certificates for `localhost` (`cert.pem` / `key.pem`) if absent;
3. generates a `.env` file with random secrets (`JWT_SECRET`, `PASSWORD_PEPPER`) if absent;
4. builds the Svelte frontend into the `public/` folder;
5. installs **Conan** + **Ninja** and downloads the backend dependencies;
6. configures and compiles the backend;
7. lets CMake copy `cert.pem`, `key.pem`, `.env` and `public/` **next to the executable**.

```bash
# Linux / macOS
chmod +x setup.sh
./setup.sh
```

```powershell
# Windows (PowerShell)
./setup.ps1
# If PowerShell blocks the script due to execution policy:
#   powershell -ExecutionPolicy Bypass -File .\setup.ps1
```

When it finishes, the script prints the command to start the server (see
[Running](#-running)).

---

### Manual build (step by step)

If you prefer to control every stage — or the automatic script fails — you can
reproduce its steps manually.

#### 1. TLS certificates

The server uses HTTPS/WSS, so it needs `key.pem` and `cert.pem` in the project root.
For local development a self-signed certificate for `localhost` is enough:

```bash
# Linux / macOS / Git Bash
openssl req -newkey rsa:2048 -nodes -x509 -days 365 \
  -keyout key.pem -out cert.pem \
  -subj "/C=IT/ST=Italy/L=Uniba/O=uni-uno/CN=localhost"
```

```powershell
# Windows (PowerShell, with OpenSSL in the PATH)
openssl req -newkey rsa:2048 -nodes -x509 -days 365 `
  -keyout key.pem -out cert.pem `
  -subj "/C=IT/ST=Italy/L=Uniba/O=uni-uno/CN=localhost"
```

#### 2. `.env` file

The backend requires `JWT_SECRET` and `PASSWORD_PEPPER`. Create a `.env` file in the root:

```bash
printf 'JWT_SECRET=%s\nPASSWORD_PEPPER=%s\n' "$(openssl rand -hex 32)" "$(openssl rand -hex 32)" > .env
```

#### 3. Frontend

Build the Svelte frontend and copy it into the `public/` folder served by the server:

```bash
./deploy_frontend.sh
```

> The script runs `npm install`/`npm run build` in the `frontend/` folder. See
> [`frontend/README.md`](frontend/README.md) for details and for development mode.
> **Important:** `public/` must be generated _before_ configuring CMake, because the
> automatic asset copy (step 5) detects its presence at configuration time.

#### 4. Backend dependencies (Conan + Ninja)

```bash
pip install --upgrade --user conan ninja
conan profile detect --force
conan install . --build=missing -s build_type=Release -s compiler.cppstd=20 \
  -c tools.cmake.cmaketoolchain:generator=Ninja
```

#### 5. Building the backend

Use the CMake presets generated automatically by Conan:

```bash
cmake --preset conan-release
cmake --build --preset conan-release
```

During the build CMake copies `cert.pem`, `key.pem`, `.env` and the `public/` folder
next to the executable, so the binary can also be launched directly from its build
folder.

> **Note for developers (clangd / LSP):** Conan generates `compile_commands.json`
> in `build/Release/generators/`. If your editor requires it in the project root:
>
> - Linux: `ln -s build/Release/generators/compile_commands.json`
> - Windows: `New-Item -ItemType SymbolicLink -Path "compile_commands.json" -Target "build\Release\generators\compile_commands.json"`

---

## 🏃 Running

The server looks for the certificates, `.env`, the `game.db` database and the `public/`
folder in the **current working directory**. Since the build copies the runtime assets
next to the executable, you can start it in two equivalent ways:

```bash
# Linux / macOS — from the project root
./build/Release/uno_server

# …or from the build folder (assets already copied next to the binary)
cd build/Release && ./uno_server
```

```powershell
# Windows — from the project root
build\Release\uno_server.exe

# …or from the build folder
cd build\Release; .\uno_server.exe
```

The SQLite database (`game.db`) is created automatically on first launch if it does
not exist. By default the server is reachable at **https://localhost:9999** (accept the
browser warning about the self-signed certificate).

---

## 📚 Documentation

The complete documentation is already attached as **`documentazione-completa.pdf`**
(analysis, use cases and frontend tests + Doxygen backend manual + TypeDoc frontend API).

To **regenerate it** you need `doxygen`, `xelatex` (TeX Live), `mutool` (MuPDF) and
Node.js; then run:

```bash
./docs/build-full-docs.sh
```

---

## 🙏 Credits

See [`ATTRIBUTION.md`](ATTRIBUTION.md) for the full list of third-party assets. In short:

- **SNKRX font** — from [SNKRX](https://github.com/a327ex/SNKRX) by a327ex (MIT License).
- **Lobby music** — "Lo-Fi 16-bit Funky Fresh Beats To Chill and Jazz Out To
  [Sega Megadrive/Genesis Oscilloscope]" by birdrun
  ([source](https://www.youtube.com/watch?v=a2xtQqsuDRI)).
- **Backgrounds** — AI-generated with Google Gemini ("Nano Banana").

---

## ⚠️ Disclaimer

This project is an independent, non-commercial work. It is **not affiliated with,
endorsed by, or sponsored by Mattel, Inc. or the UNO® brand**. UNO® is a registered
trademark of Mattel, Inc. All trademarks are the property of their respective owners.
