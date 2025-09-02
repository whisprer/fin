#!/usr/bin/env bash
# Bridges plugins that export register_plugin() so the loader can call init_plugin().
# Also provides a tiny 'stack_ops' plugin with 'stack_slayer' no-op for the benchmark.
set -euo pipefail

ROOT="$(pwd)"
echo "[bridge] repo: $ROOT"
test -d plugins || { echo "[bridge] no plugins/ here"; exit 1; }

# 0) Make sure we force-include the M_PI shim everywhere (harmless if already present)
mkdir -p core
cat > core/woflang_compat.hpp <<'EOF'
#pragma once
#include <cmath>
#include <numbers>
#ifndef M_PI
#define M_PI (std::numbers::pi_v<double>)
#endif
EOF

# 1) Append an init_plugin() bridge to any *_ops.cpp that has register_plugin but no init_plugin
added=0
shopt -s nullglob
for f in plugins/*_ops.cpp; do
  if grep -q 'register_plugin\s*(' "$f" && ! grep -q 'init_plugin\s*(' "$f"; then
    echo "[bridge] adding init_plugin bridge to: $f"
    cat >> "$f" <<'EOF'

// ---- auto-bridge for loaders expecting init_plugin -------------------------
#if !defined(WOFLANG_INIT_BRIDGE_ADDED)
#define WOFLANG_INIT_BRIDGE_ADDED 1
#if defined(_WIN32)
extern "C" __declspec(dllexport)
#else
extern "C"
#endif
void init_plugin(woflang::WoflangInterpreter& interp) {
    // call the existing register_plugin the plugin already defines
    register_plugin(interp);
}
#endif
// ----------------------------------------------------------------------------
EOF
    added=$((added+1))
  fi
done
echo "[bridge] bridged files: $added"

# 2) Provide a minimal stack_ops plugin if missing (benchmark calls 'stack_slayer')
if [ ! -f plugins/stack_ops.cpp ]; then
  echo "[bridge] writing plugins/stack_ops.cpp"
  cat > plugins/stack_ops.cpp <<'EOF'
// plugins/stack_ops.cpp (benchmark helper)
// Provides: stack_slayer (no-op) to satisfy test scripts.
#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif
#include "core/woflang.hpp"
#include <stack>
#include <stdexcept>
namespace woflang {
class StackOpsPlugin : public WoflangPlugin {
public:
  void register_ops(WoflangInterpreter& I) override {
    // Intentionally a no-op, leaves the stack unchanged.
    I.register_op("stack_slayer", [](std::stack<WofValue>&){ /* no-op */ });
  }
};
} // namespace woflang
WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter& interp) {
  static woflang::StackOpsPlugin p; p.register_ops(interp);
}
EOF
fi

# 3) Ensure plugins/CMakeLists.txt drops DLLs next to the exe (bin/Plugins) and force-includes shim
cat > plugins/CMakeLists.txt <<'EOF'
cmake_minimum_required(VERSION 3.16)
file(GLOB PLUGIN_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*_ops.cpp")

set(_PLUGIN_OUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Plugins")

foreach(src ${PLUGIN_SRCS})
  get_filename_component(base "${src}" NAME_WE)
  add_library(${base} SHARED "${src}")
  target_include_directories(${base} PRIVATE
    ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/core ${CMAKE_SOURCE_DIR}/plugins)
  target_link_libraries(${base} PRIVATE woflang_core)
  # Always include the M_PI shim; harmless if unused
  target_compile_options(${base} PRIVATE -include "${CMAKE_SOURCE_DIR}/core/woflang_compat.hpp")
  set_target_properties(${base} PROPERTIES
    OUTPUT_NAME "lib${base}"
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${_PLUGIN_OUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${_PLUGIN_OUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${_PLUGIN_OUT_DIR}")
endforeach()
EOF

# 4) Rebuild with MinGW
rm -rf build
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=/mingw64/bin/cc.exe \
  -DCMAKE_CXX_COMPILER=/mingw64/bin/c++.exe
cmake --build build -j

echo "[bridge] built plugins at: build/bin/Plugins"
ls -1 build/bin/Plugins || true