# PhoenixSim

[![Build (Windows)](https://github.com/jlfarris91/PhoenixSim/actions/workflows/build-windows.yml/badge.svg)](https://github.com/jlfarris91/PhoenixSim/actions/workflows/build-windows.yml)
[![Build (Emscripten)](https://github.com/jlfarris91/PhoenixSim/actions/workflows/build-emscripten.yml/badge.svg)](https://github.com/jlfarris91/PhoenixSim/actions/workflows/build-emscripten.yml)

PhoenixSim is a high-performance, modular simulation engine. This repository includes the core engine, sample apps, and all dependencies.

---

## Quickstart: Build & Run TestRTS

### Prerequisites
- Windows (x64)
- Visual Studio 2022 (C++20 toolchain)
- Premake5 (project generation)
- All dependencies included in `ext/` (SDL3, imgui, lua, tracy, nlohmann/json)

### Clone the Repository
```sh
git clone <repo-url>
cd PhoenixSim
```

### Generate Project Files
Run the batch script:
```sh
.\make.bat
```
This runs: `premake5 vs2022 --verbose` and generates `Phoenix.sln`.

### Build the Solution
- Open `Phoenix.sln` in Visual Studio 2022.
- Select `x64` platform and desired configuration (`Debug`, `Release`, `ReleaseWithSymbols`).
- Build the solution (F7).

### Run TestRTS
- Set `TestRTS` as startup project.
- Run (F5).
- Executable: `.build/vs2022/bin/x64/<config>/TestRTS`

### Troubleshooting
- SDL3 DLLs are copied automatically to output directory.
- All dependencies are included; no external package manager required.
- If you encounter build errors, ensure you have the correct Visual Studio version and C++20 support.

### Project Structure
- `src/`: Core engine and features
- `ext/`: External dependencies
- `tests/TestRTS/`: Test application source
- `docs/`: Detailed documentation

---

## Documentation

For in-depth guides, see the docs folder:
- [docs/BuildAndRunTestRTS.md](docs/BuildAndRunTestRTS.md): Step-by-step build/run instructions
- [docs/PhoenixSimDeepDive.md](docs/PhoenixSimDeepDive.md): Engine architecture and design
- [docs/FeatureDevelopmentWorkflow.md](docs/FeatureDevelopmentWorkflow.md): How to add new features
- [docs/WorldAndSessionBuffers.md](docs/WorldAndSessionBuffers.md): Buffer architecture and cache coherency

---

## Contributing

- Fork the repository and create a feature branch.
- See [docs/FeatureDevelopmentWorkflow.md](docs/FeatureDevelopmentWorkflow.md) for guidance on adding new features.
- Open a pull request with a clear description of your changes.

---

## Support

If you have questions or issues, check the docs folder first. For additional help, open an issue or contact the maintainers.
