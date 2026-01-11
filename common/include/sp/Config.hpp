#pragma once
#include <string>
#include <optional>
#include <filesystem>

namespace sp {

struct LogConfig {
    bool enable_console = true;
    bool enable_file = false;
    std::string log_file_path = "soundprocessor.log";
    std::string log_level = "INFO"; // TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL
    size_t max_file_size_mb = 10;
    size_t max_files = 5;
};

struct AppConfig {
    LogConfig logging;
    std::string plugins_dir = "plugins";
    
    // Load from environment variables (priority) and JSON config (fallback)
    static AppConfig load();
    
    // Save to JSON
    bool save(const std::filesystem::path& path) const;
    
private:
    static std::optional<AppConfig> load_from_json(const std::filesystem::path& path);
    static void load_from_env(AppConfig& config);
};

} // namespace sp
