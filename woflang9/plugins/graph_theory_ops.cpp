// plugins/graph_theory_ops.cpp
#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <tuple>
#include <algorithm>

#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {

namespace woflang {

using Edge=std::tuple<int,int,double>;

std::vector<double> dijkstra(const std::vector<std::vector<std::pair<int,double>>>& adj,int s){
    int n=adj.size(); std::vector<double> dist(n,std::numeric_limits<double>::infinity());
    dist[s]=0; using P=std::pair<double,int>;
    std::priority_queue<P,std::vector<P>,std::greater<P>> pq;
    pq.push({0,s});
    while(!pq.empty()){
        auto [d,u]=pq.top(); pq.pop();
        if(d>dist[u]) continue;
        for(auto [v,w]:adj[u]) if(dist[u]+w<dist[v]){ dist[v]=dist[u]+w; pq.push({dist[v],v}); }
    }
    return dist;
}

std::vector<double> bellmanFord(const std::vector<Edge>& edges,int n,int s){
    std::vector<double> dist(n,std::numeric_limits<double>::infinity());
    dist[s]=0;
    for(int i=1;i<n;++i) for(auto [u,v,w]:edges) if(dist[u]!=std::numeric_limits<double>::infinity() && dist[u]+w<dist[v]) dist[v]=dist[u]+w;
    for(auto [u,v,w]:edges) if(dist[u]!=std::numeric_limits<double>::infinity() && dist[u]+w<dist[v]) throw std::runtime_error("neg cycle");
    return dist;
}


