<#
.SYNOPSIS
    Bootstrap PhoenixSim development environment from scratch.

.DESCRIPTION
    Installs and configures the tools needed to build PhoenixSim for a given target.

.PARAMETER Target
    Which build target to set up. One of: windows, linux, emscripten, all.
    Defaults to 'windows'.

.EXAMPLE
    .\build\bootstrap.ps1
    .\build\bootstrap.ps1 -Target emscripten
    .\build\bootstrap.ps1 -Target all
#>

#Requires -Version 5.1

param(
    [ValidateSet("windows", "linux", "emscripten", "all")]
    [string]$Target = "windows"
)

$ErrorActionPreference = "Stop"
$projectRoot = Split-Path $PSScriptRoot -Parent
$EmsdkRoot = Join-Path $env:USERPROFILE ".emsdk"

function Write-Step($msg) { Write-Host "`n==> $msg" -ForegroundColor Cyan }
function Write-Ok($msg)   { Write-Host "    [OK] $msg" -ForegroundColor Green }
function Write-Warn($msg) { Write-Host "    [WARN] $msg" -ForegroundColor Yellow }
function Write-Fail($msg) { Write-Host "    [FAIL] $msg" -ForegroundColor Red }

function Install-WingetPackage($id, $name) {
    if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
        Write-Warn "winget not found. Please install $name manually."
        return $false
    }
    Write-Host "    Installing $name via winget..."
    winget install --id $id --silent --accept-package-agreements --accept-source-agreements
    return $true
}

function Refresh-Path {
    $env:PATH = [System.Environment]::GetEnvironmentVariable("PATH", "Machine") + ";" +
                [System.Environment]::GetEnvironmentVariable("PATH", "User")
}

function Install-CMake {
    Write-Step "CMake"
    if (Get-Command cmake -ErrorAction SilentlyContinue) {
        Write-Ok "Already installed: $(cmake --version | Select-Object -First 1)"
    } else {
        Install-WingetPackage "Kitware.CMake" "CMake"
        Refresh-Path
        if (Get-Command cmake -ErrorAction SilentlyContinue) {
            Write-Ok "CMake installed successfully."
        } else {
            Write-Warn "CMake installed but not in PATH yet. Restart your shell after bootstrap."
        }
    }
}

function Install-Ninja {
    Write-Step "Ninja"
    if (Get-Command ninja -ErrorAction SilentlyContinue) {
        Write-Ok "Already installed: $(ninja --version)"
    } else {
        Install-WingetPackage "Ninja-build.Ninja" "Ninja"
        Refresh-Path
        if (Get-Command ninja -ErrorAction SilentlyContinue) {
            Write-Ok "Ninja installed successfully."
        } else {
            Write-Warn "Ninja installed but not in PATH yet. Restart your shell after bootstrap."
        }
    }
}

function Install-VisualStudio {
    Write-Step "Visual Studio 2022"
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null
        if ($vsPath) {
            Write-Ok "Found: $vsPath"
        } else {
            Write-Warn "Visual Studio found but C++ workload missing. Install 'Desktop development with C++' in the VS Installer."
        }
    } else {
        Write-Warn "Visual Studio 2022 not found. The windows CMake preset requires VS 2022 with the C++ workload."
        Write-Host "    Download: https://visualstudio.microsoft.com/vs/"
    }
}

function Install-Vcpkg {
    Write-Step "vcpkg"
    $vcpkgDir = Join-Path $projectRoot "vcpkg"
    $vcpkgExe = Join-Path $vcpkgDir "vcpkg.exe"

    if (-not (Test-Path $vcpkgDir)) {
        Write-Host "    Cloning vcpkg..."
        git clone https://github.com/microsoft/vcpkg.git $vcpkgDir
    }

    if (-not (Test-Path $vcpkgExe)) {
        Write-Host "    Bootstrapping vcpkg..."
        & (Join-Path $vcpkgDir "bootstrap-vcpkg.bat")
    }

    if (Test-Path $vcpkgExe) {
        Write-Ok "vcpkg ready at $vcpkgExe"
    } else {
        Write-Fail "vcpkg bootstrap failed."
        exit 1
    }
}

function Install-Emsdk {
    Write-Step "EMSDK"

    if (-not (Test-Path $EmsdkRoot)) {
        Write-Host "    Cloning EMSDK to $EmsdkRoot..."
        git clone https://github.com/emscripten-core/emsdk.git $EmsdkRoot
    } else {
        Write-Host "    EMSDK directory exists, pulling latest..."
        git -C $EmsdkRoot pull
    }

    Write-Host "    Installing latest Emscripten..."
    & (Join-Path $EmsdkRoot "emsdk.bat") install latest

    Write-Host "    Activating latest Emscripten..."
    & (Join-Path $EmsdkRoot "emsdk.bat") activate latest

    $currentEmsdk = [System.Environment]::GetEnvironmentVariable("EMSDK", "User")
    if ($currentEmsdk -ne $EmsdkRoot) {
        Write-Host "    Setting EMSDK environment variable to $EmsdkRoot..."
        [System.Environment]::SetEnvironmentVariable("EMSDK", $EmsdkRoot, "User")
        $env:EMSDK = $EmsdkRoot
        Write-Ok "EMSDK environment variable set."
    } else {
        Write-Ok "EMSDK environment variable already set."
    }

    $emcc = Join-Path $EmsdkRoot "upstream\emscripten\emcc.bat"
    if (Test-Path $emcc) {
        Write-Ok "Emscripten ready at $EmsdkRoot"
    } else {
        Write-Fail "EMSDK installation may have failed - emcc not found at expected path."
    }
}

function Install-WSL {
    Write-Step "WSL (Windows Subsystem for Linux)"

    $wslStatus = wsl --status 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-Ok "WSL already installed."
    } else {
        Write-Host "    Installing WSL with Ubuntu..."
        wsl --install -d Ubuntu
        Write-Warn "WSL requires a reboot before it can be used."
        Write-Warn "After rebooting, run: wsl -- bash -c 'sudo apt-get install -y build-essential cmake ninja-build curl zip unzip tar pkg-config'"
        return
    }

    Write-Step "WSL build tools (Ubuntu)"
    Write-Host "    Installing build-essential, cmake, ninja-build..."
    wsl -- bash -c "sudo apt-get update -qq && sudo apt-get install -y build-essential cmake ninja-build curl zip unzip tar pkg-config"
    if ($LASTEXITCODE -eq 0) {
        Write-Ok "Linux build tools installed in WSL."
    } else {
        Write-Warn "apt-get failed. You may need to run this manually inside WSL."
    }
}

# --- Run selected targets ---

$doWindows    = $Target -eq "windows"    -or $Target -eq "all"
$doLinux      = $Target -eq "linux"      -or $Target -eq "all"
$doEmscripten = $Target -eq "emscripten" -or $Target -eq "all"

Write-Host ""
Write-Host "PhoenixSim Bootstrap" -ForegroundColor White
Write-Host "Target: $Target" -ForegroundColor Gray

# CMake and Ninja are needed by all targets
Install-CMake

if ($doWindows -or $doEmscripten) {
    # Ninja is needed for Emscripten; Windows uses VS generator but useful to have
    Install-Ninja
}

if ($doWindows) {
    Install-VisualStudio
}

# vcpkg is needed by all targets
Install-Vcpkg

if ($doEmscripten) {
    Install-Emsdk
}

if ($doLinux) {
    Install-WSL
}

# --- Done ---

Write-Host ""
Write-Host "=== Bootstrap complete ===" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:"
if ($doWindows)    { Write-Host "  Windows build:     cmake --preset windows" }
if ($doLinux)      { Write-Host "  Linux build:       (open WSL) cmake --preset linux" }
if ($doEmscripten) { Write-Host "  Emscripten build:  cmake --preset emscripten" }
Write-Host ""
