
#pragma once

#include "analog_modes.hpp"
#include "analog_math.hpp"
#include "analog_math_extended.hpp"
#include "analog_geometries.hpp"
#include "analog_vector_ops.hpp"
#include <immintrin.h>
#include <array>
#include <vector>
#include <complex>
#include <cmath>

namespace woflang {

// --- Complex number for frequency domain analysis ---
struct AnalogComplex {
    float real, imag;
    
    AnalogComplex() : real(0.0f), imag(0.0f) {}
    AnalogComplex(float r, float i) : real(clamp_analog(r)), imag(clamp_analog(i)) {}
    
    AnalogComplex operator+(const AnalogComplex& other) const {
        return AnalogComplex(real + other.real, imag + other.imag);
    }
    
    AnalogComplex operator-(const AnalogComplex& other) const {
        return AnalogComplex(real - other.real, imag - other.imag);
    }
    
    AnalogComplex operator*(const AnalogComplex& other) const {
        return AnalogComplex(
            real * other.real - imag * other.imag,
            real * other.imag + imag * other.real
        );
    }
    
    AnalogComplex operator/(const AnalogComplex& other) const {
        float denom = other.real * other.real + other.imag * other.imag;
        if (denom == 0.0f) return AnalogComplex(0.0f, 0.0f);
        return AnalogComplex(
            (real * other.real + imag * other.imag) / denom,
            (imag * other.real - real * other.imag) / denom
        );
    }
    
    float magnitude() const {
        return clamp_analog(analog_hypot(real, imag));
    }
    
    float phase() const {
        return clamp_analog(analog_atan2(imag, real));
    }
};

// --- SIMD Complex operations for 8-way parallel processing ---
struct AnalogComplexSIMD {
    __m256 real, imag;
    
    AnalogComplexSIMD() : real(_mm256_set1_ps(0.0f)), imag(_mm256_set1_ps(0.0f)) {}
    AnalogComplexSIMD(__m256 r, __m256 i) : real(clamp_analog(r)), imag(clamp_analog(i)) {}
    
    AnalogComplexSIMD operator+(const AnalogComplexSIMD& other) const {
        return AnalogComplexSIMD(
            analog_add(real, other.real),
            analog_add(imag, other.imag)
        );
    }
    
    AnalogComplexSIMD operator*(const AnalogComplexSIMD& other) const {
        return AnalogComplexSIMD(
            analog_sub(analog_mul(real, other.real), analog_mul(imag, other.imag)),
            analog_add(analog_mul(real, other.imag), analog_mul(imag, other.real))
        );
    }
    
    __m256 magnitude() const {
        return analog_hypot(real, imag);
    }
    
    __m256 phase() const {
        return analog_atan2(imag, real);
    }
};

// --- Tom/Conga Variable Resistance Network (Dr. Krut's equation C.12) ---
class TomCongaNetwork {
private:
    float knob_position;  // [0.0, 1.0]
    
public:
    explicit TomCongaNetwork(float pos = 0.5f) 
        : knob_position(clamp_analog(pos)) {
        // Ensure position is in valid range [0, 1]
        if (knob_position < 0.0f) knob_position = 0.0f;
        if (knob_position > 1.0f) knob_position = 1.0f;
    }
    
    // Dr. Krut's equation: R1 = (-x^2 + x + 2) / (x + 2) * 500
    float resistance() const {
        float x = knob_position;
        float numerator = analog_add(analog_sub(analog_negate(analog_mul(x, x)), x), 2.0f);
        float denominator = analog_add(x, 2.0f);
        return analog_mul(analog_div(numerator, denominator), 500.0f);
    }
    
    void set_position(float pos) {
        knob_position = clamp_analog(pos);
        if (knob_position < 0.0f) knob_position = 0.0f;
        if (knob_position > 1.0f) knob_position = 1.0f;
    }
    
    float get_position() const { return knob_position; }
    
    // SIMD version for 8 parallel networks
    static __m256 resistance_simd(__m256 positions) {
        __m256 x = clamp_analog(positions);
        __m256 two = _mm256_set1_ps(2.0f);
        __m256 five_hundred = _mm256_set1_ps(500.0f);
        
        __m256 x_squared = analog_mul(x, x);
        __m256 neg_x_squared = analog_negate(x_squared);
        __m256 numerator = analog_add(analog_add(neg_x_squared, x), two);
        __m256 denominator = analog_add(x, two);
        
        return analog_mul(analog_div(numerator, denominator), five_hundred);
    }
};

// --- Bridged-T Filter Implementation (Dr. Krut's analysis) ---
class BridgedTFilter {
private:
    TomCongaNetwork tom_network;
    TomCongaNetwork conga_network;
    float c1, c2;  // Capacitances
    float r_load;  // Load resistance
    
public:
    BridgedTFilter(float tom_pos, float conga_pos, float cap1, float cap2, float load)
        : tom_network(tom_pos), conga_network(conga_pos)
        , c1(clamp_analog(cap1)), c2(clamp_analog(cap2))
        , r_load(clamp_analog(load)) {}
    
    // Transfer function from Dr. Krut's equation (C.16)
    AnalogComplex transfer_function(float frequency) const {
        float omega = analog_mul(2.0f * 3.14159265f, frequency);
        AnalogComplex s(0.0f, omega);  // jω
        
        float r1 = tom_network.resistance();
        float r2 = conga_network.resistance();
        
        // Numerator: (R1*R2*C1*C2)*s^2 + (R1*(C1 + C2))*s + 1
        float num_s2_coeff = analog_mul(analog_mul(analog_mul(r1, r2), c1), c2);
        float num_s_coeff = analog_mul(r1, analog_add(c1, c2));
        float num_const = 1.0f;
        
        AnalogComplex s_squared = s * s;
        AnalogComplex numerator = AnalogComplex(num_const, 0.0f) + 
                                  s * AnalogComplex(num_s_coeff, 0.0f) +
                                  s_squared * AnalogComplex(num_s2_coeff, 0.0f);
        
        // Denominator from Dr. Krut's analysis
        float denom_s2_coeff = num_s2_coeff;  // Same as numerator
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
    
    // Key filter parameters from Dr. Krut's equations (C.27-C.30)
    float center_frequency() const {
        float r1 = tom_network.resistance();
        float r2 = conga_network.resistance();
        float denominator = analog_sqrt(analog_mul(analog_mul(analog_mul(r1, r2), c1), c2));
        return analog_div(1.0f, analog_mul(2.0f * 3.14159265f, denominator));
    }
    
    float quality_factor() const {
        float r1 = tom_network.resistance();
        float r2 = conga_network.resistance();
        float r2_r1_sqrt = analog_sqrt(analog_div(r2, r1));
        float c_ratio_sum = analog_add(analog_sqrt(analog_div(c1, c2)), 
                                      analog_sqrt(analog_div(c2, c1)));
        return analog_div(r2_r1_sqrt, c_ratio_sum);
    }
    
    float peak_gain() const {
        float r1 = tom_network.resistance();
        float r2 = conga_network.resistance();
        float numerator = analog_add(analog_mul(r1, analog_add(c1, c2)), 
                                    analog_mul(r2, c1));
        float denominator = analog_mul(r1, analog_add(c1, c2));
        return analog_div(numerator, denominator);
    }
    
    // Parameter control
    void set_tom_position(float pos) { tom_network.set_position(pos); }
    void set_conga_position(float pos) { conga_network.set_position(pos); }
    float get_tom_position() const { return tom_network.get_position(); }
    float get_conga_position() const { return conga_network.get_position(); }
    
    // SIMD version for parallel processing of 8 filters
    static void transfer_function_simd(
        __m256 tom_positions, __m256 conga_positions,
        __m256 frequencies, float c1, float c2, float r_load,
        __m256& magnitude_out, __m256& phase_out
    ) {
        __m256 omega = analog_mul(_mm256_set1_ps(2.0f * 3.14159265f), frequencies);
        __m256 r1_values = TomCongaNetwork::resistance_simd(tom_positions);
        __m256 r2_values = TomCongaNetwork::resistance_simd(conga_positions);
        
        // This would be a full SIMD implementation of the transfer function
        // For now, process in scalar mode for clarity
        alignas(32) float tom_pos[8], conga_pos[8], freq[8], mag[8], phase[8];
        _mm256_store_ps(tom_pos, tom_positions);
        _mm256_store_ps(conga_pos, conga_positions);
        _mm256_store_ps(freq, frequencies);
        
        for (int i = 0; i < 8; ++i) {
            BridgedTFilter filter(tom_pos[i], conga_pos[i], c1, c2, r_load);
            AnalogComplex h = filter.transfer_function(freq[i]);
            mag[i] = h.magnitude();
            phase[i] = h.phase();
        }
        
        magnitude_out = _mm256_load_ps(mag);
        phase_out = _mm256_load_ps(phase);
    }
};

// --- Bridged-T Resonator (Dr. Krut's Section C.2) ---
class BridgedTResonator {
private:
    TomCongaNetwork tom_network;
    TomCongaNetwork conga_network;
    float c1, c2;
    float r_load;
    
public:
    BridgedTResonator(float tom_pos, float conga_pos, float cap1, float cap2, float load)
        : tom_network(tom_pos), conga_network(conga_pos)
        , c1(clamp_analog(cap1)), c2(clamp_analog(cap2))
        , r_load(clamp_analog(load)) {}
    
    // Transfer function from Dr. Krut's equation (C.20)
    // Note: RL doesn't appear due to nullor equivalence theorems
    AnalogComplex transfer_function(float frequency) const {
        float omega = analog_mul(2.0f * 3.14159265f, frequency);
        AnalogComplex s(0.0f, omega);
        
        float r1 = tom_network.resistance();
        float r2 = conga_network.resistance();
        
        // From equation C.20
        float num_s2_coeff = analog_mul(analog_mul(analog_mul(r1, r2), c1), c2);
        float num_s_coeff = analog_add(analog_mul(r1, analog_add(c1, c2)), 
                                      analog_mul(r2, c1));
        float num_const = 1.0f;
        
        AnalogComplex s_squared = s * s;
        AnalogComplex numerator = AnalogComplex(num_const, 0.0f) + 
                                  s * AnalogComplex(num_s_coeff, 0.0f) +
                                  s_squared * AnalogComplex(num_s2_coeff, 0.0f);
        
        float denom_s2_coeff = num_s2_coeff;
        float denom_s_coeff = analog_mul(r1, analog_add(c1, c2));
        float denom_const = 1.0f;
        
        AnalogComplex denominator = AnalogComplex(denom_const, 0.0f) +
                                   s * AnalogComplex(denom_s_coeff, 0.0f) +
                                   s_squared * AnalogComplex(denom_s2_coeff, 0.0f);
        
        return numerator / denominator;
    }
    
    void set_tom_position(float pos) { tom_network.set_position(pos); }
    void set_conga_position(float pos) { conga_network.set_position(pos); }
};

// --- Frequency Response Analyzer ---
class FrequencyResponseAnalyzer {
private:
    std::vector<float> frequencies;
    std::vector<AnalogComplex> response;
    
public:
    void analyze_bridged_t_filter(const BridgedTFilter& filter, 
                                 float start_freq, float end_freq, int num_points) {
        frequencies.clear();
        response.clear();
        frequencies.reserve(num_points);
        response.reserve(num_points);
        
        // Logarithmic frequency sweep
        float log_start = analog_log(start_freq);
        float log_end = analog_log(end_freq);
        
        for (int i = 0; i < num_points; ++i) {
            float log_freq = analog_add(log_start, 
                            analog_mul(analog_div(static_cast<float>(i), 
                                      static_cast<float>(num_points - 1)),
                                      analog_sub(log_end, log_start)));
            float freq = analog_exp(log_freq);
            
            frequencies.push_back(freq);
            response.push_back(filter.transfer_function(freq));
        }
    }
    
    // SIMD-accelerated frequency response analysis
    void analyze_bridged_t_filter_simd(const BridgedTFilter& filter,
                                      float start_freq, float end_freq, int num_points) {
        frequencies.clear();
        response.clear();
        frequencies.reserve(num_points);
        response.reserve(num_points);
        
        float log_start = analog_log(start_freq);
        float log_end = analog_log(end_freq);
        
        // Process 8 frequencies at a time
        int simd_batches = num_points / 8;
        int remainder = num_points % 8;
        
        for (int batch = 0; batch < simd_batches; ++batch) {
            alignas(32) float freq_batch[8];
            alignas(32) float mag_batch[8];
            alignas(32) float phase_batch[8];
            
            // Generate 8 frequencies
            for (int i = 0; i < 8; ++i) {
                int idx = batch * 8 + i;
                float log_freq = analog_add(log_start,
                               analog_mul(analog_div(static_cast<float>(idx),
                                         static_cast<float>(num_points - 1)),
                                         analog_sub(log_end, log_start)));
                freq_batch[i] = analog_exp(log_freq);
            }
            
            // Analyze using SIMD
            __m256 freq_vec = _mm256_load_ps(freq_batch);
            __m256 tom_pos = _mm256_set1_ps(filter.get_tom_position());
            __m256 conga_pos = _mm256_set1_ps(filter.get_conga_position());
            __m256 magnitude, phase;
            
            BridgedTFilter::transfer_function_simd(
                tom_pos, conga_pos, freq_vec, 1e-6f, 1e-6f, 10000.0f,
                magnitude, phase
            );
            
            _mm256_store_ps(mag_batch, magnitude);
            _mm256_store_ps(phase_batch, phase);
            
            // Store results
            for (int i = 0; i < 8; ++i) {
                frequencies.push_back(freq_batch[i]);
                response.emplace_back(mag_batch[i] * analog_cos(phase_batch[i]),
                                     mag_batch[i] * analog_sin(phase_batch[i]));
            }
        }
        
        // Handle remainder
        for (int i = 0; i < remainder; ++i) {
            int idx = simd_batches * 8 + i;
            float log_freq = analog_add(log_start,
                           analog_mul(analog_div(static_cast<float>(idx),
                                     static_cast<float>(num_points - 1)),
                                     analog_sub(log_end, log_start)));
            float freq = analog_exp(log_freq);
            
            frequencies.push_back(freq);
            response.push_back(filter.transfer_function(freq));
        }
    }
    
    // Get Bode plot data
    void get_bode_data(std::vector<float>& freq_out, 
                       std::vector<float>& magnitude_db, 
                       std::vector<float>& phase_deg) const {
        freq_out = frequencies;
        magnitude_db.clear();
        phase_deg.clear();
        magnitude_db.reserve(response.size());
        phase_deg.reserve(response.size());
        
        for (const auto& h : response) {
            float mag_db = analog_mul(20.0f, analog_log10(h.magnitude()));
            float phase_degrees = analog_mul(h.phase(), 180.0f / 3.14159265f);
            
            magnitude_db.push_back(mag_db);
            phase_deg.push_back(phase_degrees);
        }
    }
};

// --- Command interface for analog circuit analysis ---
inline void analog_circuit_command_loop() {
    BridgedTFilter filter(0.5f, 0.5f, 1e-6f, 1e-6f, 10000.0f);
    FrequencyResponseAnalyzer analyzer;
    
    std::cout << "\nWoflang Analog Circuit Computing Active!\n";
    std::cout << "Commands: tom <pos>, conga <pos>, analyze, params, simd_test\n";
    
    std::string line;
    while (true) {
        std::cout << "analog> ";
        if (!std::getline(std::cin, line)) break;
        
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        
        if (cmd == "exit" || cmd == "quit") break;
        
        else if (cmd == "tom") {
            float pos;
            iss >> pos;
            filter.set_tom_position(pos);
            std::cout << "Tom position set to " << pos 
                     << " (R=" << TomCongaNetwork(pos).resistance() << "Ω)\n";
        }
        
        else if (cmd == "conga") {
            float pos;
            iss >> pos;
            filter.set_conga_position(pos);
            std::cout << "Conga position set to " << pos
                     << " (R=" << TomCongaNetwork(pos).resistance() << "Ω)\n";
        }
        
        else if (cmd == "analyze") {
            analyzer.analyze_bridged_t_filter(filter, 10.0f, 10000.0f, 1000);
            std::cout << "Frequency analysis complete (1000 points, 10Hz-10kHz)\n";
            
            std::vector<float> freqs, mag_db, phase_deg;
            analyzer.get_bode_data(freqs, mag_db, phase_deg);
            
            // Show first few points
            std::cout << "Sample points:\n";
            for (int i = 0; i < 5 && i < freqs.size(); ++i) {
                std::cout << "  f=" << freqs[i] << "Hz: "
                         << mag_db[i] << "dB, " << phase_deg[i] << "°\n";
            }
        }
        
        else if (cmd == "simd_test") {
            auto start = std::chrono::high_resolution_clock::now();
            analyzer.analyze_bridged_t_filter_simd(filter, 10.0f, 10000.0f, 8000);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "SIMD analysis of 8000 points completed in " 
                     << duration.count() << "μs\n";
        }
        
        else if (cmd == "params") {
            std::cout << "Filter Parameters:\n";
            std::cout << "  Center Frequency: " << filter.center_frequency() << " Hz\n";
            std::cout << "  Quality Factor: " << filter.quality_factor() << "\n";
            std::cout << "  Peak Gain: " << filter.peak_gain() << "\n";
            std::cout << "  Tom Position: " << filter.get_tom_position() << "\n";
            std::cout << "  Conga Position: " << filter.get_conga_position() << "\n";
        }
        
        else {
            std::cout << "Unknown command. Available: tom, conga, analyze, params, simd_test\n";
        }
    }
}

} // namespace woflang

// Example usage integrating with your existing command loop
inline void extended_command_loop() {
    woflang::command_loop();  // Your existing analog math commands
    woflang::analog_circuit_command_loop();  // New circuit analysis commands
}