#include "UploadHeap.h"
#include "Device.h"
#include "Misc.h"
#include "VKException.h"
#include "Foundation/Exception.h"

namespace vkgfx {

void UploadHeap::OnCreate(Device *pDevice, size_t size) {
    Exception::CondThrow(_pDevice == nullptr, "Repeated initialization");
    _pDevice = pDevice;

    vk::Device device = pDevice->GetDevice();
    VmaAllocator allocator = pDevice->GetAllocator();

    vk::CommandPoolCreateInfo poolCreateInfo;
    poolCreateInfo.queueFamilyIndex = pDevice->GetGraphicsQueueFamilyIndex();
    poolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    _commandPool = device.createCommandPool(poolCreateInfo);

    vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.commandPool = _commandPool;
    commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
    commandBufferAllocateInfo.commandBufferCount = 1;
    _commandBuffer = device.allocateCommandBuffers(commandBufferAllocateInfo).front();

    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;    // preference for CPU
    allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    VkResult res = vmaCreateBuffer(allocator,
        &bufferCreateInfo,
        &allocationCreateInfo,
        reinterpret_cast<VkBuffer *>(&_buffer),
        &_bufferAlloc,
        nullptr);
    ExceptionAssert(res == VK_SUCCESS);

    vmaMapMemory(allocator, _bufferAlloc, reinterpret_cast<void **>(&_pDataBegin));
    _pDataCur = _pDataBegin;
    _pDataEnd = _pDataBegin + size;

    vk::FenceCreateInfo fenceCreateInfo;
    _fence = device.createFence(fenceCreateInfo);

    vk::CommandBufferBeginInfo beginInfo;
    _commandBuffer.begin(beginInfo);
}

void UploadHeap::OnDestroy() {
    if (_pDevice == nullptr) {
        return;
    }

    vk::Device device = _pDevice->GetDevice();
    VmaAllocator allocator = _pDevice->GetAllocator();
    device.destroyFence(_fence);
    device.destroyCommandPool(_commandPool);
    vmaUnmapMemory(allocator, _bufferAlloc);
    vmaFreeMemory(allocator, _bufferAlloc);

    _buffer = nullptr;
    _bufferAlloc = nullptr;
    _pDevice = nullptr;
    _pDataBegin = nullptr;
    _pDataCur = nullptr;
    _pDataEnd = nullptr;
    ExceptionAssert(!_imageUploadJobs.empty());
    _imageUploadJobs.clear();
}

auto UploadHeap::AllocBuffer(size_t size, size_t align) -> uint8_t * {
    size = AlignUp<size_t>(size, align);
    uint8_t *ptr = reinterpret_cast<uint8_t *>(AlignUp<intptr_t>(reinterpret_cast<intptr_t>(_pDataCur), align));
    if ((ptr + size) >= _pDataEnd) {
        return nullptr;
    }

    _pDataCur = ptr + size;
    return ptr;
}

void UploadHeap::AddJob(const ImageUploadJob &job) {
    _imageUploadJobs.push_back(job);
}

void UploadHeap::Flush() {
    if (_imageUploadJobs.empty()) {
        return;
    }

    vk::Device device = _pDevice->GetDevice();
    vk::Queue graphicsQueue = _pDevice->GetGraphicsQueue();
    VmaAllocator allocator = _pDevice->GetAllocator();
    VKException::Throw(vmaFlushAllocation(allocator, _bufferAlloc, 0, (_pDataCur - _pDataBegin)));

    std::vector<vk::ImageMemoryBarrier> imagePrevBarriers;
    std::vector<vk::ImageMemoryBarrier> imagePostBarriers;
    for (const ImageUploadJob &job : _imageUploadJobs) {
        imagePrevBarriers.push_back(job.prevBarrier);
        imagePostBarriers.push_back(job.postBarrier);
    }

    if (imagePrevBarriers.size()) {
        _commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eHost,
            vk::PipelineStageFlagBits::eTransfer,
            {},
            0,
            nullptr,
            0,
            nullptr,
            imagePrevBarriers.size(),
            imagePrevBarriers.data());
    }

    for (const ImageUploadJob &job : _imageUploadJobs) {
        _commandBuffer.copyBufferToImage(_buffer,
            job.image,
            vk::ImageLayout::eTransferDstOptimal,
            1,
            &job.bufferImageCopy);
    }

    if (imagePostBarriers.size()) {
        _commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eFragmentShader,
            {},
            0,
            nullptr,
            0,
            nullptr,
            imagePostBarriers.size(),
            imagePostBarriers.data());
    }
    _imageUploadJobs.clear();
    _commandBuffer.end();

    vk::SubmitInfo submit;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &_commandBuffer;
    VKException::Throw((graphicsQueue.submit(1, &submit, _fence)));
    VKException::Throw(device.waitForFences(1, &_fence, VK_TRUE, UINT64_MAX));
    device.resetFences(_fence);

    vk::CommandBufferBeginInfo cmdBeginInfo;
    _commandBuffer.begin(cmdBeginInfo);
    _pDataCur = _pDataBegin;
}

}    // namespace vkgfx
