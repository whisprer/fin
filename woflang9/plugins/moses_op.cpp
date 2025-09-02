// ==================================================
// moses_op.cpp - A Mystical Riddle Plugin (C++23 Fixed)
// ==================================================
#include "../../src/core/woflang.hpp"
#include <iostream>
#include <string>
#include <random>
#include <chrono>
#include <thread>

// To correctly handle UTF-8 literals for Hebrew characters.
#if defined(_MSC_VER)
#include <windows.h>
#endif

extern "C" {

// A static flag to track if the Hebrew mode has been triggered.
// This state will persist for the entire session once activated.
static bool hebrew_mode_active = false;

// A function to set up the console for UTF-8 output, which is crucial for Hebrew.
void setup_utf8_console() {
#if defined(_MSC_VER)
    // On Windows, we need to explicitly set the console to UTF-8 mode.
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    // On Linux/macOS, the terminal is generally expected to be UTF-8 compliant.
    // We can ensure the C++ locale is set, though it's often not needed.
    std::setlocale(LC_ALL, "en_US.UTF-8");
}


#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {
    // This is a special command that has a chance to trigger the riddle.
    (*op_table)["那"] = [](std::stack<woflang::WofValue>& stack) {
        (void)stack; // Suppress unused parameter warning
        
        // Set up console on first run.
        static bool first_run = true;
        if (first_run) {
            setup_utf8_console();
            first_run = false;
        }

        // Use a static random generator so it's seeded only once.
        static std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<> dis(1, 100); // Approx 1 in 100 chance.

        // Check if the event should trigger (only triggers once).
        if (!hebrew_mode_active && dis(gen) == 1) {
            hebrew_mode_active = true;

            std::cout << "\n那... How does Moses make his tea?\n";
            std::cout.flush();
            std::this_thread::sleep_for(std::chrono::seconds(3));
            
            // The prompt changes to Hebrew, indicating the switch.
            // Note: Your terminal must support RTL rendering for this to look right.
            std::cout << "\n...העולם השתנה\n";
            std::cout << "(The world has changed... type 'answer' to respond)\n";

        } else if (hebrew_mode_active) {
             // If mode is active, this command now prints a Hebrew proverb.
            std::cout << "אם אין אני לי, מי לי? וכשאני לעצמי, מה אני? ואם לא עכשיו, אימתי?" << std::endl;
            std::cout << "(If I am not for myself, who will be for me? And when I am for myself, what am 'I'? And if not now, when?)\n";
        
        } else {
            // If not triggered, it gives a cryptic hint.
            std::cout << "The tablets are yet unbroken.\n";
        }
    };

    // The command to provide the answer to the riddle.
    (*op_table)["answer"] = [](std::stack<woflang::WofValue>& stack) {
        (void)stack; // Suppress unused parameter warning
        
        if (hebrew_mode_active) {
            std::cout << "\nHe brews it.\n";
            // Use regular string literal instead of u8 prefix
            std::cout << "הוא מכין תה... (He brews it.)\n\n";
        } else {
            std::cout << "There is no riddle to answer.\n";
        }
    };

    // A command to reset the state back to normal.
    (*op_table)["reset"] = [](std::stack<woflang::WofValue>& stack) {
        (void)stack; // Suppress unused parameter warning
        
        if (hebrew_mode_active) {
            hebrew_mode_active = false;
            std::cout << "The world returns to its former shape.\n";
        } else {
            std::cout << "Everything is already as it should be.\n";
        }
    };
}