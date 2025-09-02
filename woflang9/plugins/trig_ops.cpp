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
#include <stack>
#include <stdexcept>

namespace woflang {
static inline double need_num(const WofValue& v, const char* op){
    if (!v.is_numeric()) throw std::runtime_error(std::string(op)+": numeric required");
    return v.as_numeric();
}
static inline double deg2rad(double d){ return d*std::numbers::pi_v<double>/180.0; }
static inline double rad2deg(double r){ return r*180.0/std::numbers::pi_v<double>; }
}

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* ops){
    using namespace woflang;
    if (!ops) return;

    (*ops)["sin"]  = [](std::stack<WofValue>& S){ auto a=S.top(); S.pop(); S.push(WofValue(std::sin(need_num(a,"sin")))); };
    (*ops)["cos"]  = [](std::stack<WofValue>& S){ auto a=S.top(); S.pop(); S.push(WofValue(std::cos(need_num(a,"cos")))); };
    (*ops)["tan"]  = [](std::stack<WofValue>& S){ auto a=S.top(); S.pop(); S.push(WofValue(std::tan(need_num(a,"tan")))); };
    (*ops)["asin"] = [](std::stack<WofValue>& S){ auto a=S.top(); S.pop(); double v=need_num(a,"asin"); if(v<-1||v>1) throw std::runtime_error("asin: domain"); S.push(WofValue(std::asin(v))); };
    (*ops)["acos"] = [](std::stack<WofValue>& S){ auto a=S.top(); S.pop(); double v=need_num(a,"acos"); if(v<-1||v>1) throw std::runtime_error("acos: domain"); S.push(WofValue(std::acos(v))); };
    (*ops)["atan"] = [](std::stack<WofValue>& S){ auto a=S.top(); S.pop(); S.push(WofValue(std::atan(need_num(a,"atan")))); };
    (*ops)["atan2"]= [](std::stack<WofValue>& S){ auto x=S.top(); S.pop(); auto y=S.top(); S.pop(); S.push(WofValue(std::atan2(need_num(y,"atan2"),need_num(x,"atan2")))); };
    (*ops)["sinh"] = [](std::stack<WofValue>& S){ auto a=S.top(); S.pop(); S.push(WofValue(std::sinh(need_num(a,"sinh")))); };
    (*ops)["cosh"] = [](std::stack<WofValue>& S){ auto a=S.top(); S.pop(); S.push(WofValue(std::cosh(need_num(a,"cosh")))); };
    (*ops)["tanh"] = [](std::stack<WofValue>& S){ auto a=S.top(); S.pop(); S.push(WofValue(std::tanh(need_num(a,"tanh")))); };
    (*ops)["deg2rad"]= [](std::stack<WofValue>& S){ auto d=S.top(); S.pop(); S.push(WofValue(deg2rad(need_num(d,"deg2rad")))); };
    (*ops)["rad2deg"]= [](std::stack<WofValue>& S){ auto r=S.top(); S.pop(); S.push(WofValue(rad2deg(need_num(r,"rad2deg")))); };
}
