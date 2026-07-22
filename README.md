# CelestiaSphere (CosmoSim 3D) 🌌

Welcome to **CelestiaSphere** (also known as CosmoSim 3D), a stunning, interactive 3D Solar System and Deep Space Simulation. This project is designed to provide an educational, visually rich, and highly interactive experience of our solar system and beyond, built with modern C++ and OpenGL.

This repository contains two distinct versions of the simulation:
1. **PC Version:** A high-performance native desktop application (Windows/Linux).
2. **Web Version:** A WebAssembly (WASM) build that runs directly in your browser.

---

## ✨ Key Features

* **Interactive Solar System:** Explore the Sun, all 8 major planets, the Moon, Pluto, and even a Black Hole.
* **Realistic & Educational Scaling:** Switch between realistic distance scaling and an educational mode that makes planetary orbits easier to view.
* **Deep Space Missions:** Launch interactive spacecraft missions from Earth to any selected celestial body, complete with trajectory trails and progress tracking.
* **Dynamic Rendering:** Real-time orbit lines, dynamic asteroid belts, atmospheric glows, particle systems, and planetary rings.
* **Smart UI & Labels:** A modern, non-intrusive UI overlay with smart label filtering to prevent text overlap.
* **Multiple Camera Modes:** 
  * `Free`: Fly around the solar system anywhere you want.
  * `Follow`: Lock onto and orbit around a specific planet or moon.
  * `Top-Down`: View the solar system from above to see orbital mechanics.
  * `Cinematic`: Enjoy smooth, automated camera angles showcasing the cosmos.

---

## 🎮 Controls & Shortcuts

The simulation features an extensive set of controls for navigation and interaction:

### Navigation
* `W` `A` `S` `D` : Move the camera (Forward, Left, Backward, Right)
* `Q` / `E` : Move Down / Up
* `Shift` : Speed Boost while moving
* `Mouse Wheel` : Zoom in / out
* `Right Mouse Button` : Capture / Release cursor (for mouse look)
* `Left Mouse Button` : Click on a planet/body to select it (when cursor is free)

### Selection Shortcuts
* `0` : Select Sun
* `1` - `8` : Select Main Planets (Mercury to Neptune)
* `M` : Select Moon
* `9` : Select Pluto
* `H` : Select Black Hole
* `Left Arrow` / `Right Arrow` : Cycle through celestial bodies
* `F` : Follow Camera Mode
* `T` : Top-Down Camera Mode
* `C` : Cinematic Camera Mode
* `V` : Free Camera Mode

### Simulation & Display
* `Space` : Pause / Resume simulation time
* `-` / `+` : Decrease / Increase time speed
* `R` : Reset time speed
* `F11` : Toggle Fullscreen
* `O` : Toggle Orbit Lines
* `L` : Toggle Smart Tags/Labels
* `B` : Toggle Asteroid Belt
* `N` : Toggle Atmospheres
* `G` : Toggle Scale Mode (Educational vs Realistic)
* `J` : Launch Spacecraft Mission from Earth to the currently selected body
* `X` : Abort Spacecraft Mission
* `F1` : Show / Hide the Help Menu popup
* `Esc` : Exit Application (PC Version)

---

## 💻 PC Version (Native Desktop)

The PC version is built for maximum performance, utilizing raw OpenGL via GLEW and GLFW. 

### Prerequisites
* **MSYS2 (UCRT64)** or a standard Linux environment.
* C++17 compiler (GCC/Clang/MSVC).
* CMake (3.10+).

### Build Instructions
1. Open your terminal (e.g., MSYS2 UCRT64).
2. Navigate to the `PC_version` directory:
   ```bash
   cd PC_version
   ```
3. Run the setup and build script:
   ```bash
   ./setup.sh
   ```
4. Once built, run the executable:
   ```bash
   ./build/CosmoSim3D.exe
   ```

*For more detailed instructions, see the [PC Version README](./PC_version/README.md).*

---

## 🌐 Web Version (WebAssembly)

The Web Version allows anyone to experience the simulation directly in their web browser without installing any software, thanks to **Emscripten** (WebAssembly). 

### Prerequisites
* Python 3 (for running a local HTTP server).
* Emscripten SDK (if you wish to recompile the C++ code yourself).

### How to Play (Running Locally)
If you just want to run the pre-built web version:
1. Open a terminal and navigate to the `build-wasm` folder:
   ```bash
   cd Web_vesion/build-wasm
   ```
2. Start a local Python server:
   ```bash
   python -m http.server 8080
   ```
3. Open your web browser and go to: [http://localhost:8080/index.html](http://localhost:8080/index.html)

*For instructions on how to recompile the WASM code using Emscripten, see the [Web Version README](./Web_vesion/README.md).*

---

## 🛠️ Technology Stack
* **C++17** - Core programming language
* **OpenGL 3.3** - Graphics API (WebGL 2.0 on the web)
* **GLFW / GLEW** - Windowing and extension management (PC)
* **GLM** - OpenGL Mathematics library
* **Emscripten** - WebAssembly compiler toolchain
* **CMake** - Cross-platform build system

## 👨‍💻 Developer
Developed by **Estiuk Arafat Arnob**.
