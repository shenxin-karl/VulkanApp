#pragma once
#include "vulkan/vulkan.h"
#include "InstanceProperties.h"
#include "Foundation/NonCopyable.h"

namespace vkgfx {

class ExtDebugUtils : public NonCopyable {
public:
    static auto Attach(InstanceProperties &instanceProperties) -> std::unique_ptr<ExtDebugUtils>;
    void OnCreate();
    void OnDestroy();
    static void SetResourceName(vk::Device device, vk::ObjectType objectType, uint64_t handle, std::string_view name);
    static void SetPrefMarkerBegin(vk::CommandBuffer cmd, std::string_view name);
    static void SetPrefMarkerEnd(vk::CommandBuffer cmd);
private:
    bool _isSupported = false;
};

inline std::unique_ptr<ExtDebugUtils> gExtDebugUtils;

template<typename T>
void SetResourceName(vk::Device device, T object, std::string_view name) {
    using NativeType = typename T::NativeType;
    vk::ObjectType objectType = object.objectType;
    uint64_t handle = reinterpret_cast<uint64_t>(object.operator NativeType());
    if (gExtDebugUtils != nullptr) {
        gExtDebugUtils->SetResourceName(device, objectType, handle, name);
    }
}

class PrefMarkerGuard : NonCopyable {
public:
    PrefMarkerGuard(vk::CommandBuffer cmd, std::string_view name) : _cmd(cmd) {
        if (gExtDebugUtils != nullptr) {
            gExtDebugUtils->SetPrefMarkerBegin(_cmd, name);
        }
    }
    ~PrefMarkerGuard() {
        if (gExtDebugUtils != nullptr) {
            gExtDebugUtils->SetPrefMarkerEnd(_cmd);
        }
    }
private:
    vk::CommandBuffer _cmd;
};

}    // namespace vkgfx