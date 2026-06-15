# =============================================================================
#  Uni-Uno — Setup e build completo (Windows / PowerShell)
#
#  Eseguendo questo script da una checkout pulita ottieni un eseguibile pronto
#  all'uso. In sequenza:
#    1. verifica i prerequisiti di sistema;
#    2. genera certificati TLS self-signed (se mancanti);
#    3. genera il file .env con dei segreti casuali (se mancante);
#    4. compila il frontend Svelte in public\;
#    5. installa Conan + Ninja e scarica le dipendenze del backend;
#    6. configura e compila il backend;
#    7. CMake copia certificati, .env e public\ accanto all'eseguibile.
#
#  Suggerimento: se ricevi un errore sulle policy di esecuzione, avvia con:
#    powershell -ExecutionPolicy Bypass -File .\setup.ps1
# =============================================================================
$ErrorActionPreference = "Stop"

# Lavora sempre dalla cartella dello script (la root del progetto).
Set-Location -Path $PSScriptRoot

function Write-Step($m) { Write-Host "`n==> $m" -ForegroundColor Cyan }
function Write-Ok($m)   { Write-Host $m -ForegroundColor Green }
function Write-Warn2($m){ Write-Host $m -ForegroundColor Yellow }

# -----------------------------------------------------------------------------
# 1. Prerequisiti
# -----------------------------------------------------------------------------
Write-Step "Verifica dei prerequisiti..."
$missing = $false
function Require-Cmd($name, $hint) {
    if (-not (Get-Command $name -ErrorAction SilentlyContinue)) {
        Write-Host "  [MANCANTE] $name - $hint" -ForegroundColor Red
        $script:missing = $true
    } else {
        Write-Host "  [ok] $name"
    }
}

Require-Cmd python  "installa Python 3 da python.org o dal Microsoft Store"
Require-Cmd cmake   "installa CMake da cmake.org/download"
Require-Cmd openssl "incluso in Git Bash, oppure: winget install ShiningLight.OpenSSL.Light"
Require-Cmd node    "installa Node.js da nodejs.org"
Require-Cmd npm     "installa Node.js (include npm) da nodejs.org"

if ($missing) {
    Write-Host "`nMancano dei prerequisiti. Installali e riesegui questo script." -ForegroundColor Red
    Write-Host "Ricorda inoltre Visual Studio 2022 con il carico di lavoro 'Sviluppo di applicazioni desktop con C++'." -ForegroundColor Red
    exit 1
}
Write-Ok "Tutti i prerequisiti sono presenti."

# -----------------------------------------------------------------------------
# 2. Certificati TLS self-signed
# -----------------------------------------------------------------------------
Write-Step "Certificati TLS..."
if ((Test-Path cert.pem) -and (Test-Path key.pem)) {
    Write-Host "  cert.pem e key.pem gia' presenti - salto la generazione."
} else {
    Write-Host "  Genero un certificato self-signed per localhost (valido 365 giorni)..."
    openssl req -newkey rsa:2048 -nodes -x509 -days 365 `
        -keyout key.pem -out cert.pem `
        -subj "/C=IT/ST=Italy/L=Uniba/O=uni-uno/CN=localhost"
    Write-Ok "  Certificati generati."
}

# -----------------------------------------------------------------------------
# 3. File .env con segreti
# -----------------------------------------------------------------------------
Write-Step "File di ambiente (.env)..."
if (Test-Path .env) {
    Write-Host "  .env gia' presente - lo lascio invariato."
} else {
    Write-Host "  Genero .env con segreti casuali..."
    function New-Secret {
        $bytes = New-Object byte[] 32
        [System.Security.Cryptography.RandomNumberGenerator]::Create().GetBytes($bytes)
        ($bytes | ForEach-Object { $_.ToString("x2") }) -join ""
    }
    $jwt    = New-Secret
    $pepper = New-Secret
    "JWT_SECRET=$jwt`nPASSWORD_PEPPER=$pepper" | Out-File -FilePath .env -Encoding ascii -NoNewline
    Write-Ok "  .env generato."
}

# -----------------------------------------------------------------------------
# 4. Frontend (deve esistere prima della configurazione di CMake)
# -----------------------------------------------------------------------------
Write-Step "Build del frontend Svelte..."
Push-Location frontend
npm install
npm run build
Pop-Location
if (Test-Path public) { Remove-Item -Recurse -Force public }
New-Item -ItemType Directory -Path public | Out-Null
Copy-Item -Recurse -Force "frontend\dist\*" "public\"
Write-Ok "  Frontend compilato in public\."

# -----------------------------------------------------------------------------
# 5. Toolchain backend (Conan + Ninja)
# -----------------------------------------------------------------------------
Write-Step "Installazione di Conan e Ninja via pip..."
python -m pip install --upgrade pip conan ninja

# Funzione per invocare Conan aggirando i problemi di PATH.
function Invoke-Conan {
    if (Get-Command conan -ErrorAction SilentlyContinue) {
        conan $args
    } else {
        $check = python -m conan --version 2>&1
        if ($LASTEXITCODE -eq 0) {
            python -m conan $args
        } else {
            Write-Error "Modulo Conan non trovato. L'installazione di pip e' fallita."
            exit 1
        }
    }
}

Write-Step "Configurazione del profilo Conan..."
Invoke-Conan profile detect --force

Write-Step "Download delle dipendenze del backend..."
Invoke-Conan install . --build=missing -s build_type=Release -s compiler.cppstd=20 -c tools.cmake.cmaketoolchain:generator=Ninja

# -----------------------------------------------------------------------------
# 6. Build del backend
# -----------------------------------------------------------------------------
Write-Step "Configurazione e compilazione del backend..."
cmake --preset conan-release
cmake --build --preset conan-release

# -----------------------------------------------------------------------------
# Fatto.
# -----------------------------------------------------------------------------
Write-Ok "`nSetup completato con successo!"
Write-Host "Avvia il server con:"
Write-Host "  build\Release\uno_server.exe          (dalla root del progetto)"
Write-Host "oppure"
Write-Host "  cd build\Release; .\uno_server.exe     (asset copiati accanto all'eseguibile)"
Write-Host "Poi apri https://localhost:9999 (accetta l'avviso sul certificato self-signed)."
