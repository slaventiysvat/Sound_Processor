// RLS Echo Cancellation Example
#include "rls/RLS.hpp"
#include "sp/Logger.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <random>

int main() {
    sp::Logger::instance().set_level(sp::LogLevel::INFO);
    
    const size_t N = 2000;        // 2000 samples
    const float fs = 48000.0f;
    const int echo_delay = 50;    // 50 samples delay (~1ms @ 48kHz)
    
    // Create RLS echo canceller
    rls::RLS echo_canceller;
    rls::RLSParams params;
    params.order = 128;           // Enough to model 128/48000 = 2.7ms echo path
    params.lambda = 0.99f;
    params.delta = 0.01f;
    echo_canceller.set_params(params);
    echo_canceller.prepare(fs, 256);
    
    // Generate test signals
    std::vector<float> far_end(N);      // Far-end speech (speaker output)
    std::vector<float> echo_path(N);    // Simulated echo
    std::vector<float> near_end(N);     // Near-end = echo + noise
    std::vector<float> output(N);       // RLS output (should remove echo)
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> noise(0.0f, 0.01f);
    
    // Simulate far-end speech (sine wave)
    for (size_t i = 0; i < N; ++i) {
        float t = static_cast<float>(i) / fs;
        far_end[i] = 0.5f * std::sin(2.0f * 3.14159f * 440.0f * t); // 440 Hz
    }
    
    // Simulate echo path (delayed + attenuated far-end)
    for (size_t i = 0; i < N; ++i) {
        if (i >= echo_delay) {
            echo_path[i] = 0.3f * far_end[i - echo_delay]; // 30% echo
        }
    }
    
    // Near-end = echo + noise
    for (size_t i = 0; i < N; ++i) {
        near_end[i] = echo_path[i] + noise(gen);
    }
    
    // Process with RLS
    echo_canceller.process_adaptive(
        far_end.data(),
        near_end.data(),
        output.data(),
        N,
        false // Always adapt
    );
    
    // Compute error reduction
    float echo_power = 0.0f;
    float output_power = 0.0f;
    for (size_t i = echo_delay; i < N; ++i) {
        echo_power += near_end[i] * near_end[i];
        output_power += output[i] * output[i];
    }
    echo_power /= (N - echo_delay);
    output_power /= (N - echo_delay);
    
    float echo_rms = std::sqrt(echo_power);
    float output_rms = std::sqrt(output_power);
    float suppression_db = 20.0f * std::log10(echo_rms / (output_rms + 1e-10f));
    
    std::cout << "=== RLS Echo Cancellation Test ===\n";
    std::cout << "Echo RMS: " << echo_rms << "\n";
    std::cout << "Output RMS: " << output_rms << "\n";
    std::cout << "Echo suppression: " << suppression_db << " dB\n";
    std::cout << "Expected: >20 dB for good cancellation\n";
    
    return 0;
}
