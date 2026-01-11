#pragma once
#include <map>
#include <functional>
#include <string>

namespace kernel {

// Service IDs (kernel call numbers)
enum class ServiceID {
    // Audio processing services
    SVC_FIR_FILTER = 1,
    SVC_IIR_FILTER = 2,
    SVC_DRC_COMPRESS = 3,
    SVC_VAD_DETECT = 4,
    SVC_RLS_ADAPT = 5,
    SVC_NLMS_ADAPT = 6,
    
    // System services
    SVC_LOAD_PLUGIN = 100,
    SVC_UNLOAD_PLUGIN = 101,
    SVC_GET_PLUGIN_INFO = 102,
    
    // Pipeline services
    SVC_PIPELINE_CREATE = 200,
    SVC_PIPELINE_EXECUTE = 201,
    SVC_PIPELINE_DESTROY = 202,
};

// Service handler function
using ServiceHandler = std::function<int(void* params)>;

// Service manager - kernel call dispatcher
class ServiceManager {
public:
    ServiceManager() = default;
    
    // Register service handler
    void register_service(ServiceID id, ServiceHandler handler);
    
    // Unregister service
    void unregister_service(ServiceID id);
    
    // Invoke service (kernel call)
    int invoke(ServiceID id, void* params);
    
    // Query
    bool has_service(ServiceID id) const;
    size_t get_service_count() const { return services_.size(); }

private:
    std::map<ServiceID, ServiceHandler> services_;
};

} // namespace kernel
