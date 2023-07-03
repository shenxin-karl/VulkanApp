#include <vulkan/vulkan.h>
#include "InstanceProperties.h"
#include "ExtValidation.h"

namespace vkgfx {

auto ExtValidation::Attach(InstanceProperties &instanceProperties, const VkDebugUtilsMessengerCreateInfoEXT &createInfo)
    -> std::unique_ptr<ExtValidation> {

    if (!instanceProperties.AddLayer("VK_LAYER_KHRONOS_validation")) {
        return nullptr;
    }

    std::unique_ptr<ExtValidation> pResult = std::make_unique<ExtValidation>();
    pResult->_createInfo = createInfo;
    pResult->_createInfo.pNext = instanceProperties.GetNext();
    instanceProperties.SetNext(&pResult->_createInfo);
    return pResult;
}

auto ExtValidation::OnCreate(vk::Instance instance, const VkAllocationCallbacks *pAllocator) -> vk::Result {
    _pAllocator = pAllocator;
    PFN_vkCreateDebugUtilsMessengerEXT func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));

    vk::Result result = vk::Result::eErrorExtensionNotPresent;
    if (func != nullptr) {
        result = static_cast<vk::Result>(func(instance, &_createInfo, pAllocator, &_debugMessenger));
    }
    return result;
}

void ExtValidation::OnDestroy(vk::Instance instance) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));

    if (func != nullptr && _debugMessenger != VK_NULL_HANDLE) {
        func(instance, _debugMessenger, _pAllocator);
    }
    _debugMessenger = VK_NULL_HANDLE;
}

}    // namespace vkgfx