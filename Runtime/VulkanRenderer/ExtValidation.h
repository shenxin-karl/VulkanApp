#pragma once
#include "InstanceProperties.h"
#include "Foundation/NonCopyable.h"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_handles.hpp"
#include <memory>

namespace vkgfx {

class ExtValidation : public NonCopyable {
public:
    static auto Attach(InstanceProperties &instanceProperties, const VkDebugUtilsMessengerCreateInfoEXT &createInfo)
        -> std::unique_ptr<ExtValidation>;
    auto OnCreate(vk::Instance instance, const VkAllocationCallbacks *pAllocator) -> vk::Result;
    void OnDestroy(vk::Instance instance);
private:
    const VkAllocationCallbacks *_pAllocator = {};
    VkDebugUtilsMessengerEXT _debugMessenger = {};
    VkDebugUtilsMessengerCreateInfoEXT _createInfo = {};
};

inline std::unique_ptr<ExtValidation> gExtValidation = {};

}    // namespace vkgfx
