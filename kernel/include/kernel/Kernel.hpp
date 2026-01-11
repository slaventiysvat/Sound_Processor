#pragma once
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include "kernel/PluginRegistry.hpp"
#include "kernel/ServiceManager.hpp"
#include "kernel/Scheduler.hpp"
#include "kernel/Pipeline.hpp"

namespace kernel {

// Kernel configuration
struct KernelConfig {
    std::string plugins_dir = "plugins";
    size_t max_concurrent_services = 16;
    size_t scheduler_threads = 4;
    bool enable_hot_reload = false;
};

// Kernel - центральна система управління
class Kernel {
public:
    static Kernel& instance();
    
    // Initialization
    bool initialize(const KernelConfig& config);
    void shutdown();
    
    // Plugin management
    PluginRegistry& plugin_registry() { return *plugin_registry_; }
    
    // Service management (kernel calls)
    ServiceManager& service_manager() { return *service_manager_; }
    
    // Task scheduling
    Scheduler& scheduler() { return *scheduler_; }
    
    // Pipeline execution
    Pipeline& pipeline() { return *pipeline_; }
    
    // Status
    bool is_initialized() const { return initialized_; }
    
    // System calls (kernel-style API)
    int syscall(int service_id, void* params);
    
private:
    Kernel() = default;
    ~Kernel() = default;
    Kernel(const Kernel&) = delete;
    Kernel& operator=(const Kernel&) = delete;
    
    bool initialized_ = false;
    KernelConfig config_;
    
    std::unique_ptr<PluginRegistry> plugin_registry_;
    std::unique_ptr<ServiceManager> service_manager_;
    std::unique_ptr<Scheduler> scheduler_;
    std::unique_ptr<Pipeline> pipeline_;
    
    std::mutex kernel_mutex_;
};

} // namespace kernel
