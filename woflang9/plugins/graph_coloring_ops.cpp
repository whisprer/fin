// plugins/graph_coloring_ops.cpp
#include <iostream>
#include <vector>

#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {

namespace woflang {

std::vector<int> greedyGraphColoring(const std::vector<std::vector<int>>& graph) {
    int n=graph.size(); std::vector<int> res(n,-1); std::vector<bool> avail(n,false);
    res[0]=0;
    for(int u=1;u<n;++u){
        for(int v:graph[u]) if(res[v]!=-1) avail[res[v]]=true;
        int c; for(c=0;c<n;++c) if(!avail[c]) break;
        res[u]=c;
        for(int v:graph[u]) if(res[v]!=-1) avail[res[v]]=false;
    }
    return res;
}

