# Build & Run TestRTS (PhoenixSim)

## Prerequisites
- Windows (x64)
- Visual Studio 2022 (C++20 toolchain)
- Premake5 (project generation)
- All dependencies included in `ext/` (SDL3, imgui, lua, tracy, nlohmann/json)

## Clone the Repository
```sh
git clone <repo-url>
cd PhoenixSim
```

## Generate Project Files
Run the batch script:
```sh
.\make.bat
```
This runs: `premake5 vs2022 --verbose` and generates `Phoenix.sln`.

## Build the Solution
- Open `Phoenix.sln` in Visual Studio 2022.
- Select `x64` platform and desired configuration (`Debug`, `Release`, `ReleaseWithSymbols`).
- Build the solution (F7).

## Run TestRTS
- Set `TestRTS` as startup project.
- Run (F5).
- Executable: `.build/vs2022/bin/x64/<config>/TestRTS`

## Troubleshooting
- SDL3 DLLs are copied automatically to output directory.
- All dependencies are included; no external package manager required.
- If you encounter build errors, ensure you have the correct Visual Studio version and C++20 support.

## Project Structure
- `src/`: Core engine and features
- `ext/`: External dependencies
- `tests/TestRTS/`: Test application source
- `docs/`: Detailed documentation
