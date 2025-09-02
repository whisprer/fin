// plugins/gradient_hessian_ops.cpp
#include <iostream>
#include <vector>
#include <functional>
#include <cmath>

namespace woflang {

#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {

std::vector<double> computeGradient(const std::function<double(const std::vector<double>&)>& f,
                                    const std::vector<double>& x,double h=1e-5){
    int n=x.size(); std::vector<double> g(n);
    for(int i=0;i<n;++i){
        std::vector<double> xf=x, xb=x;
        xf[i]+=h; xb[i]-=h;
        g[i]=(f(xf)-f(xb))/(2*h);
    }
    return g;
}

std::vector<std::vector<double>> computeHessian(const std::function<double(const std::vector<double>&)>& f,
                                                const std::vector<double>& x,double h=1e-5){
    int n=x.size(); std::vector<std::vector<double>> H(n,std::vector<double>(n,0.0));
    for(int i=0;i<n;++i){
        for(int j=0;j<n;++j){
            std::vector<double> xfi=x, xbi=x, xfj=x, xbj=x, xfij=x, xbij=x;
            if(i==j){
                xfi[i]+=h; xbi[i]-=h;
                H[i][j]=(f(xfi)-2*f(x)+f(xbi))/(h*h);
            }else{
                xfij[i]+=h; xfij[j]+=h;
                xbij[i]-=h; xbij[j]-=h;
                xfi[i]+=h; xbi[i]-=h;
                xfj[j]+=h; xbj[j]-=h;
                H[i][j]=(f(xfij)-f(xfi)-f(xfj)+f(x))/(h*h);
            }
        }
    }
    return H;
}


