#include "analog_interface.hpp"
#include "analog_state.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <cmath>

namespace woflang {

inline void command_loop() {
    std::string line;

    while (true) {
        std::cout << "woflang> ";
        if (!std::getline(std::cin, line)) break;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "exit" || cmd == "quit") {
            break;
        }
        else if (cmd == "analog_status") {
            std::cout << get_analog_status() << std::endl;
        }
        else if (cmd == "analog_mode") {
            std::string mode_str;
            float min = -1.0f, max = 1.0f;
            iss >> mode_str;
            if (mode_str == "float_custom") iss >> min >> max;

            else if (cmd == "analog_deadzone") {
                float value, threshold;
                iss >> value >> threshold;
                if (threshold < 0.0f) threshold = 0.0f;
                float clamped = analog_state.clamp(value);
                float output = (std::fabs(clamped) < threshold) ? 0.0f : clamped;
                std::cout << "Deadzone Applied: " << output << std::endl;
            }
            else if (cmd == "analog_scalequery") {
                float range = analog_state.analog_max() - analog_state.analog_min();
                std::cout << "Analog Range Span: " << range << std::endl;
            }
            else if (cmd == "analog_isint") {
                bool is_int = analog_state.is_integer_mode();
                std::cout << "Integer Mode: " << (is_int ? "Yes" : "No") << std::endl;
            }

            try {
                set_analog_mode_from_string(mode_str, min, max);
                std::cout << "Analog mode updated.\n";
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }
        else if (cmd == "analog_clamp") {
            float value;
            iss >> value;
            float clamped = analog_state.clamp(value);
            std::cout << "Clamped Value: " << clamped << std::endl;
        }
        else if (cmd == "analog_range") {
            std::cout << "Min: " << analog_state.analog_min() << "\n";
            std::cout << "Max: " << analog_state.analog_max() << "\n";
        }
        else if (cmd == "analog_remap") {
            float val, from_min, from_max, to_min, to_max;
            iss >> val >> from_min >> from_max >> to_min >> to_max;
            if (from_max - from_min == 0.0f) {
                std::cerr << "Error: Invalid source range.\n";
                continue;
            }
            float t = (val - from_min) / (from_max - from_min);
            float remapped = to_min + t * (to_max - to_min);
            remapped = analog_state.clamp(remapped);
            std::cout << "Remapped Value: " << remapped << std::endl;
        }
        else {
            std::cout << "Unknown command.\n";
        }
    }
}

} // namespace woflang
