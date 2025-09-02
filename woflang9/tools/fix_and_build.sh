#!/usr/bin/env bash
set -euo pipefail

# --- paths
ROOT="$(pwd)"
echo "[fix] repo root: $ROOT"

# --- sanity: ensure we're in the right tree
test -d "$ROOT/plugins" || { echo "[fix] no plugins/ here"; exit 1; }

# --- 1) compat header for M_PI (C++20 numbers)
mkdir -p "$ROOT/core"
cat > "$ROOT/core/woflang_compat.hpp" << 'EOF'
#pragma once
#include <cmath>
#include <numbers>
#ifndef M_PI
#define M_PI (std::numbers::pi_v<double>)
#endif
EOF
echo "[fix] wrote core/woflang_compat.hpp"

# --- 2) normalize all plugin includes to point at your header
# Any form of xxx/woflang.hpp -> core/woflang.hpp
shopt -s nullglob
for f in "$ROOT"/plugins/*.cpp "$ROOT"/plugins/*.hpp; do
  sed -E -i 's|^([[:space:]]*#include[[:space:]]*")(.*/)?woflang\.hpp(")|\1core/woflang.hpp\3|' "$f" || true
done
echo "[fix] normalized #include \"core/woflang.hpp\" in plugins/*"

# --- 3) drop in the 4 fixed, stack-based modules (overwrite if present)

# trig_ops.cpp
cat > "$ROOT/plugins/trig_ops.cpp" << 'EOF'
// plugins/trig_ops.cpp
#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif
#include "core/woflang.hpp"
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <stack>
namespace woflang {
static inline double deg2rad(double d){ return d*std::numbers::pi_v<double>/180.0; }
static inline double rad2deg(double r){ return r*180.0/std::numbers::pi_v<double>; }
static inline double need_num(const WofValue& v,const char* op){ if(!v.is_numeric()) throw std::runtime_error(std::string(op)+": numeric required"); return v.as_numeric(); }
class TrigOpsPlugin : public WoflangPlugin {
public:
  void register_ops(WoflangInterpreter& I) override {
    I.register_op("sin", [](std::stack<WofValue>& S){ if(S.size()<1) throw std::runtime_error("sin: underflow"); auto a=S.top(); S.pop(); S.push(WofValue(std::sin(need_num(a,"sin")))); });
    I.register_op("cos", [](std::stack<WofValue>& S){ if(S.size()<1) throw std::runtime_error("cos: underflow"); auto a=S.top(); S.pop(); S.push(WofValue(std::cos(need_num(a,"cos")))); });
    I.register_op("tan", [](std::stack<WofValue>& S){ if(S.size()<1) throw std::runtime_error("tan: underflow"); auto a=S.top(); S.pop(); S.push(WofValue(std::tan(need_num(a,"tan")))); });
    I.register_op("asin",[](std::stack<WofValue>& S){ if(S.size()<1) throw std::runtime_error("asin: underflow"); auto a=S.top(); S.pop(); double v=need_num(a,"asin"); if(v<-1||v>1) throw std::runtime_error("asin: domain [-1,1]"); S.push(WofValue(std::asin(v))); });
    I.register_op("acos",[](std::stack<WofValue>& S){ if(S.size()<1) throw std::runtime_error("acos: underflow"); auto a=S.top(); S.pop(); double v=need_num(a,"acos"); if(v<-1||v>1) throw std::runtime_error("acos: domain [-1,1]"); S.push(WofValue(std::acos(v))); });
    I.register_op("atan",[](std::stack<WofValue>& S){ if(S.size()<1) throw std::runtime_error("atan: underflow"); auto a=S.top(); S.pop(); S.push(WofValue(std::atan(need_num(a,"atan")))); });
    I.register_op("atan2",[](std::stack<WofValue>& S){ if(S.size()<2) throw std::runtime_error("atan2: underflow"); auto x=S.top(); S.pop(); auto y=S.top(); S.pop(); S.push(WofValue(std::atan2(need_num(y,"atan2"),need_num(x,"atan2")))); });
    I.register_op("sinh",[](std::stack<WofValue>& S){ if(S.size()<1) throw std::runtime_error("sinh: underflow"); auto a=S.top(); S.pop(); S.push(WofValue(std::sinh(need_num(a,"sinh")))); });
    I.register_op("cosh",[](std::stack<WofValue>& S){ if(S.size()<1) throw std::runtime_error("cosh: underflow"); auto a=S.top(); S.pop(); S.push(WofValue(std::cosh(need_num(a,"cosh")))); });
    I.register_op("tanh",[](std::stack<WofValue>& S){ if(S.size()<1) throw std::runtime_error("tanh: underflow"); auto a=S.top(); S.pop(); S.push(WofValue(std::tanh(need_num(a,"tanh")))); });
    I.register_op("deg2rad",[](std::stack<WofValue>& S){ if(S.size()<1) throw std::runtime_error("deg2rad: underflow"); auto d=S.top(); S.pop(); S.push(WofValue(deg2rad(need_num(d,"deg2rad")))); });
    I.register_op("rad2deg",[](std::stack<WofValue>& S){ if(S.size()<1) throw std::runtime_error("rad2deg: underflow"); auto r=S.top(); S.pop(); S.push(WofValue(rad2deg(need_num(r,"rad2deg")))); });
  }
};
} // namespace woflang
WOFLANG_PLUGIN_EXPORT void register_plugin(woflang::WoflangInterpreter& interp){ static woflang::TrigOpsPlugin p; p.register_ops(interp); }
EOF
echo "[fix] wrote plugins/trig_ops.cpp"

# geom_transform_ops.cpp
cat > "$ROOT/plugins/geom_transform_ops.cpp" << 'EOF'
// plugins/geom_transform_ops.cpp
#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif
#include "core/woflang.hpp"
#include <cmath>
#include <stdexcept>
#include <stack>
namespace woflang {
static inline double deg2rad(double d){ return d*3.141592653589793238462643383279502884/180.0; }
static inline double need_num(const WofValue& v,const char* op){ if(!v.is_numeric()) throw std::runtime_error(std::string(op)+": numeric required"); return v.as_numeric(); }
class GeomTransformOps : public WoflangPlugin {
public:
  void register_ops(WoflangInterpreter& I) override {
    I.register_op("rotate2d",[](std::stack<WofValue>& S){
      if(S.size()<3) throw std::runtime_error("rotate2d: need x y angle_deg");
      auto a=S.top(); S.pop(); auto y=S.top(); S.pop(); auto x=S.top(); S.pop();
      double ang=deg2rad(need_num(a,"rotate2d")), c=std::cos(ang), s=std::sin(ang);
      S.push(WofValue(c*need_num(x,"rotate2d") - s*need_num(y,"rotate2d")));
      S.push(WofValue(s*need_num(x,"rotate2d") + c*need_num(y,"rotate2d")));
    });
    I.register_op("translate2d",[](std::stack<WofValue>& S){
      if(S.size()<4) throw std::runtime_error("translate2d: need x y dx dy");
      auto dy=S.top(); S.pop(); auto dx=S.top(); S.pop(); auto y=S.top(); S.pop(); auto x=S.top(); S.pop();
      S.push(WofValue(need_num(x,"translate2d")+need_num(dx,"translate2d")));
      S.push(WofValue(need_num(y,"translate2d")+need_num(dy,"translate2d")));
    });
    I.register_op("scale2d",[](std::stack<WofValue>& S){
      if(S.size()<4) throw std::runtime_error("scale2d: need x y sx sy");
      auto sy=S.top(); S.pop(); auto sx=S.top(); S.pop(); auto y=S.top(); S.pop(); auto x=S.top(); S.pop();
      S.push(WofValue(need_num(x,"scale2d")*need_num(sx,"scale2d")));
      S.push(WofValue(need_num(y,"scale2d")*need_num(sy,"scale2d")));
    });
    I.register_op("reflect_x",[](std::stack<WofValue>& S){
      if(S.size()<2) throw std::runtime_error("reflect_x: need x y"); auto y=S.top(); S.pop(); auto x=S.top(); S.pop();
      S.push(WofValue(need_num(x,"reflect_x"))); S.push(WofValue(-need_num(y,"reflect_x")));
    });
    I.register_op("reflect_y",[](std::stack<WofValue>& S){
      if(S.size()<2) throw std::runtime_error("reflect_y: need x y"); auto y=S.top(); S.pop(); auto x=S.top(); S.pop();
      S.push(WofValue(-need_num(x,"reflect_y"))); S.push(WofValue(need_num(y,"reflect_y")));
    });
  }
};
} // namespace woflang
WOFLANG_PLUGIN_EXPORT void register_plugin(woflang::WoflangInterpreter& interp){ static woflang::GeomTransformOps p; p.register_ops(interp); }
EOF
echo "[fix] wrote plugins/geom_transform_ops.cpp"

# crypto_ops.cpp
cat > "$ROOT/plugins/crypto_ops.cpp" << 'EOF'
// plugins/crypto_ops.cpp
#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif
#include "core/woflang.hpp"
#include <array>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>
#include <stack>
namespace woflang {
static inline double need_num(const WofValue& v,const char* op){ if(!v.is_numeric()) throw std::runtime_error(std::string(op)+": numeric required"); return v.as_numeric(); }
static std::string to_hex_bytes(const std::vector<uint8_t>& b){ static const char* H="0123456789abcdef"; std::string o; o.reserve(b.size()*2); for(uint8_t x: b){ o.push_back(H[x>>4]); o.push_back(H[x&15]); } return o; }
static std::vector<uint8_t> from_hex_bytes(const std::string& s){ auto hv=[](char c)->int{ if(c>='0'&&c<='9') return c-'0'; if(c>='a'&&c<='f') return c-'a'+10; if(c>='A'&&c<='F') return c-'A'+10; return -1; }; if(s.size()%2) throw std::runtime_error("from_hex: odd length"); std::vector<uint8_t> o; o.reserve(s.size()/2); for(size_t i=0;i<s.size();i+=2){ int hi=hv(s[i]),lo=hv(s[i+1]); if(hi<0||lo<0) throw std::runtime_error("from_hex: bad digit"); o.push_back(uint8_t((hi<<4)|lo)); } return o; }
static const char* B64="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static std::string b64_encode(const std::vector<uint8_t>& d){ std::string o; o.reserve(((d.size()+2)/3)*4); size_t i=0; while(i+3<=d.size()){ uint32_t n=(d[i]<<16)|(d[i+1]<<8)|d[i+2]; i+=3; o.push_back(B64[(n>>18)&63]); o.push_back(B64[(n>>12)&63]); o.push_back(B64[(n>>6)&63]); o.push_back(B64[n&63]); } if(i+1==d.size()){ uint32_t n=(d[i]<<16); o.push_back(B64[(n>>18)&63]); o.push_back(B64[(n>>12)&63]); o.push_back('='); o.push_back('='); } else if(i+2==d.size()){ uint32_t n=(d[i]<<16)|(d[i+1]<<8); o.push_back(B64[(n>>18)&63]); o.push_back(B64[(n>>12)&63]); o.push_back(B64[(n>>6)&63]); o.push_back('='); } return o; }
static std::vector<uint8_t> b64_decode(const std::string& s){ std::array<int,256> T{}; T.fill(-1); for(int i=0;i<64;++i) T[uint8_t(B64[i])]=i; std::vector<uint8_t> o; int val=0,valb=-8; for(unsigned char c: s){ if(c=='=') break; int d=T[c]; if(d<0) continue; val=(val<<6)|d; valb+=6; if(valb>=0){ o.push_back(uint8_t((val>>valb)&0xFF)); valb-=8; } } return o; }
class CryptoOpsPlugin : public WoflangPlugin {
public:
  void register_ops(WoflangInterpreter& I) override {
    I.register_op("to_hex",[](std::stack<WofValue>& S){
      if(S.size()<1) throw std::runtime_error("to_hex: underflow");
      std::string s = S.top().to_string(); S.pop();
      std::vector<uint8_t> b(s.begin(), s.end()); S.push(WofValue(to_hex_bytes(b)));
    });
    I.register_op("from_hex",[](std::stack<WofValue>& S){
      if(S.size()<1) throw std::runtime_error("from_hex: underflow");
      std::string hx = S.top().to_string(); S.pop();
      auto bytes = from_hex_bytes(hx); S.push(WofValue(std::string(bytes.begin(), bytes.end())));
    });
    I.register_op("base64_encode",[](std::stack<WofValue>& S){
      if(S.size()<1) throw std::runtime_error("base64_encode: underflow");
      std::string s = S.top().to_string(); S.pop();
      std::vector<uint8_t> b(s.begin(), s.end()); S.push(WofValue(b64_encode(b)));
    });
    I.register_op("base64_decode",[](std::stack<WofValue>& S){
      if(S.size()<1) throw std::runtime_error("base64_decode: underflow");
      std::string s = S.top().to_string(); S.pop();
      auto bytes = b64_decode(s); S.push(WofValue(std::string(bytes.begin(), bytes.end())));
    });
    I.register_op("random",[](std::stack<WofValue>& S){
      static thread_local std::mt19937_64 rng{std::random_device{}()}; std::uniform_real_distribution<double> d(0.0,1.0);
      S.push(WofValue(d(rng)));
    });
    I.register_op("random_bytes",[](std::stack<WofValue>& S){
      if(S.size()<1) throw std::runtime_error("random_bytes: underflow");
      int n = static_cast<int>(need_num(S.top(),"random_bytes")); S.pop();
      if(n<0 || n>1000000) throw std::runtime_error("random_bytes: n out of range");
      static thread_local std::mt19937_64 rng{std::random_device{}()}; std::uniform_int_distribution<int> d(0,255);
      std::string s; s.resize(n); for(int i=0;i<n;++i) s[i]=char(d(rng)); S.push(WofValue(std::move(s)));
    });
  }
};
} // namespace woflang
WOFLANG_PLUGIN_EXPORT void register_plugin(woflang::WoflangInterpreter& interp){ static woflang::CryptoOpsPlugin p; p.register_ops(interp); }
EOF
echo "[fix] wrote plugins/crypto_ops.cpp"

# fractal_ops.cpp
cat > "$ROOT/plugins/fractal_ops.cpp" << 'EOF'
// plugins/fractal_ops.cpp
#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif
#include "core/woflang.hpp"
#include <cmath>
#include <stdexcept>
#include <stack>
namespace woflang {
static int mandelbrot_iters(double cr,double ci,int max_iter){ double zr=0,zi=0; int i=0; while(i<max_iter){ double zr2=zr*zr-zi*zi+cr; double zi2=2*zr*zi+ci; zr=zr2; zi=zi2; if(zr*zr+zi*zi>4.0) break; ++i; } return i; }
static int julia_iters(double zr,double zi,double cr,double ci,int max_iter){ int i=0; while(i<max_iter){ double zr2=zr*zr-zi*zi+cr; double zi2=2*zr*zi+ci; zr=zr2; zi=zi2; if(zr*zr+zi*zi>4.0) break; ++i; } return i; }
static inline double need_num(const WofValue& v,const char* op){ if(!v.is_numeric()) throw std::runtime_error(std::string(op)+": numeric required"); return v.as_numeric(); }
class FractalOpsPlugin : public WoflangPlugin {
public:
  void register_ops(WoflangInterpreter& I) override {
    I.register_op("mandelbrot",[](std::stack<WofValue>& S){
      if(S.size()<3) throw std::runtime_error("mandelbrot: need cr ci max_iter");
      auto m=S.top(); S.pop(); auto ci=S.top(); S.pop(); auto cr=S.top(); S.pop();
      int it = mandelbrot_iters(need_num(cr,"mandelbrot"), need_num(ci,"mandelbrot"), (int)need_num(m,"mandelbrot"));
      S.push(WofValue((double)it));
    });
    I.register_op("julia",[](std::stack<WofValue>& S){
      if(S.size()<5) throw std::runtime_error("julia: need zr zi cr ci max_iter");
      auto m=S.top(); S.pop(); auto ci=S.top(); S.pop(); auto cr=S.top(); S.pop(); auto zi=S.top(); S.pop(); auto zr=S.top(); S.pop();
      int it = julia_iters(need_num(zr,"julia"), need_num(zi,"julia"), need_num(cr,"julia"), need_num(ci,"julia"), (int)need_num(m,"julia"));
      S.push(WofValue((double)it));
    });
  }
};
} // namespace woflang
WOFLANG_PLUGIN_EXPORT void register_plugin(woflang::WoflangInterpreter& interp){ static woflang::FractalOpsPlugin p; p.register_ops(interp); }
EOF
echo "[fix] wrote plugins/fractal_ops.cpp"

# --- 4) replace plugins/CMakeLists.txt to:
#         - build *_ops.cpp only
#         - filter out legacy sources that still use WofType/.type/.value
#         - put DLLs in build/bin/Plugins as lib<name>_ops.dll
cat > "$ROOT/plugins/CMakeLists.txt" << 'EOF'
cmake_minimum_required(VERSION 3.16)
file(GLOB ALL_PLUGIN_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*_ops.cpp")

# Filter out sources containing legacy field access
set(PLUGIN_SRCS "")
foreach(SRC ${ALL_PLUGIN_SRCS})
  file(READ "${SRC}" SRC_TEXT)
  string(REGEX MATCH "(WofType|\\.type|\\.value)" HAS_LEGACY "${SRC_TEXT}")
  if(HAS_LEGACY)
    message(STATUS "Skipping legacy plugin (needs port): ${SRC}")
  else()
    list(APPEND PLUGIN_SRCS "${SRC}")
  endif()
endforeach()

set(_PLUGIN_OUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Plugins")

foreach(plugin_src ${PLUGIN_SRCS})
  get_filename_component(base "${plugin_src}" NAME_WE)   # e.g. trig_ops
  add_library(${base} SHARED "${plugin_src}")
  target_include_directories(${base} PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/plugins
  )
  target_link_libraries(${base} PRIVATE woflang_core)
  # Always include compatibility shim
  target_compile_options(${base} PRIVATE -include "${CMAKE_SOURCE_DIR}/core/woflang_compat.hpp")
  set_target_properties(${base} PROPERTIES
    OUTPUT_NAME "lib${base}"
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${_PLUGIN_OUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${_PLUGIN_OUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${_PLUGIN_OUT_DIR}"
  )
endforeach()
EOF
echo "[fix] wrote plugins/CMakeLists.txt"

# --- 5) build with MinGW (assumes you're in MSYS2 MinGW64 shell)
rm -rf "$ROOT/build"
cmake -S "$ROOT" -B "$ROOT/build" -G "Ninja" -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=/mingw64/bin/cc.exe \
  -DCMAKE_CXX_COMPILER=/mingw64/bin/c++.exe
cmake --build "$ROOT/build" -j

echo "[fix] produced plugins:"
ls -1 "$ROOT/build/bin/Plugins" || true
echo "[fix] done."
