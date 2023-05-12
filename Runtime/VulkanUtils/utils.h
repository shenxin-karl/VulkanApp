#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace vkutils {

bool IsExtensionAvailable(const std::vector<vk::ExtensionProperties> &properties, std::string_view extension);
auto GetRequiredInstanceExtensions() -> std::vector<const char *>;

void InitDynamicLoader();
void InitInstanceExtFunc(vk::Instance &instance);
void InitDeviceExtFunc(vk::Device &device);

void LoadDebugUtilsMessengerFunc(vk::Instance &instance);

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(
    VkInstance                                 instance,
    const VkDebugUtilsMessengerCreateInfoEXT * pCreateInfo,
    const VkAllocationCallbacks *              pAllocator,
    VkDebugUtilsMessengerEXT *                 pMessenger
);

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(
    VkInstance instance, 
    VkDebugUtilsMessengerEXT messenger,
    VkAllocationCallbacks const *pAllocator
);

VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessageFunc( 
    VkDebugUtilsMessageSeverityFlagBitsEXT       messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT              messageTypes,
    VkDebugUtilsMessengerCallbackDataEXT const * pCallbackData,
    void *                                       pUserData
);

}
