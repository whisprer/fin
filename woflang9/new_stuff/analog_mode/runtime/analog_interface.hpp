#pragma once

#include "analog_state.hpp"
#include <string>
#include <sstream>
#include <stdexcept>

namespace woflang {

// --- Parse Analog Mode from String ---

inline AnalogMode parse_analog_mode(const std::string& str) {
    if (str == "int201") return AnalogMode::INT_201;
    if (str == "int2001") return AnalogMode::INT_2001;
    if (str == "float_unit") return AnalogMode::FLOAT_UNIT;
    if (str == "float_custom") return AnalogMode::FLOAT_CUSTOM;
    throw std::invalid_argument("Unknown analog mode: " + str);
}

// --- Command: Get Analog Settings as String ---

inline std::string get_analog_status() {
    std::ostringstream out;
    out << "Analog Mode: ";

    switch (analog_state.mode) {
        case AnalogMode::INT_201: out << "int201"; break;
        case AnalogMode::INT_2001: out << "int2001"; break;
        case AnalogMode::FLOAT_UNIT: out << "float_unit"; break;
        case AnalogMode::FLOAT_CUSTOM:
            out << "float_custom (" << analog_state.custom_min << " to " << analog_state.custom_max << ")";
            break;
    }
    out << "\nMin: " << analog_state.analog_min();
    out << "\nMax: " << analog_state.analog_max();
    return out.str();
}

// --- Command: Set Analog Mode from String Input ---

inline void set_analog_mode_from_string(const std::string& mode_str, float min_val = -1.0f, float max_val = 1.0f) {
    AnalogMode mode = parse_analog_mode(mode_str);
    analog_state.set_mode(mode, min_val, max_val);
}

} // namespace woflang
