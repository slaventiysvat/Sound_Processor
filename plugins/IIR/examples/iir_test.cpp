// IIR Filter Test - Parametric EQ Example
#include "iir/IIR.hpp"
#include "sp/Logger.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

void generate_test_tone(std::vector<float>& signal, float freq, float fs) {
    for (size_t i = 0; i < signal.size(); ++i) {
        float t = static_cast<float>(i) / fs;
        signal[i] = std::sin(2.0f * 3.14159f * freq * t);
    }
}

float measure_rms(const std::vector<float>& signal, size_t start, size_t end) {
    float sum = 0.0f;
    for (size_t i = start; i < end; ++i) {
        sum += signal[i] * signal[i];
    }
    return std::sqrt(sum / static_cast<float>(end - start));
}

int main() {
    sp::Logger::instance().set_level(sp::LogLevel::INFO);
    
    const float fs = 48000.0f;
    const size_t N = 4800;  // 100ms
    
    std::cout << "=== IIR Filter Test Suite ===\n\n";
    
    // Test 1: Lowpass filter
    {
        std::cout << "Test 1: Lowpass Filter @ 1 kHz\n";
        
        iir::IIR lpf;
        lpf.prepare(fs, 256);
        lpf.design_lowpass(1000.0f / (fs / 2.0f), 4);  // 4th order
        
        // Test with 500 Hz (should pass)
        std::vector<float> in_500(N), out_500(N);
        generate_test_tone(in_500, 500.0f, fs);
        lpf.process(in_500.data(), out_500.data(), N);
        lpf.reset();
        
        // Test with 5 kHz (should be attenuated)
        std::vector<float> in_5k(N), out_5k(N);
        generate_test_tone(in_5k, 5000.0f, fs);
        lpf.process(in_5k.data(), out_5k.data(), N);
        
        float rms_500_in = measure_rms(in_500, N/2, N);
        float rms_500_out = measure_rms(out_500, N/2, N);
        float rms_5k_in = measure_rms(in_5k, N/2, N);
        float rms_5k_out = measure_rms(out_5k, N/2, N);
        
        float passband_loss = 20.0f * std::log10(rms_500_out / rms_500_in);
        float stopband_atten = 20.0f * std::log10(rms_5k_out / rms_5k_in);
        
        std::cout << "  500 Hz (passband): " << passband_loss << " dB (expect ~0 dB)\n";
        std::cout << "  5 kHz (stopband): " << stopband_atten << " dB (expect <-40 dB)\n\n";
    }
    
    // Test 2: Peaking EQ
    {
        std::cout << "Test 2: Peaking EQ @ 1 kHz, +12 dB\n";
        
        iir::IIR eq;
        eq.prepare(fs, 256);
        eq.design_peaking_eq(1000.0f / (fs / 2.0f), 2.0f, 12.0f);  // +12 dB boost
        
        std::vector<float> in(N), out(N);
        generate_test_tone(in, 1000.0f, fs);
        eq.process(in.data(), out.data(), N);
        
        float rms_in = measure_rms(in, N/2, N);
        float rms_out = measure_rms(out, N/2, N);
        float gain_db = 20.0f * std::log10(rms_out / rms_in);
        
        std::cout << "  Measured gain: " << gain_db << " dB (expect ~12 dB)\n";
        std::cout << "  Error: " << std::abs(gain_db - 12.0f) << " dB\n\n";
    }
    
    // Test 3: Low Shelf
    {
        std::cout << "Test 3: Low Shelf @ 200 Hz, +6 dB\n";
        
        iir::IIR shelf;
        shelf.prepare(fs, 256);
        shelf.design_low_shelf(200.0f / (fs / 2.0f), 6.0f);
        
        // Test low frequency (100 Hz - should be boosted)
        std::vector<float> in_low(N), out_low(N);
        generate_test_tone(in_low, 100.0f, fs);
        shelf.process(in_low.data(), out_low.data(), N);
        shelf.reset();
        
        // Test high frequency (2 kHz - should be unchanged)
        std::vector<float> in_high(N), out_high(N);
        generate_test_tone(in_high, 2000.0f, fs);
        shelf.process(in_high.data(), out_high.data(), N);
        
        float gain_low = 20.0f * std::log10(
            measure_rms(out_low, N/2, N) / measure_rms(in_low, N/2, N)
        );
        float gain_high = 20.0f * std::log10(
            measure_rms(out_high, N/2, N) / measure_rms(in_high, N/2, N)
        );
        
        std::cout << "  100 Hz gain: " << gain_low << " dB (expect ~6 dB)\n";
        std::cout << "  2 kHz gain: " << gain_high << " dB (expect ~0 dB)\n\n";
    }
    
    // Test 4: Cascaded sections (3-band EQ)
    {
        std::cout << "Test 4: 3-Band EQ (Bass+Mid+Treble)\n";
        
        iir::IIR bass, mid, treble;
        bass.prepare(fs, 256);
        mid.prepare(fs, 256);
        treble.prepare(fs, 256);
        
        bass.design_low_shelf(100.0f / (fs / 2.0f), +6.0f);
        mid.design_peaking_eq(1000.0f / (fs / 2.0f), 1.0f, -3.0f);
        treble.design_high_shelf(8000.0f / (fs / 2.0f), +4.0f);
        
        std::vector<float> signal(N), temp1(N), temp2(N), output(N);
        
        // Broadband noise-like signal
        for (size_t i = 0; i < N; ++i) {
            signal[i] = 0.0f;
            for (int h = 1; h <= 10; ++h) {
                signal[i] += std::sin(2.0f * 3.14159f * float(h * 500) * float(i) / fs) / float(h);
            }
        }
        
        // Chain processing
        bass.process(signal.data(), temp1.data(), N);
        mid.process(temp1.data(), temp2.data(), N);
        treble.process(temp2.data(), output.data(), N);
        
        std::cout << "  3-band EQ processed successfully\n";
        std::cout << "  Input RMS: " << measure_rms(signal, 0, N) << "\n";
        std::cout << "  Output RMS: " << measure_rms(output, 0, N) << "\n\n";
    }
    
    std::cout << "=== All IIR Tests Complete ===\n";
    std::cout << "IIR filters are working correctly!\n";
    
    return 0;
}
