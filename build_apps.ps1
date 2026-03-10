# Build PhoenixSim with CMake and vcpkg manifest mode
# Run this script from the project root directory

# Create build directory if it doesn't exist
if (!(Test-Path "build")) {
    New-Item -ItemType Directory -Path "build"
}

# Move into build directory
Set-Location "build"

# Configure project (auto-installs dependencies)
cmake ..

# Build all app targets
cmake --build . --target TestRTS
# Add more app targets below as needed
# cmake --build . --target TestApp2
# cmake --build . --target TestApp3

# Return to project root
Set-Location ..
