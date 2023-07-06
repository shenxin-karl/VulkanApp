#pragma once
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include "VKObject.h"

namespace vkgfx {

class Device;
class UploadHeap : public VKObject {
public:
    struct ImageUploadJob {
        vk::BufferImageCopy bufferImageCopy;
        vk::ImageMemoryBarrier prevBarrier;
        vk::ImageMemoryBarrier postBarrier;
    };
public:
    void OnCreate(std::string_view name, Device *pDevice, size_t size);
    void OnDestroy();
    [[nodiscard]]
    auto AllocBuffer(size_t sizeInByte, size_t align) -> uint8_t *;
    bool AllocBuffer(const void *pInitData, size_t sizeInByte, size_t align = 1);
    void AddImageJob(const ImageUploadJob &job);
    void Flush();
    auto GetAllocatableSize(size_t align = 0) const -> size_t;

    auto GetBasePtr() const -> uint8_t * {
        return _pDataBegin;
    }
    auto GetBuffer() const -> vk::Buffer {
        return _buffer;
    }
    auto GetCommandBuffer() const -> vk::CommandBuffer {
        return _commandBuffer;
    }
private:
    vk::CommandPool _commandPool;
    vk::CommandBuffer _commandBuffer;
    vk::Fence _fence;
    vk::Buffer _buffer;
    VmaAllocation _bufferAlloc = VK_NULL_HANDLE;
    uint8_t *_pDataBegin = nullptr;
    uint8_t *_pDataCur = nullptr;
    uint8_t *_pDataEnd = nullptr;
    std::vector<ImageUploadJob> _imageUploadJobs;
};

}    // namespace vkgfx
