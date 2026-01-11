#include "kernel/ServiceManager.hpp"
#include "sp/Logger.hpp"

namespace kernel {

void ServiceManager::register_service(ServiceID id, ServiceHandler handler) {
    services_[id] = std::move(handler);
    SP_DEBUG("Registered service: ", static_cast<int>(id));
}

void ServiceManager::unregister_service(ServiceID id) {
    services_.erase(id);
    SP_DEBUG("Unregistered service: ", static_cast<int>(id));
}

int ServiceManager::invoke(ServiceID id, void* params) {
    auto it = services_.find(id);
    if (it == services_.end()) {
        SP_ERROR("Service not found: ", static_cast<int>(id));
        return -1;
    }
    
    try {
        return it->second(params);
    } catch (const std::exception& e) {
        SP_ERROR("Service invocation failed: ", e.what());
        return -1;
    }
}

bool ServiceManager::has_service(ServiceID id) const {
    return services_.find(id) != services_.end();
}

} // namespace kernel
