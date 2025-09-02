#!/usr/bin/env bash
# MSYS2 MinGW x64 launcher for Woflang
# Runs the built exe from build/bin so the relative "plugins" folder is found.

set -euo pipefail

# Resolve repo root from this script's location
ROOT="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &>/dev/null && pwd )"
BIN="$ROOT/build/bin"
PLUGINS="$BIN/plugins"
EXE="$BIN/woflang.exe"

# Ensure directories exist
mkdir -p "$BIN" "$PLUGINS"

# Build if missing
if [[ ! -x "$EXE" ]]; then
  echo "[woflang.sh] exe not found, building…"
  cmake -S "$ROOT" -B "$ROOT/build" -G "Ninja" -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_COMPILER="${CXX:-/mingw64/bin/c++.exe}"
  cmake --build "$ROOT/build" -j
fi

# Make sure MinGW runtime and our bin are discoverable (usually already in MSYS2)
export PATH="/mingw64/bin:$BIN:$PATH"

# Run from bin so the loader sees ./plugins
cd "$BIN"
exec "$EXE" "$@"
