# WASM Build Status

Analysis of compiling the existing C++ codebase to WebAssembly via Emscripten.

## Existing Infrastructure

The build system already has substantial WASM support:

- **Platform config**: `obtools/build/Tuprules.lua` defines a `web` platform using `em++`/`emar`
- **Build command**: `obtools/build/init.sh -t release -p web && tup`
- **Compiler flags**: `-DPLATFORM_WEB -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_FREETYPE=1`
- **Dynamic linking**: Main exe uses `-s MAIN_MODULE=1 -s WASM=1`; modules use `-s SIDE_MODULE=1`
- **Entry point**: `web/main.cc` — simplified engine with hardcoded test graph
- **Packaging**: `create-wasm.sh` uses Emscripten `file_packager.py` to bundle module `.so` files
- **Module list**: `web/WEB/files` lists ~120 modules to package
- **Platform guard**: `modules/module.h` has `PLATFORM_WEB` guard for logger init
- **ext- deps skipped**: Build rules silently skip `ext-*` dependencies for web platform (Emscripten provides SDL2/FreeType as built-in ports)

Modules declare `PLATFORMS` in their Tupfiles to control what gets built:
- `PLATFORMS = web` — `web/` only
- `PLATFORMS = posix web` — SDL audio/bitmap, image-in, wav-in, service, nexus-client, web-fetch
- `PLATFORMS = linux` — ALSA, OLA, Mosquitto (excluded from WASM)
- `PLATFORMS = posix` — engine, desktop, pitch-shift (excluded)
- No declaration — all platforms (most core/vector/maths/trigger/colour/binary modules)

## Blockers

### 1. Event Loop (Critical)

`web/main.cc` has a `while(true)` loop with `sleep_for` which blocks the browser main thread. Must use `emscripten_set_main_loop()` or build with `-s ASYNCIFY=1`.

Fix (~10 lines):
```cpp
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
// tick_callback calls engine.tick(Time::Duration::clock())
emscripten_set_main_loop(tick_callback, 25, 1);
#else
// existing while(true) loop
#endif
```

### 2. JavaScript-WASM Bridge (Critical for interactive use)

The web entry point bypasses the service/REST layer (which needs sockets). For interactive graph manipulation, the `vg-json` get/set/delete API needs to be exposed to JavaScript via `embind` or `EMSCRIPTEN_KEEPALIVE`. This replaces the REST API without requiring raw sockets.

### 3. Threading (Medium)

The parallel tick engine uses pthreads, condition variables, and RWMutex. Browser pthreads require `SharedArrayBuffer` which needs COOP/COEP HTTP headers. Options:
- Enable with `-pthread -s PROXY_TO_PTHREAD=1` and configure server headers
- Or run single-threaded (engine supports `threads=0`)

### 4. ObTools Headers Missing WASM Guards (Medium)

Several ObTools libraries are transitively included but have no `PLATFORM_WEB` guards:
- `ot-mt.h` — uses `pthread_kill()`, `pthread_setschedparam()`
- `ot-net.h` — raw socket APIs (`socket()`, `bind()`, `listen()`, `accept()`)
- `ot-daemon.h` — `fork()`, `signal()`, `syslog()`
- `ot-ssl-openssl.h` — OpenSSL not available in Emscripten

The web entry point avoids calling most of this code, but the headers still get included. Need `#ifndef PLATFORM_WEB` guards or stub implementations.

### 5. SDL Audio Autoplay Policy (Low)

Browser audio requires user interaction before playback starts. The SDL audio modules don't handle this. Need a JS-side click handler to unpause audio.

### 6. Dynamic Linking vs Static Linking (Recommended improvement)

The current approach loads modules as Emscripten side modules via `dlopen`. This works but is fragile (`-s ERROR_ON_UNDEFINED_SYMBOLS=0` suggests unresolved symbol issues). Statically linking all modules into one binary would:
- Eliminate dynamic loading complexity
- Produce a smaller, faster WASM binary
- Require a `vg_init_all()` that calls each module's `vg_init` (can be auto-generated)

## Module WASM Compatibility

### Work fine (pure computation, no external deps) — ~90 modules

| Category | Modules |
|----------|---------|
| core | pin, oscillator, filter, gate, compare, clone, average, interpolator, log, memory, random, switch, graph |
| maths | binary-ops, unary-ops, differentiate, integrate, limit, polar-position, wrap |
| trigger | beat, count, gate, latch, memory, pin, resample, selector, start |
| binary | clear, set, test |
| colour | blend, hsl, rgb, split, pin, switch |
| vector | add, clip, fade, figure, pattern, pin, rotate, scale, slice, stroke, svg, switch, translate |
| bitmap | blend, fade, fill, pin, rectangle, switch, tile, translate, twist, vector-fill |
| laser | add-blanking-anchors, add-vertex-repeats, beamify, infill-lines, reorder-segments, show-blanking |
| waveform | enum, pin, switch |
| time | clock, timer, now |
| time-series | average, capture, last, offset, plot, scale |
| midi (logic) | arpeggiate, assign-voice, button-in, control-in/out, keyboard-in, key-in/out, pin, switch |
| dmx (format) | artnet-out, bitmap-render, get-value, set-value |

### Work with Emscripten SDL ports (need autoplay fix)

- audio/sdl-in, audio/sdl-out, audio/wav-in
- bitmap/sdl-out, bitmap/image-in

### Already excluded from web build

- audio/alsa-in, audio/alsa-out (`PLATFORMS = linux`)
- audio/pitch-shift (`PLATFORMS = posix`)
- midi/alsa-in, midi/alsa-out (`PLATFORMS = linux`)
- midi/winmm-in, midi/winmm-out (`PLATFORMS = windows`)
- dmx/ola-in, dmx/ola-out (`PLATFORMS = linux`)
- iot/mqtt-in, iot/mqtt-out (`PLATFORMS = linux`)

### Should be excluded (hardware protocols, no browser use)

- laser/etherdream-out — hardware TCP to laser projector
- laser/idn-out — hardware UDP to laser projector

## Summary

The gap to a working WASM build is smaller than it appears. The build system, module exclusion, and platform gating are already in place. The critical path is:

1. Fix the event loop (`emscripten_set_main_loop`)
2. Add a JS bridge to expose graph manipulation (replacing REST)
3. Decide threading strategy (pthreads with headers, or single-threaded)
4. Add `PLATFORM_WEB` guards to ObTools headers
5. Optionally switch to static module linking for reliability

The ~90 core computation modules and the dataflow engine need zero changes.
