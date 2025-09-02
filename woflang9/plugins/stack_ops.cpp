// plugins/stack_ops.cpp (benchmark helper)
#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

#include "core/woflang.hpp"
#include <stack>

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* ops){
    if (!ops) return;
    // Benchmark calls this by name; it’s a no-op.
    (*ops)["stack_slayer"] = [](std::stack<woflang::WofValue>&){ /* no-op */ };
}
