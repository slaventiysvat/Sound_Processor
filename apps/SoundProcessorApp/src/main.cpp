// SoundProcessorApp - тепер з підтримкою Kernel!
#include <windows.h>
#include <filesystem>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include "sp/PluginAPI.hpp"
#include "sp/Logger.hpp"
#include "sp/Config.hpp"
#include "PluginRAII.hpp"

// Conditional kernel include (якщо kernel доступний)
#ifdef SP_KERNEL_ENABLED
#include "kernel/Kernel.hpp"
#endif

using wavfp_print_summary_t = void(*)(const char*);
using wavfp_copy_to_t = int(*)(const char*, const char*);

static void print_usage() {
    std::cout <<
        "SoundProcessorApp usage:\n"
        "  SoundProcessorApp [options]             # Run in legacy mode\n"
#ifdef SP_KERNEL_ENABLED
        "  SoundProcessorApp --kernel              # Run with kernel architecture\n"
        "  SoundProcessorApp --kernel-pipeline <stages> # Custom pipeline (e.g., VAD,DRC,FIR)\n"
#endif
        "  SoundProcessorApp --wav-summary <in>    # Print WAV/AIFF summary\n"
        "  SoundProcessorApp --wav-copy <in> <out> # Copy audio to WAV\n"
        "\n"
        "Options:\n"
        "  --debug, --trace                        # Set log level\n"
        "  --config <path>                         # Use custom config file\n"
        "  --create-config                         # Create default config file\n"
#ifdef SP_KERNEL_ENABLED
        "  --kernel                                # Use kernel-based processing\n"
        "  --list-plugins                          # List available plugins\n"
#endif
        "\n"
        "Environment Variables:\n"
        "  SP_LOG_LEVEL=DEBUG                      # Set log level\n"
        "  SP_LOG_TO_FILE=1                        # Enable file logging\n"
        "  SP_PLUGINS_DIR=./plugins                # Plugins directory\n"
#ifdef SP_KERNEL_ENABLED
        "  SP_USE_KERNEL=1                         # Force kernel mode\n"
#endif
        ;
}

static sp::LogLevel string_to_log_level(const std::string& level) {
    if (level == "TRACE") return sp::LogLevel::TRACE;
    if (level == "DEBUG") return sp::LogLevel::DEBUG;
    if (level == "INFO") return sp::LogLevel::INFO;
    if (level == "WARN") return sp::LogLevel::WARN;
    if (level == "ERROR") return sp::LogLevel::ERR;
    if (level == "CRITICAL") return sp::LogLevel::CRITICAL;
    return sp::LogLevel::INFO;
}

#ifdef SP_KERNEL_ENABLED
// Kernel-based processing
static int run_kernel_mode(const sp::AppConfig& config, const std::vector<std::string>& pipeline_stages = {}) {
    SP_INFO("=== Kernel Mode ===");
    
    // Initialize kernel
    kernel::KernelConfig kernel_config;
    kernel_config.plugins_dir = config.plugins_dir;
    kernel_config.scheduler_threads = 4;
    
    if (!kernel::Kernel::instance().initialize(kernel_config)) {
        SP_ERROR("Failed to initialize kernel");
        return 1;
    }
    
    auto& registry = kernel::Kernel::instance().plugin_registry();
    SP_INFO("Loaded ", registry.get_plugin_count(), " plugins");
    
    // Create pipeline
    kernel::PipelineConfig pipeline_config;
    pipeline_config.name = "AudioProcessing";
    pipeline_config.sample_rate = 48000.0;
    pipeline_config.block_size = 256;
    
    if (pipeline_stages.empty()) {
        // Default pipeline
        for (const auto& name : {"VAD", "DRC", "FIR"}) {
            if (registry.has_plugin(name)) {
                kernel::PipelineStage stage;
                stage.plugin_name = name;
                stage.enabled = true;
                pipeline_config.stages.push_back(stage);
                SP_INFO("  + ", name);
            }
        }
    } else {
        // Custom pipeline
        for (const auto& name : pipeline_stages) {
            if (registry.has_plugin(name)) {
                kernel::PipelineStage stage;
                stage.plugin_name = name;
                stage.enabled = true;
                pipeline_config.stages.push_back(stage);
                SP_INFO("  + ", name);
            } else {
                SP_WARN("Plugin not found: ", name);
            }
        }
    }
    
    auto& pipeline = kernel::Kernel::instance().pipeline();
    if (!pipeline.configure(pipeline_config)) {
        SP_ERROR("Failed to configure pipeline");
        kernel::Kernel::instance().shutdown();
        return 1;
    }
    
    // Generate and process test signal
    const size_t N = 256;
    std::vector<float> input(N), output(N);
    
    for (size_t i = 0; i < N; ++i) {
        float t = float(i) / 48000.0f;
        float amp = 0.1f + 0.8f * std::sin(2.0f * 3.14159f * 0.5f * t);
        input[i] = amp * std::sin(2.0f * 3.14159f * 440.0f * t);
    }
    
    SP_INFO("Processing ", N, " samples through pipeline...");
    pipeline.process(input.data(), output.data(), N);
    
    float in_peak = *std::max_element(input.begin(), input.end(), 
        [](float a, float b) { return std::abs(a) < std::abs(b); });
    float out_peak = *std::max_element(output.begin(), output.end(),
        [](float a, float b) { return std::abs(a) < std::abs(b); });
    
    SP_INFO("Input peak:  ", 20.0f * std::log10(std::abs(in_peak) + 1e-10f), " dB");
    SP_INFO("Output peak: ", 20.0f * std::log10(std::abs(out_peak) + 1e-10f), " dB");
    
    kernel::Kernel::instance().shutdown();
    SP_INFO("=== Kernel Mode Complete ===");
    
    return 0;
}
#endif

// Legacy mode (direct plugin loading)
static int run_legacy_mode(const sp::AppConfig& config, const std::filesystem::path& pluginsDir) {
    SP_INFO("=== Legacy Mode ===");
    
    std::vector<PluginHandle> handles;
    if (std::filesystem::exists(pluginsDir)) {
        for (auto& e : std::filesystem::directory_iterator(pluginsDir)) {
            if (e.path().extension() == ".dll") {
                try {
                    PluginHandle h(e.path());
                    SP_INFO("? Loaded: ", h.nameStr());
                    handles.emplace_back(std::move(h));
                }
                catch (const std::exception& ex) {
                    SP_WARN("? Skip ", e.path().filename().string(), ": ", ex.what());
                }
            }
        }
    }

    const size_t N = 256;
    std::vector<float> in(N, 0.0f), out(N, 0.0f);
    
    SP_INFO("Generating test signal: ", N, " samples @ 48kHz");
    for (size_t i = 0; i < N; ++i) {
        float t = float(i) / 48000.0f;
        float amp = 0.1f + 0.8f * std::sin(2.0f * 3.14159f * 0.5f * t);
        in[i] = amp * std::sin(2.0f * 3.14159f * 440.0f * t);
    }

    SP_INFO("Processing with ", handles.size(), " plugin(s)...");
    for (auto& h : handles) {
        try {
            PluginInstance proc(h);
            proc->prepare(48000.0, N);
            proc->process(in.data(), out.data(), N);
            
            float in_peak = 0.0f, out_peak = 0.0f;
            for (size_t i = 0; i < N; ++i) {
                if (std::abs(in[i]) > in_peak) in_peak = std::abs(in[i]);
                if (std::abs(out[i]) > out_peak) out_peak = std::abs(out[i]);
            }
            
            float in_db = 20.0f * std::log10(in_peak + 1e-10f);
            float out_db = 20.0f * std::log10(out_peak + 1e-10f);
            
            SP_INFO("  ", h.nameStr(), " | In: ", in_db, " dB | Out: ", out_db, " dB");
        }
        catch (const std::exception& e) {
            SP_ERROR("? ", h.nameStr(), " failed: ", e.what());
        }
    }
    
    SP_INFO("=== Legacy Mode Complete ===");
    return 0;
}

int wmain(int argc, wchar_t** wargv) {
    std::vector<std::string> args; 
    args.reserve(argc);
    for (int i = 0; i < argc; ++i) {
        int len = ::WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, nullptr, 0, nullptr, nullptr);
        std::string s(len - 1, '\0');
        ::WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, s.data(), len, nullptr, nullptr);
        args.push_back(std::move(s));
    }

    // Load configuration
    sp::AppConfig config = sp::AppConfig::load();
    
    // Command line parsing
    bool use_kernel = false;
    std::vector<std::string> pipeline_stages;
    
    for (size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "--create-config") {
            if (config.save("soundprocessor_config.json")) {
                std::cout << "Created config: soundprocessor_config.json\n";
                return 0;
            } else {
                std::cerr << "Failed to create config\n";
                return 1;
            }
        }
        else if (args[i] == "--debug") {
            config.logging.log_level = "DEBUG";
        }
        else if (args[i] == "--trace") {
            config.logging.log_level = "TRACE";
        }
#ifdef SP_KERNEL_ENABLED
        else if (args[i] == "--kernel") {
            use_kernel = true;
        }
        else if (args[i] == "--kernel-pipeline" && i + 1 < args.size()) {
            use_kernel = true;
            // Parse comma-separated stages
            std::string stages_str = args[++i];
            size_t start = 0, end;
            while ((end = stages_str.find(',', start)) != std::string::npos) {
                pipeline_stages.push_back(stages_str.substr(start, end - start));
                start = end + 1;
            }
            pipeline_stages.push_back(stages_str.substr(start));
        }
        else if (args[i] == "--list-plugins") {
            use_kernel = true; // Will just list and exit
        }
#endif
        else if (args[i] == "--help" || args[i] == "-h") {
            print_usage();
            return 0;
        }
    }
    
    // Check environment variable
#ifdef SP_KERNEL_ENABLED
    if (const char* env = std::getenv("SP_USE_KERNEL")) {
        if (std::string(env) == "1") use_kernel = true;
    }
#endif
    
    // Configure logger
    sp::Logger::instance().set_level(string_to_log_level(config.logging.log_level));
    sp::Logger::instance().enable_console(config.logging.enable_console);
    
    if (config.logging.enable_file) {
        if (sp::Logger::instance().enable_file(config.logging.log_file_path, 
                                              config.logging.max_file_size_mb)) {
            SP_INFO("File logging: ", config.logging.log_file_path);
        }
    }

    SP_INFO("=== SoundProcessor v1.0 ===");
    SP_INFO("Log level: ", config.logging.log_level);
#ifdef SP_KERNEL_ENABLED
    SP_INFO("Kernel support: ENABLED");
    SP_INFO("Mode: ", (use_kernel ? "KERNEL" : "LEGACY"));
#else
    SP_INFO("Kernel support: DISABLED (legacy only)");
#endif

    std::filesystem::path pluginsDir = config.plugins_dir;
    std::error_code ec; 
    std::filesystem::create_directories(pluginsDir, ec);

#ifdef SP_KERNEL_ENABLED
    if (use_kernel) {
        return run_kernel_mode(config, pipeline_stages);
    }
#endif
    
    return run_legacy_mode(config, pluginsDir);
}
