#include "kernel/PluginRegistry.hpp"
#include "sp/Logger.hpp"
#include <windows.h>

namespace kernel {

PluginRegistry::~PluginRegistry() {
    unload_all();
}

bool PluginRegistry::load_plugins(const std::filesystem::path& plugins_dir) {
    if (!std::filesystem::exists(plugins_dir)) {
        sp::Logger::instance().error("", "Plugins directory not found: ", plugins_dir.string());
        return false;
    }
    
    int loaded_count = 0;
    
    for (const auto& entry : std::filesystem::directory_iterator(plugins_dir)) {
        if (entry.path().extension() == ".dll") {
            if (load_plugin(entry.path())) {
                loaded_count++;
            }
        }
    }
    
    sp::Logger::instance().info("", "Loaded ", loaded_count, " plugins from ", plugins_dir.string());
    return loaded_count > 0;
}

bool PluginRegistry::load_plugin(const std::filesystem::path& plugin_path) {
    HMODULE handle = LoadLibraryW(plugin_path.wstring().c_str());
    if (!handle) {
        sp::Logger::instance().warn("", "Failed to load plugin: ", plugin_path.string());
        return false;
    }
    
    PluginInfo info;
    info.path = plugin_path.string();
    info.handle = handle;
    
    info.create_fn = reinterpret_cast<sp::ISignalProcessor* (*)()>(
        GetProcAddress(handle, "sp_create"));
    info.destroy_fn = reinterpret_cast<void (*)(sp::ISignalProcessor*)>(
        GetProcAddress(handle, "sp_destroy"));
    info.name_fn = reinterpret_cast<const char* (*)()>(
        GetProcAddress(handle, "sp_name"));
    
    auto version_major_fn = reinterpret_cast<int (*)()>(
        GetProcAddress(handle, "sp_version_major"));
    auto version_minor_fn = reinterpret_cast<int (*)()>(
        GetProcAddress(handle, "sp_version_minor"));
    
    if (!info.create_fn || !info.destroy_fn || !info.name_fn) {
        sp::Logger::instance().warn("", "Plugin missing required exports: ", plugin_path.string());
        FreeLibrary(handle);
        return false;
    }
    
    info.name = info.name_fn();
    if (version_major_fn) info.version_major = version_major_fn();
    if (version_minor_fn) info.version_minor = version_minor_fn();
    
    plugins_[info.name] = info;
    plugin_names_.push_back(info.name);
    
    sp::Logger::instance().debug("", "Loaded plugin: ", info.name, " v", info.version_major, ".", info.version_minor);
    return true;
}

void PluginRegistry::unload_plugin(const std::string& name) {
    auto it = plugins_.find(name);
    if (it == plugins_.end()) {
        return;
    }
    
    if (it->second.handle) {
        FreeLibrary(static_cast<HMODULE>(it->second.handle));
    }
    
    plugins_.erase(it);
    
    // Remove from names list
    auto name_it = std::find(plugin_names_.begin(), plugin_names_.end(), name);
    if (name_it != plugin_names_.end()) {
        plugin_names_.erase(name_it);
    }
    
    sp::Logger::instance().debug("", "Unloaded plugin: ", name);
}

void PluginRegistry::unload_all() {
    for (auto& [name, info] : plugins_) {
        if (info.handle) {
            FreeLibrary(static_cast<HMODULE>(info.handle));
        }
    }
    
    plugins_.clear();
    plugin_names_.clear();
}

bool PluginRegistry::has_plugin(const std::string& name) const {
    return plugins_.find(name) != plugins_.end();
}

PluginInfo* PluginRegistry::get_plugin(const std::string& name) {
    auto it = plugins_.find(name);
    return (it != plugins_.end()) ? &it->second : nullptr;
}

const std::vector<std::string>& PluginRegistry::get_plugin_names() const {
    return plugin_names_;
}

sp::ISignalProcessor* PluginRegistry::create_instance(const std::string& plugin_name) {
    auto* info = get_plugin(plugin_name);
    if (!info || !info->create_fn) {
        sp::Logger::instance().error("", "Cannot create instance of plugin: ", plugin_name);
        return nullptr;
    }
    
    return info->create_fn();
}

void PluginRegistry::destroy_instance(const std::string& plugin_name, sp::ISignalProcessor* instance) {
    if (!instance) return;
    
    auto* info = get_plugin(plugin_name);
    if (info && info->destroy_fn) {
        info->destroy_fn(instance);
    }
}

} // namespace kernel
