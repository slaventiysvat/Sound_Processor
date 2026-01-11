#include "sp/Config.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iostream>

namespace sp {

// Simple JSON parser helper (basic implementation)
namespace {
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, last - first + 1);
    }

    std::string get_json_string(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\"";
        size_t pos = json.find(search);
        if (pos == std::string::npos) return "";
        
        pos = json.find(":", pos);
        if (pos == std::string::npos) return "";
        
        size_t start = json.find("\"", pos);
        if (start == std::string::npos) return "";
        start++;
        
        size_t end = json.find("\"", start);
        if (end == std::string::npos) return "";
        
        return json.substr(start, end - start);
    }

    bool get_json_bool(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\"";
        size_t pos = json.find(search);
        if (pos == std::string::npos) return false;
        
        pos = json.find(":", pos);
        if (pos == std::string::npos) return false;
        
        size_t true_pos = json.find("true", pos);
        size_t false_pos = json.find("false", pos);
        
        if (true_pos != std::string::npos && (false_pos == std::string::npos || true_pos < false_pos)) {
            return true;
        }
        return false;
    }

    int get_json_int(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\"";
        size_t pos = json.find(search);
        if (pos == std::string::npos) return 0;
        
        pos = json.find(":", pos);
        if (pos == std::string::npos) return 0;
        
        size_t num_start = json.find_first_of("0123456789", pos);
        if (num_start == std::string::npos) return 0;
        
        size_t num_end = json.find_first_not_of("0123456789", num_start);
        std::string num_str = json.substr(num_start, num_end - num_start);
        
        return std::stoi(num_str);
    }

    const char* get_env_var(const char* name) {
        #ifdef _WIN32
        static char buf[1024];
        size_t len = 0;
        if (getenv_s(&len, buf, sizeof(buf), name) == 0 && len > 0) {
            return buf;
        }
        return nullptr;
        #else
        return std::getenv(name);
        #endif
    }
}

AppConfig AppConfig::load() {
    AppConfig config;
    
    // 1. Try to load from JSON first (defaults)
    std::filesystem::path config_path = "soundprocessor_config.json";
    auto json_config = load_from_json(config_path);
    if (json_config.has_value()) {
        config = json_config.value();
    }
    
    // 2. Override with environment variables (priority)
    load_from_env(config);
    
    return config;
}

std::optional<AppConfig> AppConfig::load_from_json(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        return std::nullopt;
    }
    
    std::ifstream file(path);
    if (!file.is_open()) {
        return std::nullopt;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json = buffer.str();
    
    AppConfig config;
    
    // Parse logging section
    config.logging.enable_console = get_json_bool(json, "enable_console");
    config.logging.enable_file = get_json_bool(json, "enable_file");
    config.logging.log_file_path = get_json_string(json, "log_file_path");
    config.logging.log_level = get_json_string(json, "log_level");
    config.logging.max_file_size_mb = get_json_int(json, "max_file_size_mb");
    config.logging.max_files = get_json_int(json, "max_files");
    
    // Parse app section
    config.plugins_dir = get_json_string(json, "plugins_dir");
    
    // Fallback to defaults if empty
    if (config.logging.log_file_path.empty()) config.logging.log_file_path = "soundprocessor.log";
    if (config.logging.log_level.empty()) config.logging.log_level = "INFO";
    if (config.logging.max_file_size_mb == 0) config.logging.max_file_size_mb = 10;
    if (config.logging.max_files == 0) config.logging.max_files = 5;
    if (config.plugins_dir.empty()) config.plugins_dir = "plugins";
    
    return config;
}

void AppConfig::load_from_env(AppConfig& config) {
    // SP_LOG_LEVEL
    if (const char* level = get_env_var("SP_LOG_LEVEL")) {
        config.logging.log_level = level;
    }
    
    // SP_LOG_FILE
    if (const char* file = get_env_var("SP_LOG_FILE")) {
        config.logging.log_file_path = file;
        config.logging.enable_file = true;
    }
    
    // SP_LOG_TO_FILE
    if (const char* enable = get_env_var("SP_LOG_TO_FILE")) {
        std::string val(enable);
        config.logging.enable_file = (val == "1" || val == "true" || val == "TRUE");
    }
    
    // SP_LOG_TO_CONSOLE
    if (const char* enable = get_env_var("SP_LOG_TO_CONSOLE")) {
        std::string val(enable);
        config.logging.enable_console = (val == "1" || val == "true" || val == "TRUE");
    }
    
    // SP_PLUGINS_DIR
    if (const char* dir = get_env_var("SP_PLUGINS_DIR")) {
        config.plugins_dir = dir;
    }
    
    // SP_LOG_MAX_SIZE_MB
    if (const char* size = get_env_var("SP_LOG_MAX_SIZE_MB")) {
        config.logging.max_file_size_mb = std::stoul(size);
    }
}

bool AppConfig::save(const std::filesystem::path& path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    
    file << "{\n";
    file << "  \"logging\": {\n";
    file << "    \"enable_console\": " << (logging.enable_console ? "true" : "false") << ",\n";
    file << "    \"enable_file\": " << (logging.enable_file ? "true" : "false") << ",\n";
    file << "    \"log_file_path\": \"" << logging.log_file_path << "\",\n";
    file << "    \"log_level\": \"" << logging.log_level << "\",\n";
    file << "    \"max_file_size_mb\": " << logging.max_file_size_mb << ",\n";
    file << "    \"max_files\": " << logging.max_files << "\n";
    file << "  },\n";
    file << "  \"plugins_dir\": \"" << plugins_dir << "\"\n";
    file << "}\n";
    
    return true;
}

} // namespace sp
