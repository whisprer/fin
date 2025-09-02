#pragma once

#include "analog_modes.hpp"
#include "analog_math.hpp"
#include "analog_math_extended.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <regex>
#include <sstream>

namespace woflang {

// --- Modelica Unit Types ---
enum class ModelicaUnit {
    Resistance,      // SI.Resistance (Ohm)
    Conductance,     // SI.Conductance (Siemens)
    Capacitance,     // SI.Capacitance (Farad)
    Inductance,      // SI.Inductance (Henry)
    Voltage,         // SI.Voltage (Volt)
    Current,         // SI.Current (Ampere)
    Temperature,     // SI.Temperature (Kelvin)
    Frequency,       // SI.Frequency (Hz)
    Dimensionless    // Pure numbers
};

// --- Modelica Parameter Definition ---
struct ModelicaParameter {
    std::string name;
    ModelicaUnit unit;
    float default_value;
    float start_value;
    std::string description;
    bool is_temperature_dependent;
    
    ModelicaParameter(const std::string& n, ModelicaUnit u, float def_val = 0.0f, 
                      float start_val = 0.0f, const std::string& desc = "",
                      bool temp_dep = false)
        : name(n), unit(u), default_value(def_val), start_value(start_val),
          description(desc), is_temperature_dependent(temp_dep) {}
};

// --- Modelica Equation Types ---
enum class EquationType {
    Algebraic,       // v = R*i
    Differential,    // i = C*der(v)
    Conditional,     // if-then-else
    Assert          // assert(condition, "message")
};

// --- Modelica Equation ---
struct ModelicaEquation {
    EquationType type;
    std::string lhs;        // Left-hand side
    std::string rhs;        // Right-hand side
    std::string condition;  // For conditional equations
    
    ModelicaEquation(EquationType t, const std::string& left, const std::string& right,
                     const std::string& cond = "")
        : type(t), lhs(left), rhs(right), condition(cond) {}
};

// --- Parsed Modelica Component ---
struct ModelicaComponent {
    std::string name;
    std::string path;                              // within X.Y.Z
    std::string description;
    std::vector<ModelicaParameter> parameters;
    std::vector<ModelicaEquation> equations;
    std::vector<std::string> extends;              // Base classes
    std::unordered_map<std::string, std::string> variables; // name -> type
    bool has_heat_port;
    bool is_one_port;
    
    ModelicaComponent(const std::string& n = "") 
        : name(n), has_heat_port(false), is_one_port(false) {}
};

// --- Modelica Parser ---
class ModelicaParser {
private:
    std::unordered_map<std::string, ModelicaUnit> unit_map;
    
public:
    ModelicaParser() {
        // Initialize unit mapping
        unit_map["SI.Resistance"] = ModelicaUnit::Resistance;
        unit_map["SI.Conductance"] = ModelicaUnit::Conductance;
        unit_map["SI.Capacitance"] = ModelicaUnit::Capacitance;
        unit_map["SI.Inductance"] = ModelicaUnit::Inductance;
        unit_map["SI.Voltage"] = ModelicaUnit::Voltage;
        unit_map["SI.Current"] = ModelicaUnit::Current;
        unit_map["SI.Temperature"] = ModelicaUnit::Temperature;
        unit_map["SI.Frequency"] = ModelicaUnit::Frequency;
        unit_map["SI.LinearTemperatureCoefficient"] = ModelicaUnit::Dimensionless;
    }
    
    ModelicaComponent parse_component(const std::string& modelica_code) {
        ModelicaComponent component;
        
        std::istringstream stream(modelica_code);
        std::string line;
        bool in_equation_section = false;
        bool in_parameter_section = false;
        
        while (std::getline(stream, line)) {
            line = trim(line);
            if (line.empty() || line.starts_with("//")) continue;
            
            // Parse component name and path
            if (line.starts_with("within ")) {
                component.path = extract_path(line);
            }
            else if (line.starts_with("model ")) {
                auto parts = extract_model_declaration(line);
                component.name = parts.first;
                component.description = parts.second;
            }
            // Parse extends
            else if (line.starts_with("extends ")) {
                component.extends.push_back(extract_extends(line));
                if (line.find("OnePort") != std::string::npos) {
                    component.is_one_port = true;
                }
                if (line.find("ConditionalHeatPort") != std::string::npos) {
                    component.has_heat_port = true;
                }
            }
            // Parse parameters
            else if (line.starts_with("parameter ")) {
                auto param = parse_parameter(line);
                if (param.name != "") {
                    component.parameters.push_back(param);
                }
            }
            // Parse variable declarations
            else if (is_variable_declaration(line)) {
                auto var = parse_variable_declaration(line);
                component.variables[var.first] = var.second;
            }
            // Parse equations section
            else if (line == "equation") {
                in_equation_section = true;
            }
            else if (in_equation_section && !line.starts_with("annotation")) {
                auto equation = parse_equation(line);
                if (equation.lhs != "") {
                    component.equations.push_back(equation);
                }
            }
            else if (line.starts_with("end ")) {
                break; // End of component
            }
        }
        
        return component;
    }
    
private:
    std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }
    
    std::string extract_path(const std::string& line) {
        std::regex pattern(R"(within\s+([\w\.]+);)");
        std::smatch match;
        if (std::regex_search(line, match, pattern)) {
            return match[1].str();
        }
        return "";
    }
    
    std::pair<std::string, std::string> extract_model_declaration(const std::string& line) {
        std::regex pattern(R"(model\s+(\w+)\s*"([^"]*)")");
        std::smatch match;
        if (std::regex_search(line, match, pattern)) {
            return {match[1].str(), match[2].str()};
        }
        
        // Fallback for model without description
        std::regex simple_pattern(R"(model\s+(\w+))");
        if (std::regex_search(line, match, simple_pattern)) {
            return {match[1].str(), ""};
        }
        return {"", ""};
    }
    
    std::string extract_extends(const std::string& line) {
        std::regex pattern(R"(extends\s+([\w\.]+))");
        std::smatch match;
        if (std::regex_search(line, match, pattern)) {
            return match[1].str();
        }
        return "";
    }
    
    ModelicaParameter parse_parameter(const std::string& line) {
        // Example: parameter SI.Resistance R(start=1) "Resistance at temperature T_ref";
        std::regex pattern(R"(parameter\s+(SI\.\w+|\w+)\s+(\w+)(?:\(([^)]*)\))?\s*(?:"([^"]*)")?;?)");
        std::smatch match;
        
        if (std::regex_search(line, match, pattern)) {
            std::string unit_str = match[1].str();
            std::string name = match[2].str();
            std::string params = match[3].str();
            std::string description = match[4].str();
            
            ModelicaUnit unit = ModelicaUnit::Dimensionless;
            if (unit_map.find(unit_str) != unit_map.end()) {
                unit = unit_map[unit_str];
            }
            
            float start_value = 0.0f;
            float default_value = 0.0f;
            
            // Parse parameter attributes
            if (!params.empty()) {
                std::regex start_pattern(R"(start\s*=\s*([\d\.]+))");
                std::smatch start_match;
                if (std::regex_search(params, start_match, start_pattern)) {
                    start_value = std::stof(start_match[1].str());
                    default_value = start_value;
                }
            }
            
            bool temp_dependent = (name.find("alpha") != std::string::npos) ||
                                 (description.find("temperature") != std::string::npos);
            
            return ModelicaParameter(name, unit, default_value, start_value, description, temp_dependent);
        }
        
        return ModelicaParameter("", ModelicaUnit::Dimensionless);
    }
    
    bool is_variable_declaration(const std::string& line) {
        // Check for variable declarations like "SI.Resistance R_actual"
        return std::regex_search(line, std::regex(R"(SI\.\w+\s+\w+)")) && 
               !line.starts_with("parameter") &&
               line.find("=") == std::string::npos;
    }
    
    std::pair<std::string, std::string> parse_variable_declaration(const std::string& line) {
        std::regex pattern(R"((SI\.\w+|\w+)\s+(\w+))");
        std::smatch match;
        if (std::regex_search(line, match, pattern)) {
            return {match[2].str(), match[1].str()};
        }
        return {"", ""};
    }
    
    ModelicaEquation parse_equation(const std::string& line) {
        if (line.find("assert") != std::string::npos) {
            return parse_assert_equation(line);
        }
        else if (line.find("der(") != std::string::npos) {
            return parse_differential_equation(line);
        }
        else if (line.find("=") != std::string::npos) {
            return parse_algebraic_equation(line);
        }
        
        return ModelicaEquation(EquationType::Algebraic, "", "");
    }
    
    ModelicaEquation parse_algebraic_equation(const std::string& line) {
        size_t eq_pos = line.find("=");
        if (eq_pos != std::string::npos) {
            std::string lhs = trim(line.substr(0, eq_pos));
            std::string rhs = trim(line.substr(eq_pos + 1));
            
            // Remove semicolon
            if (rhs.back() == ';') {
                rhs.pop_back();
                rhs = trim(rhs);
            }
            
            return ModelicaEquation(EquationType::Algebraic, lhs, rhs);
        }
        return ModelicaEquation(EquationType::Algebraic, "", "");
    }
    
    ModelicaEquation parse_differential_equation(const std::string& line) {
        size_t eq_pos = line.find("=");
        if (eq_pos != std::string::npos) {
            std::string lhs = trim(line.substr(0, eq_pos));
            std::string rhs = trim(line.substr(eq_pos + 1));
            
            if (rhs.back() == ';') {
                rhs.pop_back();
                rhs = trim(rhs);
            }
            
            return ModelicaEquation(EquationType::Differential, lhs, rhs);
        }
        return ModelicaEquation(EquationType::Differential, "", "");
    }
    
    ModelicaEquation parse_assert_equation(const std::string& line) {
        // Extract assert condition and message
        std::regex pattern(R"(assert\s*\(\s*([^,]+),\s*"([^"]*)"\s*\))");
        std::smatch match;
        if (std::regex_search(line, match, pattern)) {
            return ModelicaEquation(EquationType::Assert, match[1].str(), match[2].str());
        }
        return ModelicaEquation(EquationType::Assert, "", "");
    }
};

// --- Woflang Circuit Primitive Generator ---
class ModelicaToWoflangGenerator {
public:
    std::string generate_circuit_primitive(const ModelicaComponent& component) {
        std::ostringstream code;
        
        // Generate class declaration
        code << "class " << component.name << "Primitive : public CircuitPrimitive {\n";
        code << "public:\n";
        code << "    " << component.name << "Primitive() : CircuitPrimitive(\"" << component.name << "\") {\n";
        
        // Generate parameter registration
        for (const auto& param : component.parameters) {
            code << "        add_parameter(\"" << param.name << "\", " 
                 << param.default_value << "f, " << get_unit_range(param.unit)
                 << ", " << (param.is_temperature_dependent ? "true" : "false") << ");\n";
        }
        
        code << "    }\n\n";
        
        // Generate transfer function (simplified for basic components)
        code << "    AnalogComplex transfer_function(float frequency) const override {\n";
        code << generate_transfer_function_body(component);
        code << "    }\n\n";
        
        // Generate SIMD version
        code << "    void transfer_function_simd(__m256 frequencies, __m256& magnitude_out, __m256& phase_out) const override {\n";
        code << generate_simd_transfer_function_body(component);
        code << "    }\n\n";
        
        // Generate component-specific methods
        code << generate_component_specific_methods(component);
        
        code << "};\n";
        
        return code.str();
    }
    
private:
    std::string get_unit_range(ModelicaUnit unit) {
        switch (unit) {
            case ModelicaUnit::Resistance:
                return "0.001f, 1e6f"; // 1mΩ to 1MΩ
            case ModelicaUnit::Conductance:
                return "1e-6f, 1000.0f"; // 1μS to 1kS
            case ModelicaUnit::Capacitance:
                return "1e-12f, 1e-3f"; // 1pF to 1mF
            case ModelicaUnit::Inductance:
                return "1e-9f, 1.0f"; // 1nH to 1H
            case ModelicaUnit::Temperature:
                return "200.0f, 500.0f"; // 200K to 500K
            default:
                return "-1.0f, 1.0f";
        }
    }
    
    std::string generate_transfer_function_body(const ModelicaComponent& component) {
        std::ostringstream body;
        
        if (component.name == "Resistor") {
            body << "        float omega = analog_mul(2.0f * 3.14159265f, frequency);\n";
            body << "        float R = get_parameter(\"R\");\n";
            body << "        \n";
            body << "        // For resistor: Z(jω) = R (frequency independent)\n";
            body << "        return AnalogComplex(R, 0.0f);\n";
        }
        else if (component.name == "Capacitor") {
            body << "        float omega = analog_mul(2.0f * 3.14159265f, frequency);\n";
            body << "        float C = get_parameter(\"C\");\n";
            body << "        \n";
            body << "        // For capacitor: Z(jω) = 1/(jωC) = -j/(ωC)\n";
            body << "        float impedance_mag = analog_div(1.0f, analog_mul(omega, C));\n";
            body << "        return AnalogComplex(0.0f, analog_negate(impedance_mag));\n";
        }
        else if (component.name == "Conductor") {
            body << "        float omega = analog_mul(2.0f * 3.14159265f, frequency);\n";
            body << "        float G = get_parameter(\"G\");\n";
            body << "        \n";
            body << "        // For conductor: Z(jω) = 1/G (frequency independent)\n";
            body << "        float R_equiv = analog_div(1.0f, G);\n";
            body << "        return AnalogComplex(R_equiv, 0.0f);\n";
        }
        else {
            body << "        // Generic component - implement specific transfer function\n";
            body << "        return AnalogComplex(1.0f, 0.0f);\n";
        }
        
        return body.str();
    }
    
    std::string generate_simd_transfer_function_body(const ModelicaComponent& component) {
        std::ostringstream body;
        
        body << "        // SIMD implementation - process 8 frequencies in parallel\n";
        body << "        alignas(32) float freq_array[8], mag_array[8], phase_array[8];\n";
        body << "        _mm256_store_ps(freq_array, frequencies);\n";
        body << "        \n";
        body << "        for (int i = 0; i < 8; ++i) {\n";
        body << "            AnalogComplex h = transfer_function(freq_array[i]);\n";
        body << "            mag_array[i] = h.magnitude();\n";
        body << "            phase_array[i] = h.phase();\n";
        body << "        }\n";
        body << "        \n";
        body << "        magnitude_out = _mm256_load_ps(mag_array);\n";
        body << "        phase_out = _mm256_load_ps(phase_array);\n";
        
        return body.str();
    }
    
    std::string generate_component_specific_methods(const ModelicaComponent& component) {
        std::ostringstream methods;
        
        if (component.name == "Resistor") {
            methods << "    float get_resistance() const {\n";
            methods << "        float R = get_parameter(\"R\");\n";
            methods << "        if (get_parameter(\"alpha\") != 0.0f) {\n";
            methods << "            float T_ref = get_parameter(\"T_ref\");\n";
            methods << "            float alpha = get_parameter(\"alpha\");\n";
            methods << "            float T_heatPort = 300.15f; // Default temperature\n";
            methods << "            R = analog_mul(R, analog_add(1.0f, analog_mul(alpha, analog_sub(T_heatPort, T_ref))));\n";
            methods << "        }\n";
            methods << "        return R;\n";
            methods << "    }\n\n";
        }
        else if (component.name == "Capacitor") {
            methods << "    float get_capacitance() const {\n";
            methods << "        return get_parameter(\"C\");\n";
            methods << "    }\n\n";
            
            methods << "    float get_impedance_at_frequency(float freq) const {\n";
            methods << "        float omega = analog_mul(2.0f * 3.14159265f, freq);\n";
            methods << "        return analog_div(1.0f, analog_mul(omega, get_parameter(\"C\")));\n";
            methods << "    }\n\n";
        }
        
        return methods.str();
    }
};

// --- Factory Registration Helper ---
template<typename ComponentType>
void register_modelica_component(CircuitFactory& factory, const std::string& name) {
    factory.register_primitive(name, []() {
        return std::make_unique<ComponentType>();
    });
}

// --- Usage Example ---
inline void demo_modelica_parsing() {
    ModelicaParser parser;
    ModelicaToWoflangGenerator generator;
    
    // Parse Modelica resistor code
    std::string resistor_code = R"(
        within Modelica.Electrical.Analog.Basic;
        model Resistor "Ideal linear electrical resistor"
          parameter SI.Resistance R(start=1) "Resistance at temperature T_ref";
          parameter SI.Temperature T_ref=300.15 "Reference temperature";
          parameter SI.LinearTemperatureCoefficient alpha=0
            "Temperature coefficient of resistance";
          
        equation
          R_actual = R*(1 + alpha*(T_heatPort - T_ref));
          v = R_actual*i;
        end Resistor;
    )";
    
    ModelicaComponent resistor = parser.parse_component(resistor_code);
    std::string woflang_code = generator.generate_circuit_primitive(resistor);
    
    std::cout << "Generated Woflang Circuit Primitive:\n";
    std::cout << woflang_code << std::endl;
}

} // namespace woflang