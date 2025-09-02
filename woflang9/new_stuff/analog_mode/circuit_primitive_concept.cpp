#pragma once

#include "analog_modes.hpp"
#include "analog_math.hpp"
#include "analog_math_extended.hpp"
#include "analog_vector_simd.hpp"
#include <functional>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

namespace woflang {

// Forward declarations
struct AnalogComplex;
class CircuitPrimitive;

// --- Circuit Parameter Interface ---
struct CircuitParameter {
    std::string name;
    float value;
    float min_val;
    float max_val;
    bool is_knob;  // Can be controlled in real-time
    
    CircuitParameter(const std::string& n, float val, float min_v = -1.0f, float max_v = 1.0f, bool knob = false)
        : name(n), value(clamp_analog(val)), min_val(min_v), max_val(max_v), is_knob(knob) {}
    
    void set_value(float val) {
        value = clamp_analog(std::max(min_val, std::min(max_val, val)));
    }
    
    float get_normalized() const {
        if (max_val == min_val) return 0.0f;
        return (value - min_val) / (max_val - min_val);
    }
    
    void set_normalized(float norm) {
        float clamped_norm = clamp_analog(std::max(0.0f, std::min(1.0f, norm)));
        value = min_val + clamped_norm * (max_val - min_val);
    }
};

// --- SIMD-friendly parameter batch ---
struct ParameterBatch {
    static constexpr int BATCH_SIZE = 8;
    std::array<float, BATCH_SIZE> values;
    std::string parameter_name;
    
    ParameterBatch(const std::string& name) : parameter_name(name) {
        values.fill(0.0f);
    }
    
    __m256 to_simd() const {
        return _mm256_load_ps(values.data());
    }
    
    void from_simd(__m256 vec) {
        _mm256_store_ps(values.data(), clamp_analog(vec));
    }
    
    void set_uniform(float val) {
        values.fill(clamp_analog(val));
    }
};

// --- Circuit Primitive Base Class ---
class CircuitPrimitive {
protected:
    std::unordered_map<std::string, CircuitParameter> parameters;
    std::string primitive_name;
    
public:
    CircuitPrimitive(const std::string& name) : primitive_name(name) {}
    virtual ~CircuitPrimitive() = default;
    
    // Pure virtual - must be implemented by each circuit type
    virtual AnalogComplex transfer_function(float frequency) const = 0;
    virtual void transfer_function_simd(__m256 frequencies, __m256& magnitude_out, __m256& phase_out) const = 0;
    
    // Parameter management
    void add_parameter(const std::string& name, float value, float min_val = -1.0f, float max_val = 1.0f, bool is_knob = false) {
        parameters.emplace(name, CircuitParameter(name, value, min_val, max_val, is_knob));
    }
    
    void set_parameter(const std::string& name, float value) {
        auto it = parameters.find(name);
        if (it != parameters.end()) {
            it->second.set_value(value);
        }
    }
    
    float get_parameter(const std::string& name) const {
        auto it = parameters.find(name);
        return (it != parameters.end()) ? it->second.value : 0.0f;
    }
    
    void set_parameter_normalized(const std::string& name, float norm_value) {
        auto it = parameters.find(name);
        if (it != parameters.end()) {
            it->second.set_normalized(norm_value);
        }
    }
    
    float get_parameter_normalized(const std::string& name) const {
        auto it = parameters.find(name);
        return (it != parameters.end()) ? it->second.get_normalized() : 0.0f;
    }
    
    // Get all knob-controllable parameters
    std::vector<std::string> get_knob_parameters() const {
        std::vector<std::string> knobs;
        for (const auto& [name, param] : parameters) {
            if (param.is_knob) {
                knobs.push_back(name);
            }
        }
        return knobs;
    }
    
    const std::string& get_name() const { return primitive_name; }
    
    // Virtual methods for derived classes to override
    virtual float center_frequency() const { return 1000.0f; }
    virtual float quality_factor() const { return 1.0f; }
    virtual float peak_gain() const { return 1.0f; }
    virtual float bandwidth() const { return center_frequency() / quality_factor(); }
};

// --- Dr. Krut's Bridged-T Filter Implementation ---
class BridgedTFilterPrimitive : public CircuitPrimitive {
public:
    BridgedTFilterPrimitive() : CircuitPrimitive("BridgedT_Filter") {
        // Tom/Conga positions as knob-controllable parameters
        add_parameter("tom_position", 0.5f, 0.0f, 1.0f, true);
        add_parameter("conga_position", 0.5f, 0.0f, 1.0f, true);
        
        // Capacitor values
        add_parameter("c1", 1e-6f, 1e-9f, 1e-3f, false);
        add_parameter("c2", 1e-6f, 1e-9f, 1e-3f, false);
        
        // Load resistance
        add_parameter("r_load", 10000.0f, 1.0f, 1e6f, false);
    }
    
private:
    // Dr. Krut's Tom/Conga resistance equation (C.12)
    float tom_conga_resistance(float position) const {
        float x = clamp_analog(std::max(0.0f, std::min(1.0f, position)));
        float numerator = analog_add(analog_sub(analog_negate(analog_mul(x, x)), x), 2.0f);
        float denominator = analog_add(x, 2.0f);
        return analog_mul(analog_div(numerator, denominator), 500.0f);
    }
    
    static __m256 tom_conga_resistance_simd(__m256 positions) {
        __m256 x = clamp_analog(positions);
        __m256 two = _mm256_set1_ps(2.0f);
        __m256 five_hundred = _mm256_set1_ps(500.0f);
        
        __m256 x_squared = analog_mul(x, x);
        __m256 neg_x_squared = analog_negate(x_squared);
        __m256 numerator = analog_add(analog_add(neg_x_squared, x), two);
        __m256 denominator = analog_add(x, two);
        
        return analog_mul(analog_div(numerator, denominator), five_hundred);
    }
    
public:
    AnalogComplex transfer_function(float frequency) const override {
        float omega = analog_mul(2.0f * 3.14159265f, frequency);
        AnalogComplex s(0.0f, omega);
        
        float r1 = tom_conga_resistance(get_parameter("tom_position"));
        float r2 = tom_conga_resistance(get_parameter("conga_position"));
        float c1 = get_parameter("c1");
        float c2 = get_parameter("c2");
        float r_load = get_parameter("r_load");
        
        // Dr. Krut's transfer function implementation (from our previous code)
        float num_s2_coeff = analog_mul(analog_mul(analog_mul(r1, r2), c1), c2);
        float num_s_coeff = analog_mul(r1, analog_add(c1, c2));
        float num_const = 1.0f;
        
        AnalogComplex s_squared = s * s;
        AnalogComplex numerator = AnalogComplex(num_const, 0.0f) + 
                                  s * AnalogComplex(num_s_coeff, 0.0f) +
                                  s_squared * AnalogComplex(num_s2_coeff, 0.0f);
        
        float denom_s2_coeff = num_s2_coeff;
        float denom_s_coeff = analog_add(
            analog_mul(r1, analog_mul(analog_add(1.0f, analog_div(r2, r_load)), analog_add(c1, c2))),
            analog_mul(r2, c2)
        );
        float denom_const = analog_add(analog_div(r2, r_load), 1.0f);
        
        AnalogComplex denominator = AnalogComplex(denom_const, 0.0f) +
                                   s * AnalogComplex(denom_s_coeff, 0.0f) +
                                   s_squared * AnalogComplex(denom_s2_coeff, 0.0f);
        
        return numerator / denominator;
    }
    
    void transfer_function_simd(__m256 frequencies, __m256& magnitude_out, __m256& phase_out) const override {
        // SIMD implementation for 8-way parallel processing
        __m256 omega = analog_mul(_mm256_set1_ps(2.0f * 3.14159265f), frequencies);
        
        __m256 tom_pos = _mm256_set1_ps(get_parameter("tom_position"));
        __m256 conga_pos = _mm256_set1_ps(get_parameter("conga_position"));
        __m256 r1_values = tom_conga_resistance_simd(tom_pos);
        __m256 r2_values = tom_conga_resistance_simd(conga_pos);
        
        // For now, fall back to scalar for complex arithmetic
        alignas(32) float freq_array[8], mag_array[8], phase_array[8];
        _mm256_store_ps(freq_array, frequencies);
        
        for (int i = 0; i < 8; ++i) {
            AnalogComplex h = transfer_function(freq_array[i]);
            mag_array[i] = h.magnitude();
            phase_array[i] = h.phase();
        }
        
        magnitude_out = _mm256_load_ps(mag_array);
        phase_out = _mm256_load_ps(phase_array);
    }
    
    float center_frequency() const override {
        float r1 = tom_conga_resistance(get_parameter("tom_position"));
        float r2 = tom_conga_resistance(get_parameter("conga_position"));
        float c1 = get_parameter("c1");
        float c2 = get_parameter("c2");
        
        float denominator = analog_sqrt(analog_mul(analog_mul(analog_mul(r1, r2), c1), c2));
        return analog_div(1.0f, analog_mul(2.0f * 3.14159265f, denominator));
    }
    
    float quality_factor() const override {
        float r1 = tom_conga_resistance(get_parameter("tom_position"));
        float r2 = tom_conga_resistance(get_parameter("conga_position"));
        float c1 = get_parameter("c1");
        float c2 = get_parameter("c2");
        
        float r2_r1_sqrt = analog_sqrt(analog_div(r2, r1));
        float c_ratio_sum = analog_add(analog_sqrt(analog_div(c1, c2)), 
                                      analog_sqrt(analog_div(c2, c1)));
        return analog_div(r2_r1_sqrt, c_ratio_sum);
    }
};

// --- Circuit Primitive Factory ---
class CircuitFactory {
private:
    std::unordered_map<std::string, std::function<std::unique_ptr<CircuitPrimitive>()>> creators;
    
public:
    CircuitFactory() {
        // Register Dr. Krut's circuits
        register_primitive("bridged_t_filter", []() {
            return std::make_unique<BridgedTFilterPrimitive>();
        });
        
        // Future circuits from Dr. Krut's thesis would be registered here:
        // register_primitive("bridged_t_resonator", []() { ... });
        // register_primitive("nullor_amplifier", []() { ... });
        // register_primitive("twin_t_notch", []() { ... });
    }
    
    void register_primitive(const std::string& name, std::function<std::unique_ptr<CircuitPrimitive>()> creator) {
        creators[name] = creator;
    }
    
    std::unique_ptr<CircuitPrimitive> create(const std::string& name) const {
        auto it = creators.find(name);
        if (it != creators.end()) {
            return it->second();
        }
        return nullptr;
    }
    
    std::vector<std::string> get_available_primitives() const {
        std::vector<std::string> names;
        for (const auto& [name, creator] : creators) {
            names.push_back(name);
        }
        return names;
    }
};

// --- Circuit Chain for Complex Topologies ---
class CircuitChain {
private:
    std::vector<std::unique_ptr<CircuitPrimitive>> stages;
    std::string chain_name;
    
public:
    CircuitChain(const std::string& name) : chain_name(name) {}
    
    void add_stage(std::unique_ptr<CircuitPrimitive> stage) {
        stages.push_back(std::move(stage));
    }
    
    // Chain transfer function (multiply individual H(jω) functions)
    AnalogComplex transfer_function(float frequency) const {
        AnalogComplex result(1.0f, 0.0f);  // Start with unity
        
        for (const auto& stage : stages) {
            result = result * stage->transfer_function(frequency);
        }
        
        return result;
    }
    
    // SIMD version for chained analysis
    void transfer_function_simd(__m256 frequencies, __m256& magnitude_out, __m256& phase_out) const {
        if (stages.empty()) {
            magnitude_out = _mm256_set1_ps(1.0f);
            phase_out = _mm256_set1_ps(0.0f);
            return;
        }
        
        // Process each stage and multiply results
        __m256 total_mag = _mm256_set1_ps(1.0f);
        __m256 total_phase = _mm256_set1_ps(0.0f);
        
        for (const auto& stage : stages) {
            __m256 stage_mag, stage_phase;
            stage->transfer_function_simd(frequencies, stage_mag, stage_phase);
            
            total_mag = analog_mul(total_mag, stage_mag);
            total_phase = analog_add(total_phase, stage_phase);
        }
        
        magnitude_out = total_mag;
        phase_out = total_phase;
    }
    
    // Parameter control for the entire chain
    void set_stage_parameter(size_t stage_idx, const std::string& param_name, float value) {
        if (stage_idx < stages.size()) {
            stages[stage_idx]->set_parameter(param_name, value);
        }
    }
    
    float get_stage_parameter(size_t stage_idx, const std::string& param_name) const {
        if (stage_idx < stages.size()) {
            return stages[stage_idx]->get_parameter(param_name);
        }
        return 0.0f;
    }
    
    size_t get_stage_count() const { return stages.size(); }
    const CircuitPrimitive* get_stage(size_t idx) const {
        return (idx < stages.size()) ? stages[idx].get() : nullptr;
    }
};

// --- SIMD Batch Analyzer for Multiple Circuits ---
class BatchCircuitAnalyzer {
private:
    std::vector<std::unique_ptr<CircuitPrimitive>> circuits;
    
public:
    void add_circuit(std::unique_ptr<CircuitPrimitive> circuit) {
        circuits.push_back(std::move(circuit));
    }
    
    // Analyze up to 8 circuits in parallel across frequency range
    struct BatchResult {
        std::vector<std::vector<float>> frequencies;    // [circuit][freq_point]
        std::vector<std::vector<float>> magnitudes_db;  // [circuit][freq_point]
        std::vector<std::vector<float>> phases_deg;     // [circuit][freq_point]
    };
    
    BatchResult analyze_batch(float start_freq, float end_freq, int num_points) const {
        BatchResult result;
        result.frequencies.resize(circuits.size());
        result.magnitudes_db.resize(circuits.size());
        result.phases_deg.resize(circuits.size());
        
        // Generate frequency points
        std::vector<float> freqs;
        float log_start = analog_log(start_freq);
        float log_end = analog_log(end_freq);
        
        for (int i = 0; i < num_points; ++i) {
            float log_freq = analog_add(log_start, 
                           analog_mul(analog_div(static_cast<float>(i), 
                                     static_cast<float>(num_points - 1)),
                                     analog_sub(log_end, log_start)));
            freqs.push_back(analog_exp(log_freq));
        }
        
        // Analyze each circuit
        for (size_t circuit_idx = 0; circuit_idx < circuits.size(); ++circuit_idx) {
            const auto& circuit = circuits[circuit_idx];
            
            result.frequencies[circuit_idx] = freqs;
            result.magnitudes_db[circuit_idx].reserve(num_points);
            result.phases_deg[circuit_idx].reserve(num_points);
            
            // Process frequencies in SIMD batches
            int simd_batches = num_points / 8;
            int remainder = num_points % 8;
            
            for (int batch = 0; batch < simd_batches; ++batch) {
                alignas(32) float freq_batch[8];
                for (int i = 0; i < 8; ++i) {
                    freq_batch[i] = freqs[batch * 8 + i];
                }
                
                __m256 freq_vec = _mm256_load_ps(freq_batch);
                __m256 magnitude, phase;
                circuit->transfer_function_simd(freq_vec, magnitude, phase);
                
                alignas(32) float mag_result[8], phase_result[8];
                _mm256_store_ps(mag_result, magnitude);
                _mm256_store_ps(phase_result, phase);
                
                for (int i = 0; i < 8; ++i) {
                    float mag_db = analog_mul(20.0f, analog_log10(mag_result[i]));
                    float phase_deg = analog_mul(phase_result[i], 180.0f / 3.14159265f);
                    
                    result.magnitudes_db[circuit_idx].push_back(mag_db);
                    result.phases_deg[circuit_idx].push_back(phase_deg);
                }
            }
            
            // Handle remainder
            for (int i = 0; i < remainder; ++i) {
                int idx = simd_batches * 8 + i;
                AnalogComplex h = circuit->transfer_function(freqs[idx]);
                
                float mag_db = analog_mul(20.0f, analog_log10(h.magnitude()));
                float phase_deg = analog_mul(h.phase(), 180.0f / 3.14159265f);
                
                result.magnitudes_db[circuit_idx].push_back(mag_db);
                result.phases_deg[circuit_idx].push_back(phase_deg);
            }
        }
        
        return result;
    }
};

// --- Global factory instance ---
inline CircuitFactory& get_circuit_factory() {
    static CircuitFactory factory;
    return factory;
}

} // namespace woflang

// Example usage demonstrating the extensible architecture
inline void demo_circuit_factory() {
    using namespace woflang;
    
    auto& factory = get_circuit_factory();
    
    // Create a bridged-T filter
    auto filter = factory.create("bridged_t_filter");
    if (filter) {
        // Set Tom/Conga positions
        filter->set_parameter("tom_position", 0.3f);
        filter->set_parameter("conga_position", 0.7f);
        
        std::cout << "Filter created:\n";
        std::cout << "Center Frequency: " << filter->center_frequency() << " Hz\n";
        std::cout << "Quality Factor: " << filter->quality_factor() << "\n";
        
        // Test at 1kHz
        AnalogComplex response = filter->transfer_function(1000.0f);
        std::cout << "Response at 1kHz: " << response.magnitude() << " magnitude, " 
                  << response.phase() * 180.0f / 3.14159265f << "° phase\n";
    }
    
    // Create a circuit chain
    CircuitChain chain("test_chain");
    chain.add_stage(factory.create("bridged_t_filter"));
    chain.add_stage(factory.create("bridged_t_filter"));
    
    // Set parameters for each stage
    chain.set_stage_parameter(0, "tom_position", 0.2f);
    chain.set_stage_parameter(1, "tom_position", 0.8f);
    
    AnalogComplex chain_response = chain.transfer_function(1000.0f);
    std::cout << "Chain response: " << chain_response.magnitude() << "\n";
}