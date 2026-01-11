#include "iir/IIR.hpp"
#include "sp/Logger.hpp"
#include <cmath>
#include <algorithm>

using namespace iir;

namespace {
    constexpr float PI = 3.14159265358979323846f;
}

IIR::IIR() {
    compute_biquads();
    PLUGIN_INFO("IIR", "IIR instance created");
}

IIR::~IIR() {
    PLUGIN_INFO("IIR", "IIR instance destroyed");
}

void IIR::prepare(double fs, std::size_t block) {
    sample_rate_ = fs;
    block_size_ = block;
    
    // Recompute biquads for new sample rate
    compute_biquads();
    
    PLUGIN_INFO("IIR", "Prepared: fs=", fs, " Hz, block=", block, 
                ", sections=", biquads_.size(), ", type=", static_cast<int>(params_.type));
}

void IIR::process(const float* in, float* out, std::size_t n) {
    if (!in || !out || n == 0) return;
    
    if (biquads_.empty()) {
        // Passthrough if no filters
        std::copy(in, in + n, out);
        return;
    }
    
    // Process through cascade of biquad sections
    for (std::size_t i = 0; i < n; ++i) {
        float sample = in[i];
        
        // Process through each biquad section in series
        for (auto& bq : biquads_) {
            sample = bq.process(sample);
        }
        
        out[i] = sample;
    }
}

void IIR::set_params(const IIRParams& params) {
    params_ = params;
    compute_biquads();
    reset();
    
    PLUGIN_INFO("IIR", "Parameters updated: type=", static_cast<int>(params_.type),
                ", cutoff=", params_.cutoff, ", order=", params_.order);
}

void IIR::design_lowpass(float cutoff, int order, FilterDesign design) {
    params_.type = FilterType::LOWPASS;
    params_.design = design;
    params_.cutoff = cutoff;
    params_.order = order;
    compute_biquads();
}

void IIR::design_highpass(float cutoff, int order, FilterDesign design) {
    params_.type = FilterType::HIGHPASS;
    params_.design = design;
    params_.cutoff = cutoff;
    params_.order = order;
    compute_biquads();
}

void IIR::design_bandpass(float center, float bandwidth, int order) {
    params_.type = FilterType::BANDPASS;
    params_.cutoff = center;
    params_.bandwidth = bandwidth;
    params_.order = order;
    compute_biquads();
}

void IIR::design_peaking_eq(float center, float Q, float gain_db) {
    params_.type = FilterType::PEAKING;
    params_.cutoff = center;
    params_.Q = Q;
    params_.gain_db = gain_db;
    params_.order = 2; // Single biquad
    compute_biquads();
}

void IIR::design_low_shelf(float cutoff, float gain_db) {
    params_.type = FilterType::LOW_SHELF;
    params_.cutoff = cutoff;
    params_.gain_db = gain_db;
    params_.order = 2; // Single biquad
    compute_biquads();
}

void IIR::design_high_shelf(float cutoff, float gain_db) {
    params_.type = FilterType::HIGH_SHELF;
    params_.cutoff = cutoff;
    params_.gain_db = gain_db;
    params_.order = 2; // Single biquad
    compute_biquads();
}

void IIR::reset() {
    for (auto& bq : biquads_) {
        bq.reset();
    }
}

void IIR::compute_biquads() {
    biquads_.clear();
    
    // Normalized frequency: omega = 2*pi*fc/fs
    float omega = 2.0f * PI * params_.cutoff;
    
    // Number of biquad sections = order / 2 (rounded up)
    int num_sections = (params_.order + 1) / 2;
    
    switch (params_.type) {
        case FilterType::LOWPASS:
        case FilterType::HIGHPASS: {
            // For higher orders, create multiple cascaded biquads
            for (int i = 0; i < num_sections; ++i) {
                // Compute Q for Butterworth poles
                // For Butterworth: poles evenly distributed on unit circle
                float section_Q = params_.Q;
                if (params_.design == FilterDesign::BUTTERWORTH && num_sections > 1) {
                    float pole_angle = PI * (2.0f * static_cast<float>(i) + 1.0f) 
                                      / (2.0f * static_cast<float>(params_.order));
                    section_Q = 1.0f / (2.0f * std::cos(pole_angle));
                }
                
                BiquadSection bq;
                if (params_.type == FilterType::LOWPASS) {
                    bq = design_biquad_lowpass(omega, section_Q);
                } else {
                    bq = design_biquad_highpass(omega, section_Q);
                }
                biquads_.push_back(bq);
            }
            break;
        }
        
        case FilterType::BANDPASS:
        case FilterType::BANDSTOP: {
            float Q = params_.cutoff / params_.bandwidth;
            BiquadSection bq = design_biquad_bandpass(omega, Q);
            biquads_.push_back(bq);
            break;
        }
        
        case FilterType::PEAKING: {
            BiquadSection bq = design_biquad_peaking(omega, params_.Q, params_.gain_db);
            biquads_.push_back(bq);
            break;
        }
        
        case FilterType::LOW_SHELF: {
            BiquadSection bq = design_biquad_low_shelf(omega, params_.gain_db);
            biquads_.push_back(bq);
            break;
        }
        
        case FilterType::HIGH_SHELF: {
            BiquadSection bq = design_biquad_high_shelf(omega, params_.gain_db);
            biquads_.push_back(bq);
            break;
        }
    }
    
    PLUGIN_DEBUG("IIR", "Computed ", biquads_.size(), " biquad sections");
}

BiquadSection IIR::design_biquad_lowpass(float omega, float Q) {
    BiquadSection bq;
    
    float alpha = std::sin(omega) / (2.0f * Q);
    float cos_omega = std::cos(omega);
    
    float a0 = 1.0f + alpha;
    bq.b0 = (1.0f - cos_omega) / (2.0f * a0);
    bq.b1 = (1.0f - cos_omega) / a0;
    bq.b2 = bq.b0;
    bq.a1 = (-2.0f * cos_omega) / a0;
    bq.a2 = (1.0f - alpha) / a0;
    
    return bq;
}

BiquadSection IIR::design_biquad_highpass(float omega, float Q) {
    BiquadSection bq;
    
    float alpha = std::sin(omega) / (2.0f * Q);
    float cos_omega = std::cos(omega);
    
    float a0 = 1.0f + alpha;
    bq.b0 = (1.0f + cos_omega) / (2.0f * a0);
    bq.b1 = -(1.0f + cos_omega) / a0;
    bq.b2 = bq.b0;
    bq.a1 = (-2.0f * cos_omega) / a0;
    bq.a2 = (1.0f - alpha) / a0;
    
    return bq;
}

BiquadSection IIR::design_biquad_bandpass(float omega, float Q) {
    BiquadSection bq;
    
    float alpha = std::sin(omega) / (2.0f * Q);
    float cos_omega = std::cos(omega);
    
    float a0 = 1.0f + alpha;
    bq.b0 = alpha / a0;
    bq.b1 = 0.0f;
    bq.b2 = -alpha / a0;
    bq.a1 = (-2.0f * cos_omega) / a0;
    bq.a2 = (1.0f - alpha) / a0;
    
    return bq;
}

BiquadSection IIR::design_biquad_peaking(float omega, float Q, float gain_db) {
    BiquadSection bq;
    
    float A = std::pow(10.0f, gain_db / 40.0f);  // sqrt of linear gain
    float alpha = std::sin(omega) / (2.0f * Q);
    float cos_omega = std::cos(omega);
    
    float a0 = 1.0f + alpha / A;
    bq.b0 = (1.0f + alpha * A) / a0;
    bq.b1 = (-2.0f * cos_omega) / a0;
    bq.b2 = (1.0f - alpha * A) / a0;
    bq.a1 = bq.b1;  // Same as b1
    bq.a2 = (1.0f - alpha / A) / a0;
    
    return bq;
}

BiquadSection IIR::design_biquad_low_shelf(float omega, float gain_db) {
    BiquadSection bq;
    
    float A = std::pow(10.0f, gain_db / 40.0f);
    float cos_omega = std::cos(omega);
    float sin_omega = std::sin(omega);
    float beta = std::sqrt(A) / params_.Q;  // Shelf slope
    
    float a0 = (A + 1.0f) + (A - 1.0f) * cos_omega + beta * sin_omega;
    bq.b0 = (A * ((A + 1.0f) - (A - 1.0f) * cos_omega + beta * sin_omega)) / a0;
    bq.b1 = (2.0f * A * ((A - 1.0f) - (A + 1.0f) * cos_omega)) / a0;
    bq.b2 = (A * ((A + 1.0f) - (A - 1.0f) * cos_omega - beta * sin_omega)) / a0;
    bq.a1 = (-2.0f * ((A - 1.0f) + (A + 1.0f) * cos_omega)) / a0;
    bq.a2 = ((A + 1.0f) + (A - 1.0f) * cos_omega - beta * sin_omega) / a0;
    
    return bq;
}

BiquadSection IIR::design_biquad_high_shelf(float omega, float gain_db) {
    BiquadSection bq;
    
    float A = std::pow(10.0f, gain_db / 40.0f);
    float cos_omega = std::cos(omega);
    float sin_omega = std::sin(omega);
    float beta = std::sqrt(A) / params_.Q;
    
    float a0 = (A + 1.0f) - (A - 1.0f) * cos_omega + beta * sin_omega;
    bq.b0 = (A * ((A + 1.0f) + (A - 1.0f) * cos_omega + beta * sin_omega)) / a0;
    bq.b1 = (-2.0f * A * ((A - 1.0f) + (A + 1.0f) * cos_omega)) / a0;
    bq.b2 = (A * ((A + 1.0f) + (A - 1.0f) * cos_omega - beta * sin_omega)) / a0;
    bq.a1 = (2.0f * ((A - 1.0f) - (A + 1.0f) * cos_omega)) / a0;
    bq.a2 = ((A + 1.0f) - (A - 1.0f) * cos_omega - beta * sin_omega) / a0;
    
    return bq;
}

// Plugin exports
extern "C" {
    SP_API sp::ISignalProcessor* sp_create() { 
        return new IIR(); 
    }
    
    SP_API void sp_destroy(sp::ISignalProcessor* p) { 
        delete p; 
    }
    
    SP_API const char* sp_name() { 
        return "IIR"; 
    }
    
    SP_API int sp_version_major() { 
        return 1; 
    }
    
    SP_API int sp_version_minor() { 
        return 0; 
    }
}
