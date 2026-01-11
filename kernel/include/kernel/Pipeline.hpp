#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "sp/ISignalProcessor.hpp"

namespace kernel {

// Pipeline stage configuration
struct PipelineStage {
    std::string plugin_name;
    std::map<std::string, std::string> parameters;
    bool enabled = true;
};

// Pipeline configuration
struct PipelineConfig {
    std::string name;
    std::vector<PipelineStage> stages;
    double sample_rate = 48000.0;
    size_t block_size = 256;
};

// Pipeline executor - chains multiple plugins
class Pipeline {
public:
    Pipeline() = default;
    ~Pipeline();
    
    // Configuration
    bool configure(const PipelineConfig& config);
    bool load_from_json(const std::string& json_path);
    bool save_to_json(const std::string& json_path) const;
    
    // Execution
    void process(const float* input, float* output, size_t n);
    
    // Stage management
    bool add_stage(const std::string& plugin_name);
    bool remove_stage(size_t index);
    bool enable_stage(size_t index, bool enabled);
    
    // Query
    size_t get_stage_count() const { return stages_.size(); }
    const PipelineConfig& get_config() const { return config_; }
    
    // Reset all stages
    void reset();

private:
    struct StageInstance {
        std::string plugin_name;
        sp::ISignalProcessor* processor = nullptr;
        bool enabled = true;
        std::vector<float> temp_buffer;
    };
    
    PipelineConfig config_;
    std::vector<StageInstance> stages_;
    std::vector<float> internal_buffer_;
};

} // namespace kernel
