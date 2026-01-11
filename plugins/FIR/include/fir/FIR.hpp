#pragma once
#include "sp/ISignalProcessor.hpp"
#include "sp/export.hpp"
#include <vector>
#include <cmath>
#include <windows.h>

namespace fir {

enum class FilterType {
    LOWPASS,
    HIGHPASS,
    BANDPASS,
    BANDSTOP
};

enum class WindowType {
    RECTANGULAR,
    HAMMING,
    HANNING,
    BLACKMAN
};

struct FIRParams {
    FilterType type = FilterType::LOWPASS;
    WindowType window = WindowType::HAMMING;
    float cutoff_low = 0.1f;
    float cutoff_high = 0.4f;
    int order = 64;
    float gain = 1.0f;
};

class SP_API FIR final : public sp::ISignalProcessor {
public:
    FIR();
    ~FIR() override;
    
    void prepare(double fs, std::size_t block) override;
    void process(const float* in, float* out, std::size_t n) override;
    
    void set_params(const FIRParams& params);
    FIRParams get_params() const { return params_; }
    
    void design_filter(FilterType type, float cutoff_low, float cutoff_high, int order, WindowType window);
    
    const std::vector<float>& get_coefficients() const { return coeffs_; }

private:
    void compute_coefficients();
    void apply_window(std::vector<float>& coeffs, WindowType window);
    void load_eigen_api();
    
    // EIGEN math API
    using eig_dot_f32_t = int(*)(const float*, const float*, std::size_t, float*);
    
    HMODULE hEigen_ = nullptr;
    eig_dot_f32_t p_dot_ = nullptr;
    
    FIRParams params_;
    std::vector<float> coeffs_;
    std::vector<float> buffer_;
    std::size_t buffer_index_ = 0;
    
    double sample_rate_ = 48000.0;
    std::size_t block_size_ = 0;
};

} // namespace fir
