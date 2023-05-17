#include "Device.h"
#include "DeviceProperties.h"
#include "InstanceProperties.h"
#include "ExtDebugUtils.h"
#include "ExtValidation.h"
#include "Foundation/Exception.h"

#include <map>
#include <vulkan/vulkan_win32.h>

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

    _surfaceKHR = surface;
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
               instanceProperties,
               deviceProperties);
}

void Device::OnDestroy() {
    if (_surfaceKHR) {
        _instance.destroySurfaceKHR(_surfaceKHR);
        _surfaceKHR = nullptr;
    }
    vmaDestroyAllocator(_hAllocator);
    _hAllocator = nullptr;

    if (_device) {
        _device.destroy();
        _device = nullptr;
    }

    ExtDebugReportOnDestroy(_instance);
    _instance.destroy();
    _instance = nullptr;
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
    vk::PipelineCacheCreateInfo pipelineCacheInfo = {};
    _pipelineCache = _device.createPipelineCache(pipelineCacheInfo);
}

void Device::DestroyPipelineCache() {
    _device.destroyPipelineCache(_pipelineCache);
    _pipelineCache = nullptr;
}

auto Device::GetPipelineCache() const -> vk::PipelineCache {
    return _pipelineCache;
}

void Device::GPUFlush() {
    _device.waitIdle();
}

void Device::OnCreateEx(const char *pAppName,
                        const char *pEngineName,
                        bool cpuValidationLayerEnabled,
                        bool gpuValidationLayerEnabled,
                        InstanceProperties &instanceProperties,
                        const DeviceProperties &deviceProperties) {

    std::vector<vk::QueueFamilyProperties> queueProps = _physicalDevice.getQueueFamilyProperties();
    ExceptionAssert(queueProps.size() > 1);

    _physicalDeviceMemoryProperties = _physicalDevice.getMemoryProperties();
    _physicalDeviceProperties = _physicalDevice.getProperties();
    _physicalDeviceProperties2 = _physicalDevice.getProperties2();

    for (size_t i = 0; i < queueProps.size(); ++i) {
        const vk::QueueFamilyProperties &prop = queueProps[i];
        if (prop.queueFlags & vk::QueueFlagBits::eGraphics) {
            if (_graphicsQueueFamilyIndex != -1) {
                _graphicsQueueFamilyIndex = i;
            }
            if (_physicalDevice.getSurfaceSupportKHR(i, _surfaceKHR)) {
                _graphicsQueueFamilyIndex = i;
                _presentQueueFamilyIndex = i;
                break;
            }
        }
    }

    if (_presentQueueFamilyIndex == -1) {
        for (size_t i = 0; i < queueProps.size(); ++i) {
            const vk::QueueFamilyProperties &prop = queueProps[i];
            if (_physicalDevice.getSurfaceSupportKHR(i, _surfaceKHR)) {
                _presentQueueFamilyIndex = i;
                break;
            }
        }
    }

    for (size_t i = 0; i < queueProps.size(); ++i) {
        const vk::QueueFamilyProperties &prop = queueProps[i];
        if (prop.queueFlags & vk::QueueFlagBits::eCompute) {
            if (_computeQueueFamilyIndex != -1) {
                _graphicsQueueFamilyIndex = i;
            }
            if (i != _graphicsQueueFamilyIndex) {
                _computeQueueFamilyIndex = i;
                break;
            }
        }
    }

    float queuePriorities[1] = {0.f};
    vk::DeviceQueueCreateInfo queueCreateInfos[2];
    queueCreateInfos[0].sType = vk::StructureType::eDeviceQueueCreateInfo;
    queueCreateInfos[0].pNext = nullptr;
    queueCreateInfos[0].queueCount = 1;
    queueCreateInfos[0].pQueuePriorities = queuePriorities;
    queueCreateInfos[0].queueFamilyIndex = _graphicsQueueFamilyIndex;

    queueCreateInfos[1].sType = vk::StructureType::eDeviceQueueCreateInfo;
    queueCreateInfos[1].pNext = nullptr;
    queueCreateInfos[1].queueCount = 1;
    queueCreateInfos[1].pQueuePriorities = queuePriorities;
    queueCreateInfos[1].queueFamilyIndex = _computeQueueFamilyIndex;

    vk::PhysicalDeviceFeatures physicalDeviceFeatures = {
        .fillModeNonSolid = true,
        .pipelineStatisticsQuery = true,
        .fragmentStoresAndAtomics = true,
        .vertexPipelineStoresAndAtomics = true,
        .shaderImageGatherExtended = true,
        .wideLines = true,
        .independentBlend = true,
    };

    // enable feature to support fp16 with subgroup operations
    vk::PhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR shaderSubgroupExtendedType = {
        .sType = vk::StructureType::ePhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR,
        .pNext = deviceProperties.GetNext(),
        .shaderSubgroupExtendedTypes = VK_TRUE,
    };

    // 描述纹理和buffer越界行为
    vk::PhysicalDeviceRobustness2FeaturesEXT robustness2 = {
        .sType = vk::StructureType::ePhysicalDeviceRobustness2FeaturesEXT,
        .pNext = &shaderSubgroupExtendedType,
        .nullDescriptor = VK_TRUE,
    };

    // 能够绑定 null 视图
    vk::PhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
    physicalDeviceFeatures2.sType = vk::StructureType::ePhysicalDeviceFeatures2;
    physicalDeviceFeatures2.features = physicalDeviceFeatures;
    physicalDeviceFeatures2.pNext = &robustness2;

    vk::DeviceCreateInfo deviceCreateInfo = {
        {},
        2,
        queueCreateInfos,
        deviceProperties._deviceExtensionNames.size(),
        deviceProperties._deviceExtensionProperties.data(),
    };

    _device = _physicalDevice.createDevice(deviceCreateInfo);

    // 初始化 VMA 分配器
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = GetPhysicalDevice();
    allocatorInfo.device = GetDevice();
    allocatorInfo.instance = _instance;
    vmaCreateAllocator(&allocatorInfo, &_hAllocator);

    _graphicsQueue = _device.getQueue(_graphicsQueueFamilyIndex, 0);
    if (_presentQueueFamilyIndex == _graphicsQueueFamilyIndex) {
        _presentQueue = _graphicsQueue;
    } else {
        _presentQueue = _device.getQueue(_presentQueueFamilyIndex, 0);
    }

    if (_computeQueueFamilyIndex != -1) {
        _computeQueue = _device.getQueue(_computeQueueFamilyIndex, 0);
    }

    ExtDebugUtilsCheckInstanceExtensions(instanceProperties);
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

static vk::PhysicalDevice SelectPhysicalDevice(const std::vector<vk::PhysicalDevice> &physicalDevices) {
    ExceptionAssert(physicalDevices.size() > 0 && "No GPU found");
    std::multimap<uint32_t, vk::PhysicalDevice> ratings;
    for (auto it = physicalDevices.begin(); it != physicalDevices.end(); ++it) {
        ratings.insert(std::make_pair(GetScore(*it), *it));
    }
    return ratings.rbegin()->second;
}

void Device::CreateInstance(const char *pAppName, const char *pEngineName, const InstanceProperties &ip) {
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