#pragma once
#include "sp/ISignalProcessor.hpp"
#include "sp/export.hpp"
#include <vector>
#include <cstddef>

namespace rls {

struct RLSParams {
    // Filter order (number of coefficients)
    int order = 25;
    
    // Forgetting factor (0.9 - 1.0)
    // Higher = more stability, lower = faster adaptation
    float lambda = 0.99f;
    
    // Regularization parameter for P matrix initialization
    float delta = 0.001f;
    
    // Enable/disable adaptation based on VAD
    bool adapt_on_noise_only = false;
};

class SP_API RLS final : public sp::ISignalProcessor {
public:
    RLS();
    ~RLS() override;
    
    void prepare(double fs, std::size_t block) override;
    void process(const float* in, float* out, std::size_t n) override;
    
    // Process with reference signal (for echo cancellation / noise reduction)
    // noise_signal = reference (far-end or noise reference)
    // desired_signal = desired output (near-end with echo/noise)
    // Returns: error signal (cleaned output)
    void process_adaptive(const float* noise_signal, const float* desired_signal, 
                         float* error_out, std::size_t n, bool vad_decision = false);
    
    // Parameter control
    void set_params(const RLSParams& params);
    RLSParams get_params() const { return params_; }
    
    // Reset filter state
    void reset();
    
    // Get current filter coefficients
    const std::vector<float>& get_coefficients() const { return w_; }
    
    // Get adaptation metrics
    float get_error_power() const { return error_power_; }

private:
    void initialize_filter();
    
    RLSParams params_;
    
    // Filter state
    std::vector<float> w_;           // Filter coefficients (order x 1)
    std::vector<std::vector<float>> P_; // Inverse correlation matrix (order x order)
    std::vector<float> buffer_noise_;   // Circular buffer for noise signal
    std::vector<float> buffer_desired_; // Circular buffer for desired signal
    std::size_t buffer_index_ = 0;
    
    // Pre-computed values
    float lambda_inv_ = 1.0f;        // 1 / lambda
    
    // Metrics
    float error_power_ = 0.0f;
    std::size_t samples_processed_ = 0;
    
    double sample_rate_ = 48000.0;
    std::size_t block_size_ = 0;
};

} // namespace rls
