#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <filesystem>
#include "sp/ISignalProcessor.hpp"

namespace kernel {

// Plugin metadata
struct PluginInfo {
    std::string name;
    std::string path;
    int version_major = 0;
    int version_minor = 0;
    void* handle = nullptr;  // HMODULE on Windows
    
    // Function pointers
    sp::ISignalProcessor* (*create_fn)() = nullptr;
    void (*destroy_fn)(sp::ISignalProcessor*) = nullptr;
    const char* (*name_fn)() = nullptr;
};

// Plugin registry - manages all available plugins
class PluginRegistry {
public:
    PluginRegistry() = default;
    ~PluginRegistry();
    
    // Load plugins from directory
    bool load_plugins(const std::filesystem::path& plugins_dir);
    
    // Load single plugin
    bool load_plugin(const std::filesystem::path& plugin_path);
    
    // Unload plugin
    void unload_plugin(const std::string& name);
    
    // Unload all plugins
    void unload_all();
    
    // Query
    bool has_plugin(const std::string& name) const;
    PluginInfo* get_plugin(const std::string& name);
    const std::vector<std::string>& get_plugin_names() const;
    
    // Create plugin instance
    sp::ISignalProcessor* create_instance(const std::string& plugin_name);
    void destroy_instance(const std::string& plugin_name, sp::ISignalProcessor* instance);
    
    // Statistics
    size_t get_plugin_count() const { return plugins_.size(); }

private:
    std::map<std::string, PluginInfo> plugins_;
    std::vector<std::string> plugin_names_;  // Cached for iteration
};

} // namespace kernel
