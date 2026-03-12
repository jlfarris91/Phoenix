
param(
    [string]$Config = "Release",
    [string]$Preset = "default"
)

# CMake must be installed and available in PATH
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "CMake not found. Please install CMake."
    exit 1
}

# Set project root to parent directory of script, fallback to current directory if empty
$projectRoot = Split-Path $PSScriptRoot -Parent
if (-not $projectRoot -or $projectRoot -eq "") {
    $projectRoot = Get-Location | Select-Object -ExpandProperty Path
    Write-Host "Fallback: Using current directory as projectRoot: $projectRoot"
} else {
    Write-Host "Using parent directory of script as projectRoot: $projectRoot"
}

# Find the build directory from the preset
$presets = Get-Content "CMakePresets.json" -Raw | ConvertFrom-Json
$targetPreset = $presets.configurePresets | Where-Object { $_.name -eq $Preset }
$buildDir = $targetPreset.binaryDir

if (-not $buildDir -or $buildDir -eq "") {
    Write-Host "Error: Build directory not found in preset '$Preset'."
    exit 1
}

$buildDir = $buildDir -replace '\$\{sourceDir\}', $projectRoot

Write-Host "Building with preset $Preset to build dir $buildDir"
cmake --preset $Preset

Write-Host "Building with preset: $Config"
cmake --build --preset $Config

Write-Host "Packaging artifacts for $Config"

$artifactDir = "$projectRoot/.artifact/$Config"
$artifactDocsDir = "$artifactDir/docs"

$docsDir = "$projectRoot/docs"

New-Item -ItemType Directory -Force -Path $artifactDocsDir | Out-Null

cmake --install $buildDir --prefix $artifactDir --config $Config

Copy-Item "LICENSE" $artifactDir -ErrorAction SilentlyContinue
Copy-Item "README.md" $artifactDir -ErrorAction SilentlyContinue
Copy-Item "CHANGELOG.md" $artifactDir -ErrorAction SilentlyContinue
Copy-Item $docsDir $artifactDocsDir -Recurse -Force -ErrorAction SilentlyContinue

Write-Host "=== $Config build complete ===`n"