#pragma once

#include "analog_modes.hpp"
#include "analog_math.hpp"
#include "analog_math_extended.hpp"
#include <vector>
#include <string>
#include <memory>

namespace woflang {

// --- Essential Component Categories ---

/* TIER 1: PASSIVE FUNDAMENTALS (Must Have)
 * These 6 components handle 80% of analog circuits
 */
struct Tier1Components {
    static const std::vector<std::string> components;
    static const std::string description;
};

const std::vector<std::string> Tier1Components::components = {
    "Resistor",           // R - voltage/current relationship
    "Capacitor",          // C - energy storage, frequency response  
    "Inductor",           // L - magnetic energy storage
    "Voltage_Source",     // DC/AC voltage generation
    "Current_Source",     // DC/AC current generation
    "Ground"              // Reference potential
};

const std::string Tier1Components::description = 
    "Linear passive components + sources. Handles basic filters, "
    "power supplies, timing circuits, impedance networks.";

/* TIER 2: ACTIVE FUNDAMENTALS (High Priority)
 * These 4 components enable 95% of analog signal processing
 */
struct Tier2Components {
    static const std::vector<std::string> components;
    static const std::string description;
};

const std::vector<std::string> Tier2Components::components = {
    "Op_Amp",             // Operational amplifier - gain, buffering
    "Transistor_NPN",     // Bipolar NPN - switching, amplification
    "Transistor_PNP",     // Bipolar PNP - complementary circuits
    "Diode"               // PN junction - rectification, protection
};

const std::string Tier2Components::description = 
    "Active devices for amplification, switching, signal conditioning. "
    "Enables op-amp circuits, differential pairs, current mirrors.";

/* TIER 3: PRACTICAL EXTENSIONS (Medium Priority) 
 * These 6 components handle real-world circuit needs
 */
struct Tier3Components {
    static const std::vector<std::string> components;
    static const std::string description;
};

const std::vector<std::string> Tier3Components::components = {
    "MOSFET_N",           // N-channel MOSFET - power switching
    "MOSFET_P",           // P-channel MOSFET - complementary
    "Zener_Diode",        // Voltage regulation, protection
    "Variable_Resistor",  // Potentiometers, trimmers
    "Transformer",        // AC coupling, isolation, impedance matching
    "Transmission_Line"   // High-frequency effects, PCB traces
};

const std::string Tier3Components::description = 
    "Power management, precision control, high-frequency effects. "
    "Covers switching converters, precision references, RF basics.";

/* TIER 4: SPECIALIZED (Lower Priority)
 * These handle specific domain needs
 */
struct Tier4Components {
    static const std::vector<std::string> components;
    static const std::string description;
};

const std::vector<std::string> Tier4Components::components = {
    "Comparator",         // Digital logic interface
    "Current_Mirror",     // Bias circuits, analog computation
    "Differential_Pair",  // Input stages, sensing
    "Crystal_Oscillator", // Precise timing reference
    "Gyrator",            // Synthetic inductance
    "Memristor"           // Emerging memory/computation
};

const std::string Tier4Components::description = 
    "Specialized functions for advanced circuits. Mixed-signal, "
    "precision timing, novel computing architectures.";

// --- Component Implementation Priority Matrix ---
struct ComponentPriorityMatrix {
    struct ComponentInfo {
        std::string name;
        int tier;
        float complexity_score;     // 1.0 = simple, 10.0 = very complex
        float usage_frequency;      // 0.0-1.0 how often it appears
        std::vector<std::string> enables;  // What circuits this unlocks
        std::string modelica_equivalent;
        bool has_temperature_effects;
        bool has_nonlinear_behavior;
    };
    
    static const std::vector<ComponentInfo> priority_list;
};

const std::vector<ComponentPriorityMatrix::ComponentInfo> ComponentPriorityMatrix::priority_list = {
    // TIER 1 - Implement these first
    {"Resistor", 1, 1.0f, 1.0f, 
     {"voltage dividers", "current limiting", "biasing", "RC filters"}, 
     "Modelica.Electrical.Analog.Basic.Resistor", true, false},
    
    {"Capacitor", 1, 2.0f, 0.95f,
     {"AC coupling", "filtering", "energy storage", "timing circuits"},
     "Modelica.Electrical.Analog.Basic.Capacitor", false, false},
     
    {"Ground", 1, 0.5f, 1.0f,
     {"reference potential", "return path", "safety"},
     "Modelica.Electrical.Analog.Basic.Ground", false, false},
     
    {"Voltage_Source", 1, 1.5f, 0.90f,
     {"power supply", "signal generation", "biasing"},
     "Modelica.Electrical.Analog.Sources.ConstantVoltage", false, false},
     
    {"Inductor", 1, 3.0f, 0.70f,
     {"energy storage", "EMI filtering", "switching converters"},
     "Modelica.Electrical.Analog.Basic.Inductor", false, false},
     
    {"Current_Source", 1, 2.0f, 0.60f,
     {"biasing", "load testing", "current mirrors"},
     "Modelica.Electrical.Analog.Sources.ConstantCurrent", false, false},
     
    // TIER 2 - High impact active components
    {"Op_Amp", 2, 6.0f, 0.85f,
     {"amplifiers", "filters", "comparators", "analog computation"},
     "Modelica.Electrical.Analog.Ideal.IdealOpAmp", true, true},
     
    {"Diode", 2, 4.0f, 0.80f,
     {"rectification", "protection", "voltage references", "logic"},
     "Modelica.Electrical.Analog.Semiconductors.Diode", true, true},
     
    {"Transistor_NPN", 2, 7.0f, 0.75f,
     {"amplification", "switching", "current mirrors", "differential pairs"},
     "Modelica.Electrical.Analog.Semiconductors.NPN", true, true},
     
    {"Transistor_PNP", 2, 7.0f, 0.65f,
     {"complementary amplifiers", "current sources", "level shifting"},
     "Modelica.Electrical.Analog.Semiconductors.PNP", true, true},
     
    // TIER 3 - Practical extensions
    {"MOSFET_N", 3, 8.0f, 0.70f,
     {"power switching", "digital logic", "high-efficiency converters"},
     "Modelica.Electrical.Analog.Semiconductors.NMOS", true, true},
     
    {"Variable_Resistor", 3, 3.0f, 0.60f,
     {"user controls", "calibration", "adaptive circuits"},
     "Modelica.Electrical.Analog.Basic.VariableResistor", true, false},
     
    {"Zener_Diode", 3, 4.5f, 0.55f,
     {"voltage regulation", "protection", "reference generation"},
     "Modelica.Electrical.Analog.Semiconductors.ZDiode", true, true},
     
    {"MOSFET_P", 3, 8.0f, 0.50f,
     {"complementary switching", "high-side drivers", "protection"},
     "Modelica.Electrical.Analog.Semiconductors.PMOS", true, true},
     
    {"Transformer", 3, 9.0f, 0.40f,
     {"isolation", "impedance matching", "power conversion"},
     "Modelica.Electrical.Analog.Basic.Transformer", false, true},
     
    {"Transmission_Line", 3, 9.5f, 0.30f,
     {"high-frequency circuits", "PCB effects", "impedance control"},
     "Modelica.Electrical.Analog.Lines.TLine", false, false}
};

// --- Circuit Coverage Analysis ---
struct CircuitCoverageAnalysis {
    
    // What percentage of common circuits can be built with each tier
    static float get_coverage_percentage(int max_tier) {
        switch(max_tier) {
            case 1: return 80.0f;   // Passive circuits, basic filters, power supplies
            case 2: return 95.0f;   // + Active filters, amplifiers, switching circuits  
            case 3: return 99.0f;   // + Power management, precision circuits, RF basics
            case 4: return 99.8f;   // + Specialized/emerging applications
            default: return 0.0f;
        }
    }
    
    // Common circuit types enabled by each tier
    static std::vector<std::string> get_enabled_circuits(int max_tier) {
        std::vector<std::string> circuits;
        
        if (max_tier >= 1) {
            circuits.insert(circuits.end(), {
                "RC/RL/LC filters", "Voltage dividers", "Basic power supplies",
                "Timing circuits", "Impedance networks", "AC coupling"
            });
        }
        
        if (max_tier >= 2) {
            circuits.insert(circuits.end(), {
                "Op-amp amplifiers", "Active filters", "Comparators", 
                "Rectifiers", "Switching circuits", "Current mirrors",
                "Differential amplifiers", "Oscillators"
            });
        }
        
        if (max_tier >= 3) {
            circuits.insert(circuits.end(), {
                "Switching converters", "Motor drivers", "Precision references",
                "High-frequency amplifiers", "Impedance matching networks",
                "Isolated power supplies", "Protection circuits"
            });
        }
        
        if (max_tier >= 4) {
            circuits.insert(circuits.end(), {
                "Mixed-signal interfaces", "Precision timing", "Novel computing",
                "Neuromorphic circuits", "RF transceivers", "Sensor interfaces"
            });
        }
        
        return circuits;
    }
    
    // Implementation effort estimate (person-weeks)
    static float get_implementation_effort(int max_tier) {
        switch(max_tier) {
            case 1: return 2.0f;    // 2 weeks for basic passives + sources
            case 2: return 8.0f;    // +6 weeks for active components (complex models)
            case 3: return 16.0f;   // +8 weeks for power devices, RF effects
            case 4: return 24.0f;   // +8 weeks for specialized components
            default: return 0.0f;
        }
    }
};

// --- Recommended Implementation Strategy ---
struct ImplementationStrategy {
    
    static std::string get_phase_1_plan() {
        return R"(
PHASE 1: MVP Circuit Library (2 weeks)
==========================================
Target: 80% circuit coverage with minimal effort

Components (6):
- Resistor (temperature-dependent)
- Capacitor  
- Ground
- Voltage_Source (DC + AC)
- Inductor
- Current_Source

Validation Circuits:
- RC low-pass filter
- Voltage divider
- LC oscillator tank
- Basic power supply filter
- AC coupling network

Success Criteria:
- All Tier 1 components working in woflang factory
- Frequency response analysis functional
- SIMD acceleration operational
- Dr. Krut's bridged-T filters implementable
        )";
    }
    
    static std::string get_phase_2_plan() {
        return R"(
PHASE 2: Active Circuit Library (6 weeks) 
==========================================
Target: 95% circuit coverage - professional grade

Add Components (4):
- Op_Amp (ideal + real models)
- Diode (Shockley equation)
- Transistor_NPN (Ebers-Moll/Gummel-Poon)
- Transistor_PNP

Validation Circuits:
- Non-inverting op-amp amplifier
- Active low-pass filter
- Full-wave rectifier
- Common-emitter amplifier
- Differential pair
- Current mirror

Success Criteria:
- Nonlinear equation solving
- Temperature-dependent device models
- AC+DC analysis capabilities
- Distortion analysis
        )";
    }
    
    static std::string get_modelica_extraction_priority() {
        return R"(
MODELICA EXTRACTION PRIORITY ORDER:
==================================
1. Resistor.mo          - Template for temperature effects
2. Capacitor.mo         - Template for reactive components  
3. Ground.mo            - Simple reference implementation
4. ConstantVoltage.mo   - Source template
5. Inductor.mo          - Magnetic storage model
6. ConstantCurrent.mo   - Current source template
7. IdealOpAmp.mo        - First active component
8. Diode.mo             - Nonlinear device template
9. NPN.mo               - Complex semiconductor model
10. PNP.mo              - Complementary device

Extract in this order to build complexity gradually.
Each component validates the transpiler pipeline.
        )";
    }
};

// --- Usage Analysis ---
struct UsageAnalysis {
    
    // Real-world circuit analysis based on common designs
    static std::string get_coverage_report() {
        return R"(
CIRCUIT COVERAGE ANALYSIS:
=========================

WITH TIER 1 ONLY (6 components):
✓ Power supply filtering (RC, LC)
✓ Audio crossovers and tone controls  
✓ Antenna matching networks
✓ Basic timing circuits
✓ DC bias networks
✓ Passive filters up to 4th order
✗ Amplification (requires active devices)
✗ Signal conditioning (requires op-amps)
✗ Switching/logic (requires semiconductors)

WITH TIER 1+2 (10 components):
✓ Everything above, plus:
✓ Op-amp circuits (amplifiers, filters, comparators)
✓ Small-signal amplifiers
✓ Rectifiers and voltage multipliers
✓ Basic switching circuits
✓ Current sources and mirrors
✓ Differential signaling
✗ High-power switching (needs MOSFETs)
✗ Precision voltage references (needs Zeners)
✗ Isolated designs (needs transformers)

RECOMMENDATION:
Start with Tier 1 for rapid prototyping capability.
Tier 1+2 provides professional-grade analog design coverage.
        )";
    }
};

} // namespace woflang

// Example usage showing prioritized implementation
inline void demo_component_prioritization() {
    using namespace woflang;
    
    std::cout << "WOFLANG ANALOG COMPONENT IMPLEMENTATION STRATEGY\n";
    std::cout << "===============================================\n\n";
    
    // Show coverage analysis
    std::cout << "COVERAGE BY TIER:\n";
    for (int tier = 1; tier <= 4; ++tier) {
        float coverage = CircuitCoverageAnalysis::get_coverage_percentage(tier);
        float effort = CircuitCoverageAnalysis::get_implementation_effort(tier);
        std::cout << "Tier " << tier << ": " << coverage << "% coverage, " 
                  << effort << " person-weeks\n";
    }
    
    std::cout << "\n" << ImplementationStrategy::get_phase_1_plan() << "\n";
    std::cout << ImplementationStrategy::get_modelica_extraction_priority() << "\n";
    std::cout << UsageAnalysis::get_coverage_report() << "\n";
    
    // Show priority-ordered component list
    std::cout << "\nCOMPONENT IMPLEMENTATION ORDER:\n";
    std::cout << "==============================\n";
    
    auto components = ComponentPriorityMatrix::priority_list;
    
    // Sort by tier, then by usage frequency  
    std::sort(components.begin(), components.end(), 
        [](const auto& a, const auto& b) {
            if (a.tier != b.tier) return a.tier < b.tier;
            return a.usage_frequency > b.usage_frequency;
        });
    
    for (const auto& comp : components) {
        std::cout << comp.tier << "." << " " << comp.name 
                  << " (usage: " << (comp.usage_frequency * 100) << "%, "
                  << "complexity: " << comp.complexity_score << "/10)\n";
        std::cout << "    Enables: ";
        for (size_t i = 0; i < comp.enables.size(); ++i) {
            std::cout << comp.enables[i];
            if (i < comp.enables.size() - 1) std::cout << ", ";
        }
        std::cout << "\n    Modelica: " << comp.modelica_equivalent << "\n\n";
    }
}