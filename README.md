# PhoenixSim

[![Build (Windows)](https://github.com/jlfarris91/PhoenixSim/actions/workflows/build-windows.yml/badge.svg)](https://github.com/jlfarris91/PhoenixSim/actions/workflows/build-windows.yml)
[![Build (Linux)](https://github.com/jlfarris91/PhoenixSim/actions/workflows/build-linux.yml/badge.svg)](https://github.com/jlfarris91/PhoenixSim/actions/workflows/build-linux.yml)
[![Build (Emscripten)](https://github.com/jlfarris91/PhoenixSim/actions/workflows/build-emscripten.yml/badge.svg)](https://github.com/jlfarris91/PhoenixSim/actions/workflows/build-emscripten.yml)

PhoenixSim is a high-performance, modular simulation engine for real-time strategy games. It provides an archetype-based ECS, a data-driven game layer (LDS), pluggable Feature systems, and a full RTS gameplay stack (units, abilities, orders, effects, physics, steering).

---

## Quickstart

### Prerequisites

- Windows 10/11 x64
- Visual Studio 2022 with the **Desktop development with C++** workload
- Git

### Clone and bootstrap

```sh
git clone --recurse-submodules https://github.com/jlfarris91/PhoenixSim.git
cd PhoenixSim
.\build\bootstrap.ps1
```

The bootstrap script installs CMake and Ninja (via winget) and compiles the vcpkg binary. Run it once after cloning.

### Build

```sh
cmake --preset windows
cmake --build .build/windows --config Release
```

### Run TestRTS

```sh
.build\windows\tests\TestRTS\Release\TestRTS.exe
```

See [docs/BuildAndRunTestRTS.md](docs/BuildAndRunTestRTS.md) for Visual Studio setup, VS Code setup, Emscripten builds, and troubleshooting.

---

## Project Structure

```
src/
  PhoenixSim/       Core engine: ECS, Session, Features, LDS, memory, threading
  PhoenixPhysics/   2D rigid-body physics
  PhoenixSteering/  Pathfinding and unit steering
  PhoenixRTS/       RTS gameplay: units, abilities, orders, effects, vitals
  PhoenixLua/       Lua scripting bridge
tests/
  TestRTS/          Interactive RTS sandbox (SDL3 + Dear ImGui)
vcpkg/              Package manager (submodule)
build/              Build scripts (bootstrap.ps1)
docs/               Documentation
```

---

## Documentation

| Doc | What it covers |
|---|---|
| [BuildAndRunTestRTS.md](docs/BuildAndRunTestRTS.md) | Build setup, running TestRTS, Emscripten builds |
| [FeatureDevelopmentWorkflow.md](docs/FeatureDevelopmentWorkflow.md) | How to write a Feature — the main extension point |
| [ECS.md](docs/ECS.md) | Entities, components, queries, systems |
| [WorldAndSessionBuffers.md](docs/WorldAndSessionBuffers.md) | Block buffer architecture and memory management |
| [PhoenixSimDeepDive.md](docs/PhoenixSimDeepDive.md) | PhoenixSim core: Session, World, LDS, threading, FName |
| [PhoenixPhysics.md](docs/PhoenixPhysics.md) | 2D physics: BodyComponent, PhysicsSystem, force API |
| [PhoenixSteering.md](docs/PhoenixSteering.md) | Pathfinding, movement, rotation, spatial queries |
| [PhoenixRTS.md](docs/PhoenixRTS.md) | Units, abilities, orders, effects, vitals, projectiles |

---

## Contributing

1. Fork the repo and create a feature branch off `main`.
2. Read [FeatureDevelopmentWorkflow.md](docs/FeatureDevelopmentWorkflow.md) — Features are the primary way to extend the engine.
3. Open a pull request. CI will run Windows and Emscripten builds automatically.
