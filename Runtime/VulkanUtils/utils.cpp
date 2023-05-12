#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "VKException.h"
#include "Foundation/Logger.h"
#include "utils.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace vkutils {

bool IsExtensionAvailable(const std::vector<vk::ExtensionProperties> &properties, std::string_view extension) {
	for (const auto &p : properties) {
		if (p.extensionName == extension) {
            return true;
        }
	}
    return false;
}

auto GetRequiredInstanceExtensions() -> std::vector<const char *> {
    uint32_t extensionsCount = 0;
    std::vector<const char *> extensions;
    const char **pGlfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
    for (uint32_t i = 0; i < extensionsCount; i++) {
        extensions.push_back(pGlfwExtensions[i]);
    }
    std::vector<vk::ExtensionProperties> properties = vk::enumerateInstanceExtensionProperties();
    if (IsExtensionAvailable(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    return extensions;
}


void InitDynamicLoader() {
    vk::DynamicLoader dl;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
}

void InitInstanceExtFunc(vk::Instance &instance) {
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
}

void InitDeviceExtFunc(vk::Device &device) {
    VULKAN_HPP_DEFAULT_DISPATCHER.init(device);
}

static PFN_vkCreateDebugUtilsMessengerEXT  pfnVkCreateDebugUtilsMessengerEXT = nullptr;
static PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT = nullptr;

void LoadDebugUtilsMessengerFunc(vk::Instance &instance) {
    pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>( 
        instance.getProcAddr("vkCreateDebugUtilsMessengerEXT")
    );
    Exception::CondThrow(pfnVkCreateDebugUtilsMessengerEXT != nullptr, 
        "GetProcAddr vkCreateDebugUtilsMessengerEXT failed!"
    );

    pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>( 
        instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT") 
    );
    Exception::CondThrow(pfnVkDestroyDebugUtilsMessengerEXT != nullptr, 
        "GetProcAddr vkDestroyDebugUtilsMessengerEXT failed!"
    );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(
    VkInstance instance, 
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, 
    VkDebugUtilsMessengerEXT*pMessenger)
{
    return pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(
    VkInstance instance, 
    VkDebugUtilsMessengerEXT messenger,
    VkAllocationCallbacks const *pAllocator)
{
    return pfnVkDestroyDebugUtilsMessengerEXT( instance, messenger, pAllocator );
}

VkBool32 DebugMessageFunc(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes, 
    VkDebugUtilsMessengerCallbackDataEXT const *pCallbackData,
    void *pUserData)
{
    std::ostringstream message;
    message << vk::to_string( static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>( messageSeverity ) ) << ": "
            << vk::to_string( static_cast<vk::DebugUtilsMessageTypeFlagsEXT>( messageTypes ) ) << ":\n";
    message << std::string("\t") << "messageIDName   = <" << pCallbackData->pMessageIdName << ">\n";
    message << std::string("\t") << "messageIdNumber = " << pCallbackData->messageIdNumber << "\n";
    message << std::string("\t") << "message         = <" << pCallbackData->pMessage << ">\n";
    if (0 < pCallbackData->queueLabelCount) {
        message << std::string("\t" ) << "Queue Labels:\n";
        for ( uint32_t i = 0; i < pCallbackData->queueLabelCount; i++ ) {
            message << std::string("\t\t") << "labelName = <" << pCallbackData->pQueueLabels[i].pLabelName << ">\n";
        }
    }
    if (0 < pCallbackData->cmdBufLabelCount) {
        message << std::string("\t" ) << "CommandBuffer Labels:\n";
        for ( uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; i++ )    {
            message << std::string("\t\t") << "labelName = <" << pCallbackData->pCmdBufLabels[i].pLabelName << ">\n";
        }
    }
    if (0 < pCallbackData->objectCount) {
        message << std::string( "\t" ) << "Objects:\n";
        for (uint32_t i = 0; i < pCallbackData->objectCount; i++) {
            message << std::string( "\t\t" ) << "Object " << i << "\n";
            message << std::string( "\t\t\t" ) << "objectType   = " << vk::to_string( static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType)) << "\n";
            message << std::string( "\t\t\t" ) << "objectHandle = " << pCallbackData->pObjects[i].objectHandle << "\n";
            if ( pCallbackData->pObjects[i].pObjectName ) {
                message << std::string("\t\t\t") << "objectName   = <" << pCallbackData->pObjects[i].pObjectName << ">\n";
            }
        }
    }
    gLogger->Error("[vulkan] message: {}", message.str());
    return false;
}


}
