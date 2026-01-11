// PluginRAII.hpp
#pragma once
#include <windows.h>
#include <filesystem>
#include <stdexcept>
#include <string>
#include "sp/PluginAPI.hpp"

class PluginHandle {
public:
    PluginHandle() = default;

    explicit PluginHandle(const std::filesystem::path& dllPath) {
        h_ = ::LoadLibraryW(dllPath.wstring().c_str());
        if (!h_) throw std::runtime_error("LoadLibrary failed: " + dllPath.string());
        create_ = reinterpret_cast<CreateFn>(::GetProcAddress(h_, "sp_create"));
        destroy_ = reinterpret_cast<DestroyFn>(::GetProcAddress(h_, "sp_destroy"));
        name_ = reinterpret_cast<NameFn>(::GetProcAddress(h_, "sp_name"));
        if (!create_ || !destroy_ || !name_) {
            ::FreeLibrary(h_); h_ = nullptr;
            throw std::runtime_error("Missing plugin exports in: " + dllPath.string());
        }
    }

    // move-only
    PluginHandle(PluginHandle&& o) noexcept { moveFrom(std::move(o)); }
    PluginHandle& operator=(PluginHandle&& o) noexcept {
        if (this != &o) { reset(); moveFrom(std::move(o)); }
        return *this;
    }
    PluginHandle(const PluginHandle&) = delete;
    PluginHandle& operator=(const PluginHandle&) = delete;
    ~PluginHandle() { reset(); }

    const char* nameStr() const noexcept { return name_ ? name_() : ""; }
    CreateFn  create()  const noexcept { return create_; }
    DestroyFn destroy() const noexcept { return destroy_; }

    template<class Fn>
    Fn getProc(const char* sym) const noexcept {
        return h_ ? reinterpret_cast<Fn>(::GetProcAddress(h_, sym)) : nullptr;
    }

private:
    void reset() noexcept { if (h_) { ::FreeLibrary(h_); h_ = nullptr; } create_ = nullptr; destroy_ = nullptr; name_ = nullptr; }
    void moveFrom(PluginHandle&& o) noexcept { h_ = o.h_; create_ = o.create_; destroy_ = o.destroy_; name_ = o.name_; o.h_ = nullptr; o.create_ = nullptr; o.destroy_ = nullptr; o.name_ = nullptr; }

    HMODULE  h_ = nullptr;
    CreateFn  create_ = nullptr;
    DestroyFn destroy_ = nullptr;
    NameFn    name_ = nullptr;
};

class PluginInstance {
public:
    PluginInstance() = default;

    explicit PluginInstance(PluginHandle& h) : handle_(&h), ptr_(nullptr) {
        auto c = handle_->create();               // öå CreateFn
        if (!c) throw std::runtime_error("sp_create not found");
        ptr_ = c();                               // ÂÀÆËÈÂÎ: ÂÈÊËÈÊ ôóíêö³¿ -> ISignalProcessor*
        if (!ptr_) throw std::runtime_error("sp_create returned null");
    }

    // move-only
    PluginInstance(PluginInstance&& o) noexcept { moveFrom(std::move(o)); }
    PluginInstance& operator=(PluginInstance&& o) noexcept {
        if (this != &o) { reset(); moveFrom(std::move(o)); }
        return *this;
    }
    PluginInstance(const PluginInstance&) = delete;
    PluginInstance& operator=(const PluginInstance&) = delete;

    ~PluginInstance() { reset(); }

    sp::ISignalProcessor* get() const noexcept { return ptr_; }
    sp::ISignalProcessor* operator->() const noexcept { return ptr_; }
    explicit operator bool() const noexcept { return ptr_ != nullptr; }

private:
    void reset() noexcept {
        if (handle_ && ptr_) {
            if (auto d = handle_->destroy()) d(ptr_);
        }
        ptr_ = nullptr; handle_ = nullptr;
    }
    void moveFrom(PluginInstance&& o) noexcept { handle_ = o.handle_; ptr_ = o.ptr_; o.handle_ = nullptr; o.ptr_ = nullptr; }

    PluginHandle* handle_ = nullptr;
    sp::ISignalProcessor* ptr_ = nullptr;
};
