---
title: PhoenixSim
---

# PhoenixSim

A high-performance, modular simulation engine for real-time strategy games.

[![Build (Windows)](https://github.com/jlfarris91/PhoenixSim/actions/workflows/build-windows.yml/badge.svg)](https://github.com/jlfarris91/PhoenixSim/actions/workflows/build-windows.yml)
[![Build (Emscripten)](https://github.com/jlfarris91/PhoenixSim/actions/workflows/build-emscripten.yml/badge.svg)](https://github.com/jlfarris91/PhoenixSim/actions/workflows/build-emscripten.yml)

---

## Getting Started

- [Build & Run TestRTS](BuildAndRunTestRTS) — clone, bootstrap, build, run

## Contributing

- [Writing a Feature](FeatureDevelopmentWorkflow) — the primary extension point; start here
- [ECS Guide](ECS) — entities, components, queries, systems

## Architecture

- [PhoenixSim Core](PhoenixSimDeepDive) — Session, World, LDS, memory, threading
- [PhoenixPhysics](PhoenixPhysics) — 2D rigid-body physics
- [PhoenixSteering](PhoenixSteering) — pathfinding and unit movement
- [PhoenixRTS](PhoenixRTS) — units, abilities, orders, effects, vitals, projectiles
- [Block Buffers](WorldAndSessionBuffers) — memory model and pre-allocated storage
- [World View Sync](WorldViewSync) — sim→render snapshot strategy and dirty-page tracking
- [Job System Roadmap](JobSystemRoadmap) — gap analysis & staged improvements vs. modern work-stealing designs
