<#
.SYNOPSIS
    Bootstrap PhoenixSim development environment from scratch.

.DESCRIPTION
    Installs and configures:
      - CMake
      - Ninja (required for Emscripten builds)
      - vcpkg (bootstrapped from the submodule)
      - EMSDK (for WebAssembly builds)

.PARAMETER SkipEmsdk
    Skip EMSDK installation (if you only need Windows builds).

.EXAMPLE
    .\build\bootstrap.ps1
    .\build\bootstrap.ps1 -SkipEmsdk
#>

#Requires -Version 5.1

param(
    [switch]$SkipEmsdk
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

# --- CMake ---

Write-Step "CMake"
if (Get-Command cmake -ErrorAction SilentlyContinue) {
    $v = (cmake --version | Select-Object -First 1)
    Write-Ok "Already installed: $v"
} else {
    Write-Host "    CMake not found. Installing..."
    Install-WingetPackage "Kitware.CMake" "CMake"
    $env:PATH = [System.Environment]::GetEnvironmentVariable("PATH", "Machine") + ";" +
                [System.Environment]::GetEnvironmentVariable("PATH", "User")
    if (Get-Command cmake -ErrorAction SilentlyContinue) {
        Write-Ok "CMake installed successfully."
    } else {
        Write-Warn "CMake installed but not in PATH yet. Restart your shell after bootstrap."
    }
}

# --- Ninja ---

Write-Step "Ninja"
if (Get-Command ninja -ErrorAction SilentlyContinue) {
    $v = (ninja --version)
    Write-Ok "Already installed: $v"
} else {
    Write-Host "    Ninja not found. Installing..."
    Install-WingetPackage "Ninja-build.Ninja" "Ninja"
    $env:PATH = [System.Environment]::GetEnvironmentVariable("PATH", "Machine") + ";" +
                [System.Environment]::GetEnvironmentVariable("PATH", "User")
    if (Get-Command ninja -ErrorAction SilentlyContinue) {
        Write-Ok "Ninja installed successfully."
    } else {
        Write-Warn "Ninja installed but not in PATH yet. Restart your shell after bootstrap."
    }
}

# --- Visual Studio ---

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
    Write-Warn "Visual Studio 2022 not found. The default CMake preset requires VS 2022 with the C++ workload."
    Write-Host "    Download: https://visualstudio.microsoft.com/vs/"
}

# --- vcpkg ---

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

# --- EMSDK ---

if ($SkipEmsdk) {
    Write-Step "EMSDK (skipped)"
} else {
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

# --- Done ---

Write-Host ""
Write-Host "=== Bootstrap complete ===" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:"
Write-Host "  Windows build:      cmake --preset windows"
Write-Host "  Emscripten build:   cmake --preset emscripten"
if ($SkipEmsdk) {
    Write-Host "  (Re-run without -SkipEmsdk to install EMSDK)"
}
Write-Host ""
