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
#include <stack>
#include <stdexcept>

namespace woflang {
static inline double need_num(const WofValue& v,const char* op){
    if(!v.is_numeric()) throw std::runtime_error(std::string(op)+": numeric required");
    return v.as_numeric();
}
static inline double deg2rad(double d){ return d*3.141592653589793238462643383279502884/180.0; }
}

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* ops){
    using namespace woflang;
    if (!ops) return;

    (*ops)["rotate2d"] = [](std::stack<WofValue>& S){
        auto a=S.top(); S.pop(); auto y=S.top(); S.pop(); auto x=S.top(); S.pop();
        double ang=deg2rad(need_num(a,"rotate2d")), c=std::cos(ang), s=std::sin(ang);
        double xr = c*need_num(x,"rotate2d") - s*need_num(y,"rotate2d");
        double yr = s*need_num(x,"rotate2d") + c*need_num(y,"rotate2d");
        S.push(WofValue(xr)); S.push(WofValue(yr));
    };
    (*ops)["translate2d"] = [](std::stack<WofValue>& S){
        auto dy=S.top(); S.pop(); auto dx=S.top(); S.pop(); auto y=S.top(); S.pop(); auto x=S.top(); S.pop();
        S.push(WofValue(need_num(x,"translate2d")+need_num(dx,"translate2d")));
        S.push(WofValue(need_num(y,"translate2d")+need_num(dy,"translate2d")));
    };
    (*ops)["scale2d"] = [](std::stack<WofValue>& S){
        auto sy=S.top(); S.pop(); auto sx=S.top(); S.pop(); auto y=S.top(); S.pop(); auto x=S.top(); S.pop();
        S.push(WofValue(need_num(x,"scale2d")*need_num(sx,"scale2d")));
        S.push(WofValue(need_num(y,"scale2d")*need_num(sy,"scale2d")));
    };
    (*ops)["reflect_x"] = [](std::stack<WofValue>& S){
        auto y=S.top(); S.pop(); auto x=S.top(); S.pop();
        S.push(WofValue(need_num(x,"reflect_x"))); S.push(WofValue(-need_num(y,"reflect_x")));
    };
    (*ops)["reflect_y"] = [](std::stack<WofValue>& S){
        auto y=S.top(); S.pop(); auto x=S.top(); S.pop();
        S.push(WofValue(-need_num(x,"reflect_y"))); S.push(WofValue(need_num(y,"reflect_y")));
    };
}
