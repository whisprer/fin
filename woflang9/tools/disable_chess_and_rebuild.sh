#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
echo "[disable-chess] root: $ROOT"

test -d plugins || { echo "[disable-chess] plugins/ not found"; exit 1; }

# Write a plugins/CMakeLists.txt that skips chess and legacy APIs
cat > plugins/CMakeLists.txt <<'EOF'
cmake_minimum_required(VERSION 3.16)

# build only *_ops.cpp
file(GLOB ALL_PLUGIN_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*_ops.cpp")

set(PLUGIN_SRCS "")

foreach(SRC ${ALL_PLUGIN_SRCS})
  get_filename_component(NAME "${SRC}" NAME)
  string(TOLOWER "${NAME}" LNAME)

  # 1) skip chess while it's being rewritten
  string(REGEX MATCH "chess" HAS_CHESS "${LNAME}")
  if (HAS_CHESS)
    message(STATUS "Skipping plugin (chess disabled): ${NAME}")
    continue()
  endif()

  # 2) skip legacy field access (must be ported to new API)
  file(READ "${SRC}" SRC_TXT)
  string(REGEX MATCH "(WofType|\\.type|\\.value)" HAS_LEGACY "${SRC_TXT}")
  if (HAS_LEGACY)
    message(STATUS "Skipping plugin (legacy API): ${NAME}")
    continue()
  endif()

  list(APPEND PLUGIN_SRCS "${SRC}")
endforeach()

# output next to the EXE, in lowercase 'plugins'
set(_OUT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins")

foreach(SRC ${PLUGIN_SRCS})
  get_filename_component(base "${SRC}" NAME_WE)
  add_library(${base} SHARED "${SRC}")
  target_include_directories(${base} PRIVATE
    ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/core ${CMAKE_SOURCE_DIR}/plugins)
  target_link_libraries(${base} PRIVATE woflang_core)
  # helpful shim for MinGW M_PI etc.
  target_compile_options(${base} PRIVATE -include "${CMAKE_SOURCE_DIR}/core/woflang_compat.hpp")
  set_target_properties(${base} PROPERTIES
    OUTPUT_NAME "lib${base}"
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${_OUT}"
    LIBRARY_OUTPUT_DIRECTORY "${_OUT}"
    ARCHIVE_OUTPUT_DIRECTORY "${_OUT}")
endforeach()
EOF
echo "[disable-chess] wrote plugins/CMakeLists.txt"

# Remove any already-built chess DLLs so they can't load
rm -f build/bin/plugins/*chess*.dll 2>/dev/null || true
rm -f build/bin/Plugins/*chess*.dll 2>/dev/null || true

# Rebuild with MinGW
rm -rf build
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=/mingw64/bin/cc.exe \
  -DCMAKE_CXX_COMPILER=/mingw64/bin/c++.exe
cmake --build build -j

echo "[disable-chess] plugins present now:"
ls -1 build/bin/plugins || true
echo "[disable-chess] done."
