# Getting Started: Build & Run TestRTS

## Prerequisites

- Windows 10/11 x64
- [Visual Studio 2022](https://visualstudio.microsoft.com/) with the **Desktop development with C++** workload (C++20 required)
- [Git](https://git-scm.com/) with submodule support
- PowerShell 5.1 or later (built into Windows)

---

## First-Time Setup

### 1. Clone the repository

```sh
git clone --recurse-submodules https://github.com/jlfarris91/PhoenixSim.git
cd PhoenixSim
```

If you cloned without `--recurse-submodules`, initialize the vcpkg submodule manually:

```sh
git submodule update --init --recursive
```

### 2. Run the bootstrap script

The bootstrap script installs all required tooling (CMake, Ninja) and bootstraps vcpkg:

```powershell
.\build\bootstrap.ps1
```

This will:
- Install **CMake** and **Ninja** via winget if not already present
- Bootstrap **vcpkg** (the `vcpkg/` submodule) by compiling the vcpkg binary

> **Note:** The script requires an internet connection and winget (available by default on Windows 11 and recent Windows 10).

---

## Building

### Configure and build (command line)

```sh
cmake --preset windows
cmake --build .build/windows --config Release
```

Dependencies are fetched and compiled automatically by vcpkg on the first configure — this takes a few minutes. Subsequent builds are fast.

### Open in Visual Studio

After running the configure step above:

1. Open Visual Studio 2022
2. **File → Open → CMake...** and select the root `CMakeLists.txt`
3. Visual Studio will detect the `windows` preset automatically
4. Select **Release** from the configuration dropdown and build (Ctrl+Shift+B)

### Open in VS Code

Install the [CMake Tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools), open the repo folder, and select the `windows` preset when prompted.

---

## Running TestRTS

After a successful Release build:

```sh
.build\windows\tests\TestRTS\Release\TestRTS.exe
```

From Visual Studio, set `TestRTS` as the startup project and press F5.

TestRTS is an RTS sandbox that exercises the full engine stack: spawning units, physics, steering, abilities, effects, and the Lua scripting layer. It uses SDL3 for windowing and Dear ImGui for the debug overlay.

---

## Emscripten Build (WebAssembly)

To build the library targets for WebAssembly (no TestRTS):

1. Install EMSDK. The bootstrap script can do this for you — check `build/bootstrap.ps1` for details, or install manually to `$HOME\.emsdk`.
2. Activate EMSDK so `EMSDK` is in your environment.
3. Configure and build:

```sh
cmake --preset emscripten
cmake --build --preset EmscriptenRelease
```

Output is in `.build/wasm/`.

---

## Project Structure

```
PhoenixSim/
├── src/
│   ├── PhoenixSim/         # Core engine (ECS, Session, Features, LDS, memory, threading)
│   ├── PhoenixPhysics/     # 2D physics integrated with ECS
│   ├── PhoenixSteering/    # Pathfinding and steering behaviors
│   ├── PhoenixRTS/         # RTS gameplay systems (units, abilities, orders, effects)
│   └── PhoenixLua/         # Lua scripting bridge
├── tests/
│   └── TestRTS/            # Interactive RTS sandbox application
├── vcpkg/                  # vcpkg submodule (package manager)
├── build/                  # Build scripts (bootstrap.ps1, build_local.ps1)
├── docs/                   # Documentation
├── CMakeLists.txt
└── CMakePresets.json
```

---

## Common Issues

**vcpkg package build fails on first run**
Make sure Visual Studio 2022 with the C++ workload is installed — vcpkg uses MSVC to compile dependencies.

**`cmake --preset windows` says preset not found**
You need CMake 3.21+. Run `cmake --version` to check; re-run the bootstrap script to install the latest version.

**TestRTS crashes on startup**
Check that you built in **Release** or **RelWithDebInfo** — Debug builds require the debug CRT and may conflict if mixing configurations.
