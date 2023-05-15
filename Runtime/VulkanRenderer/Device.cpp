#include "Device.h"
#include "InstanceProperties.h"
#include <vulkan/vulkan_win32.h>

#include "ExtValidation.h"

namespace vkgfx {

Device::Device() {
}

Device::~Device() {
}

void Device::OnCreate(const char *pAppName, const char *pEngineName, bool cpuValidationLayerEnabled,
    bool gpuValidationLayerEnabled, vk::SurfaceKHR surface)
{
    InstanceProperties ip;
    ip.Init();
    ip.AddInstanceExtensionName(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    ip.AddInstanceExtensionName(VK_KHR_SURFACE_EXTENSION_NAME);
    if (cpuValidationLayerEnabled) {
        ExtDebugReportCheckInstanceExtensions(&ip, gpuValidationLayerEnabled);
    }
    CreateInstance(pAppName, pEngineName, ip);

}

void Device::OnDestroy() {
}

auto Device::GetDevice() const -> vk::Device {
    return _device;
}

auto Device::GetPresentQueue() const -> vk::Queue {
    return _presentQueue;
}

auto Device::GetGraphicsQueue() const -> vk::Queue {
    return _graphicsQueue;
}

auto Device::GetComputeQueue() const -> vk::Queue {
    return _computeQueue;
}

auto Device::GetPresentQueueFamilyIndex() const -> size_t {
    return _presentQueueFamilyIndex;
}

auto Device::GetGraphicsQueueFamilyIndex() const -> size_t {
    return _graphicsQueueFamilyIndex;
}

auto Device::GetComputeQueueFamilyIndex() const -> size_t {
    return _computeQueueFamilyIndex;
}

auto Device::GetPhysicalDevice() const -> vk::PhysicalDevice {
    return _physicalDevice;
}

auto Device::GetSurface() const -> vk::SurfaceKHR {
    return _surfaceKHR;
}

auto Device::GetAllocator() const -> VmaAllocator {
    return _hAllocator;
}

auto Device::GetPhysicalDeviceMemoryProperties() const -> vk::PhysicalDeviceMemoryProperties {
    return _physicalDeviceMemoryProperties;
}

auto Device::GetPhysicalDeviceProperties() const -> vk::PhysicalDeviceProperties {
    return _physicalDeviceProperties;
}

auto Device::GetPhysicalDeviceSubgroupProperties() const -> vk::PhysicalDeviceSubgroupProperties {
    return _physicalDeviceSubgroupProperties;
}

void Device::CreatePipelineCache() {

}

void Device::DestroyPipelineCache() {
}

auto Device::GetPipelineCache() const -> vk::PipelineCache {
}

void Device::GPUFlush() {
}

void Device::CreateInstance(const char *pAppName, const char *pEngineName, const InstanceProperties &ip) {
    vk::ApplicationInfo applicationInfo = {
        pAppName, 
        1, 
        pEngineName, 
        1, 
        VK_VERSION_1_1
    };
    vk::InstanceCreateInfo instanceCreateInfo {
        {},
        &applicationInfo,
        ip._instanceLayerNames,
        ip._instanceExtensionNames,
        ip._pNext,
    };
    _instance = vk::createInstance(instanceCreateInfo);
    ExtDebugReportGetProcAddresses(_instance);
    ExtDebugReportOnCreate(_instance);
}
}
