// plugins/io_debug_ops.cpp — print ops with flushing
#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

#include "core/woflang.hpp"
#include <iostream>
#include <stack>
#include <vector>
#include <string>

static std::string wof_to_string(const woflang::WofValue& v) {
    try { return v.to_string(); } catch (...) {}
    if (v.is_numeric()) return std::to_string(v.as_numeric());
    return "<val>";
}

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* ops){
    if (!ops) return;

    (*ops)["."] = [](std::stack<woflang::WofValue>& S){
        if (S.empty()) throw std::runtime_error(". : stack underflow");
        auto v = S.top(); S.pop();
        std::cout << wof_to_string(v) << std::endl; // flush
    };
    (*ops)["print"] = [](std::stack<woflang::WofValue>& S){
        if (S.empty()) throw std::runtime_error("print: stack underflow");
        std::cout << wof_to_string(S.top()) << std::endl; // flush
    };
    (*ops)["stack_dump"] = [](std::stack<woflang::WofValue>& S){
        auto copy = S;
        std::vector<woflang::WofValue> buf;
        while (!copy.empty()) { buf.push_back(copy.top()); copy.pop(); }
        std::cout << "[top]\n";
        for (auto& v : buf) std::cout << "  " << wof_to_string(v) << "\n";
        std::cout << "[bottom]\n" << std::flush;
    };
}
