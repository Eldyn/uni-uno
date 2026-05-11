Write-Host "Setting up UniUno for Windows..." -ForegroundColor Cyan

# 1. Update submodules
Write-Host "Updating git submodules..."
git submodule update --init --recursive

# 2. Setup local vcpkg if it doesn't exist
$VCPKG_DIR = "$PSScriptRoot\external\vcpkg"
if (-Not (Test-Path "$VCPKG_DIR")) {
    Write-Host "Cloning vcpkg..."
    git clone https://github.com/microsoft/vcpkg.git "$VCPKG_DIR"
    Write-Host "Bootstrapping vcpkg..."
    & "$VCPKG_DIR\bootstrap-vcpkg.bat" -disableMetrics
}

# 3. Install dependencies via vcpkg
Write-Host "Installing OpenSSL and libuv via vcpkg (this might take a while)..."
& "$VCPKG_DIR\vcpkg.exe" install openssl:x64-windows libuv:x64-windows

# 4. Run CMake using the vcpkg toolchain
Write-Host "Configuring CMake..."
New-Item -ItemType Directory -Force -Path "$PSScriptRoot\build" | Out-Null
Set-Location -Path "$PSScriptRoot\build"

cmake .. -DCMAKE_TOOLCHAIN_FILE="$VCPKG_DIR\scripts\buildsystems\vcpkg.cmake" -A x64
cd ..
./deploy_frontend.sh

Write-Host "Setup complete! You can now open the Visual Studio solution in the 'build' folder or run 'cmake --build .' inside the build folder." -ForegroundColor Green
Set-Location -Path "$PSScriptRoot"
