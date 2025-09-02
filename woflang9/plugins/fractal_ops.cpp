// plugins/fractal_ops.cpp
#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

#include "../../src/core/woflang.hpp"
#include <cmath>
#include <stack>
#include <stdexcept>

namespace woflang {
static int mandelbrot_iters(double cr,double ci,int max_iter){
    double zr=0,zi=0; int i=0;
    while(i<max_iter){
        double zr2=zr*zr-zi*zi+cr;
        double zi2=2*zr*zi+ci;
        zr=zr2; zi=zi2;
        if(zr*zr+zi*zi>4.0) break;
        ++i;
    }
    return i;
}
static int julia_iters(double zr,double zi,double cr,double ci,int max_iter){
    int i=0;
    while(i<max_iter){
        double zr2=zr*zr-zi*zi+cr;
        double zi2=2*zr*zi+ci;
        zr=zr2; zi=zi2;
        if(zr*zr+zi*zi>4.0) break;
        ++i;
    }
    return i;
}
static inline double need_num(const WofValue& v,const char* op){
    return v.d;  // Use direct field access instead of method calls
}

// Sierpinski triangle check
static bool sierpinski_triangle(int x, int y) {
    return (x & y) == 0;
}

// Sierpinski carpet/square check  
static bool sierpinski_carpet(int x, int y, int level) {
    for (int i = 0; i < level; i++) {
        if ((x % 3 == 1) && (y % 3 == 1)) {
            return false;
        }
        x /= 3;
        y /= 3;
    }
    return true;
}

// Box-counting dimension approximation
static double hausdorff_dimension(double scale, double count) {
    if (scale <= 1.0 || count <= 1.0) return 1.0;
    return log(count) / log(1.0 / scale);
}
}

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* ops){
    using namespace woflang;
    if (!ops) return;

    (*ops)["mandelbrot"] = [](std::stack<WofValue>& S){
        if (S.size() < 3) { 
            std::cerr << "mandelbrot: need 3 values (real imag max_iter)\n";
            return;
        }
        auto m=S.top(); S.pop(); auto ci=S.top(); S.pop(); auto cr=S.top(); S.pop();
        int it = mandelbrot_iters(need_num(cr,"mandelbrot"), need_num(ci,"mandelbrot"), (int)need_num(m,"mandelbrot"));
        WofValue result;
        result.d = (double)it;
        S.push(result);
    };

    (*ops)["julia"] = [](std::stack<WofValue>& S){
        if (S.size() < 5) {
            std::cerr << "julia: need 5 values (zr zi cr ci max_iter)\n";
            return;
        }
        auto m=S.top(); S.pop(); auto ci=S.top(); S.pop(); auto cr=S.top(); S.pop(); auto zi=S.top(); S.pop(); auto zr=S.top(); S.pop();
        int it = julia_iters(need_num(zr,"julia"), need_num(zi,"julia"), need_num(cr,"julia"), need_num(ci,"julia"), (int)need_num(m,"julia"));
        WofValue result;
        result.d = (double)it;
        S.push(result);
    };

    (*ops)["sierpinski"] = [](std::stack<WofValue>& S){
        if (S.size() < 2) {
            std::cerr << "sierpinski: need 2 values (x y)\n";
            return;
        }
        auto y=S.top(); S.pop(); auto x=S.top(); S.pop();
        bool in_triangle = sierpinski_triangle((int)need_num(x,"sierpinski"), (int)need_num(y,"sierpinski"));
        WofValue result;
        result.d = in_triangle ? 1.0 : 0.0;
        S.push(result);
    };

    (*ops)["menger_square"] = [](std::stack<WofValue>& S){
        if (S.size() < 3) {
            std::cerr << "menger_square: need 3 values (x y level)\n";
            return;
        }
        auto level=S.top(); S.pop(); auto y=S.top(); S.pop(); auto x=S.top(); S.pop();
        bool filled = sierpinski_carpet((int)need_num(x,"menger_square"), (int)need_num(y,"menger_square"), (int)need_num(level,"menger_square"));
        WofValue result;
        result.d = filled ? 1.0 : 0.0;
        S.push(result);
    };

    (*ops)["hausdorff"] = [](std::stack<WofValue>& S){
        if (S.size() < 2) {
            std::cerr << "hausdorff: need 2 values (scale count)\n";
            return;
        }
        auto count=S.top(); S.pop(); auto scale=S.top(); S.pop();
        double dimension = hausdorff_dimension(need_num(scale,"hausdorff"), need_num(count,"hausdorff"));
        WofValue result;
        result.d = dimension;
        S.push(result);
    };
}