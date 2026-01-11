#pragma once
#include "sp/ISignalProcessor.hpp"
#include "sp/export.hpp"
#include <vector>
#include <cmath>

namespace iir {

enum class FilterType {
    LOWPASS,
    HIGHPASS,
    BANDPASS,
    BANDSTOP,
    PEAKING,      // Parametric EQ
    LOW_SHELF,    // Bass shelf
    HIGH_SHELF    // Treble shelf
};

enum class FilterDesign {
    BUTTERWORTH,  // Maximally flat passband
    CHEBYSHEV,    // Steeper rolloff, passband ripple
    BIQUAD        // Generic biquad (for EQ)
};

struct IIRParams {
    FilterType type = FilterType::LOWPASS;
    FilterDesign design = FilterDesign::BUTTERWORTH;
    
    // Cutoff frequency (normalized 0.0-1.0, where 1.0 = Nyquist)
    float cutoff = 0.2f;
    
    // Bandwidth (for bandpass/bandstop/peaking)
    float bandwidth = 0.1f;
    
    // Q factor (quality factor, higher = narrower)
    float Q = 0.707f;  // sqrt(2)/2 for Butterworth
    
    // Gain in dB (for peaking/shelving filters)
    float gain_db = 0.0f;
    
    // Filter order (number of biquad sections)
    // Higher order = steeper rolloff
    int order = 2;  // 2 = one biquad section (2nd order)
};

// Single biquad section (Direct Form II)
struct BiquadSection {
    // Numerator coefficients (feedforward)
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    
    // Denominator coefficients (feedback)
    float a1 = 0.0f, a2 = 0.0f;  // a0 is normalized to 1.0
    
    // State variables (Direct Form II)
    float z1 = 0.0f, z2 = 0.0f;
    
    // Process one sample
    float process(float input) {
        float output = b0 * input + z1;
        z1 = b1 * input - a1 * output + z2;
        z2 = b2 * input - a2 * output;
        return output;
    }
    
    // Reset state
    void reset() {
        z1 = z2 = 0.0f;
    }
};

class SP_API IIR final : public sp::ISignalProcessor {
public:
    IIR();
    ~IIR() override;
    
    void prepare(double fs, std::size_t block) override;
    void process(const float* in, float* out, std::size_t n) override;
    
    // Parameter control
    void set_params(const IIRParams& params);
    IIRParams get_params() const { return params_; }
    
    // Design specific filter types
    void design_lowpass(float cutoff, int order = 2, FilterDesign design = FilterDesign::BUTTERWORTH);
    void design_highpass(float cutoff, int order = 2, FilterDesign design = FilterDesign::BUTTERWORTH);
    void design_bandpass(float center, float bandwidth, int order = 2);
    void design_peaking_eq(float center, float Q, float gain_db);
    void design_low_shelf(float cutoff, float gain_db);
    void design_high_shelf(float cutoff, float gain_db);
    
    // Reset all state
    void reset();
    
    // Get biquad coefficients (for inspection/debugging)
    const std::vector<BiquadSection>& get_biquads() const { return biquads_; }

private:
    void compute_biquads();
    
    // Biquad design functions
    BiquadSection design_biquad_lowpass(float omega, float Q);
    BiquadSection design_biquad_highpass(float omega, float Q);
    BiquadSection design_biquad_bandpass(float omega, float Q);
    BiquadSection design_biquad_peaking(float omega, float Q, float gain_db);
    BiquadSection design_biquad_low_shelf(float omega, float gain_db);
    BiquadSection design_biquad_high_shelf(float omega, float gain_db);
    
    IIRParams params_;
    std::vector<BiquadSection> biquads_;  // Cascade of biquad sections
    
    double sample_rate_ = 48000.0;
    std::size_t block_size_ = 0;
};

} // namespace iir
