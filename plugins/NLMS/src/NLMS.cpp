#include "nlms/NLMS.hpp"
#include "sp/Logger.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>

using namespace nlms;

NLMS::NLMS() {
    initialize_filter();
    PLUGIN_INFO("NLMS", "NLMS instance created");
}

NLMS::~NLMS() {
    PLUGIN_INFO("NLMS", "NLMS instance destroyed");
}

void NLMS::prepare(double fs, std::size_t block) {
    sample_rate_ = fs;
    block_size_ = block;
    
    // Initialize buffers
    const size_t buf_size = params_.order + block;
    buffer_ref_.resize(buf_size, 0.0f);
    buffer_des_.resize(buf_size, 0.0f);
    buffer_index_ = 0;
    
    initialize_filter();
    
    PLUGIN_INFO("NLMS", "Prepared: fs=", fs, " Hz, block=", block, 
                ", order=", params_.order, ", mu=", params_.mu);
}

void NLMS::process(const float* in, float* out, std::size_t n) {
    if (!in || !out || n == 0) {
        return;
    }
    
    // Simple passthrough for ISignalProcessor interface
    // For actual NLMS processing, use process_adaptive()
    std::copy(in, in + n, out);
}

void NLMS::process_adaptive(const float* reference_signal, const float* desired_signal, 
                           float* error_out, std::size_t n, bool vad_decision) {
    if (!reference_signal || !desired_signal || !error_out || n == 0) {
        return;
    }
    
    const int L = params_.order; // Filter order
    
    // Concatenate buffer with new samples
    std::copy(buffer_ref_.end() - L, buffer_ref_.end(), buffer_ref_.begin());
    std::copy(reference_signal, reference_signal + n, buffer_ref_.begin() + L);
    
    std::copy(buffer_des_.end() - L, buffer_des_.end(), buffer_des_.begin());
    std::copy(desired_signal, desired_signal + n, buffer_des_.begin() + L);
    
    float total_error_sq = 0.0f;
    float total_step_size = 0.0f;
    
    // Process each sample
    for (std::size_t m = L; m < L + n; ++m) {
        // Get reference signal vector (reversed for convolution)
        // x = reference[m:-1:m-L+1]
        std::vector<float> x(L);
        for (int i = 0; i < L; ++i) {
            x[i] = buffer_ref_[m - i];
        }
        
        // Compute filter output: y(m) = w' * x
        float y = 0.0f;
        for (int i = 0; i < L; ++i) {
            y += w_[i] * x[i];
        }
        
        // Compute error: e(m) = d(m) - y(m)
        float d = buffer_des_[m];
        float e = d - y;
        error_out[m - L] = e;
        
        total_error_sq += e * e;
        
        // Update filter coefficients only if VAD allows (or always if disabled)
        if (!params_.adapt_on_noise_only || !vad_decision) {
            // Compute power of reference signal: ||x||^2
            float x_power = params_.epsilon; // Regularization
            for (int i = 0; i < L; ++i) {
                x_power += x[i] * x[i];
            }
            
            // Normalized step size: mu / (epsilon + ||x||^2)
            float normalized_mu = params_.mu / x_power;
            current_step_size_ = normalized_mu;
            total_step_size += normalized_mu;
            
            // NLMS update rule: w(m+1) = gamma * w(m) + mu * e(m) * x / ||x||^2
            for (int i = 0; i < L; ++i) {
                w_[i] = params_.gamma * w_[i] + normalized_mu * e * x[i];
            }
        }
    }
    
    // Update metrics
    error_power_ = total_error_sq / static_cast<float>(n);
    if (total_step_size > 0.0f) {
        current_step_size_ = total_step_size / static_cast<float>(n);
    }
    samples_processed_ += n;
    
    if (sp::Logger::instance().get_level() <= sp::LogLevel::DEBUG) {
        float error_rms = std::sqrt(error_power_);
        PLUGIN_DEBUG("NLMS", "Processed ", n, " samples, error RMS: ", error_rms,
                     ", avg step: ", current_step_size_);
    }
}

void NLMS::set_params(const NLMSParams& params) {
    params_ = params;
    initialize_filter();
    
    PLUGIN_INFO("NLMS", "Parameters updated: order=", params_.order, 
                ", mu=", params_.mu, ", epsilon=", params_.epsilon,
                ", gamma=", params_.gamma);
}

void NLMS::reset() {
    initialize_filter();
    
    // Clear buffers
    std::fill(buffer_ref_.begin(), buffer_ref_.end(), 0.0f);
    std::fill(buffer_des_.begin(), buffer_des_.end(), 0.0f);
    buffer_index_ = 0;
    
    error_power_ = 0.0f;
    current_step_size_ = 0.0f;
    samples_processed_ = 0;
    
    PLUGIN_INFO("NLMS", "Filter reset");
}

void NLMS::initialize_filter() {
    const int L = params_.order;
    
    // Initialize filter coefficients to zero
    w_.resize(L, 0.0f);
    
    error_power_ = 0.0f;
    current_step_size_ = 0.0f;
    samples_processed_ = 0;
    
    PLUGIN_DEBUG("NLMS", "Filter initialized: order=", L, 
                 ", mu=", params_.mu, ", epsilon=", params_.epsilon);
}

// Plugin exports
extern "C" {
    SP_API sp::ISignalProcessor* sp_create() { 
        return new NLMS(); 
    }
    
    SP_API void sp_destroy(sp::ISignalProcessor* p) { 
        delete p; 
    }
    
    SP_API const char* sp_name() { 
        return "NLMS"; 
    }
    
    SP_API int sp_version_major() { 
        return 1; 
    }
    
    SP_API int sp_version_minor() { 
        return 0; 
    }
}
