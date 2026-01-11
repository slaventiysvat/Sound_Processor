// NLMS Echo Cancellation Test
#include "nlms/NLMS.hpp"
#include "sp/Logger.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

int main() {
    sp::Logger::instance().set_level(sp::LogLevel::INFO);
    
    const size_t N = 5000;        // 5000 samples (~100ms @ 48kHz)
    const float fs = 48000.0f;
    const int echo_delay = 100;   // 100 samples delay (~2ms)
    
    // Create NLMS echo canceller
    nlms::NLMS echo_canceller;
    nlms::NLMSParams params;
    params.order = 256;           // Enough for ~5ms echo path
    params.mu = 0.5f;             // Fast adaptation
    params.epsilon = 1e-6f;
    params.gamma = 1.0f;
    echo_canceller.set_params(params);
    echo_canceller.prepare(fs, 256);
    
    // Generate test signals
    std::vector<float> far_end(N);
    std::vector<float> echo_path(N);
    std::vector<float> near_end(N);
    std::vector<float> output(N);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> noise(0.0f, 0.01f);
    
    // Far-end: mixture of frequencies
    for (size_t i = 0; i < N; ++i) {
        float t = static_cast<float>(i) / fs;
        far_end[i] = 0.3f * std::sin(2.0f * 3.14159f * 300.0f * t)  // 300 Hz
                   + 0.2f * std::sin(2.0f * 3.14159f * 1000.0f * t) // 1 kHz
                   + 0.1f * std::sin(2.0f * 3.14159f * 2500.0f * t); // 2.5 kHz
    }
    
    // Simulate echo (delayed + attenuated + multi-path)
    for (size_t i = 0; i < N; ++i) {
        echo_path[i] = 0.0f;
        // Direct echo
        if (i >= echo_delay) {
            echo_path[i] += 0.4f * far_end[i - echo_delay];
        }
        // Secondary reflection
        if (i >= echo_delay + 50) {
            echo_path[i] += 0.2f * far_end[i - echo_delay - 50];
        }
    }
    
    // Near-end = echo + noise
    for (size_t i = 0; i < N; ++i) {
        near_end[i] = echo_path[i] + noise(gen);
    }
    
    // Process with NLMS
    echo_canceller.process_adaptive(
        far_end.data(),
        near_end.data(),
        output.data(),
        N,
        false
    );
    
    // Analyze convergence over time
    const size_t segments = 10;
    const size_t seg_size = N / segments;
    
    std::cout << "=== NLMS Echo Cancellation Convergence ===\n";
    std::cout << "Samples: " << N << " @ " << fs << " Hz\n";
    std::cout << "Filter order: " << params.order << "\n";
    std::cout << "Step size (mu): " << params.mu << "\n\n";
    
    for (size_t seg = 0; seg < segments; ++seg) {
        size_t start = seg * seg_size;
        size_t end = start + seg_size;
        
        float echo_power = 0.0f;
        float output_power = 0.0f;
        
        for (size_t i = start; i < end; ++i) {
            echo_power += near_end[i] * near_end[i];
            output_power += output[i] * output[i];
        }
        
        echo_power /= seg_size;
        output_power /= seg_size;
        
        float echo_rms = std::sqrt(echo_power);
        float output_rms = std::sqrt(output_power);
        float suppression_db = 20.0f * std::log10(echo_rms / (output_rms + 1e-10f));
        
        std::cout << "Segment " << seg << " (" 
                  << (start * 1000 / static_cast<int>(fs)) << "-" 
                  << (end * 1000 / static_cast<int>(fs)) << " ms): "
                  << suppression_db << " dB suppression\n";
    }
    
    // Overall performance
    float total_echo_power = 0.0f;
    float total_output_power = 0.0f;
    
    // Skip first 1000 samples (convergence period)
    for (size_t i = 1000; i < N; ++i) {
        total_echo_power += near_end[i] * near_end[i];
        total_output_power += output[i] * output[i];
    }
    
    total_echo_power /= (N - 1000);
    total_output_power /= (N - 1000);
    
    float final_suppression = 20.0f * std::log10(
        std::sqrt(total_echo_power) / (std::sqrt(total_output_power) + 1e-10f)
    );
    
    std::cout << "\n=== Final Results (after convergence) ===\n";
    std::cout << "Echo suppression: " << final_suppression << " dB\n";
    std::cout << "Expected: >15 dB for NLMS (fast convergence)\n";
    std::cout << "Compare: RLS typically achieves >20 dB but slower computation\n";
    
    return 0;
}
