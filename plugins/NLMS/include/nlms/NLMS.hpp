#pragma once
#include "sp/ISignalProcessor.hpp"
#include "sp/export.hpp"
#include <vector>
#include <cstddef>

namespace nlms {

struct NLMSParams {
    // Filter order (number of coefficients)
    int order = 64;
    
    // Step size (learning rate): 0.0 - 2.0
    // Higher = faster convergence but less stable
    // Typical: 0.1 - 0.5
    float mu = 0.3f;
    
    // Regularization parameter (prevents division by zero)
    float epsilon = 1e-6f;
    
    // Enable/disable adaptation based on VAD
    bool adapt_on_noise_only = false;
    
    // Leaky factor (0.0-1.0, 1.0 = no leakage)
    // Used to prevent coefficient drift
    float gamma = 1.0f;
};

class SP_API NLMS final : public sp::ISignalProcessor {
public:
    NLMS();
    ~NLMS() override;
    
    void prepare(double fs, std::size_t block) override;
    void process(const float* in, float* out, std::size_t n) override;
    
    // Process with reference signal (for echo cancellation / noise reduction)
    // reference_signal = far-end or noise reference
    // desired_signal = desired output (near-end with echo/noise)
    // Returns: error signal (cleaned output)
    void process_adaptive(const float* reference_signal, const float* desired_signal, 
                         float* error_out, std::size_t n, bool vad_decision = false);
    
    // Parameter control
    void set_params(const NLMSParams& params);
    NLMSParams get_params() const { return params_; }
    
    // Reset filter state
    void reset();
    
    // Get current filter coefficients
    const std::vector<float>& get_coefficients() const { return w_; }
    
    // Get adaptation metrics
    float get_error_power() const { return error_power_; }
    float get_step_size() const { return current_step_size_; }

private:
    void initialize_filter();
    
    NLMSParams params_;
    
    // Filter state
    std::vector<float> w_;           // Filter coefficients (order x 1)
    std::vector<float> buffer_ref_;  // Circular buffer for reference signal
    std::vector<float> buffer_des_;  // Circular buffer for desired signal
    std::size_t buffer_index_ = 0;
    
    // Metrics
    float error_power_ = 0.0f;
    float current_step_size_ = 0.0f;
    std::size_t samples_processed_ = 0;
    
    double sample_rate_ = 48000.0;
    std::size_t block_size_ = 0;
};

} // namespace nlms
