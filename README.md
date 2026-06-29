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

## Building

This project requires a C++20 compiler and CMake. It is tested on Windows 11 with the MSYS2 UCRT64 toolchain.

### Debug Build
```bash
cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug
```

### Release Build
```bash
cmake -B build/release -DCMAKE_BUILD_TYPE=Release
cmake --build build/release
```

After building, the executable and required UI assets can be found in the respective `build/debug` or `build/release` directory.
