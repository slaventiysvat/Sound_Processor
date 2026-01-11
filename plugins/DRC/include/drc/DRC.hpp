#pragma once
#include "sp/ISignalProcessor.hpp"
#include "sp/export.hpp"
#include <cmath>
#include <algorithm>
#include <vector>

namespace drc {

enum class EnvelopeMode {
    PEAK,
    RMS
};

struct DRCParams {
    // Threshold points (0.0 - 1.0 normalized)
    float thresh_NT = 0.01f;   // Noise gate threshold (1%)
    float thresh_EX = 0.10f;   // Expander threshold (10%)
    float thresh_CT = 0.70f;   // Compressor threshold (70%)
    float thresh_LT = 0.90f;   // Limiter threshold (90%)
    
    // Angles in radians for each zone slope
    float angle_NG = 0.0f;                    // Noise gate (0 degrees = full gate)
    float angle_EX = 16.0f * 3.14159f / 180.0f; // Expander (16 degrees)
    float angle_CM = 65.0f * 3.14159f / 180.0f; // Compressor (65 degrees)
    float angle_LT = 60.0f * 3.14159f / 180.0f; // Limiter (60 degrees)
    float angle_GT = 60.0f * 3.14159f / 180.0f; // Gain/Linear (60 degrees)
    
    // Gain factor for linear region
    float gain = 3.0f; // 300% = 3.0
    
    // Envelope detector parameters
    EnvelopeMode envelope_mode = EnvelopeMode::PEAK;
    float attack_time_ms = 10.0f;   // Attack time in milliseconds
    float release_time_ms = 80.0f;  // Release time in milliseconds
    
    // RMS window size (only for RMS mode)
    int rms_window_size = 128;
};

class SP_API DRC final : public sp::ISignalProcessor {
public:
    DRC();
    ~DRC() override;
    
    void prepare(double fs, std::size_t block) override;
    void process(const float* in, float* out, std::size_t n) override;
    
    // Parameter setters
    void set_params(const DRCParams& params);
    DRCParams get_params() const { return params_; }

private:
    // Envelope detection
    float process_peak_envelope(float input);
    float process_rms_envelope(const float* input, std::size_t n);
    
    // Static curve computation
    void compute_curve_coefficients();
    float compute_gain_from_curve(float envelope) const;
    
    // State
    DRCParams params_;
    double sample_rate_ = 48000.0;
    std::size_t block_size_ = 0;
    
    // Envelope follower state (PEAK mode)
    float attack_coeff_ = 0.0f;
    float release_coeff_ = 0.0f;
    float envelope_state_ = 0.0f;
    
    // RMS state
    std::vector<float> rms_buffer_;
    std::size_t rms_index_ = 0;
    
    // Curve coefficients (y = k*x + b)
    struct CurveCoeffs {
        float k = 0.0f; // slope
        float b = 0.0f; // offset
    } coeff_ng_, coeff_ex_, coeff_cm_, coeff_lt_, coeff_gt_;
};

} // namespace drc
