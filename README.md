# Audio Splitter 🎧

Hey there! Welcome to Audio Splitter, a super lightweight, minimal C++ app that lets you route a single audio stream to two output devices at the same time. We designed it with a modern, premium UI (think slick mobile app design) right on your desktop!

## What makes it tick? ⚙️

The project is split into four neat pieces:
* **Platform (Window)**: We run a pure native Win32 window and an OpenGL 3.3 Core context. No massive UI frameworks pulling down performance!
* **UI (UIManager)**: Powered by HTML/CSS via **RmlUi**. We've heavily optimized this part—no debuggers, no extra samples, just a custom OpenGL 3.3 backend to keep it blazing fast and the binary tiny.
* **Audio (AudioManager)**: Uses `miniaudio` under the hood. It grabs the loopback audio from your source and streams it in real-time to two separate playback devices using safe, lock-free ring buffers.
* **App (Application)**: The main loop that ties it all beautifully together.

## Code Style 📝

If you're jumping into the code, you'll notice we follow the **Unreal Engine Coding Standard**. That means `PascalCase`, `F` / `U` class prefixes, `b` for booleans, and Allman-style bracing. 
Don't worry about memorizing it though; there's a `.clang-format` file included to format things for you automatically!

## Getting Started 🚀

Want to build it yourself? It's surprisingly easy. The build system handles almost everything for you—including downloading and statically compiling a minimal version of Freetype from source so you don't have to manage external DLLs!

### Prerequisites

* **OS**: Windows 10 or 11
* **Toolchain**: The `MSYS2 UCRT64` environment (you'll need `c++`, `gcc`, `cmake`, and `ninja` installed).
* *Note: You do **not** need to install any external libraries. CMake will automatically fetch and compile everything it needs.*

### Building the Project 🔨

To keep everything clean, the project statically links the C++ runtime and its dependencies. The final release binary is a single, self-contained executable that comes out to about ~5.4 MB!

**1. Configure and Build (Release Mode):**
Open your MSYS2 UCRT64 terminal (or PowerShell) and run:
```bash
cmake -B build/release -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/gcc.exe \
      -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/c++.exe
cmake --build build/release
```

**2. Configure and Build (Debug Mode for development):**
```bash
cmake -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/gcc.exe \
      -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/c++.exe
cmake --build build/debug
```

### Running the App ▶️

Once the build finishes, you're good to go!
Head over to the `build/release` (or `build/debug`) directory. Inside, you'll find the `AudioSplitter.exe` file sitting right next to an `Assets/` folder. 

Because we statically link everything, there are **no messy DLLs** to worry about. Just double-click `AudioSplitter.exe` to launch the app!
