#pragma once
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <span>
#include "VKObject.h"

namespace vkgfx {

class Device;
class StaticBufferPool : public VKObject {
public:
    StaticBufferPool();
    auto OnCreate(Device *pDevice, std::size_t totalMemorySize, std::string_view name) -> vk::Result;
    void OnDestroy();
    auto AllocBuffer(size_t numElement, size_t stride, void **pData) -> std::optional<vk::DescriptorBufferInfo>;
    auto AllocBuffer(size_t numElement, size_t stride, const void *pInitData) -> std::optional<vk::DescriptorBufferInfo>;
    void UploadData(vk::CommandBuffer cmd, const vk::DescriptorBufferInfo &bufferInfo);
    void UploadData(vk::CommandBuffer cmd);
    auto GetAllocatableSize() const -> size_t;
    void FreeUploadHeap();

    template<typename T> requires requires { std::span(std::declval<T>()); }
    auto AllocBuffer(T &&buffer) -> std::optional<vk::DescriptorBufferInfo> {
        std::span view = buffer;
        using ElementType = typename decltype(view)::element_type;
        return AllocBuffer(view.size(), sizeof(ElementType), view.data());
    }
private:
    void *_pData = nullptr;
    size_t _memoryOffset = 0;
    size_t _totalMemorySize = 0;
    vk::Buffer _staticBuffer;
    vk::Buffer _uploadBuffer;
    VmaAllocation _staticBufferAlloc = VK_NULL_HANDLE;
    VmaAllocation _uploadBufferAlloc = VK_NULL_HANDLE;
};

}    // namespace vkgfx
