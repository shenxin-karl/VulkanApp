#pragma once
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include "Ring.h"
#include "Foundation/PreprocessorDirectives.h"
#include "VKObject.h"

namespace vkgfx {

class Device;
class DynamicBufferRing : public VKObject {
public:
    enum BufferType {
        Vertex = 1 << 0,
        Index = 1 << 1,
        Constant = 1 << 2,
        Structured = 1 << 3,
    };
    ENUM_FLAGS_AS_MEMBER(BufferType);
public:
    void OnCreate(Device *pDevice,
        BufferType bufferType,
        size_t numBackBuffers,
        size_t memoryTotalSize,
        std::string_view name);
    void OnDestroy();
    auto AllocBuffer(size_t size, void **pData) -> std::optional<vk::DescriptorBufferInfo>;
    auto AllocBuffer(size_t size, void *pInitData) -> std::optional<vk::DescriptorBufferInfo>;
    auto GetAllocatableSize() const -> size_t;
    void SetDescriptorSet(uint32_t index, size_t size, vk::DescriptorSet descriptorSet);
    void OnBeginFrame();

    template<typename T>
    auto AllocBuffer(const T &data) -> std::optional<vk::DescriptorBufferInfo> {
        return AllocBuffer(sizeof(T), &data);
    }
private:
    void *_pData = nullptr;
    RingWithTabs _mem;
    size_t _memTotalSize = 0;
    vk::Buffer _buffer;
    VmaAllocation _bufferAlloc = VK_NULL_HANDLE;
};

}    // namespace vkgfx
