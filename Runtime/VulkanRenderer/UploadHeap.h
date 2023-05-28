#pragma once
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>
#include "Foundation/NonCopyable.h"

namespace vkgfx {

class Device;
class UploadHeap : public NonCopyable {
public:
    struct ImageUploadJob {
        vk::Image image;
        vk::BufferImageCopy bufferImageCopy;
        vk::ImageMemoryBarrier prevBarrier;
        vk::ImageMemoryBarrier postBarrier;
    };
public:
    void OnCreate(Device *pDevice, size_t size);
    void OnDestroy();
    auto AllocBuffer(size_t size, size_t align) -> uint8_t *;
    void AddJob(const ImageUploadJob &job);
    void Flush();

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
    Device *_pDevice = nullptr;
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
