// ==================================================
// FIXED: music.cpp - Simple extern C style
// ==================================================
#include "core/woflang.hpp"
#include <iostream>
#include <cmath>
#include <map>

extern "C" {

#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {
    // Note frequencies (A4 = 440Hz) - using ASCII names
    std::map<std::string, double> notes = {
        {"C", 261.63}, {"Cs", 277.18}, {"D", 293.66}, {"Ds", 311.13},
        {"E", 329.63}, {"F", 349.23}, {"Fs", 369.99}, {"G", 392.00},
        {"Gs", 415.30}, {"A", 440.00}, {"As", 466.16}, {"B", 493.88}
    };
    
    // Register note operations
    for (const auto& [note, freq] : notes) {
        (*op_table)[note] = [freq](std::stack<woflang::WofValue>& stack) {
            woflang::WofValue val;
            val.d = freq;
            stack.push(val);
            std::cout << "♪ " << freq << " Hz\n";
        };
    }
    
    // Chord operations
    (*op_table)["major"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "major: needs a root note frequency\n";
            return;
        }
        
        auto root = stack.top(); stack.pop();
        double root_freq = root.as_numeric();
        
        // Major chord: root, major third, perfect fifth
        woflang::WofValue v1, v2, v3;
        v1.d = root_freq;
        v2.d = root_freq * 1.25;    // Major third
        v3.d = root_freq * 1.5;     // Perfect fifth
        
        stack.push(v1);
        stack.push(v2);
        stack.push(v3);
        
        std::cout << "♫ Major chord: " << root_freq << " Hz\n";
    };
    
    (*op_table)["bpm"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "bpm: needs a tempo value\n";
            return;
        }
        
        auto tempo = stack.top(); stack.pop();
        double bpm = tempo.as_numeric();
        double beat_duration = 60.0 / bpm;
        
        std::cout << "Tempo: " << bpm << " BPM (beat = " 
                 << beat_duration << " seconds)\n";
        
        woflang::WofValue result;
        result.d = beat_duration;
        stack.push(result);
    };
}
