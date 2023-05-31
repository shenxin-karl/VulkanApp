#include "StaticBufferPool.h"
#include "Device.h"
#include "ExtDebugUtils.h"
#include "Misc.h"
#include "Foundation/Exception.h"
#include "Foundation/TypeAlias.h"

namespace vkgfx {

StaticBufferPool::StaticBufferPool() {
}

auto StaticBufferPool::OnCreate(Device *pDevice, std::size_t totalMemorySize, std::string_view name) -> vk::Result {
    _totalMemorySize = totalMemorySize;
    _memoryOffset = 0;

    VkResult res = {};
    vk::Device device = pDevice->GetVKDevice();
    VmaAllocator allocator = pDevice->GetAllocator();

    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = totalMemorySize;
    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;    // preference for CPU

    // create static buffer
    VkBuffer staticBuffer = nullptr;
    res = vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &staticBuffer, &_staticBufferAlloc, nullptr);
    ExceptionAssert(res == VK_SUCCESS);

    _staticBuffer = vk::Buffer(staticBuffer);
    std::string bufferName = fmt::format("StaticBufferPool_{}_Store", name.data());
    SetResourceName(device, _staticBuffer, bufferName);

    // create upload buffer
    VkBuffer uploadBuffer = nullptr;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;    // preference for CPU
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    res = vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &staticBuffer, &_uploadBufferAlloc, nullptr);
    ExceptionAssert(res == VK_SUCCESS);

    _uploadBuffer = vk::Buffer(uploadBuffer);
    bufferName = fmt::format("StaticBufferPool_{}_Upload", name.data());
    SetResourceName(device, _uploadBuffer, bufferName);

    res = vmaMapMemory(allocator, _uploadBufferAlloc, &_pData);
    ExceptionAssert(res == VK_SUCCESS);

    SetIsCreate(true);
    SetDevice(pDevice);
    return static_cast<vk::Result>(res);
}

void StaticBufferPool::OnDestroy() {
    ExceptionAssert(GetIsCreate());
    VmaAllocator allocator = GetDevice()->GetAllocator();
    if (_staticBuffer) {
        vmaDestroyBuffer(allocator, _staticBuffer, _staticBufferAlloc);
        _staticBuffer = nullptr;
        _staticBufferAlloc = nullptr;
    }

    FreeUploadHeap();
    _memoryOffset = 0;
    _totalMemorySize = 0;
}

auto StaticBufferPool::AllocBuffer(size_t numElement, size_t stride, void **pData)
    -> std::optional<vk::DescriptorBufferInfo> {

    ExceptionAssert(_uploadBuffer);
    ExceptionAssert(pData != nullptr);

    size_t size = AlignUp(numElement * stride, static_cast<size_t>(256));
    // bad allocate
    if (_memoryOffset + size >= _totalMemorySize) {
        *pData = nullptr;
        return std::nullopt;
    }

    *pData = (static_cast<uint8 *>(_pData) + _memoryOffset);
    vk::DescriptorBufferInfo info;
    info.buffer = _staticBuffer;
    info.offset = _memoryOffset;
    info.range = size;
    _memoryOffset += size;
    return std::make_optional(info);
}

auto StaticBufferPool::AllocBuffer(size_t numElement, size_t stride, void *pInitData)
    -> std::optional<vk::DescriptorBufferInfo> {

    void *pBufferPtr = nullptr;
    std::optional<vk::DescriptorBufferInfo> info = AllocBuffer(numElement, stride, &pBufferPtr);
    if (pBufferPtr != nullptr) {
        std::memcpy(pBufferPtr, pInitData, numElement * stride);
    }
    return info;
}

void StaticBufferPool::UploadData(vk::CommandBuffer cmd, const vk::DescriptorBufferInfo &bufferInfo) {
    ExceptionAssert(_uploadBuffer);
    vk::BufferCopy region;
    region.srcOffset = bufferInfo.offset;
    region.dstOffset = bufferInfo.offset;
    region.size = bufferInfo.range;
    cmd.copyBuffer(_staticBuffer, _uploadBuffer, region);
}

void StaticBufferPool::UploadData(vk::CommandBuffer cmd) {
    ExceptionAssert(_uploadBuffer);
    vk::BufferCopy region;
    region.srcOffset = 0;
    region.dstOffset = 0;
    region.size = _totalMemorySize;
    cmd.copyBuffer(_staticBuffer, _uploadBuffer, region);
}

void StaticBufferPool::FreeUploadHeap() {
    ExceptionAssert(_uploadBuffer);
    VmaAllocator allocator = GetDevice()->GetAllocator();
    if (_uploadBuffer) {
        vmaUnmapMemory(allocator, _uploadBufferAlloc);
        vmaDestroyBuffer(allocator, _uploadBuffer, _uploadBufferAlloc);
        _uploadBuffer = nullptr;
        _uploadBufferAlloc = nullptr;
    }
    _pData = nullptr;
}

}    // namespace vkgfx
