#pragma once

#include <cstdint>
#include <stdexcept>
#include <cmath>

namespace woflang {

// --- Analog Modes Enum ---

enum class AnalogMode : uint8_t {
    INT_201,         // -100 to +100
    INT_2001,        // -1000 to +1000
    FLOAT_UNIT,      // -1.0 to +1.0
    FLOAT_CUSTOM     // user-defined
};

// --- Analog State Singleton ---

struct AnalogState {
    AnalogMode mode = AnalogMode::INT_201;
    float custom_min = -1.0f;
    float custom_max = 1.0f;

    float analog_min() const {
        switch (mode) {
            case AnalogMode::INT_201: return -100.0f;
            case AnalogMode::INT_2001: return -1000.0f;
            case AnalogMode::FLOAT_UNIT: return -1.0f;
            case AnalogMode::FLOAT_CUSTOM: return custom_min;
        }
        throw std::runtime_error("Unknown analog mode");
    }

    float analog_max() const {
        switch (mode) {
            case AnalogMode::INT_201: return 100.0f;
            case AnalogMode::INT_2001: return 1000.0f;
            case AnalogMode::FLOAT_UNIT: return 1.0f;
            case AnalogMode::FLOAT_CUSTOM: return custom_max;
        }
        throw std::runtime_error("Unknown analog mode");
    }

    void set_mode(AnalogMode new_mode, float min_val = -1.0f, float max_val = 1.0f) {
        mode = new_mode;
        if (mode == AnalogMode::FLOAT_CUSTOM) {
            if (min_val >= max_val) throw std::invalid_argument("Invalid custom analog range");
            custom_min = min_val;
            custom_max = max_val;
        }
    }

    bool is_integer_mode() const {
        return (mode == AnalogMode::INT_201) || (mode == AnalogMode::INT_2001);
    }

    float clamp(float value) const {
        float min_val = analog_min();
        float max_val = analog_max();
        return fmaxf(min_val, fminf(value, max_val));
    }
};

// --- Global Analog State Instance ---

inline AnalogState analog_state;

} // namespace woflang
