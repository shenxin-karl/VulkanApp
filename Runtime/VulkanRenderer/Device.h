#pragma once
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

namespace vkgfx {

class InstanceProperties;
class DeviceProperties;

class Device {
public:
    Device();
    ~Device();
    void OnCreate(const char *pAppName,
                  const char *pEngineName,
                  bool cpuValidationLayerEnabled,
                  bool gpuValidationLayerEnabled,
                  InstanceProperties &instanceProperties,
                  DeviceProperties &deviceProperties,
                  vk::SurfaceKHR surface);
    void OnDestroy();
    auto GetDevice() const -> vk::Device;
    auto GetPresentQueue() const -> vk::Queue;
    auto GetGraphicsQueue() const -> vk::Queue;
    auto GetComputeQueue() const -> vk::Queue;
    auto GetPresentQueueFamilyIndex() const -> size_t;
    auto GetGraphicsQueueFamilyIndex() const -> size_t;
    auto GetComputeQueueFamilyIndex() const -> size_t;
    auto GetPhysicalDevice() const -> vk::PhysicalDevice;
    auto GetSurface() const -> vk::SurfaceKHR;
    auto GetAllocator() const -> VmaAllocator;
    auto GetPhysicalDeviceMemoryProperties() const -> vk::PhysicalDeviceMemoryProperties;
    auto GetPhysicalDeviceProperties() const -> vk::PhysicalDeviceProperties;
    auto GetPhysicalDeviceSubgroupProperties() const -> vk::PhysicalDeviceSubgroupProperties;
    void CreatePipelineCache();
    void DestroyPipelineCache();
    auto GetPipelineCache() const -> vk::PipelineCache;
    void GPUFlush();
private:
    void OnCreateEx(const char *pAppName,
                    const char *pEngineName,
                    bool cpuValidationLayerEnabled,
                    bool gpuValidationLayerEnabled,
                    const InstanceProperties &instanceProperties);
    void CreateInstance(const char *pAppName, const char *pEngineName, const InstanceProperties &ip);
private:
    vk::Instance _instance;
    vk::Device _device;
    vk::PhysicalDevice _physicalDevice;
    vk::PhysicalDeviceMemoryProperties _physicalDeviceMemoryProperties;
    vk::PhysicalDeviceProperties _physicalDeviceProperties;
    vk::PhysicalDeviceProperties2 _physicalDeviceProperties2;
    vk::PhysicalDeviceSubgroupProperties _physicalDeviceSubgroupProperties;
    vk::SurfaceKHR _surfaceKHR;
    vk::Queue _presentQueue;
    vk::Queue _graphicsQueue;
    vk::Queue _computeQueue;
    size_t _presentQueueFamilyIndex = -1;
    size_t _graphicsQueueFamilyIndex = -1;
    size_t _computeQueueFamilyIndex = -1;
    bool _usingValidationLayer = false;
    bool _usingFp16 = false;
    VmaAllocator _hAllocator = nullptr;
    vk::PipelineCache _pipelineCache;
};
}    // namespace vkgfx
