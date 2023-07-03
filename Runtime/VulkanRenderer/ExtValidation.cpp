#include <vulkan/vulkan.h>
#include "InstanceProperties.h"
#include "ExtValidation.h"

namespace vkgfx {

static VkDebugUtilsMessengerEXT *sDebugMessenger = nullptr;
static bool sIsExtSupported = false;

auto ExtDebugMessenger::OnCreate(vk::Instance instance,
    const VkDebugUtilsMessengerCreateInfoEXT &createInfo,
    const VkAllocationCallbacks *pAllocator) -> vk::Result {

    PFN_vkCreateDebugUtilsMessengerEXT func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr) {
        return static_cast<vk::Result>(func(instance, &createInfo, pAllocator, sDebugMessenger));
    }
    return vk::Result::eErrorExtensionNotPresent;
}

void ExtDebugMessenger::OnDestroy(vk::Instance instance, const VkAllocationCallbacks *pAllocator) {
	PFN_vkDestroyDebugUtilsMessengerEXT func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
		vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr) {
        func(instance, *sDebugMessenger, pAllocator);
    }
    sDebugMessenger = VK_NULL_HANDLE;
}

void ExtDebugMessenger::AddLayers(InstanceProperties &instanceProperties) {
    sIsExtSupported = instanceProperties.AddLayer("VK_LAYER_KHRONOS_validation");
}

bool ExtDebugMessenger::IsSupported() {
    return sIsExtSupported;
}

bool ExtDebugMessenger::IsLoaded() {
    return sDebugMessenger != nullptr;
}

}    // namespace vkgfx