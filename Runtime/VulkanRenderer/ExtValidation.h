#pragma once
#include "InstanceProperties.h"

namespace vkgfx {

class ExtDebugMessenger {
public:
    static auto OnCreate(vk::Instance instance,
        const VkDebugUtilsMessengerCreateInfoEXT &createInfo,
        const VkAllocationCallbacks *pAllocator) -> vk::Result;
    static void OnDestroy(vk::Instance instance, const VkAllocationCallbacks *pAllocator);
    static void AddExtensions(InstanceProperties &instanceProperties);
};

}    // namespace vkgfx
