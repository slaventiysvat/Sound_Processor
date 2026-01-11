// FIR Filter Test Example
#include "fir/FIR.hpp"
#include <iostream>
#include <vector>
#include <cmath>

int main() {
    fir::FIR filter;
    
    // Design lowpass filter: cutoff @ 1kHz, fs = 48kHz
    fir::FIRParams params;
    params.type = fir::FilterType::LOWPASS;
    params.cutoff_low = 1000.0f / 24000.0f; // Normalized
    params.order = 64;
    params.window = fir::WindowType::HAMMING;
    
    filter.set_params(params);
    filter.prepare(48000.0, 256);
    
    // Generate test signal: 500Hz + 5kHz
    const size_t N = 1000;
    std::vector<float> input(N);
    std::vector<float> output(N);
    
    for (size_t i = 0; i < N; ++i) {
        float t = static_cast<float>(i) / 48000.0f;
        // 500Hz (should pass) + 5kHz (should be attenuated)
        input[i] = std::sin(2.0f * 3.14159f * 500.0f * t) 
                 + 0.5f * std::sin(2.0f * 3.14159f * 5000.0f * t);
    }
    
    // Process
    filter.process(input.data(), output.data(), N);
    
    // Print results
    std::cout << "Input peak: " << *std::max_element(input.begin(), input.end()) << "\n";
    std::cout << "Output peak: " << *std::max_element(output.begin(), output.end()) << "\n";
    
    return 0;
}
