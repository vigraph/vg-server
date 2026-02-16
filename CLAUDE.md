# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ViGraph is a C++14 dataflow-based visual programming engine for multimedia (audio, laser graphics, DMX lighting, MIDI, IoT). Users connect modules in a graph; the engine ticks the graph at a configurable rate (default 25 Hz). The UI is a separate web app ([vg-ui-web](https://github.com/vigraph/vg-ui-web)); this repo is the headless server/engine.

Licensed under AGPLv3.

## Build System

Uses **Tup** (not Make/CMake) with Lua-based configuration. Each directory has a `Tupfile` declaring `NAME`, `TYPE` (lib/shared/exe), and `DEPENDS`.

### Build Commands

```bash
git submodule init && git submodule update   # first time only
obtools/build/init.sh -t debug               # -t release for production
tup                                           # incremental build
```

Compiler: `clang++`. The ObTools submodule (`obtools/`) provides the build rules and core C++ libraries.

### Dependencies

SDL2, SDL2-Image, ALSA, SoundTouch, FreeType, OLA, Mosquitto, plus ObTools dependencies (see `obtools/README.md`).

## Architecture

### Core Libraries (`libs/`)

- **dataflow** (`vg-dataflow.h`): The central abstraction. Defines `Element`, `Graph`, `Engine`, typed `Input`/`Output` pins, `Setting`, `Clone`, `Path` (XPath-like navigation), `Visitor` pattern, and the tick/sample-rate system. Key types: `Number` (double), `Integer` (int64_t), `Trigger` (unsigned), plus domain types from other libs.
- **service** (`vg-service.h`): Server layer — REST API, WebSocket support, module loader (dynamic `.so` loading), graph management, `MainThreadRunner` for thread safety.
- **geometry** (`vg-geometry.h`): Vector math — `Vector`, `Point`, `Frame` (list of vectors with metadata).
- **colour**: RGB, HSL, RGBA colour types.
- **bitmap/laser/waveform**: Domain-specific data structures consumed by modules.

### Modules (`modules/`)

~156 dynamically loaded shared libraries organized in categories: `core/`, `audio/`, `vector/`, `bitmap/`, `laser/`, `midi/`, `dmx/`, `colour/`, `maths/`, `iot/`, `trigger/`, `binary/`, `control/`, `time/`, `object/`, `waveform/`, `time-series/`, `ui/`.

Each module category has a shared header (e.g., `modules/core/core-module.h`) providing base template classes.

### Module Structure Pattern

A module is a single `.cc` file that:
1. Defines an element class inheriting from a dataflow primitive (e.g., `Pin<Number>`, `SimpleElement`)
2. Declares a `Dataflow::SimpleModule` with name, display name, category, settings, inputs, outputs
3. Ends with `VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ClassName, module)`

Example (`modules/core/pin/pin.cc`): a pass-through number pin.

Tupfile for modules uses `TYPE = shared` and depends on `vg-dataflow` plus relevant libs.

### Engine (`engine/`)

The daemon executable. Configuration via XML files (`engine.cfg.xml`, `engine.dev.cfg.xml`). Loads modules from configured paths, starts the REST API, and runs the dataflow tick loop.

### Nexus (`nexus/`)

Optional distributed message-queue server for connecting multiple engines. REST + WebSocket protocol.

## Dependency Resolution

`Tuppath.lua` maps dependency names to source paths. Convention:
- `ot-*` → `obtools/libs/*` (ObTools libraries)
- `vg-*` → `libs/*` (ViGraph libraries)
- `vg-module-<category>-<name>` → `modules/<category>/<name>`

## Namespaces

All ViGraph code lives under `ViGraph::Dataflow`, `ViGraph::Service`, `ViGraph::Geometry`, etc. ObTools libraries use `ObTools::` namespace. Modules use anonymous namespaces.

## Testing

Test files follow the pattern `test-*.cc` in library directories (e.g., `libs/dataflow/test-graph.cc`). Tests are built via Tup alongside the libraries.
