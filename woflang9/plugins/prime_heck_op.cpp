// ==================================================
// pime_heck_op.cpp - A Typo-Summoned Easter Egg
// ==================================================
#include "../../src/core/woflang.hpp"
#include <iostream>

#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {
    (*op_table)["pime_heck"] = [](std::stack<woflang::WofValue>& stack) {
        // Using a C++ raw string literal (R"()") makes embedding ASCII art
        // clean and easy, as no characters need to be escaped.
        const char* art = R"(
          .--.
         |o_o |
         |:_/ |
        //   \ \
       (|     | )
      /'\_   _/`\
      \___)=(___/

A typo, or a summons from the deep?
You typed 'pime_heck' instead of 'prime_check'.
)";

        std::cout << art << std::endl;

        // A chaotic operation deserves a chaotic result.
        // Let's clear the stack.
        if (!stack.empty()) {
            std::cout << "The stack has been dragged to heck.\n";
            while(!stack.empty()) {
                stack.pop();
            }
        }
    };
}

