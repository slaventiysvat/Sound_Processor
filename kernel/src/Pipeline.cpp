#include "kernel/Pipeline.hpp"
#include "kernel/Kernel.hpp"
#include "sp/Logger.hpp"
#include <algorithm>

namespace kernel {

Pipeline::~Pipeline() {
    reset();
}

bool Pipeline::configure(const PipelineConfig& config) {
    reset();
    
    config_ = config;
    
    // Create instances for each stage
    for (const auto& stage_config : config.stages) {
        auto* processor = Kernel::instance().plugin_registry().create_instance(stage_config.plugin_name);
        if (!processor) {
            SP_ERROR("Failed to create plugin instance: ", stage_config.plugin_name);
            reset();
            return false;
        }
        
        // Prepare the processor
        processor->prepare(config.sample_rate, config.block_size);
        
        StageInstance stage;
        stage.plugin_name = stage_config.plugin_name;
        stage.processor = processor;
        stage.enabled = stage_config.enabled;
        stage.temp_buffer.resize(config.block_size);
        
        stages_.push_back(std::move(stage));
    }
    
    // Allocate internal buffer
    internal_buffer_.resize(config.block_size);
    
    SP_INFO("Pipeline configured: ", config.name, " with ", stages_.size(), " stages");
    return true;
}

void Pipeline::process(const float* input, float* output, size_t n) {
    if (stages_.empty() || n == 0) {
        if (input != output) {
            std::copy(input, input + n, output);
        }
        return;
    }
    
    // First stage: input -> temp
    const float* current_input = input;
    float* current_output = internal_buffer_.data();
    
    for (size_t i = 0; i < stages_.size(); ++i) {
        auto& stage = stages_[i];
        
        if (!stage.enabled) {
            // Skip disabled stage (pass through)
            if (current_input != current_output) {
                std::copy(current_input, current_input + n, current_output);
            }
            continue;
        }
        
        // Process through this stage
        stage.processor->process(current_input, current_output, n);
        
        // For next stage
        if (i < stages_.size() - 1) {
            current_input = current_output;
            current_output = stages_[i + 1].temp_buffer.data();
        }
    }
    
    // Copy final result to output
    const float* final_output = (stages_.size() > 1) 
        ? stages_[stages_.size() - 2].temp_buffer.data() 
        : internal_buffer_.data();
    
    if (final_output != output) {
        std::copy(final_output, final_output + n, output);
    }
}

bool Pipeline::add_stage(const std::string& plugin_name) {
    auto* processor = Kernel::instance().plugin_registry().create_instance(plugin_name);
    if (!processor) {
        SP_ERROR("Failed to add stage: ", plugin_name);
        return false;
    }
    
    processor->prepare(config_.sample_rate, config_.block_size);
    
    StageInstance stage;
    stage.plugin_name = plugin_name;
    stage.processor = processor;
    stage.enabled = true;
    stage.temp_buffer.resize(config_.block_size);
    
    stages_.push_back(std::move(stage));
    
    PipelineStage stage_config;
    stage_config.plugin_name = plugin_name;
    config_.stages.push_back(stage_config);
    
    SP_INFO("Added stage: ", plugin_name);
    return true;
}

bool Pipeline::remove_stage(size_t index) {
    if (index >= stages_.size()) {
        return false;
    }
    
    auto& stage = stages_[index];
    
    if (stage.processor) {
        Kernel::instance().plugin_registry().destroy_instance(stage.plugin_name, stage.processor);
    }
    
    stages_.erase(stages_.begin() + index);
    config_.stages.erase(config_.stages.begin() + index);
    
    SP_INFO("Removed stage at index: ", index);
    return true;
}

bool Pipeline::enable_stage(size_t index, bool enabled) {
    if (index >= stages_.size()) {
        return false;
    }
    
    stages_[index].enabled = enabled;
    config_.stages[index].enabled = enabled;
    
    SP_DEBUG("Stage ", index, " enabled: ", enabled);
    return true;
}

void Pipeline::reset() {
    for (auto& stage : stages_) {
        if (stage.processor) {
            Kernel::instance().plugin_registry().destroy_instance(stage.plugin_name, stage.processor);
        }
    }
    
    stages_.clear();
    internal_buffer_.clear();
}

} // namespace kernel
