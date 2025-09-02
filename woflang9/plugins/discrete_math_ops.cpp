// plugins/discrete_math_ops.cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>

#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {

namespace woflang {

unsigned long long factorial(int n) {
    if (n < 0) throw std::runtime_error("factorial domain error");
    unsigned long long result = 1;
    for (int i = 2; i <= n; ++i) result *= i;
    return result;
}

unsigned long long permutations(int n, int r) {
    if (n < r) return 0;
    return factorial(n) / factorial(n - r);
}

unsigned long long combinations(int n, int r) {
    if (n < r) return 0;
    return factorial(n) / (factorial(r) * factorial(n - r));
}

std::vector<int> greedyGraphColoring(const std::vector<std::vector<int>>& graph) {
    int n = graph.size();
    std::vector<int> result(n, -1);
    std::vector<bool> available(n, false);
    result[0] = 0;
    for (int u = 1; u < n; ++u) {
        for (int v : graph[u]) if (result[v] != -1) available[result[v]] = true;
        int c;
        for (c = 0; c < n; ++c) if (!available[c]) break;
        result[u] = c;
        for (int v : graph[u]) if (result[v] != -1) available[result[v]] = false;
    }
    return result;
}
