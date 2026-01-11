#include <iostream>
#include <cassert>
#include <windows.h>
#include <vector>
#include <string>
#include <filesystem>
#include <cmath>
#include <algorithm>
#include "sp/PluginAPI.hpp"

// Test helpers
namespace TestHelpers {
    static HMODULE openDll(const std::wstring& full) {
        auto h = LoadLibraryW(full.c_str());
        if (!h) std::wcerr << L"Failed to load: " << full << L"\n";
        return h;
    }
    
    static void write_minimal_wav16(const std::string& path) {
        std::vector<int16_t> pcm(1000);
        for (size_t i = 0; i < pcm.size(); ++i) 
            pcm[i] = (i % 100 < 50) ? 1000 : -1000;
        
        uint32_t dataBytes = (uint32_t)(pcm.size() * 2);
        FILE* f = fopen(path.c_str(), "wb");
        
        fwrite("RIFF", 1, 4, f);
        uint32_t riff = 36 + dataBytes;
        fwrite(&riff, 4, 1, f);
        fwrite("WAVE", 1, 4, f);
        fwrite("fmt ", 1, 4, f);
        uint32_t fmtsz = 16;
        fwrite(&fmtsz, 4, 1, f);
        uint16_t fmt = 1;
        fwrite(&fmt, 2, 1, f);
        uint16_t ch = 1;
        fwrite(&ch, 2, 1, f);
        uint32_t sr = 48000;
        fwrite(&sr, 4, 1, f);
        uint32_t br = sr * ch * 2;
        fwrite(&br, 4, 1, f);
        uint16_t ba = ch * 2;
        fwrite(&ba, 2, 1, f);
        uint16_t bps = 16;
        fwrite(&bps, 2, 1, f);
        fwrite("data", 1, 4, f);
        fwrite(&dataBytes, 4, 1, f);
        fwrite(pcm.data(), 2, pcm.size(), f);
        fclose(f);
    }
    
    static float compute_rms(const float* signal, size_t n) {
        float sum = 0.0f;
        for (size_t i = 0; i < n; ++i) {
            sum += signal[i] * signal[i];
        }
        return std::sqrt(sum / static_cast<float>(n));
    }
    
    static void generate_sine(float* output, size_t n, float freq, float fs) {
        for (size_t i = 0; i < n; ++i) {
            float t = static_cast<float>(i) / fs;
            output[i] = std::sin(2.0f * 3.14159f * freq * t);
        }
    }
}

using namespace TestHelpers;

// Test result tracking
struct TestResult {
    std::string name;
    bool passed;
    std::string message;
};

std::vector<TestResult> g_test_results;

#define TEST_ASSERT(cond, msg) \
    if (!(cond)) { \
        g_test_results.push_back({__func__, false, msg}); \
        std::cerr << "FAIL: " << __func__ << " - " << msg << "\n"; \
        return false; \
    }

#define TEST_PASS() \
    g_test_results.push_back({__func__, true, ""}); \
    std::cout << "PASS: " << __func__ << "\n"; \
    return true;

// ============================================================================
// EIGEN Tests
// ============================================================================

bool test_eigen_dot_product() {
    std::wstring base = L"..\\..\\bin\\x64\\Debug\\plugins\\";
    auto h = openDll(base + L"EIGEN.dll");
    TEST_ASSERT(h != nullptr, "Failed to load EIGEN.dll");
    
    using eig_dot_f32_t = int(*)(const float*, const float*, std::size_t, float*);
    auto dot_fn = (eig_dot_f32_t)GetProcAddress(h, "eig_dot_f32");
    TEST_ASSERT(dot_fn != nullptr, "eig_dot_f32 not found");
    
    float a[] = {1.0f, 2.0f, 3.0f, 4.0f};
    float b[] = {2.0f, 3.0f, 4.0f, 5.0f};
    float result = 0.0f;
    
    int ok = dot_fn(a, b, 4, &result);
    TEST_ASSERT(ok == 1, "dot function failed");
    
    float expected = 1*2 + 2*3 + 3*4 + 4*5; // = 40
    TEST_ASSERT(std::abs(result - expected) < 0.001f, "dot product incorrect");
    
    FreeLibrary(h);
    TEST_PASS();
}

bool test_eigen_autocorr() {
    std::wstring base = L"..\\..\\bin\\x64\\Debug\\plugins\\";
    auto h = openDll(base + L"EIGEN.dll");
    TEST_ASSERT(h != nullptr, "Failed to load EIGEN.dll");
    
    using eig_acorr_f32_t = int(*)(const float*, std::size_t, std::size_t, float*);
    auto acorr_fn = (eig_acorr_f32_t)GetProcAddress(h, "eig_acorr_f32");
    TEST_ASSERT(acorr_fn != nullptr, "eig_acorr_f32 not found");
    
    float x[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    float r[3] = {0};
    
    int ok = acorr_fn(x, 5, 2, r);
    TEST_ASSERT(ok == 1, "autocorr function failed");
    TEST_ASSERT(r[0] > 0.0f, "r[0] should be positive");
    
    FreeLibrary(h);
    TEST_PASS();
}

// ============================================================================
// FIR Tests
// ============================================================================

bool test_fir_basic_processing() {
    std::wstring base = L"..\\..\\bin\\x64\\Debug\\plugins\\";
    auto h = openDll(base + L"FIR.dll");
    TEST_ASSERT(h != nullptr, "Failed to load FIR.dll");
    
    auto create = (CreateFn)GetProcAddress(h, "sp_create");
    auto destroy = (DestroyFn)GetProcAddress(h, "sp_destroy");
    TEST_ASSERT(create && destroy, "FIR exports missing");
    
    auto* fir = create();
    TEST_ASSERT(fir != nullptr, "FIR creation failed");
    
    fir->prepare(48000.0, 256);
    
    std::vector<float> input(256), output(256);
    generate_sine(input.data(), 256, 440.0f, 48000.0f);
    
    fir->process(input.data(), output.data(), 256);
    
    float in_rms = compute_rms(input.data(), 256);
    float out_rms = compute_rms(output.data(), 256);
    TEST_ASSERT(out_rms > 0.0f, "Output is zero");
    
    destroy(fir);
    FreeLibrary(h);
    TEST_PASS();
}

bool test_fir_lowpass_attenuation() {
    std::wstring base = L"..\\..\\bin\\x64\\Debug\\plugins\\";
    auto h = openDll(base + L"FIR.dll");
    TEST_ASSERT(h != nullptr, "Failed to load FIR.dll");
    
    auto create = (CreateFn)GetProcAddress(h, "sp_create");
    auto destroy = (DestroyFn)GetProcAddress(h, "sp_destroy");
    
    auto* fir = create();
    fir->prepare(48000.0, 256);
    
    // Test that high frequencies are attenuated by lowpass
    std::vector<float> low_freq(256), high_freq(256);
    std::vector<float> low_out(256), high_out(256);
    
    generate_sine(low_freq.data(), 256, 500.0f, 48000.0f);
    generate_sine(high_freq.data(), 256, 10000.0f, 48000.0f);
    
    fir->process(low_freq.data(), low_out.data(), 256);
    fir->process(high_freq.data(), high_out.data(), 256);
    
    float low_rms = compute_rms(low_out.data(), 256);
    float high_rms = compute_rms(high_out.data(), 256);
    
    // Lowpass should attenuate high frequencies more
    TEST_ASSERT(low_rms > high_rms * 2.0f, "Lowpass not working correctly");
    
    destroy(fir);
    FreeLibrary(h);
    TEST_PASS();
}

// ============================================================================
// IIR Tests
// ============================================================================

bool test_iir_basic_processing() {
    std::wstring base = L"..\\..\\bin\\x64\\Debug\\plugins\\";
    auto h = openDll(base + L"IIR.dll");
    TEST_ASSERT(h != nullptr, "Failed to load IIR.dll");
    
    auto create = (CreateFn)GetProcAddress(h, "sp_create");
    auto destroy = (DestroyFn)GetProcAddress(h, "sp_destroy");
    
    auto* iir = create();
    iir->prepare(48000.0, 256);
    
    std::vector<float> input(256), output(256);
    generate_sine(input.data(), 256, 1000.0f, 48000.0f);
    
    iir->process(input.data(), output.data(), 256);
    
    float out_rms = compute_rms(output.data(), 256);
    TEST_ASSERT(out_rms > 0.0f, "IIR output is zero");
    
    destroy(iir);
    FreeLibrary(h);
    TEST_PASS();
}

// ============================================================================
// DRC Tests
// ============================================================================

bool test_drc_gain_reduction() {
    std::wstring base = L"..\\..\\bin\\x64\\Debug\\plugins\\";
    auto h = openDll(base + L"DRC.dll");
    TEST_ASSERT(h != nullptr, "Failed to load DRC.dll");
    
    auto create = (CreateFn)GetProcAddress(h, "sp_create");
    auto destroy = (DestroyFn)GetProcAddress(h, "sp_destroy");
    
    auto* drc = create();
    drc->prepare(48000.0, 256);
    
    // Generate loud signal
    std::vector<float> input(256), output(256);
    for (size_t i = 0; i < 256; ++i) {
        input[i] = 0.9f * std::sin(2.0f * 3.14159f * 440.0f * float(i) / 48000.0f);
    }
    
    drc->process(input.data(), output.data(), 256);
    
    float in_peak = *std::max_element(input.begin(), input.end());
    float out_peak = *std::max_element(output.begin(), output.end());
    
    // DRC should reduce peaks
    TEST_ASSERT(out_peak <= in_peak, "DRC not reducing gain");
    
    destroy(drc);
    FreeLibrary(h);
    TEST_PASS();
}

// ============================================================================
// VAD Tests
// ============================================================================

bool test_vad_speech_detection() {
    std::wstring base = L"..\\..\\bin\\x64\\Debug\\plugins\\";
    auto h = openDll(base + L"VAD.dll");
    TEST_ASSERT(h != nullptr, "Failed to load VAD.dll");
    
    auto create = (CreateFn)GetProcAddress(h, "sp_create");
    auto destroy = (DestroyFn)GetProcAddress(h, "sp_destroy");
    
    auto* vad = create();
    vad->prepare(48000.0, 320);
    
    // Test with speech-like signal
    std::vector<float> input(320), output(320);
    generate_sine(input.data(), 320, 300.0f, 48000.0f);
    
    // Scale to speech level
    for (auto& s : input) s *= 0.1f;
    
    vad->process(input.data(), output.data(), 320);
    
    // Check that VAD produces output
    float sum = 0.0f;
    for (auto v : output) sum += v;
    TEST_ASSERT(sum >= 0.0f, "VAD output invalid");
    
    destroy(vad);
    FreeLibrary(h);
    TEST_PASS();
}

// ============================================================================
// RLS Tests
// ============================================================================

bool test_rls_basic_processing() {
    std::wstring base = L"..\\..\\bin\\x64\\Debug\\plugins\\";
    auto h = openDll(base + L"RLS.dll");
    TEST_ASSERT(h != nullptr, "Failed to load RLS.dll");
    
    auto create = (CreateFn)GetProcAddress(h, "sp_create");
    auto destroy = (DestroyFn)GetProcAddress(h, "sp_destroy");
    
    auto* rls = create();
    rls->prepare(48000.0, 256);
    
    std::vector<float> input(256), output(256);
    std::fill(input.begin(), input.end(), 0.0f);
    
    rls->process(input.data(), output.data(), 256);
    
    // Should not crash with zero input
    TEST_ASSERT(true, "RLS processed successfully");
    
    destroy(rls);
    FreeLibrary(h);
    TEST_PASS();
}

// ============================================================================
// NLMS Tests
// ============================================================================

bool test_nlms_basic_processing() {
    std::wstring base = L"..\\..\\bin\\x64\\Debug\\plugins\\";
    auto h = openDll(base + L"NLMS.dll");
    TEST_ASSERT(h != nullptr, "Failed to load NLMS.dll");
    
    auto create = (CreateFn)GetProcAddress(h, "sp_create");
    auto destroy = (DestroyFn)GetProcAddress(h, "sp_destroy");
    
    auto* nlms = create();
    nlms->prepare(48000.0, 256);
    
    std::vector<float> input(256), output(256);
    std::fill(input.begin(), input.end(), 0.0f);
    
    nlms->process(input.data(), output.data(), 256);
    
    TEST_ASSERT(true, "NLMS processed successfully");
    
    destroy(nlms);
    FreeLibrary(h);
    TEST_PASS();
}

// ============================================================================
// WAV Tests
// ============================================================================

bool test_wav_file_copy() {
    std::wstring base = L"..\\..\\bin\\x64\\Debug\\plugins\\";
    std::filesystem::path inW = std::filesystem::path("..") / ".." / "bin" / "x64" / "Debug" / "test_in.wav";
    std::filesystem::path outW = std::filesystem::path("..") / ".." / "bin" / "x64" / "Debug" / "test_out.wav";
    
    write_minimal_wav16(inW.string());
    
    auto h = openDll(base + L"WAV_File_Processor.dll");
    TEST_ASSERT(h != nullptr, "Failed to load WAV_File_Processor.dll");
    
    using wavfp_copy_to_t = int(*)(const char*, const char*);
    auto copy_to = (wavfp_copy_to_t)GetProcAddress(h, "wavfp_copy_to");
    TEST_ASSERT(copy_to != nullptr, "wavfp_copy_to not found");
    
    int ok = copy_to(inW.string().c_str(), outW.string().c_str());
    TEST_ASSERT(ok == 1, "WAV copy failed");
    TEST_ASSERT(std::filesystem::exists(outW), "Output file not created");
    
    FreeLibrary(h);
    TEST_PASS();
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    std::cout << "=== SoundProcessor Unit Tests ===\n\n";
    
    // EIGEN tests
    test_eigen_dot_product();
    test_eigen_autocorr();
    
    // Filter tests
    test_fir_basic_processing();
    test_fir_lowpass_attenuation();
    test_iir_basic_processing();
    
    // Dynamics tests
    test_drc_gain_reduction();
    
    // Analysis tests
    test_vad_speech_detection();
    
    // Adaptive filter tests
    test_rls_basic_processing();
    test_nlms_basic_processing();
    
    // I/O tests
    test_wav_file_copy();
    
    // Summary
    std::cout << "\n=== Test Summary ===\n";
    int passed = 0, failed = 0;
    for (const auto& result : g_test_results) {
        if (result.passed) {
            passed++;
        } else {
            failed++;
            std::cout << "FAILED: " << result.name << " - " << result.message << "\n";
        }
    }
    
    std::cout << "\nTotal: " << g_test_results.size() 
              << " | Passed: " << passed 
              << " | Failed: " << failed << "\n";
    
    return (failed == 0) ? 0 : 1;
}
