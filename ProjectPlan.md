## Simple Architecture

```text
AudioSplitter/

├── Assets/
│   ├── UI/
│   │   ├── Main.rml
│   │   └── Main.rcss
│   │
│   └── Fonts/
│
├── Source/
│   ├── Main.cpp
│   │
│   ├── App/
│   │   ├── Application.h
│   │   └── Application.cpp
│   │
│   ├── UI/
│   │   ├── UIManager.h
│   │   └── UIManager.cpp
│   │
│   ├── Audio/
│   │   ├── AudioManager.h
│   │   └── AudioManager.cpp
│   │
│   └── Platform/
│       ├── Window.h
│       └── Window.cpp
│
├── ThirdParty/
│   └── RmlUi/
│
└── CMakeLists.txt
```

---

## Core Systems

### Application

Controls everything.

```cpp
Application
{
    Window
    UIManager
    AudioManager
}
```

---

### Window

Responsibilities:

* Create GLFW window
* Process input
* OpenGL context

---

### UIManager

Responsibilities:

* Load RML files
* Handle button clicks
* Update device list

Example:

```cpp
Start Button
    ↓
UIManager
    ↓
AudioManager.Start()
```

---

### AudioManager

Responsibilities:

* Get audio devices
* Select Device 1
* Select Device 2
* Start audio duplication
* Stop audio duplication

Example:

```cpp
AudioManager
{
    EnumerateDevices();
    Start();
    Stop();
}
```

---

## Main Screen

Only one screen.

```text
+----------------------------+
|      Audio Splitter        |
+----------------------------+

Output Device 1
[ Headphones ▼ ]

Output Device 2
[ Earbuds ▼ ]

[ Start ]

[ Stop ]

Status:
Running
```

No settings page.
No themes.
No latency graph.

Just make it work.

---

## Development Phases

### Phase 1

Create:

* GLFW Window
* OpenGL
* RmlUi

Goal:

```text
Window opens
UI loads
Button clicks work
```

---

### Phase 2

Create:

```cpp
GetAudioDevices()
```

Display devices in dropdowns.

Goal:

```text
Select Headphones
Select Earbuds
```

---

### Phase 3

Implement:

```cpp
Start()
Stop()
```

Duplicate audio.

Goal:

```text
Audio plays on both devices
```

---

### Phase 4

Add quality-of-life features:

* Refresh devices
* Remember last selection
* Minimize to tray

---

### Final Architecture

```text
Application
│
├── Window
│
├── UIManager
│
└── AudioManager
```

That's all you need initially. Keep it under **10 source files** and get a working prototype before worrying about event systems, latency managers, plugins, or advanced architecture.
