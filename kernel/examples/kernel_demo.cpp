// Example: Using SoundProcessor Kernel
#include "kernel/Kernel.hpp"
#include "sp/Logger.hpp"
#include <iostream>
#include <vector>

int main() {
    sp::Logger::instance().set_level(sp::LogLevel::INFO);
    
    std::cout << "=== SoundProcessor Kernel Demo ===\n\n";
    
    // Initialize kernel
    kernel::KernelConfig config;
    config.plugins_dir = "plugins";
    config.scheduler_threads = 4;
    
    if (!kernel::Kernel::instance().initialize(config)) {
        std::cerr << "Failed to initialize kernel\n";
        return 1;
    }
    
    // List loaded plugins
    auto& registry = kernel::Kernel::instance().plugin_registry();
    std::cout << "Loaded plugins:\n";
    for (const auto& name : registry.get_plugin_names()) {
        std::cout << "  - " << name << "\n";
    }
    std::cout << "\n";
    
    // Create pipeline
    kernel::PipelineConfig pipeline_config;
    pipeline_config.name = "VoiceEnhancement";
    pipeline_config.sample_rate = 48000.0;
    pipeline_config.block_size = 256;
    
    // Add stages
    kernel::PipelineStage vad_stage;
    vad_stage.plugin_name = "VAD";
    vad_stage.enabled = true;
    pipeline_config.stages.push_back(vad_stage);
    
    kernel::PipelineStage drc_stage;
    drc_stage.plugin_name = "DRC";
    drc_stage.enabled = true;
    pipeline_config.stages.push_back(drc_stage);
    
    kernel::PipelineStage fir_stage;
    fir_stage.plugin_name = "FIR";
    fir_stage.enabled = true;
    pipeline_config.stages.push_back(fir_stage);
    
    auto& pipeline = kernel::Kernel::instance().pipeline();
    if (!pipeline.configure(pipeline_config)) {
        std::cerr << "Failed to configure pipeline\n";
        return 1;
    }
    
    std::cout << "Pipeline configured: " << pipeline.get_stage_count() << " stages\n\n";
    
    // Process audio
    const size_t N = 256;
    std::vector<float> input(N), output(N);
    
    // Generate test signal
    for (size_t i = 0; i < N; ++i) {
        float t = static_cast<float>(i) / 48000.0f;
        input[i] = 0.5f * std::sin(2.0f * 3.14159f * 440.0f * t);
    }
    
    std::cout << "Processing " << N << " samples through pipeline...\n";
    pipeline.process(input.data(), output.data(), N);
    
    // Compute statistics
    float in_rms = 0.0f, out_rms = 0.0f;
    for (size_t i = 0; i < N; ++i) {
        in_rms += input[i] * input[i];
        out_rms += output[i] * output[i];
    }
    in_rms = std::sqrt(in_rms / N);
    out_rms = std::sqrt(out_rms / N);
    
    std::cout << "Input RMS:  " << in_rms << "\n";
    std::cout << "Output RMS: " << out_rms << "\n";
    std::cout << "\nProcessing complete!\n\n";
    
    // Shutdown kernel
    kernel::Kernel::instance().shutdown();
    
    std::cout << "=== Demo Complete ===\n";
    return 0;
}
