// KernelDemoApp - демонстрація роботи з ядром
#include "kernel/Kernel.hpp"
#include "sp/Logger.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <windows.h>

// Undefine Windows macros
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

// Helper to get executable directory
static std::filesystem::path get_exe_dir() {
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    std::filesystem::path exe_path(buffer);
    return exe_path.parent_path();
}

void print_banner() {
    // Set console to UTF-8
    SetConsoleOutputCP(CP_UTF8);
    
    std::cout << "\n";
    std::cout << "===============================================================\n";
    std::cout << "       SoundProcessor Kernel Demo Application                 \n";
    std::cout << "       Microkernel Architecture for DSP Processing            \n";
    std::cout << "===============================================================\n";
    std::cout << "\n";
}

void demo_plugin_registry() {
    std::cout << "=== Demo 1: Plugin Registry ===\n";
    
    auto& registry = kernel::Kernel::instance().plugin_registry();
    
    std::cout << "Loaded " << registry.get_plugin_count() << " plugins:\n";
    for (const auto& name : registry.get_plugin_names()) {
        auto* info = registry.get_plugin(name);
        if (info) {
            std::cout << "  [OK] " << name 
                     << " v" << info->version_major << "." << info->version_minor << "\n";
        }
    }
    std::cout << "\n";
}

void demo_simple_pipeline() {
    std::cout << "=== Demo 2: Simple Pipeline ===\n";
    
    kernel::PipelineConfig config;
    config.name = "VoiceEnhancement";
    config.sample_rate = 48000.0;
    config.block_size = 256;
    
    auto& registry = kernel::Kernel::instance().plugin_registry();
    
    if (registry.has_plugin("VAD")) {
        kernel::PipelineStage stage;
        stage.plugin_name = "VAD";
        stage.enabled = true;
        config.stages.push_back(stage);
        std::cout << "  [+] VAD (Voice Activity Detection)\n";
    }
    
    if (registry.has_plugin("DRC")) {
        kernel::PipelineStage stage;
        stage.plugin_name = "DRC";
        stage.enabled = true;
        config.stages.push_back(stage);
        std::cout << "  [+] DRC (Dynamic Range Compression)\n";
    }
    
    if (registry.has_plugin("FIR")) {
        kernel::PipelineStage stage;
        stage.plugin_name = "FIR";
        stage.enabled = true;
        config.stages.push_back(stage);
        std::cout << "  [+] FIR (Finite Impulse Response Filter)\n";
    }
    
    auto& pipeline = kernel::Kernel::instance().pipeline();
    if (!pipeline.configure(config)) {
        std::cerr << "Failed to configure pipeline!\n";
        return;
    }
    
    std::cout << "\nPipeline configured with " << pipeline.get_stage_count() << " stages\n";
    
    const size_t N = 256;
    std::vector<float> input(N), output(N);
    
    for (size_t i = 0; i < N; ++i) {
        float t = static_cast<float>(i) / 48000.0f;
        float envelope = 0.1f + 0.8f * std::sin(2.0f * 3.14159f * 2.0f * t);
        input[i] = envelope * std::sin(2.0f * 3.14159f * 440.0f * t);
    }
    
    std::cout << "\nProcessing " << N << " samples through pipeline...\n";
    pipeline.process(input.data(), output.data(), N);
    
    float in_rms = 0.0f, out_rms = 0.0f;
    float in_peak = 0.0f, out_peak = 0.0f;
    
    for (size_t i = 0; i < N; ++i) {
        in_rms += input[i] * input[i];
        out_rms += output[i] * output[i];
        in_peak = std::max(in_peak, std::abs(input[i]));
        out_peak = std::max(out_peak, std::abs(output[i]));
    }
    
    in_rms = std::sqrt(in_rms / N);
    out_rms = std::sqrt(out_rms / N);
    
    float in_db = 20.0f * std::log10(in_peak + 1e-10f);
    float out_db = 20.0f * std::log10(out_peak + 1e-10f);
    
    std::cout << "\nResults:\n";
    std::cout << "  Input  - RMS: " << in_rms << ", Peak: " << in_peak 
              << " (" << in_db << " dB)\n";
    std::cout << "  Output - RMS: " << out_rms << ", Peak: " << out_peak 
              << " (" << out_db << " dB)\n";
    
    // Analysis
    float gain_db = out_db - in_db;
    std::cout << "\nAnalysis:\n";
    std::cout << "  Gain: " << gain_db << " dB\n";
    std::cout << "  Processing: " << (std::abs(out_rms) > 1e-6f ? "ACTIVE" : "INACTIVE") << "\n";
    std::cout << "\n";
}

void demo_scheduler() {
    std::cout << "=== Demo 3: Scheduler (Async Processing) ===\n";
    
    auto& scheduler = kernel::Kernel::instance().scheduler();
    
    std::cout << "Submitting 10 async tasks...\n";
    
    for (int i = 0; i < 10; ++i) {
        scheduler.submit([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10 + (i % 3) * 10));
            sp::Logger::instance().debug("", "Task ", i, " completed");
        }, kernel::TaskPriority::NORMAL);
    }
    
    std::cout << "Tasks submitted. Waiting for completion...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::cout << "Completed tasks: " << scheduler.get_completed_tasks() << "\n";
    std::cout << "Pending tasks: " << scheduler.get_pending_tasks() << "\n";
    std::cout << "\n";
}

void demo_dynamic_pipeline() {
    std::cout << "=== Demo 4: Dynamic Pipeline Modification ===\n";
    
    auto& pipeline = kernel::Kernel::instance().pipeline();
    
    std::cout << "Initial pipeline stages: " << pipeline.get_stage_count() << "\n";
    
    if (pipeline.get_stage_count() >= 2) {
        std::cout << "Disabling stage 1 (DRC)...\n";
        pipeline.enable_stage(1, false);
    }
    
    const size_t N = 256;
    std::vector<float> input(N), output(N);
    
    for (size_t i = 0; i < N; ++i) {
        input[i] = 0.5f * std::sin(2.0f * 3.14159f * 1000.0f * float(i) / 48000.0f);
    }
    
    pipeline.process(input.data(), output.data(), N);
    
    std::cout << "Processed with DRC disabled\n";
    
    if (pipeline.get_stage_count() >= 2) {
        std::cout << "Re-enabling stage 1...\n";
        pipeline.enable_stage(1, true);
    }
    
    std::cout << "\n";
}

void demo_plugin_instances() {
    std::cout << "=== Demo 5: Direct Plugin Usage ===\n";
    
    auto& registry = kernel::Kernel::instance().plugin_registry();
    
    if (!registry.has_plugin("IIR")) {
        std::cout << "IIR plugin not available\n\n";
        return;
    }
    
    auto* iir = registry.create_instance("IIR");
    if (!iir) {
        std::cout << "Failed to create IIR instance\n\n";
        return;
    }
    
    std::cout << "Created IIR filter instance\n";
    
    iir->prepare(48000.0, 256);
    
    std::vector<float> input(256), output(256);
    for (size_t i = 0; i < 256; ++i) {
        input[i] = std::sin(2.0f * 3.14159f * 2000.0f * float(i) / 48000.0f);
    }
    
    iir->process(input.data(), output.data(), 256);
    
    std::cout << "Processed 256 samples through IIR filter\n";
    
    registry.destroy_instance("IIR", iir);
    std::cout << "Destroyed IIR instance\n\n";
}

int main(int argc, char* argv[]) {
    print_banner();
    
    // Configure logger
    sp::Logger::instance().set_level(sp::LogLevel::INFO);
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--debug") {
            sp::Logger::instance().set_level(sp::LogLevel::DEBUG);
        } else if (arg == "--trace") {
            sp::Logger::instance().set_level(sp::LogLevel::TRACE);
        }
    }
    
    // Get exe directory and find plugins
    auto exe_dir = get_exe_dir();
    auto plugins_dir = exe_dir / "plugins";
    
    std::cout << "Executable directory: " << exe_dir.string() << "\n";
    std::cout << "Looking for plugins in: " << plugins_dir.string() << "\n";
    
    // Check if plugins directory exists
    if (!std::filesystem::exists(plugins_dir)) {
        std::cerr << "\nERROR: Plugins directory not found!\n";
        std::cerr << "Expected location: " << plugins_dir.string() << "\n";
        std::cerr << "\nPlease make sure plugins are built and copied to:\n";
        std::cerr << "  " << plugins_dir.string() << "\n\n";
        
        std::cout << "Press Enter to exit...";
        std::cin.get();
        return 1;
    }
    
    // Initialize kernel
    std::cout << "\nInitializing Kernel...\n";
    
    kernel::KernelConfig kernel_config;
    kernel_config.plugins_dir = plugins_dir.string();
    kernel_config.scheduler_threads = 4;
    kernel_config.max_concurrent_services = 16;
    
    if (!kernel::Kernel::instance().initialize(kernel_config)) {
        std::cerr << "\nERROR: Failed to initialize kernel!\n";
        std::cerr << "Check that plugin DLLs are present in: " << plugins_dir.string() << "\n\n";
        
        std::cout << "Press Enter to exit...";
        std::cin.get();
        return 1;
    }
    
    std::cout << "Kernel initialized successfully!\n\n";
    
    // Run demos
    try {
        demo_plugin_registry();
        demo_simple_pipeline();
        demo_scheduler();
        demo_dynamic_pipeline();
        demo_plugin_instances();
    }
    catch (const std::exception& e) {
        std::cerr << "\nDemo failed: " << e.what() << "\n";
        kernel::Kernel::instance().shutdown();
        
        std::cout << "\nPress Enter to exit...";
        std::cin.get();
        return 1;
    }
    
    // Shutdown
    std::cout << "=== Shutting Down ===\n";
    kernel::Kernel::instance().shutdown();
    
    std::cout << "\n";
    std::cout << "===============================================================\n";
    std::cout << "                   Demo Complete!                              \n";
    std::cout << "===============================================================\n";
    std::cout << "\n";
    
    std::cout << "Press Enter to exit...";
    std::cin.get();
    
    return 0;
}
