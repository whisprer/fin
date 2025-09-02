// plugins/graph_search_ops.cpp
#include <iostream>
#include <vector>
#include <stack>
#include <queue>

#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {

namespace woflang {

void depthFirstSearch(const std::vector<std::vector<int>>& adjList, int start) {
    int n = adjList.size();
    std::vector<bool> visited(n,false);
    std::stack<int> s;
    s.push(start);
    while (!s.empty()) {
        int u = s.top(); s.pop();
        if (!visited[u]) {
            std::cout << u << " ";
            visited[u] = true;
        }
        for (int v : adjList[u]) if (!visited[v]) s.push(v);
    }
}

void breadthFirstSearch(const std::vector<std::vector<int>>& adjList, int start) {
    int n = adjList.size();
    std::vector<bool> visited(n,false);
    std::queue<int> q;
    visited[start] = true; q.push(start);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        std::cout << u << " ";
        for (int v : adjList[u]) if (!visited[v]) { visited[v]=true; q.push(v); }
    }
}

