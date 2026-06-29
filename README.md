# Audio Splitter

Audio Splitter is a lightweight, minimal C++ application designed to route a single audio stream to two output devices simultaneously. It features a modern, premium UI inspired by mobile app design.

## Architecture

The project is structured into four main components:
* **Platform (Window)**: Manages a pure native Win32 window and an OpenGL 3.3 Core context.
* **UI (UIManager)**: Handles the HTML/CSS-based interface using RmlUi. The RmlUi dependency is deeply optimized, using a custom OpenGL 3.3 backend and omitting unnecessary samples, debuggers, and shell wrappers to minimize build times and binary size.
* **Audio (AudioManager)**: Uses `miniaudio` to capture loopback audio from a source device and streams it in real-time to two separate playback devices using safe, lock-free ring buffers.
* **App (Application)**: Ties the systems together in the main loop.

## Code Style

This project adheres to the **Unreal Engine Coding Standard**. The codebase uses PascalCase, `F` / `U` prefixes for classes, `b` prefixes for booleans, and Allman-style bracing. A `.clang-format` file is provided to automatically enforce these rules.

## Prerequisites

* **OS**: Windows 10+ (tested on Windows 11)
* **Toolchain**: MSYS2 UCRT64 (`c++`, `gcc`, `cmake`, `ninja`)
* **Libraries**: `freetype` (installed via `pacman -S mingw-w64-ucrt-x86_64-freetype`)
* **CMake**: 3.20+

## Building

The build system statically links the C++ runtime, so no external DLLs are needed. The release binary is stripped to ~5 MB.

### Configure + Build (Debug)
```bash
cmake -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/gcc.exe \
      -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/c++.exe
cmake --build build/debug
```

### Configure + Build (Release)
```bash
cmake -B build/release -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/gcc.exe \
      -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/c++.exe
cmake --build build/release
```

### Clean Rebuild
```bash
cmake --build build/debug --clean-first
cmake --build build/release --clean-first
```

After building, the executable and `Assets/` folder are in the respective `build/debug` or `build/release` directory. The release binary is a single self-contained `.exe` — no DLLs required.
