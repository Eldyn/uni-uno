#!/bin/bash
set -e

echo -e "\e[36mSetting up uni-uno for Linux...\e[0m"

# 1. Update submodules
echo "Updating git submodules..."
git submodule update --init --recursive

# 2. Install system dependencies (Ubuntu/Debian based)
echo "Installing OpenSSL, libuv, and build tools..."
if command -v apt-get &> /dev/null; then
    sudo apt-get update
    sudo apt-get install -y build-essential cmake libssl-dev libuv1-dev pkg-config
elif command -v dnf &> /dev/null; then
    sudo dnf install -y gcc-c++ cmake openssl-devel libuv-devel pkgconf-pkg-config
elif command -v pacman &> /dev/null; then
    sudo pacman -S --needed base-devel cmake openssl libuv pkgconf
else
    echo -e "\e[33mWarning: Unsupported package manager. Please manually install OpenSSL and libuv headers.\e[0m"
fi

# 3. Build the project
echo "Configuring CMake..."
mkdir -p build
cd build
cmake ..

echo "Compiling the project..."
cmake --build . -j$(nproc)

echo "Compiling frontend..."
cd ..
./deploy_frontend.sh

echo -e "\e[32mSetup complete! Executable is located in the 'build' directory.\e[0m"
