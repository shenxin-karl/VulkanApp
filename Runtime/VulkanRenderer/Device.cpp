#include "Device.h"

#include <map>

#include "DeviceProperties.h"
#include "InstanceProperties.h"
#include <vulkan/vulkan_win32.h>
#include "ExtValidation.h"
#include "Foundation/Exception.h"

namespace vkgfx {

Device::Device() {
}

Device::~Device() {
}

void Device::OnCreate(const char *pAppName,
                      const char *pEngineName,
                      bool cpuValidationLayerEnabled,
                      bool gpuValidationLayerEnabled,
                      InstanceProperties &instanceProperties,
                      DeviceProperties &deviceProperties,
                      vk::SurfaceKHR surface) {
    instanceProperties.AddInstanceExtensionName(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    instanceProperties.AddInstanceExtensionName(VK_KHR_SURFACE_EXTENSION_NAME);
    if (cpuValidationLayerEnabled) {
        ExtDebugReportCheckInstanceExtensions(&instanceProperties, gpuValidationLayerEnabled);
    }
    CreateInstance(pAppName, pEngineName, instanceProperties);

    deviceProperties.AddDeviceExtensionName(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    deviceProperties.AddDeviceExtensionName(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
    OnCreateEx(pAppName,
               pEngineName,
               cpuValidationLayerEnabled,
               gpuValidationLayerEnabled,
               surface,
               instanceProperties);
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

void Device::OnCreateEx(const char *pAppName,
                        const char *pEngineName,
                        bool cpuValidationLayerEnabled,
                        bool gpuValidationLayerEnabled,
                        vk::SurfaceKHR surface,
                        const InstanceProperties &instanceProperties) {
    std::vector<vk::QueueFamilyProperties> queueProps = _physicalDevice.getQueueFamilyProperties();
    ExceptionAssert(queueProps.size() > 1);

    _physicalDeviceMemoryProperties = _physicalDevice.getMemoryProperties();
    _physicalDeviceProperties = _physicalDevice.getProperties();
    _physicalDeviceProperties2 = _physicalDevice.getProperties2();
}

static uint32_t GetScore(vk::PhysicalDevice physicalDevice) {
    uint32_t score = 0;
    vk::PhysicalDeviceProperties deviceProperties = physicalDevice.getProperties();
    switch (deviceProperties.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        score += 1000;
        break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        score += 10000;
        break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        score += 100;
        break;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        score += 10;
        break;
    default:
        break;
    }
    return score;
}

static vk::PhysicalDevice
SelectPhysicalDevice(const std::vector<vk::PhysicalDevice> &physicalDevices) {
    ExceptionAssert(physicalDevices.size() > 0 && "No GPU found");
    std::multimap<uint32_t, vk::PhysicalDevice> ratings;
    for (auto it = physicalDevices.begin(); it != physicalDevices.end(); ++it) {
        ratings.insert(std::make_pair(GetScore(*it), *it));
    }
    return ratings.rbegin()->second;
}

void Device::CreateInstance(const char *pAppName,
                            const char *pEngineName,
                            const InstanceProperties &ip) {
    vk::ApplicationInfo applicationInfo = {pAppName, 1, pEngineName, 1, VK_VERSION_1_1};

    vk::InstanceCreateInfo instanceCreateInfo = {
        {},
        &applicationInfo,
        ip._instanceLayerNames,
        ip._instanceExtensionNames,
        ip._pNext,
    };
    _instance = vk::createInstance(instanceCreateInfo);
    ExtDebugReportGetProcAddresses(_instance);
    ExtDebugReportOnCreate(_instance);

    std::vector<vk::PhysicalDevice> physicalDevices = _instance.enumeratePhysicalDevices();
    _physicalDevice = SelectPhysicalDevice(physicalDevices);
}

}    // namespace vkgfx