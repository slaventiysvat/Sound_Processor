#include "fir/FIR.hpp"
#include "sp/Logger.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <filesystem>

using namespace fir;

namespace {
    constexpr float PI = 3.14159265358979323846f;
    
    // Sinc function: sin(pi*x) / (pi*x)
    float sinc(float x) {
        if (std::abs(x) < 1e-8f) return 1.0f;
        float pix = PI * x;
        return std::sin(pix) / pix;
    }
    
    std::filesystem::path dll_dir_of(HMODULE mod) {
        wchar_t buf[MAX_PATH];
        GetModuleFileNameW(mod, buf, MAX_PATH);
        return std::filesystem::path(buf).parent_path();
    }
}

FIR::FIR() {
    compute_coefficients();
    PLUGIN_INFO("FIR", "FIR instance created");
}

FIR::~FIR() {
    if (hEigen_) {
        FreeLibrary(hEigen_);
        hEigen_ = nullptr;
    }
    PLUGIN_INFO("FIR", "FIR instance destroyed");
}

void FIR::load_eigen_api() {
    if (hEigen_) return;
    
    HMODULE self = nullptr;
    if (!GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&dll_dir_of),
        &self)) {
        self = nullptr;
    }
    
    auto dir = dll_dir_of(self);
    auto path = dir / L"EIGEN.dll";
    
    hEigen_ = LoadLibraryW(path.c_str());
    if (!hEigen_) hEigen_ = LoadLibraryW(L"EIGEN.dll");
    
    if (!hEigen_) {
        PLUGIN_WARN("FIR", "Failed to load EIGEN.dll, using fallback convolution");
        return;
    }
    
    p_dot_ = reinterpret_cast<eig_dot_f32_t>(GetProcAddress(hEigen_, "eig_dot_f32"));
    
    if (p_dot_) {
        PLUGIN_INFO("FIR", "EIGEN dot product loaded successfully");
    } else {
        PLUGIN_WARN("FIR", "EIGEN dot function missing, using fallback");
    }
}

void FIR::prepare(double fs, std::size_t block) {
    sample_rate_ = fs;
    block_size_ = block;
    
    // Initialize circular buffer (size = number of taps)
    const size_t num_taps = params_.order + 1;
    buffer_.resize(num_taps, 0.0f);
    buffer_index_ = 0;
    
    // Recompute coefficients if needed
    compute_coefficients();
    load_eigen_api();
    
    PLUGIN_INFO("FIR", "Prepared: fs=", fs, " Hz, block=", block, 
                ", taps=", num_taps, ", type=", static_cast<int>(params_.type));
}

void FIR::process(const float* in, float* out, std::size_t n) {
    if (!in || !out || n == 0) return;
    
    const size_t num_taps = coeffs_.size();
    if (num_taps == 0) {
        // Passthrough if no coefficients
        std::copy(in, in + n, out);
        return;
    }
    
    // Temporary buffer for dot product (reversed order)
    std::vector<float> x_reversed(num_taps);
    
    for (std::size_t i = 0; i < n; ++i) {
        // Add new sample to circular buffer
        buffer_[buffer_index_] = in[i];
        
        // Prepare reversed buffer for dot product
        for (size_t j = 0; j < num_taps; ++j) {
            size_t buf_idx = (buffer_index_ + num_taps - j) % num_taps;
            x_reversed[j] = buffer_[buf_idx];
        }
        
        float result = 0.0f;
        
        // Use EIGEN dot product if available
        if (p_dot_) {
            if (!p_dot_(coeffs_.data(), x_reversed.data(), num_taps, &result)) {
                // Fallback to manual if EIGEN fails
                for (size_t j = 0; j < num_taps; ++j) {
                    result += coeffs_[j] * x_reversed[j];
                }
            }
        } else {
            // Manual dot product
            for (size_t j = 0; j < num_taps; ++j) {
                result += coeffs_[j] * x_reversed[j];
            }
        }
        
        out[i] = result * params_.gain;
        // Advance circular buffer index
        buffer_index_ = (buffer_index_ + 1) % num_taps;
    }
}

void FIR::set_params(const FIRParams& params) {
    params_ = params;
    compute_coefficients();
    
    // Resize buffer if order changed
    const size_t num_taps = params_.order + 1;
    if (buffer_.size() != num_taps) {
        buffer_.resize(num_taps, 0.0f);
        buffer_index_ = 0;
    }
    
    PLUGIN_INFO("FIR", "Parameters updated");
}

void FIR::design_filter(FilterType type, float cutoff_low, float cutoff_high, int order, WindowType window) {
    params_.type = type;
    params_.cutoff_low = cutoff_low;
    params_.cutoff_high = cutoff_high;
    params_.order = order;
    params_.window = window;
    
    compute_coefficients();
}

void FIR::compute_coefficients() {
    const int N = params_.order + 1; // Number of taps
    coeffs_.resize(N);
    
    const int M = params_.order;
    const float fc_low = params_.cutoff_low;   // Normalized cutoff (0.0 - 1.0)
    const float fc_high = params_.cutoff_high;
    
    // Design filter using windowed-sinc method
    switch (params_.type) {
        case FilterType::LOWPASS: {
            // h[n] = 2*fc*sinc(2*fc*(n - M/2))
            for (int n = 0; n < N; ++n) {
                float t = static_cast<float>(n) - static_cast<float>(M) / 2.0f;
                coeffs_[n] = 2.0f * fc_low * sinc(2.0f * fc_low * t);
            }
            break;
        }
        
        case FilterType::HIGHPASS: {
            // h[n] = delta[n - M/2] - 2*fc*sinc(2*fc*(n - M/2))
            for (int n = 0; n < N; ++n) {
                float t = static_cast<float>(n) - static_cast<float>(M) / 2.0f;
                float delta = (n == M / 2) ? 1.0f : 0.0f;
                coeffs_[n] = delta - 2.0f * fc_low * sinc(2.0f * fc_low * t);
            }
            break;
        }
        
        case FilterType::BANDPASS: {
            // h[n] = 2*fc_high*sinc(2*fc_high*t) - 2*fc_low*sinc(2*fc_low*t)
            for (int n = 0; n < N; ++n) {
                float t = static_cast<float>(n) - static_cast<float>(M) / 2.0f;
                coeffs_[n] = 2.0f * fc_high * sinc(2.0f * fc_high * t) 
                           - 2.0f * fc_low * sinc(2.0f * fc_low * t);
            }
            break;
        }
        
        case FilterType::BANDSTOP: {
            // h[n] = 2*fc_low*sinc(2*fc_low*t) - 2*fc_high*sinc(2*fc_high*t) + delta[n - M/2]
            for (int n = 0; n < N; ++n) {
                float t = static_cast<float>(n) - static_cast<float>(M) / 2.0f;
                float delta = (n == M / 2) ? 1.0f : 0.0f;
                coeffs_[n] = 2.0f * fc_low * sinc(2.0f * fc_low * t) 
                           - 2.0f * fc_high * sinc(2.0f * fc_high * t) + delta;
            }
            break;
        }
    }
    
    // Apply window function
    apply_window(coeffs_, params_.window);
    
    // Normalize coefficients (DC gain = 1.0 for lowpass)
    if (params_.type == FilterType::LOWPASS || params_.type == FilterType::BANDPASS) {
        float sum = std::accumulate(coeffs_.begin(), coeffs_.end(), 0.0f);
        if (std::abs(sum) > 1e-8f) {
            for (auto& c : coeffs_) {
                c /= sum;
            }
        }
    }
    
    PLUGIN_DEBUG("FIR", "Coefficients computed: ", N, " taps, type=", static_cast<int>(params_.type));
}

void FIR::apply_window(std::vector<float>& coeffs, WindowType window) {
    const size_t N = coeffs.size();
    
    for (size_t n = 0; n < N; ++n) {
        float w = 1.0f;
        
        switch (window) {
            case WindowType::RECTANGULAR:
                w = 1.0f;
                break;
                
            case WindowType::HAMMING:
                // w[n] = 0.54 - 0.46*cos(2*pi*n/(N-1))
                w = 0.54f - 0.46f * std::cos(2.0f * PI * static_cast<float>(n) / static_cast<float>(N - 1));
                break;
                
            case WindowType::HANNING:
                // w[n] = 0.5 - 0.5*cos(2*pi*n/(N-1))
                w = 0.5f - 0.5f * std::cos(2.0f * PI * static_cast<float>(n) / static_cast<float>(N - 1));
                break;
                
            case WindowType::BLACKMAN:
                // w[n] = 0.42 - 0.5*cos(2*pi*n/(N-1)) + 0.08*cos(4*pi*n/(N-1))
                {
                    float angle = 2.0f * PI * static_cast<float>(n) / static_cast<float>(N - 1);
                    w = 0.42f - 0.5f * std::cos(angle) + 0.08f * std::cos(2.0f * angle);
                }
                break;
        }
        
        coeffs[n] *= w;
    }
}

extern "C" {
    SP_API sp::ISignalProcessor* sp_create() { 
        return new FIR(); 
    }
    
    SP_API void sp_destroy(sp::ISignalProcessor* p) { 
        delete p; 
    }
    
    SP_API const char* sp_name() { 
        return "FIR"; 
    }
    
    SP_API int sp_version_major() { 
        return 1; 
    }
    
    SP_API int sp_version_minor() { 
        return 0; 
    }
}
