# =============================================================================
#  Uni-Uno — Complete setup and build (Windows / PowerShell)
#
#  Running this script from a clean checkout gives you a ready-to-use executable.
#  In sequence, it:
#    1. checks the system prerequisites;
#    2. generates self-signed TLS certificates (if missing);
#    3. generates the .env file with random secrets (if missing);
#    4. builds the Svelte frontend into public\;
#    5. installs Conan + Ninja and downloads the backend dependencies;
#    6. configures and compiles the backend;
#    7. CMake copies the certificates, .env and public\ next to the executable.
#
#  Tip: if you get an execution policy error, start it with:
#    powershell -ExecutionPolicy Bypass -File .\setup.ps1
# =============================================================================
$ErrorActionPreference = "Stop"

# Always work from the script's folder (the project root).
Set-Location -Path $PSScriptRoot

function Write-Step($m) { Write-Host "`n==> $m" -ForegroundColor Cyan }
function Write-Ok($m)   { Write-Host $m -ForegroundColor Green }
function Write-Warn2($m){ Write-Host $m -ForegroundColor Yellow }

# -----------------------------------------------------------------------------
# 1. Prerequisites
# -----------------------------------------------------------------------------
Write-Step "Checking prerequisites..."
$missing = $false
function Require-Cmd($name, $hint) {
    if (-not (Get-Command $name -ErrorAction SilentlyContinue)) {
        Write-Host "  [MISSING] $name - $hint" -ForegroundColor Red
        $script:missing = $true
    } else {
        Write-Host "  [ok] $name"
    }
}

Require-Cmd python  "install Python 3 from python.org or the Microsoft Store"
Require-Cmd cmake   "install CMake from cmake.org/download"
Require-Cmd openssl "included in Git Bash, or: winget install ShiningLight.OpenSSL.Light"
Require-Cmd node    "install Node.js from nodejs.org"
Require-Cmd npm     "install Node.js (includes npm) from nodejs.org"

if ($missing) {
    Write-Host "`nSome prerequisites are missing. Install them and rerun this script." -ForegroundColor Red
    Write-Host "Remember also Visual Studio 2022 with the 'Desktop development with C++' workload." -ForegroundColor Red
    exit 1
}
Write-Ok "All prerequisites are present."

# -----------------------------------------------------------------------------
# 2. Self-signed TLS certificates
# -----------------------------------------------------------------------------
Write-Step "TLS certificates..."
if ((Test-Path cert.pem) -and (Test-Path key.pem)) {
    Write-Host "  cert.pem and key.pem already present - skipping generation."
} else {
    Write-Host "  Generating a self-signed certificate for localhost (valid 365 days)..."
    openssl req -newkey rsa:2048 -nodes -x509 -days 365 `
        -keyout key.pem -out cert.pem `
        -subj "/C=IT/ST=Italy/L=Uniba/O=uni-uno/CN=localhost"
    Write-Ok "  Certificates generated."
}

# -----------------------------------------------------------------------------
# 3. .env file with secrets
# -----------------------------------------------------------------------------
Write-Step "Environment file (.env)..."
if (Test-Path .env) {
    Write-Host "  .env already present - leaving it unchanged."
} else {
    Write-Host "  Generating .env with random secrets..."
    function New-Secret {
        $bytes = New-Object byte[] 32
        [System.Security.Cryptography.RandomNumberGenerator]::Create().GetBytes($bytes)
        ($bytes | ForEach-Object { $_.ToString("x2") }) -join ""
    }
    $jwt    = New-Secret
    $pepper = New-Secret
    "JWT_SECRET=$jwt`nPASSWORD_PEPPER=$pepper" | Out-File -FilePath .env -Encoding ascii -NoNewline
    Write-Ok "  .env generated."
}

# -----------------------------------------------------------------------------
# 4. Frontend (must exist before configuring CMake)
# -----------------------------------------------------------------------------
Write-Step "Building the Svelte frontend..."
Push-Location frontend
npm install
npm run build
Pop-Location
if (Test-Path public) { Remove-Item -Recurse -Force public }
New-Item -ItemType Directory -Path public | Out-Null
Copy-Item -Recurse -Force "frontend\dist\*" "public\"
Write-Ok "  Frontend built into public\."

# -----------------------------------------------------------------------------
# 5. Backend toolchain (Conan + Ninja)
# -----------------------------------------------------------------------------
Write-Step "Installing Conan and Ninja via pip..."
python -m pip install --upgrade pip conan ninja

# Function to invoke Conan, working around PATH issues.
function Invoke-Conan {
    if (Get-Command conan -ErrorAction SilentlyContinue) {
        conan $args
    } else {
        $check = python -m conan --version 2>&1
        if ($LASTEXITCODE -eq 0) {
            python -m conan $args
        } else {
            Write-Error "Conan module not found. The pip installation failed."
            exit 1
        }
    }
}

Write-Step "Configuring the Conan profile..."
Invoke-Conan profile detect --force

Write-Step "Downloading the backend dependencies..."
Invoke-Conan install . --build=missing -s build_type=Release -s compiler.cppstd=20 -c tools.cmake.cmaketoolchain:generator=Ninja

# -----------------------------------------------------------------------------
# 6. Backend build
# -----------------------------------------------------------------------------
Write-Step "Configuring and compiling the backend..."
cmake --preset conan-release
cmake --build --preset conan-release

# -----------------------------------------------------------------------------
# Done.
# -----------------------------------------------------------------------------
Write-Ok "`nSetup completed successfully!"
Write-Host "Start the server with:"
Write-Host "  build\Release\uno_server.exe          (from the project root)"
Write-Host "or"
Write-Host "  cd build\Release; .\uno_server.exe     (assets copied next to the executable)"
Write-Host "Then open https://localhost:9999 (accept the self-signed certificate warning)."
