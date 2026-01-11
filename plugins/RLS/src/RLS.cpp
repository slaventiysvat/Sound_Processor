#include "rls/RLS.hpp"
#include "sp/Logger.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>

using namespace rls;

RLS::RLS() {
    initialize_filter();
    PLUGIN_INFO("RLS", "RLS instance created");
}

RLS::~RLS() {
    PLUGIN_INFO("RLS", "RLS instance destroyed");
}

void RLS::prepare(double fs, std::size_t block) {
    sample_rate_ = fs;
    block_size_ = block;
    
    // Initialize buffers
    const size_t buf_size = params_.order + block;
    buffer_noise_.resize(buf_size, 0.0f);
    buffer_desired_.resize(buf_size, 0.0f);
    buffer_index_ = 0;
    
    initialize_filter();
    
    PLUGIN_INFO("RLS", "Prepared: fs=", fs, " Hz, block=", block, 
                ", order=", params_.order, ", lambda=", params_.lambda);
}

void RLS::process(const float* in, float* out, std::size_t n) {
    if (!in || !out || n == 0) {
        return;
    }
    
    // Simple passthrough for ISignalProcessor interface
    // For actual RLS processing, use process_adaptive()
    std::copy(in, in + n, out);
}

void RLS::process_adaptive(const float* noise_signal, const float* desired_signal, 
                          float* error_out, std::size_t n, bool vad_decision) {
    if (!noise_signal || !desired_signal || !error_out || n == 0) {
        return;
    }
    
    const int p = params_.order; // Filter order
    
    // Concatenate buffer with new samples
    // buffer = [old_buffer(end-p+1:end); new_samples]
    std::copy(buffer_noise_.end() - p, buffer_noise_.end(), buffer_noise_.begin());
    std::copy(noise_signal, noise_signal + n, buffer_noise_.begin() + p);
    
    std::copy(buffer_desired_.end() - p, buffer_desired_.end(), buffer_desired_.begin());
    std::copy(desired_signal, desired_signal + n, buffer_desired_.begin() + p);
    
    float total_error_sq = 0.0f;
    
    // Process each sample
    for (std::size_t m = p; m < p + n; ++m) {
        // Acquire chunk of data (reversed order for convolution)
        // y = noise_signal[m:-1:m-p+1]
        std::vector<float> y(p);
        for (int i = 0; i < p; ++i) {
            y[i] = buffer_noise_[m - i];
        }
        
        // Error signal equation: e(m) = x(m) - w' * y
        float predicted = 0.0f;
        for (int i = 0; i < p; ++i) {
            predicted += w_[i] * y[i];
        }
        
        float e = buffer_desired_[m] - predicted;
        error_out[m - p] = e;
        
        total_error_sq += e * e;
        
        // Update filter coefficients only if VAD allows (or always if disabled)
        if (!params_.adapt_on_noise_only || !vad_decision) {
            // Compute P * y
            std::vector<float> Pi(p, 0.0f);
            for (int i = 0; i < p; ++i) {
                for (int j = 0; j < p; ++j) {
                    Pi[i] += P_[i][j] * y[j];
                }
            }
            
            // Compute Kalman gain: k = (P * y) / (lambda + y' * P * y)
            float y_Pi = 0.0f;
            for (int i = 0; i < p; ++i) {
                y_Pi += y[i] * Pi[i];
            }
            
            float denominator = params_.lambda + y_Pi;
            
            std::vector<float> k(p);
            if (std::abs(denominator) > 1e-10f) {
                for (int i = 0; i < p; ++i) {
                    k[i] = Pi[i] / denominator;
                }
            } else {
                // Avoid division by zero
                std::fill(k.begin(), k.end(), 0.0f);
            }
            
            // Update inverse correlation matrix: P = (P - k * y' * P) / lambda
            for (int i = 0; i < p; ++i) {
                for (int j = 0; j < p; ++j) {
                    float k_yP = 0.0f;
                    for (int l = 0; l < p; ++l) {
                        k_yP += k[i] * y[l] * P_[l][j];
                    }
                    P_[i][j] = (P_[i][j] - k_yP) * lambda_inv_;
                }
            }
            
            // Update filter coefficients: w = w + k * e(m)
            for (int i = 0; i < p; ++i) {
                w_[i] += k[i] * e;
            }
        }
    }
    
    // Update metrics
    error_power_ = total_error_sq / static_cast<float>(n);
    samples_processed_ += n;
    
    if (sp::Logger::instance().get_level() <= sp::LogLevel::DEBUG) {
        float error_rms = std::sqrt(error_power_);
        PLUGIN_DEBUG("RLS", "Processed ", n, " samples, error RMS: ", error_rms);
    }
}

void RLS::set_params(const RLSParams& params) {
    params_ = params;
    lambda_inv_ = 1.0f / params_.lambda;
    initialize_filter();
    
    PLUGIN_INFO("RLS", "Parameters updated: order=", params_.order, 
                ", lambda=", params_.lambda, ", delta=", params_.delta);
}

void RLS::reset() {
    initialize_filter();
    
    // Clear buffers
    std::fill(buffer_noise_.begin(), buffer_noise_.end(), 0.0f);
    std::fill(buffer_desired_.begin(), buffer_desired_.end(), 0.0f);
    buffer_index_ = 0;
    
    error_power_ = 0.0f;
    samples_processed_ = 0;
    
    PLUGIN_INFO("RLS", "Filter reset");
}

void RLS::initialize_filter() {
    const int p = params_.order;
    
    // Initialize filter coefficients to zero
    w_.resize(p, 0.0f);
    
    // Initialize inverse correlation matrix P = delta * I
    P_.resize(p, std::vector<float>(p, 0.0f));
    for (int i = 0; i < p; ++i) {
        P_[i][i] = params_.delta;
    }
    
    lambda_inv_ = 1.0f / params_.lambda;
    
    error_power_ = 0.0f;
    samples_processed_ = 0;
    
    PLUGIN_DEBUG("RLS", "Filter initialized: order=", p, 
                 ", delta=", params_.delta, ", lambda=", params_.lambda);
}

// Plugin exports
extern "C" {
    SP_API sp::ISignalProcessor* sp_create() { 
        return new RLS(); 
    }
    
    SP_API void sp_destroy(sp::ISignalProcessor* p) { 
        delete p; 
    }
    
    SP_API const char* sp_name() { 
        return "RLS"; 
    }
    
    SP_API int sp_version_major() { 
        return 1; 
    }
    
    SP_API int sp_version_minor() { 
        return 0; 
    }
}
