#pragma once
#include "Foundation/NonCopyable.h"
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

namespace vkgfx {

class Device;
class StaticBufferPool : public NonCopyable {
public:
    StaticBufferPool();
    ~StaticBufferPool();
    auto OnCreate(Device *pDevice, std::size_t totalMemorySize, std::string_view name) -> vk::Result;
    void OnDestroy();
    auto AllocBuffer(size_t numElement, size_t stride, void **pData) -> std::optional<vk::DescriptorBufferInfo>;
    auto AllocBuffer(size_t numElement, size_t stride, void *pInitData) -> std::optional<vk::DescriptorBufferInfo>;
    void UploadData(vk::CommandBuffer cmd, const vk::DescriptorBufferInfo &bufferInfo);
    void UploadData(vk::CommandBuffer cmd);
    void FreeUploadHeap();

    template<typename T>
    auto AllocBuffer(std::span<T> buffer) -> std::optional<vk::DescriptorBufferInfo> {
        return AllocBuffer(buffer.size(), sizeof(T), buffer.data());
    }
private:
    void *_pData = nullptr;
    Device *_pDevice = nullptr;
    size_t _memoryOffset = 0;
    size_t _totalMemorySize = 0;
    vk::Buffer _staticBuffer;
    vk::Buffer _uploadBuffer;
    VmaAllocation _staticBufferAlloc = VK_NULL_HANDLE;
    VmaAllocation _uploadBufferAlloc = VK_NULL_HANDLE;
};

}    // namespace vkgfx
