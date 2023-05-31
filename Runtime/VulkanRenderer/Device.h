#pragma once
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>
#include <GLFW/glfw3.h>
#include "Foundation/NonCopyable.h"
#include "Foundation/RuntimeStatic.h"

namespace vkgfx {

class InstanceProperties;
class DeviceProperties;

class Device : private NonCopyable {
public:
    Device();
    ~Device();
    void OnCreate(const char *pAppName,
        const char *pEngineName,
        bool cpuValidationLayerEnabled,
        bool gpuValidationLayerEnabled,
        GLFWwindow *pWindow);
    void OnDestroy();
    auto GetVKDevice() const -> vk::Device;
    auto GetPresentQueue() const -> vk::Queue;
    auto GetGraphicsQueue() const -> vk::Queue;
    auto GetComputeQueue() const -> vk::Queue;
    auto GetPresentQueueFamilyIndex() const -> uint32_t;
    auto GetGraphicsQueueFamilyIndex() const -> uint32_t;
    auto GetComputeQueueFamilyIndex() const -> uint32_t;
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
        InstanceProperties &instanceProperties,
        const DeviceProperties &deviceProperties);
    void CreateInstance(const char *pAppName, const char *pEngineName, const InstanceProperties &ip);
    void InitDynamicLoader();
    void InitInstanceExtFunc();
    void InitDeviceExtFunc();
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
    uint32_t _presentQueueFamilyIndex = -1;
    uint32_t _graphicsQueueFamilyIndex = -1;
    uint32_t _computeQueueFamilyIndex = -1;
    bool _usingValidationLayer = false;
    bool _usingFp16 = false;
    VmaAllocator _hAllocator = nullptr;
    vk::PipelineCache _pipelineCache;
};


inline RuntimeStatic<Device> gDevice;

}    // namespace vkgfx
