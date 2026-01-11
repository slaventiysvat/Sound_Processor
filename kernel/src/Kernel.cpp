#include "kernel/Kernel.hpp"
#include "sp/Logger.hpp"

namespace kernel {

Kernel& Kernel::instance() {
    static Kernel instance;
    return instance;
}

bool Kernel::initialize(const KernelConfig& config) {
    std::lock_guard<std::mutex> lock(kernel_mutex_);
    
    if (initialized_) {
        SP_WARN("Kernel already initialized");
        return false;
    }
    
    config_ = config;
    
    SP_INFO("Initializing SoundProcessor Kernel...");
    
    // Create subsystems
    plugin_registry_ = std::make_unique<PluginRegistry>();
    service_manager_ = std::make_unique<ServiceManager>();
    scheduler_ = std::make_unique<Scheduler>(config.scheduler_threads);
    pipeline_ = std::make_unique<Pipeline>();
    
    // Load plugins
    if (!plugin_registry_->load_plugins(config.plugins_dir)) {
        SP_ERROR("Failed to load plugins from: ", config.plugins_dir);
        return false;
    }
    
    SP_INFO("Loaded ", plugin_registry_->get_plugin_count(), " plugins");
    
    // Start scheduler
    scheduler_->start();
    
    initialized_ = true;
    SP_INFO("Kernel initialized successfully");
    
    return true;
}

void Kernel::shutdown() {
    std::lock_guard<std::mutex> lock(kernel_mutex_);
    
    if (!initialized_) {
        return;
    }
    
    SP_INFO("Shutting down kernel...");
    
    // Stop scheduler
    scheduler_->stop();
    
    // Cleanup
    pipeline_.reset();
    scheduler_.reset();
    service_manager_.reset();
    plugin_registry_.reset();
    
    initialized_ = false;
    SP_INFO("Kernel shutdown complete");
}

int Kernel::syscall(int service_id, void* params) {
    if (!initialized_) {
        SP_ERROR("Kernel not initialized");
        return -1;
    }
    
    return service_manager_->invoke(static_cast<ServiceID>(service_id), params);
}

} // namespace kernel
