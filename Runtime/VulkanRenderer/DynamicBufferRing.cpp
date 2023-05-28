#include "DynamicBufferRing.h"
#include "Device.h"
#include "VKException.h"
#include "ExtDebugUtils.h"
#include "Misc.h"

namespace vkgfx {

void DynamicBufferRing::OnCreate(Device *pDevice,
    BufferType bufferType,
    size_t numBackBuffers,
    size_t memoryTotalSize,
    std::string_view name) {

    _pDevice = pDevice;
    _memTotalSize = memoryTotalSize;
    _mem.OnCreate(numBackBuffers, _memTotalSize);

    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = _memTotalSize;
    bufferInfo.usage = 0;
    if (HasFlag(bufferType, Vertex)) {
        bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (HasFlag(bufferType, Index)) {
        bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (HasFlag(bufferType, Constant)) {
        bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (HasFlag(bufferType, Structured)) {
        bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    VkBuffer buffer;
    vk::Device device = pDevice->GetDevice();
    VmaAllocator allocator = pDevice->GetAllocator();
    VkResult res = vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &_bufferAlloc, nullptr);
    VKException::Throw(res);

    _buffer = vk::Buffer(buffer);
    SetResourceName(device, _buffer, fmt::format("DynamicBufferRing_{}", name.data()));

    res = vmaMapMemory(pDevice->GetAllocator(), _bufferAlloc, &_pData);
    VKException::Throw(res);
}

void DynamicBufferRing::OnDestroy() {
    if (_buffer) {
        VmaAllocator allocator = _pDevice->GetAllocator();
        vmaUnmapMemory(allocator, _bufferAlloc);
        vmaDestroyBuffer(allocator, _buffer, _bufferAlloc);
        _pData = nullptr;
        _pDevice = nullptr;
        _memTotalSize = 0;
    }
    _mem.OnDestroy();
}

auto DynamicBufferRing::AllocBuffer(size_t size, void **pData) -> std::optional<vk::DescriptorBufferInfo> {
    size = AlignUp<size_t>(size, 256u);
    uint32_t memOffset;
    if (!_mem.Alloc(size, &memOffset)) {
        *pData = nullptr;
        return std::nullopt;
    }

    *pData = _pData + memOffset;
    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = _buffer;
    bufferInfo.offset = memOffset;
    bufferInfo.range = size;
    return std::make_optional(bufferInfo);
}

auto DynamicBufferRing::AllocBuffer(size_t size, void *pInitData) -> std::optional<vk::DescriptorBufferInfo> {
    void *pBuffer = nullptr;
    std::optional<vk::DescriptorBufferInfo> res = AllocBuffer(size, &pBuffer);
    if (pBuffer) {
        memcpy(pBuffer, pInitData, size);
    }
    return res;
}

void DynamicBufferRing::SetDescriptorSet(uint32_t index, size_t size, vk::DescriptorSet descriptorSet) {
    vk::DescriptorBufferInfo out = {};
    out.buffer = _buffer;
    out.offset = 0;
    out.range = size;

    vk::WriteDescriptorSet write;
    write.dstSet = descriptorSet;
    write.descriptorCount = 1;
    write.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
    write.pBufferInfo = &out;
    write.dstArrayElement = 0;
    write.dstBinding = index;

    vk::Device device = _pDevice->GetDevice();
    device.updateDescriptorSets(1, &write, 0, nullptr);
}

void DynamicBufferRing::OnBeginFrame() {
    _mem.OnBeginFrame();
}

}    // namespace vkgfx
