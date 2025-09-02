// plugins/numeric_diff_ops.cpp
#include <iostream>
#include <vector>

#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {#include <cmath>

namespace woflang {

double forward(const std::vector<double>& x,const std::vector<double>& y,int i){
    double h=x[i+1]-x[i]; return (y[i+1]-y[i])/h;
}
double backward(const std::vector<double>& x,const std::vector<double>& y,int i){
    double h=x[i]-x[i-1]; return (y[i]-y[i-1])/h;
}
double central(const std::vector<double>& x,const std::vector<double>& y,int i){
    double h=x[i+1]-x[i-1]; return (y[i+1]-y[i-1])/h;
}

