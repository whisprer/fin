// plugins/numerica_differentianion.cpp
#include <iostream>
#include <vector>
#include <cmath>
#include <string>

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {

namespace woflang {

double forwardDifference(const std::vector<double>& x,const std::vector<double>& y,int i){
    double h=x[i+1]-x[i]; return (y[i+1]-y[i])/h;
}
double backwardDifference(const std::vector<double>& x,const std::vector<double>& y,int i){
    double h=x[i]-x[i-1]; return (y[i]-y[i-1])/h;
}
double centralDifference(const std::vector<double>& x,const std::vector<double>& y,int i){
    double h=x[i+1]-x[i-1]; return (y[i+1]-y[i-1])/h;
}

void computeDerivatives(const std::vector<double>& x,const std::vector<double>& y,std::vector<double>& deriv,const std::string& method="central"){
    deriv.resize(x.size());
    for(int i=0;i<x.size();++i){
        if(method=="forward"){
            if(i < x.size()-1) deriv[i]=forwardDifference(x,y,i); else deriv[i]=NAN;
        } else if(method=="backward"){
            if(i>0) deriv[i]=backwardDifference(x,y,i); else deriv[i]=NAN;
        } else {
            if(i>0 && i<x.size()-1) deriv[i]=centralDifference(x,y,i);
            else if(i==0) deriv[i]=forwardDifference(x,y,i);
            else deriv[i]=backwardDifference(x,y,i);
        }
    }
}

