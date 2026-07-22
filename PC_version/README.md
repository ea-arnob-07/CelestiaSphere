# CelestiaSphere - PC Edition

**CelestiaSphere**, An Interactive 3D Solar System and Deep Space Explorer, is a complete C++17/OpenGL computer graphics project designed for MSYS2 UCRT64 on Windows. It renders an interactive procedural solar system without requiring downloaded textures or 3D model files.

## 🚀 Main Features

- **Advanced Rendering:** OpenGL 3.3 Core Profile.
- **Solar System Simulation:** Sun, eight planets, Pluto, Earth's Moon, and Phobos.
- **Accurate Orbital Physics:** Kepler-equation-based elliptical orbital motion including inclination, ascending node, periapsis, rotation, and axial tilt.
- **Procedural Generation:** Procedural rocky, Earth-like, gas giant, ice giant, and stellar surfaces.
- **Atmospheric Effects:** Atmospheric Fresnel shells.
- **Rings & Asteroids:** Saturn and Uranus ring systems, and 1,500 animated asteroids using GPU instanced rendering.
- **Deep Space:** 4,800-star procedural starfield with twinkling, solar corona, and spacecraft engine particle systems.
- **Spacecraft Mission Simulation:** Earth-to-selected-body spacecraft mission simulation with Bezier trajectory, spacecraft trail, progress, fuel, speed, and remaining-distance telemetry.
- **Camera System:** Free, follow, top-down, and cinematic cameras.
- **Interactivity:** Mouse ray-casting selection, built-in bitmap HUD, and information panels.
- **Educational Tools:** Educational and expanded distance scale modes.
- **Self-Contained:** No external asset folder is required!

## 🛠️ MSYS2 UCRT64 Setup & Installation

This project is built for Windows using MSYS2. 

1. Install MSYS2 from its official installer.
2. Open **MSYS2 UCRT64** (Do not use the plain MSYS or CLANG shell).

Run the following commands to update your package database and base packages:

```bash
pacman -Syu
```

If the terminal asks you to close it, close the UCRT64 window, reopen it, and run:

```bash
pacman -Syu
pacman -S --needed \
  mingw-w64-ucrt-x86_64-toolchain \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-glfw \
  mingw-w64-ucrt-x86_64-glew \
  mingw-w64-ucrt-x86_64-glm
```

## 🏗️ Build and Run

Navigate to the project directory in your MSYS2 UCRT64 terminal:

```bash
cd /d/CelestiaSphere/PC_version
```

### Option 1: Using the provided script
You can use the provided build script to compile and run automatically:
```bash
./build_and_run.sh
```

### Option 2: Manual Build (CMake + Ninja)
```bash
# Configure the project
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build build

# Run the executable
./build/CelestiaSphere.exe
```

## 🎮 Controls

| Key / Input | Action |
|-------------|--------|
| `W A S D` | Move free camera |
| `Q / E` | Move down / up |
| `Shift` | Camera boost |
| `Mouse` | Look around while cursor is captured |
| `Mouse wheel` | Zoom or change follow distance |
| `Right mouse` | Capture or release cursor |
| `Left mouse` | Select a celestial body while cursor is free |
| `1` to `8` | Select Mercury through Neptune |
| `0` | Select Sun |
| `9` | Select Pluto |
| `Left / Right arrow`| Previous / next body |
| `F` | Follow selected body |
| `T` | Top-down view |
| `C` | Cinematic camera |
| `V` | Free camera |
| `Tab` | Cycle camera modes |
| `Space` | Pause or resume simulation |
| `- / +` | Decrease or increase simulation speed |
| `O` | Toggle orbit lines |
| `L` | Toggle labels |
| `B` | Toggle asteroid belt |
| `N` | Toggle atmospheres |
| `G` | Toggle scale mode |
| `M` | Launch Earth-to-selected-body mission |
| `X` | Abort mission |
| `R` | Reset epoch and mission |
| `F1` | Show or hide complete help |
| `Esc` | Exit |

## 📁 Project Architecture

```text
CelestiaSphere/PC_version/
├── CMakeLists.txt
├── README.md
├── build_and_run.sh
├── src/
│   ├── Application.*       Window, loop, input, picking
│   ├── Camera.*            Four camera modes
│   ├── Geometry.*          Procedural meshes
│   ├── Mesh.*              VAO/VBO/EBO and instancing
│   ├── ParticleSystem.*    Sun and engine particles
│   ├── Renderer.*          Rendering pipeline and HUD data
│   ├── Shader.*            GLSL compilation and uniforms
│   ├── SolarSystem.*       Bodies and orbital physics
│   ├── Spacecraft.*        Mission and trajectory logic
│   ├── Starfield.*         GPU point starfield
│   ├── UiOverlay.*         Procedural bitmap font UI
│   └── main.cpp
└── shaders/
    ├── planet.*            Planet shading
    ├── asteroid.*          Instanced asteroids
    ├── atmosphere.*        Atmosphere rendering
    ├── particle.*          Particles
    ├── star.*              Star rendering
    ├── line.*              Orbit lines
    ├── unlit.*             Unlit models
    └── ui.*                UI rendering
```

## 🐛 Troubleshooting

### `glfw3Config.cmake` not found
Confirm that you opened **MSYS2 UCRT64** and installed GLFW:
```bash
pacman -S mingw-w64-ucrt-x86_64-glfw
```
Delete the old build directory and configure again:
```bash
rm -rf build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

### `GLEW::GLEW` or `glm::glm` not found
Ensure GLEW and GLM are installed:
```bash
pacman -S mingw-w64-ucrt-x86_64-glew mingw-w64-ucrt-x86_64-glm
```

### Black screen or OpenGL error
Update your graphics driver. This project requires OpenGL 3.3 Core Profile or newer. If you have an AMD integrated GPU, there might be driver-specific quirks.

### Low FPS
Press `B` to disable the asteroid belt. You can also reduce `asteroidCount` in `Renderer.cpp` from 1500 to 600 and reduce the sphere segments in `Renderer::initialize`.
