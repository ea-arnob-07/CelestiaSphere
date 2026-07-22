# CelestiaSphere - Web Version (WebAssembly)

**CelestiaSphere**, An Interactive 3D Solar System and Deep Space Explorer, ported to run in your web browser using WebAssembly (Wasm) and WebGL 2! This project is written in C++17/OpenGL and compiled to WebAssembly using Emscripten.

## 🚀 Main Features

- **Browser Native:** Runs entirely in the browser using WebAssembly and WebGL 2.0.
- **Simulation Features:** Includes all the core features from the PC version (procedural planets, Kepler orbital mechanics, asteroid instancing, starfields, spacecraft missions).
- **Self-Contained Data:** Shaders are preloaded and packaged via Emscripten's virtual file system (`--preload-file`).

## 🛠️ Emscripten (emsdk) Setup & Installation

To build the WebAssembly version yourself, you need to use the Emscripten SDK, which is already included in the `emsdk` folder of this project!

### 1. Activate Emscripten Environment

Before building, you must activate the Emscripten environment. Open a terminal (PowerShell or Command Prompt) in the `Web_vesion` directory:

**For Windows (PowerShell):**
```powershell
cd d:\CelestiaSphere\Web_vesion
.\emsdk\emsdk_env.ps1
```

**For Windows (Command Prompt):**
```cmd
cd d:\CelestiaSphere\Web_vesion
.\emsdk\emsdk_env.bat
```

*(Note: You must run this activation script every time you open a new terminal window to build the web project).*

### 2. Configure with CMake

Use Emscripten's CMake wrapper `emcmake` to configure the project. This ensures the correct WebAssembly toolchain is used:

```bash
# Create and navigate to the build directory (e.g., build-wasm)
mkdir build-wasm
cd build-wasm

# Configure with emcmake
emcmake cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
```

### 3. Build the Project

Build the project using `cmake --build`:

```bash
cmake --build .
```
This will compile the C++ code and generate the following essential web files in your `build-wasm` directory:
- `CosmoSim3D.wasm` (The compiled WebAssembly binary)
- `CosmoSim3D.js` (Emscripten JavaScript runtime)
- `CosmoSim3D.data` (Packaged assets, like shaders)
- `index.html` (The web page to run the application)

## 🌐 Running the Web Version

Browsers restrict loading WebAssembly and local files directly via `file://` protocols for security reasons. **You must use a local web server to run the application.**

If you have Python installed, you can start a simple HTTP server from your `build-wasm` directory:

```bash
# Ensure you are inside the build-wasm folder
python -m http.server 8000
```

Then, open your web browser and go to:
[http://localhost:8000/index.html](http://localhost:8000/index.html)

## 🎮 Controls

The controls are identical to the PC version. Click on the canvas to capture your mouse (for looking around). Press `Esc` to release the mouse.

| Key / Input | Action |
|-------------|--------|
| `W A S D` | Move free camera |
| `Q / E` | Move down / up |
| `Shift` | Camera boost |
| `Mouse` | Look around while cursor is captured |
| `Mouse wheel` | Zoom or change follow distance |
| `Right / Esc`| Release cursor |
| `Left mouse` | Select a celestial body while cursor is free |
| `1` to `9` | Select Planets |
| `0` | Select Sun |
| `F` | Follow selected body |
| `M` | Launch Earth-to-selected-body mission |
| `F1` | Show or hide complete help |

## 📁 Project Architecture (Web specifics)

- `emsdk/`: Emscripten toolchain for compiling C++ to WebAssembly.
- `CMakeLists.txt`: Configured to fetch dependencies like GLM if Emscripten is detected, and applies WebGL2 / Emscripten linker flags (e.g., `-s USE_WEBGL2=1 -s WASM=1 -s FULL_ES3=1 --preload-file shaders@/shaders`).
- `build-wasm/`: The designated directory for the compiled web files.
