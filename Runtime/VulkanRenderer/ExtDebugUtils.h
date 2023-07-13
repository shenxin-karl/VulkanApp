#pragma once
#include <glm/vec4.hpp>
#include "vulkan/vulkan.h"
#include "InstanceProperties.h"
#include "Foundation/ColorUtil.hpp"
#include "Foundation/NonCopyable.h"

namespace vkgfx {

class ExtDebugUtils : public NonCopyable {
public:
    static auto Attach(InstanceProperties &instanceProperties) -> std::unique_ptr<ExtDebugUtils>;
    void OnCreate();
    void OnDestroy();
    static void SetResourceName(vk::Device device, vk::ObjectType objectType, uint64_t handle, std::string_view name);
    static void SetPrefMarkerBegin(vk::CommandBuffer cmd, std::string_view name, const glm::vec4 &color);
    static void SetPrefMarkerEnd(vk::CommandBuffer cmd);
private:
    bool _isSupported = false;
};

inline std::unique_ptr<ExtDebugUtils> gExtDebugUtils;

template<typename T>
void SetResourceName(vk::Device device, T object, std::string_view name, const glm::vec4 &color = Colors::Green) {
    using NativeType = typename T::NativeType;
    vk::ObjectType objectType = object.objectType;
    uint64_t handle = reinterpret_cast<uint64_t>(object.operator NativeType());
    if (gExtDebugUtils != nullptr) {
        gExtDebugUtils->SetResourceName(device, objectType, handle, name);
    }
}

class PrefMarkerGuard : NonCopyable {
public:
    PrefMarkerGuard(vk::CommandBuffer cmd, std::string_view name, const glm::vec4 &color = Colors::Green) : _cmd(cmd) {
        if (gExtDebugUtils != nullptr) {
            gExtDebugUtils->SetPrefMarkerBegin(_cmd, name, color);
        }
    }
    ~PrefMarkerGuard() {
        if (gExtDebugUtils != nullptr) {
            gExtDebugUtils->SetPrefMarkerEnd(_cmd);
        }
    }

    static bool Sample() noexcept {
	    return true;
    }
private:
    vk::CommandBuffer _cmd;
};

}    // namespace vkgfx