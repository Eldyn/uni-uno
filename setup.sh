#!/bin/bash
# =============================================================================
#  UNI — Complete setup and build (Linux / macOS)
#
#  Running this script from a clean checkout gives you a ready-to-use executable.
#  In sequence, it:
#    1. checks the system prerequisites;
#    2. generates self-signed TLS certificates (if missing);
#    3. generates the .env file with random secrets (if missing);
#    4. builds the Svelte frontend into public/;
#    5. installs Conan + Ninja and downloads the backend dependencies;
#    6. configures and compiles the backend;
#    7. CMake copies the certificates, .env and public/ next to the executable.
# =============================================================================
set -euo pipefail

# Always work from the script's folder (the project root).
cd "$(dirname "$0")"

c_info()  { echo -e "\e[36m$*\e[0m"; }
c_step()  { echo -e "\n\e[36m==> $*\e[0m"; }
c_ok()    { echo -e "\e[32m$*\e[0m"; }
c_warn()  { echo -e "\e[33m$*\e[0m"; }
c_err()   { echo -e "\e[31m$*\e[0m" >&2; }

# -----------------------------------------------------------------------------
# 1. Prerequisites
# -----------------------------------------------------------------------------
c_step "Checking prerequisites..."

missing=0
require() {
    if ! command -v "$1" &>/dev/null; then
        c_err "  [MISSING] $1 — $2"
        missing=1
    else
        echo "  [ok] $1"
    fi
}

# pip may be called pip3 or pip: we handle it separately.
if command -v pip3 &>/dev/null; then PIP_CMD="pip3"
elif command -v pip &>/dev/null; then PIP_CMD="pip"
else
    c_err "  [MISSING] pip — install Python 3 with pip"
    missing=1
fi

require python3 "install Python 3 (package: python3)"
require cmake   "install CMake (package: cmake)"
require openssl "needed to generate the TLS certificates (package: openssl)"
require node    "install Node.js (package: nodejs)"
require npm     "install npm (package: npm)"
if ! command -v g++ &>/dev/null && ! command -v clang++ &>/dev/null; then
    c_err "  [MISSING] g++/clang++ — a C++23 compiler is required (package: gcc-c++ or clang)"
    missing=1
else
    echo "  [ok] C++ compiler"
fi

if [ "$missing" -ne 0 ]; then
    c_err "\nSome prerequisites are missing. Install them and rerun this script."
    c_err "  Fedora/RHEL : sudo dnf install gcc-c++ python3 pip cmake openssl nodejs npm"
    c_err "  Ubuntu/Debian: sudo apt install g++ python3 python3-pip cmake openssl nodejs npm"
    exit 1
fi
c_ok "All prerequisites are present."

# -----------------------------------------------------------------------------
# 2. Self-signed TLS certificates
# -----------------------------------------------------------------------------
c_step "TLS certificates..."
if [ -f cert.pem ] && [ -f key.pem ]; then
    echo "  cert.pem and key.pem already present — skipping generation."
else
    echo "  Generating a self-signed certificate for localhost (valid 365 days)..."
    openssl req -newkey rsa:2048 -nodes -x509 -days 365 \
        -keyout key.pem -out cert.pem \
        -subj "/C=IT/ST=Italy/L=Uniba/O=uni/CN=localhost"
    c_ok "  Certificates generated."
fi

# -----------------------------------------------------------------------------
# 3. .env file with secrets
# -----------------------------------------------------------------------------
c_step "Environment file (.env)..."
if [ -f .env ]; then
    echo "  .env already present — leaving it unchanged."
else
    echo "  Generating .env with random secrets..."
    JWT_SECRET=$(openssl rand -hex 32)
    PASSWORD_PEPPER=$(openssl rand -hex 32)
    {
        echo "JWT_SECRET=${JWT_SECRET}"
        echo "PASSWORD_PEPPER=${PASSWORD_PEPPER}"
    } > .env
    c_ok "  .env generated."
fi

# -----------------------------------------------------------------------------
# 4. Frontend (must exist before configuring CMake)
# -----------------------------------------------------------------------------
c_step "Building the Svelte frontend..."
# Vite is configured with outDir "../public" and emptyOutDir: the build writes
# directly into the public/ folder at the project root (and cleans it).
( cd frontend && npm install && npm run build )
c_ok "  Frontend built into public/."

# -----------------------------------------------------------------------------
# 5. Backend toolchain (Conan + Ninja)
# -----------------------------------------------------------------------------
c_step "Installing Conan and Ninja via pip..."
$PIP_CMD install --upgrade --user conan ninja

# Determine how to invoke Conan, working around PATH issues.
if command -v conan &>/dev/null; then
    CONAN_CMD="conan"
elif python3 -m conan --version &>/dev/null; then
    CONAN_CMD="python3 -m conan"
elif [ -x "$HOME/.local/bin/conan" ]; then
    CONAN_CMD="$HOME/.local/bin/conan"
else
    c_err "Unable to find the Conan executable after installation."
    c_err "Add ~/.local/bin to your PATH and try again."
    exit 1
fi

c_step "Configuring the Conan profile..."
$CONAN_CMD profile detect --force

c_step "Downloading the backend dependencies..."
$CONAN_CMD install . --build=missing -s build_type=Release -s compiler.cppstd=20 \
    -c tools.cmake.cmaketoolchain:generator=Ninja

# -----------------------------------------------------------------------------
# 6. Backend build
# -----------------------------------------------------------------------------
c_step "Configuring and compiling the backend..."
cmake --preset conan-release
cmake --build --preset conan-release

# -----------------------------------------------------------------------------
# Done.
# -----------------------------------------------------------------------------
c_ok "\nSetup completed successfully!"
echo "Start the server with:"
echo "  ./build/Release/uni_server          (from the project root)"
echo "or"
echo "  cd build/Release && ./uni_server    (assets copied next to the executable)"
echo "Then open https://localhost:9999 (accept the self-signed certificate warning)."
