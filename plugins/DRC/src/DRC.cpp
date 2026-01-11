#include "drc/DRC.hpp"
#include "sp/Logger.hpp"
#include <cmath>
#include <algorithm>

using namespace drc;

DRC::DRC() {
    compute_curve_coefficients();
    PLUGIN_INFO("DRC", "DRC instance created");
}

DRC::~DRC() {
    PLUGIN_INFO("DRC", "DRC instance destroyed");
}

void DRC::prepare(double fs, std::size_t block) {
    sample_rate_ = fs;
    block_size_ = block;
    
    const float attack_samples = params_.attack_time_ms * static_cast<float>(fs) / 1000.0f;
    const float release_samples = params_.release_time_ms * static_cast<float>(fs) / 1000.0f;
    
    attack_coeff_ = 1.0f - std::exp(-2.2f / attack_samples);
    release_coeff_ = 1.0f - std::exp(-2.2f / release_samples);
    
    envelope_state_ = 0.0f;
    
    if (params_.envelope_mode == EnvelopeMode::RMS) {
        rms_buffer_.resize(params_.rms_window_size, 0.0f);
        rms_index_ = 0;
    }
    
    compute_curve_coefficients();
    
    PLUGIN_INFO("DRC", "Prepared: fs=", fs, " Hz, block=", block, 
                ", attack=", params_.attack_time_ms, "ms, release=", params_.release_time_ms, "ms");
}

void DRC::process(const float* in, float* out, std::size_t n) {
    if (!in || !out || n == 0) return;
    
    float total_gr_db = 0.0f; // Gain reduction tracking
    size_t gr_count = 0;
    
    for (std::size_t i = 0; i < n; ++i) {
        float envelope = 0.0f;
        if (params_.envelope_mode == EnvelopeMode::PEAK) {
            envelope = process_peak_envelope(in[i]);
        } else {
            envelope = process_peak_envelope(in[i]);
        }
        
        float gain = compute_gain_from_curve(envelope);
        
        // Track gain reduction
        if (envelope > 1e-8f && gain < 1.0f) {
            total_gr_db += 20.0f * std::log10(gain);
            gr_count++;
        }
        
        out[i] = in[i] * gain;
    }
    
    // Log average gain reduction (only in debug mode)
    if (gr_count > 0 && sp::Logger::instance().get_level() <= sp::LogLevel::DEBUG) {
        float avg_gr = total_gr_db / static_cast<float>(gr_count);
        PLUGIN_DEBUG("DRC", "Processed ", n, " samples, avg GR: ", avg_gr, " dB");
    }
}

void DRC::set_params(const DRCParams& params) {
    params_ = params;
    compute_curve_coefficients();
    
    if (sample_rate_ > 0.0) {
        const float attack_samples = params_.attack_time_ms * static_cast<float>(sample_rate_) / 1000.0f;
        const float release_samples = params_.release_time_ms * static_cast<float>(sample_rate_) / 1000.0f;
        
        attack_coeff_ = 1.0f - std::exp(-2.2f / attack_samples);
        release_coeff_ = 1.0f - std::exp(-2.2f / release_samples);
    }
    
    PLUGIN_INFO("DRC", "Parameters updated");
}

float DRC::process_peak_envelope(float input) {
    const float abs_input = std::abs(input);
    
    if (abs_input > envelope_state_) {
        envelope_state_ = (1.0f - attack_coeff_) * envelope_state_ + attack_coeff_ * abs_input;
    } else {
        envelope_state_ = (1.0f - release_coeff_) * envelope_state_;
    }
    
    return envelope_state_;
}

float DRC::process_rms_envelope(const float* input, std::size_t n) {
    float sum_sq = 0.0f;
    for (std::size_t i = 0; i < n; ++i) {
        sum_sq += input[i] * input[i];
    }
    return std::sqrt(sum_sq / static_cast<float>(n));
}

void DRC::compute_curve_coefficients() {
    auto compute_coeff = [](float angle, float threshold) -> CurveCoeffs {
        CurveCoeffs c;
        if (angle > 0.0f) {
            c.k = 1.0f / std::tan(angle);
            c.b = (1.0f - c.k) * threshold;
        } else {
            c.k = 0.0f;
            c.b = 0.0f;
        }
        return c;
    };
    
    coeff_ng_ = compute_coeff(params_.angle_NG, params_.thresh_NT);
    coeff_ex_ = compute_coeff(params_.angle_EX, params_.thresh_NT);
    coeff_cm_ = compute_coeff(params_.angle_CM, params_.thresh_LT);
    coeff_lt_ = compute_coeff(params_.angle_LT, params_.thresh_LT);
    coeff_gt_ = compute_coeff(params_.angle_GT, params_.thresh_EX);
    
    PLUGIN_DEBUG("DRC", "Curve coefficients computed");
}

float DRC::compute_gain_from_curve(float envelope) const {
    envelope = std::clamp(envelope, 0.0f, 1.0f);
    
    float output = envelope;
    
    if (envelope < params_.thresh_NT) {
        output = coeff_ng_.k * envelope + envelope * coeff_ng_.b;
    } 
    else if (envelope < params_.thresh_EX) {
        output = coeff_ex_.k * envelope + envelope * coeff_ex_.b;
    }
    else if (envelope < params_.thresh_CT) {
        output = (coeff_gt_.k * envelope + envelope * coeff_gt_.b) * (params_.gain * 0.01f);
    }
    else if (envelope < params_.thresh_LT) {
        output = coeff_cm_.k * envelope + envelope * coeff_cm_.b;
    }
    else {
        output = coeff_lt_.k * envelope + coeff_lt_.b;
    }
    
    output = std::max(0.0f, output);
    
    if (envelope > 1e-8f) {
        return output / envelope;
    }
    return 1.0f;
}

extern "C" {
    SP_API sp::ISignalProcessor* sp_create() { 
        return new DRC(); 
    }
    
    SP_API void sp_destroy(sp::ISignalProcessor* p) { 
        delete p; 
    }
    
    SP_API const char* sp_name() { 
        return "DRC"; 
    }
    
    SP_API int sp_version_major() { 
        return 1; 
    }
    
    SP_API int sp_version_minor() { 
        return 0; 
    }
}
