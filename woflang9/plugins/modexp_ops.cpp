// plugins/modexp_ops.cpp
#include <iostream>

#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {

namespace woflang {

long long modexp(long long base,long long exp,long long mod){
    long long res=1; base%=mod;
    while(exp>0){
        if(exp&1) res=(res*base)%mod;
        exp>>=1;
        base=(base*base)%mod;
    }
    return res;
}
