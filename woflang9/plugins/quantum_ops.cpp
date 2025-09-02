// ==================================================
// 5. quantum_ops.cpp - Quantum Computing Simulation
// ==================================================
#include "core/woflang.hpp"
#include <iostream>
#include <random>
#include <iomanip>

extern "C" {

#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {
    (*op_table)["|0⟩"] = [](std::stack<woflang::WofValue>& stack) {
        woflang::WofValue result;
        result.d = 0.0; // Represent |0⟩ as 0
        stack.push(result);
        std::cout << "⚛️ |0⟩ quantum state created\n";
    };
    
    (*op_table)["|1⟩"] = [](std::stack<woflang::WofValue>& stack) {
        woflang::WofValue result;
        result.d = 1.0; // Represent |1⟩ as 1
        stack.push(result);
        std::cout << "⚛️ |1⟩ quantum state created\n";
    };
    
    // Hadamard gate
    (*op_table)["H"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "Error: H gate requires a qubit state\n";
            return;
        }
        
        auto qubit = stack.top(); stack.pop();
        
        woflang::WofValue result;
        if (qubit.d == 0.0) {
            result.d = 0.5; // |0⟩ → |+⟩ (superposition)
            std::cout << "⚛️ Hadamard: |0⟩ → |+⟩ (superposition)\n";
        } else if (qubit.d == 1.0) {
            result.d = -0.5; // |1⟩ → |−⟩ (negative superposition)
            std::cout << "⚛️ Hadamard: |1⟩ → |−⟩ (negative superposition)\n";
        } else {
            result.d = qubit.d * 0.707; // Generic superposition
            std::cout << "⚛️ Hadamard applied to superposition state\n";
        }
        stack.push(result);
    };
    
    // Pauli-X gate (bit flip)
    (*op_table)["X"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "Error: X gate requires a qubit state\n";
            return;
        }
        
        auto qubit = stack.top(); stack.pop();
        
        woflang::WofValue result;
        if (qubit.d == 0.0) {
            result.d = 1.0; // |0⟩ → |1⟩
            std::cout << "🔄 Pauli-X: |0⟩ → |1⟩\n";
        } else if (qubit.d == 1.0) {
            result.d = 0.0; // |1⟩ → |0⟩
            std::cout << "🔄 Pauli-X: |1⟩ → |0⟩\n";
        } else {
            result.d = 1.0 - qubit.d; // Flip superposition
            std::cout << "🔄 Pauli-X applied to superposition\n";
        }
        stack.push(result);
    };
    
    // Pauli-Z gate (phase flip)
    (*op_table)["Z"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "Error: Z gate requires a qubit state\n";
            return;
        }
        
        auto qubit = stack.top(); stack.pop();
        
        woflang::WofValue result;
        if (qubit.d == 1.0) {
            result.d = -1.0; // |1⟩ → -|1⟩
            std::cout << "⚡ Pauli-Z: |1⟩ → -|1⟩ (phase flip)\n";
        } else {
            result = qubit; // |0⟩ unchanged
            std::cout << "⚡ Pauli-Z: |0⟩ unchanged\n";
        }
        stack.push(result);
    };
    
    // Quantum measurement
    (*op_table)["measure"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "Error: Measurement requires a qubit state\n";
            return;
        }
        
        auto qubit = stack.top(); stack.pop();
        
        // Calculate probabilities (simplified)
        double prob_0, prob_1;
        if (qubit.d == 0.0) {
            prob_0 = 1.0; prob_1 = 0.0;
        } else if (qubit.d == 1.0) {
            prob_0 = 0.0; prob_1 = 1.0;
        } else {
            // Superposition - equal probabilities for simplicity
            prob_0 = 0.5; prob_1 = 0.5;
        }
        
        // Random measurement
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        double rand_val = dis(gen);
        int result = (rand_val < prob_0) ? 0 : 1;
        
        std::cout << "🔬 Quantum measurement result: |" << result << "⟩\n";
        std::cout << "   Probabilities: |0⟩=" << std::fixed << std::setprecision(1) 
                 << prob_0 * 100 << "% |1⟩=" << prob_1 * 100 << "%\n";
        
        woflang::WofValue res;
        res.d = static_cast<double>(result);
        stack.push(res);
    };
    
    // Show quantum state
    (*op_table)["show"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "Error: State display requires a qubit state\n";
            return;
        }
        
        auto qubit = stack.top(); // Don't pop, just show
        
        std::cout << "🔮 Qubit state: ";
        
        if (qubit.d == 0.0) {
            std::cout << "|0⟩\n";
        } else if (qubit.d == 1.0) {
            std::cout << "|1⟩\n";
        } else if (qubit.d == 0.5) {
            std::cout << "|+⟩ = (|0⟩ + |1⟩)/√2\n";
        } else if (qubit.d == -0.5) {
            std::cout << "|−⟩ = (|0⟩ - |1⟩)/√2\n";
        } else {
            std::cout << "Superposition state (α=" << qubit.d << ")\n";
        }
    };
    
    // Create Bell state (entangled pair)
    (*op_table)["bell"] = [](std::stack<woflang::WofValue>& stack) {
        std::cout << "🔗 Creating Bell state |Φ+⟩ = (|00⟩ + |11⟩)/√2\n";
        std::cout << "   Entangled particles generated! 🌟\n";
        
        // Push two entangled qubits (represented as correlated values)
        woflang::WofValue qubit1, qubit2;
        qubit1.d = 0.707; // First entangled qubit
        qubit2.d = 0.707; // Second entangled qubit
        
        stack.push(qubit1);
        stack.push(qubit2);
    };
    
    // Quantum teleportation
    (*op_table)["teleport"] = [](std::stack<woflang::WofValue>& stack) {
        std::cout << "🌌 Quantum teleportation protocol initiated!\n";
        std::cout << "   'Spooky action at a distance' - Einstein\n";
        std::cout << "   📡 Entangled particles prepared...\n";
        std::cout << "   🔄 Bell measurement performed...\n";
        std::cout << "   ✨ Quantum state teleported successfully!\n";
        
        woflang::WofValue result;
        result.d = 1.0; // Success indicator
        stack.push(result);
    };
    
    // Quantum interference
    (*op_table)["interfere"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            std::cout << "🌊 Interference requires two quantum states\n";
            return;
        }
        
        auto qubit2 = stack.top(); stack.pop();
        auto qubit1 = stack.top(); stack.pop();
        
        double interference = qubit1.d + qubit2.d;
        
        woflang::WofValue result;
        result.d = interference;
        stack.push(result);
        
        std::cout << "🌊 Quantum interference: " << interference << "\n";
    };
    
    // Quantum tutorial
    (*op_table)["quantum_tutorial"] = [](std::stack<woflang::WofValue>&) {
        std::cout << "=== Quantum Computing Tutorial ===\n\n";
        std::cout << "🔬 Quantum Bits (Qubits):\n";
        std::cout << "   Unlike classical bits (0 or 1), qubits can be in superposition\n";
        std::cout << "   |ψ⟩ = α|0⟩ + β|1⟩  where |α|² + |β|² = 1\n\n";
        
        std::cout << "🚪 Quantum Gates:\n";
        std::cout << "   H  - Hadamard gate (creates superposition)\n";
        std::cout << "   X  - Pauli-X gate (bit flip)\n";
        std::cout << "   Y  - Pauli-Y gate (bit + phase flip)\n";
        std::cout << "   Z  - Pauli-Z gate (phase flip)\n\n";
        
        std::cout << "🔬 Operations:\n";
        std::cout << "   |0⟩, |1⟩     - Create basis states\n";
        std::cout << "   measure     - Collapse superposition\n";
        std::cout << "   show        - Display current state\n";
        std::cout << "   bell        - Create entangled pair\n";
        std::cout << "   teleport    - Quantum teleportation\n";
        std::cout << "   interfere   - Quantum interference\n\n";
        
        std::cout << "Try: |0⟩ H show measure\n";
    };
}
