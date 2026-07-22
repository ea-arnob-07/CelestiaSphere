#!/usr/bin/env bash
set -e

echo "Cleaning previous build directory..."
rm -rf build

echo "Configuring project using CMake and Ninja..."
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

echo "Building project..."
cmake --build build

echo "Build complete. Running executable..."
# Find the exact executable name in the build folder
EXE_NAME=$(find build -maxdepth 1 -name "*.exe" | head -n 1)

if [ -n "$EXE_NAME" ]; then
    "$EXE_NAME"
else
    echo "Error: Executable not found in build directory."
    exit 1
fi
