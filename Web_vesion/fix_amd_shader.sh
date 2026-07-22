#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
shader="shaders/planet.frag"
if grep -q 'float noise3(vec3 p)' "$shader"; then
  sed -i 's/float noise3(vec3 p)/float valueNoise3D(vec3 p)/g; s/noise3(p)/valueNoise3D(p)/g' "$shader"
  echo "AMD GLSL compatibility fix applied."
elif grep -q 'float valueNoise3D(vec3 p)' "$shader"; then
  echo "Fix is already applied."
else
  echo "Expected shader function was not found." >&2
  exit 1
fi
