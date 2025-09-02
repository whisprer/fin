// ==================================================
// chemistry_ops.cpp - Chemistry Operations (Dynamic Loading)
// ==================================================
#include "core/woflang.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cmath>

extern "C" {

namespace {
    // Structure to hold element data
    struct Element {
        std::string symbol;
        std::string name;
        double atomic_weight;
        int atomic_number;
        std::string category;
    };
    
    // Map of element symbols to element data (simplified set)
    const std::map<std::string, Element> ELEMENTS = {
        {"H", {"H", "Hydrogen", 1.008, 1, "Nonmetal"}},
        {"He", {"He", "Helium", 4.0026, 2, "Noble Gas"}},
        {"Li", {"Li", "Lithium", 6.94, 3, "Alkali Metal"}},
        {"C", {"C", "Carbon", 12.011, 6, "Nonmetal"}},
        {"N", {"N", "Nitrogen", 14.007, 7, "Nonmetal"}},
        {"O", {"O", "Oxygen", 15.999, 8, "Nonmetal"}},
        {"F", {"F", "Fluorine", 18.998, 9, "Halogen"}},
        {"Ne", {"Ne", "Neon", 20.180, 10, "Noble Gas"}},
        {"Na", {"Na", "Sodium", 22.990, 11, "Alkali Metal"}},
        {"Mg", {"Mg", "Magnesium", 24.305, 12, "Alkaline Earth Metal"}},
        {"Al", {"Al", "Aluminum", 26.982, 13, "Post-Transition Metal"}},
        {"Si", {"Si", "Silicon", 28.085, 14, "Metalloid"}},
        {"P", {"P", "Phosphorus", 30.974, 15, "Nonmetal"}},
        {"S", {"S", "Sulfur", 32.06, 16, "Nonmetal"}},
        {"Cl", {"Cl", "Chlorine", 35.45, 17, "Halogen"}},
        {"Ar", {"Ar", "Argon", 39.948, 18, "Noble Gas"}},
        {"K", {"K", "Potassium", 39.098, 19, "Alkali Metal"}},
        {"Ca", {"Ca", "Calcium", 40.078, 20, "Alkaline Earth Metal"}},
        {"Fe", {"Fe", "Iron", 55.845, 26, "Transition Metal"}},
        {"Cu", {"Cu", "Copper", 63.546, 29, "Transition Metal"}},
        {"Zn", {"Zn", "Zinc", 65.38, 30, "Transition Metal"}},
        {"Ag", {"Ag", "Silver", 107.87, 47, "Transition Metal"}},
        {"Au", {"Au", "Gold", 196.97, 79, "Transition Metal"}}
    };
    
    // Physical constants
    const double AVOGADRO_NUMBER = 6.02214076e23;
    const double GAS_CONSTANT = 8.31446;
    
    // Get element by atomic number (simplified lookup)
    const Element* get_element_by_number(int atomic_number) {
        for (const auto& [symbol, element] : ELEMENTS) {
            if (element.atomic_number == atomic_number) {
                return &element;
            }
        }
        return nullptr;
    }
    
    // Calculate pH from hydrogen ion concentration
    double pH_from_concentration(double h_concentration) {
        return -std::log10(h_concentration);
    }
    
    // Calculate hydrogen ion concentration from pH
    double concentration_from_pH(double pH) {
        return std::pow(10, -pH);
    }
}

#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {
    // Get atomic weight by atomic number
    (*op_table)["atomic_weight"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("atomic_weight requires an atomic number");
        }
        
        auto element = stack.top(); stack.pop();
        
        int atomic_number = static_cast<int>(element.d);
        
        const Element* elem = get_element_by_number(atomic_number);
        if (!elem) {
            throw std::runtime_error("Unknown element with atomic number: " + std::to_string(atomic_number));
        }
        
        double weight = elem->atomic_weight;
        
        woflang::WofValue result;
        result.d = weight;
        stack.push(result);
        
        std::cout << "Atomic weight of " << elem->symbol << ": " << weight << " g/mol" << std::endl;
    };
    
    // Get element information by atomic number
    (*op_table)["element_info"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("element_info requires an atomic number");
        }
        
        auto element = stack.top(); stack.pop();
        
        int atomic_number = static_cast<int>(element.d);
        
        const Element* elem = get_element_by_number(atomic_number);
        if (!elem) {
            throw std::runtime_error("Unknown element with atomic number: " + std::to_string(atomic_number));
        }
        
        std::cout << "Element: " << elem->name << " (" << elem->symbol << ")" << std::endl;
        std::cout << "  Atomic Number: " << elem->atomic_number << std::endl;
        std::cout << "  Atomic Weight: " << elem->atomic_weight << " g/mol" << std::endl;
        std::cout << "  Category: " << elem->category << std::endl;
        
        // Push the atomic weight onto the stack
        woflang::WofValue result;
        result.d = elem->atomic_weight;
        stack.push(result);
    };
    
    // Calculate molecular weight (simplified - assumes common molecules)
    (*op_table)["molecular_weight"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("molecular_weight requires molecule_type and count");
        }
        
        auto count = stack.top(); stack.pop();
        auto molecule_type = stack.top(); stack.pop();
        
        int mol_type = static_cast<int>(molecule_type.d);
        int mol_count = static_cast<int>(count.d);
        
        double weight = 0.0;
        std::string formula;
        
        switch (mol_type) {
            case 1: // H2O (water)
                weight = 2 * 1.008 + 15.999; // 2H + O
                formula = "H2O";
                break;
            case 2: // CO2 (carbon dioxide)
                weight = 12.011 + 2 * 15.999; // C + 2O
                formula = "CO2";
                break;
            case 3: // CH4 (methane)
                weight = 12.011 + 4 * 1.008; // C + 4H
                formula = "CH4";
                break;
            case 4: // NH3 (ammonia)
                weight = 14.007 + 3 * 1.008; // N + 3H
                formula = "NH3";
                break;
            case 5: // NaCl (salt)
                weight = 22.990 + 35.45; // Na + Cl
                formula = "NaCl";
                break;
            case 6: // C6H12O6 (glucose)
                weight = 6 * 12.011 + 12 * 1.008 + 6 * 15.999; // 6C + 12H + 6O
                formula = "C6H12O6";
                break;
            default:
                throw std::runtime_error("Unknown molecule type. Use 1-6 for H2O, CO2, CH4, NH3, NaCl, C6H12O6");
        }
        
        double total_weight = weight * mol_count;
        
        woflang::WofValue result;
        result.d = total_weight;
        stack.push(result);
        
        std::cout << "Molecular weight of " << mol_count << " " << formula << ": " 
                 << total_weight << " g/mol" << std::endl;
    };
    
    // Calculate pH from hydrogen ion concentration
    (*op_table)["pH_from_conc"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("pH_from_conc requires H+ concentration");
        }
        
        auto conc = stack.top(); stack.pop();
        
        double h_concentration = conc.d;
        
        if (h_concentration <= 0.0) {
            throw std::runtime_error("H+ concentration must be positive");
        }
        
        double pH = pH_from_concentration(h_concentration);
        
        woflang::WofValue result;
        result.d = pH;
        stack.push(result);
        
        std::cout << "pH: " << pH << std::endl;
        
        if (pH < 7.0) {
            std::cout << "Solution is acidic" << std::endl;
        } else if (pH > 7.0) {
            std::cout << "Solution is basic" << std::endl;
        } else {
            std::cout << "Solution is neutral" << std::endl;
        }
    };
    
    // Calculate H+ concentration from pH
    (*op_table)["conc_from_pH"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("conc_from_pH requires pH value");
        }
        
        auto pH_val = stack.top(); stack.pop();
        
        double pH = pH_val.d;
        
        double conc = concentration_from_pH(pH);
        
        woflang::WofValue result;
        result.d = conc;
        stack.push(result);
        
        std::cout << "H⁺ concentration: " << conc << " mol/L" << std::endl;
    };
    
    // Calculate molarity (moles per liter)
    (*op_table)["molarity"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("molarity requires moles and volume (L)");
        }
        
        auto volume = stack.top(); stack.pop();
        auto moles = stack.top(); stack.pop();
        
        double n = moles.d;
        double v = volume.d;
        
        if (v <= 0) {
            throw std::runtime_error("Volume must be positive");
        }
        
        double molarity = n / v;
        
        woflang::WofValue result;
        result.d = molarity;
        stack.push(result);
        
        std::cout << "Molarity: " << molarity << " mol/L" << std::endl;
    };
    
    // Convert between temperature units
    (*op_table)["celsius_to_kelvin"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("celsius_to_kelvin requires a temperature");
        }
        
        auto temp = stack.top(); stack.pop();
        
        double celsius = temp.d;
        double kelvin = celsius + 273.15;
        
        woflang::WofValue result;
        result.d = kelvin;
        stack.push(result);
        
        std::cout << celsius << "°C = " << kelvin << " K" << std::endl;
    };
    
    (*op_table)["kelvin_to_celsius"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("kelvin_to_celsius requires a temperature");
        }
        
        auto temp = stack.top(); stack.pop();
        
        double kelvin = temp.d;
        double celsius = kelvin - 273.15;
        
        woflang::WofValue result;
        result.d = celsius;
        stack.push(result);
        
        std::cout << kelvin << " K = " << celsius << "°C" << std::endl;
    };
    
    // Convert moles to grams
    (*op_table)["moles_to_grams"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("moles_to_grams requires moles and molecular weight");
        }
        
        auto mw = stack.top(); stack.pop();
        auto moles = stack.top(); stack.pop();
        
        double n = moles.d;
        double molecular_weight = mw.d;
        double grams = n * molecular_weight;
        
        woflang::WofValue result;
        result.d = grams;
        stack.push(result);
        
        std::cout << n << " mol × " << molecular_weight << " g/mol = " << grams << " g" << std::endl;
    };
    
    // Convert grams to moles
    (*op_table)["grams_to_moles"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("grams_to_moles requires grams and molecular weight");
        }
        
        auto mw = stack.top(); stack.pop();
        auto grams = stack.top(); stack.pop();
        
        double g = grams.d;
        double molecular_weight = mw.d;
        
        if (molecular_weight <= 0) {
            throw std::runtime_error("Molecular weight must be positive");
        }
        
        double moles = g / molecular_weight;
        
        woflang::WofValue result;
        result.d = moles;
        stack.push(result);
        
        std::cout << g << " g ÷ " << molecular_weight << " g/mol = " << moles << " mol" << std::endl;
    };
    
    // Push Avogadro's number
    (*op_table)["avogadro"] = [](std::stack<woflang::WofValue>& stack) {
        woflang::WofValue result;
        result.d = AVOGADRO_NUMBER;
        stack.push(result);
        std::cout << "Avogadro's number: " << AVOGADRO_NUMBER << " mol⁻¹" << std::endl;
    };
    
    // Push the gas constant
    (*op_table)["gas_constant"] = [](std::stack<woflang::WofValue>& stack) {
        woflang::WofValue result;
        result.d = GAS_CONSTANT;
        stack.push(result);
        std::cout << "Gas constant: " << GAS_CONSTANT << " J/(mol·K)" << std::endl;
    };
    
    // Calculate density (mass per volume)
    (*op_table)["density"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("density requires mass and volume");
        }
        
        auto volume = stack.top(); stack.pop();
        auto mass = stack.top(); stack.pop();
        
        double m = mass.d;
        double v = volume.d;
        
        if (v <= 0) {
            throw std::runtime_error("Volume must be positive");
        }
        
        double density = m / v;
        
        woflang::WofValue result;
        result.d = density;
        stack.push(result);
        
        std::cout << "Density: " << density << " g/mL" << std::endl;
    };
    
    // List available elements
    (*op_table)["list_elements"] = [](std::stack<woflang::WofValue>&) {
        std::cout << "Available elements:" << std::endl;
        
        for (const auto& [symbol, elem] : ELEMENTS) {
            std::cout << "  " << elem.atomic_number << ": " << elem.symbol 
                     << " (" << elem.name << ") - " << elem.atomic_weight << " g/mol" << std::endl;
        }
        
        std::cout << std::endl << "Usage examples:" << std::endl;
        std::cout << "  1 element_info    # Get info for Hydrogen" << std::endl;
        std::cout << "  6 atomic_weight   # Get atomic weight of Carbon" << std::endl;
        std::cout << "  1 2 molecular_weight  # Get molecular weight of 2 H2O molecules" << std::endl;
    };
    
    // Chemistry tutorial
    (*op_table)["chemistry_tutorial"] = [](std::stack<woflang::WofValue>&) {
        std::cout << "=== Basic Chemistry Tutorial ===" << std::endl << std::endl;
        
        std::cout << "1. Atoms and Elements:" << std::endl;
        std::cout << "   An atom is the smallest unit of matter that retains the properties of an element." << std::endl;
        std::cout << "   Each element has a unique atomic number and atomic weight." << std::endl << std::endl;
        
        std::cout << "2. Molecules and Compounds:" << std::endl;
        std::cout << "   Molecules form when atoms bond together." << std::endl;
        std::cout << "   Compounds are molecules containing different elements." << std::endl << std::endl;
        
        std::cout << "3. Stoichiometry:" << std::endl;
        std::cout << "   Stoichiometry is the calculation of reactants and products in chemical reactions." << std::endl;
        std::cout << "   The mole is a unit that helps relate mass to number of particles." << std::endl << std::endl;
        
        std::cout << "4. Solutions and Concentration:" << std::endl;
        std::cout << "   pH measures the acidity or alkalinity of a solution." << std::endl;
        std::cout << "   Molarity is the number of moles of solute per liter of solution." << std::endl << std::endl;
        
        std::cout << "Available operations:" << std::endl;
        std::cout << "  - Elements: element_info, atomic_weight, list_elements" << std::endl;
        std::cout << "  - Molecules: molecular_weight, moles_to_grams, grams_to_moles" << std::endl;
        std::cout << "  - Solutions: pH_from_conc, conc_from_pH, molarity" << std::endl;
        std::cout << "  - Conversions: celsius_to_kelvin, kelvin_to_celsius" << std::endl;
        std::cout << "  - Constants: avogadro, gas_constant" << std::endl << std::endl;
        
        std::cout << "Molecule types for molecular_weight:" << std::endl;
        std::cout << "  1: H2O (water)    2: CO2 (carbon dioxide)    3: CH4 (methane)" << std::endl;
        std::cout << "  4: NH3 (ammonia)  5: NaCl (salt)             6: C6H12O6 (glucose)" << std::endl;
    };
}
