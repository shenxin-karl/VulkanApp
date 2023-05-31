#include "CommandBufferRing.h"
#include "Device.h"
#include "VKException.h"
#include "Foundation/Exception.h"

namespace vkgfx {

void CommandBufferRing::OnCreate(Device *pDevice,
    uint32_t numberFrameOfBackBuffers,
    uint32_t commandBufferPreFrame,
    bool compute) {

    _numberFrameOfAllocators = numberFrameOfBackBuffers;
    _commandBufferPreBackBuffer = commandBufferPreFrame;
    _frameCommandBuffers.resize(numberFrameOfBackBuffers);

    vk::Flags<vk::CommandPoolCreateFlagBits> commandPoolFlags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer |
                                                                vk::CommandPoolCreateFlagBits::eTransient;
    uint32_t familyIndex = (compute ? pDevice->GetComputeQueueFamilyIndex() : pDevice->GetGraphicsQueueFamilyIndex());
    vk::Device device = pDevice->GetVKDevice();

    vk::CommandPoolCreateInfo commandPoolCreateInfo;
    commandPoolCreateInfo.queueFamilyIndex = familyIndex;
    commandPoolCreateInfo.flags = commandPoolFlags;

    vk::CommandBufferAllocateInfo cmdCreateInfo;
    cmdCreateInfo.level = vk::CommandBufferLevel::ePrimary;
    cmdCreateInfo.commandBufferCount = commandBufferPreFrame;

    vk::FenceCreateInfo fenceCreateInfo;
    fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    for (auto &frame : _frameCommandBuffers) {
        frame.commandPool = device.createCommandPool(commandPoolCreateInfo);
        cmdCreateInfo.commandPool = frame.commandPool;
        frame.commandBuffers = device.allocateCommandBuffers(cmdCreateInfo);
        frame.executedFinishedFence = device.createFence(fenceCreateInfo);
    }

    SetIsCreate(true);
    SetDevice(pDevice);
}

void CommandBufferRing::OnDestroy() {
    vk::Device device = GetDevice()->GetVKDevice();
    for (auto &frame : _frameCommandBuffers) {
        device.freeCommandBuffers(frame.commandPool, frame.commandBuffers);
        device.destroyCommandPool(frame.commandPool);
        device.destroyFence(frame.executedFinishedFence);
    }
    _frameCommandBuffers.clear();
    SetIsCreate(false);
    SetDevice(nullptr);
}

void CommandBufferRing::OnBeginFrame() {
    _frameIndex = (_frameIndex + 1) % _numberFrameOfAllocators;
    vk::Device device = GetDevice()->GetVKDevice();
    auto &currentFrame = _frameCommandBuffers[_frameIndex];
    VKException::Throw(device.waitForFences(1, &currentFrame.executedFinishedFence, VK_TRUE, UINT64_MAX));
    VKException::Throw(device.resetFences(1, &currentFrame.executedFinishedFence));
}

auto CommandBufferRing::GetNewCommandBuffer() -> vk::CommandBuffer {
    auto &currentFrame = _frameCommandBuffers[_frameIndex];
    ExceptionAssert(currentFrame.currentAllocateIndex <= _commandBufferPreBackBuffer);
    return currentFrame.commandBuffers[currentFrame.currentAllocateIndex++];
}

auto CommandBufferRing::GetCommandPool() const -> vk::CommandPool {
    return _frameCommandBuffers[_frameIndex].commandPool;
}

auto CommandBufferRing::GetExecutedFinishedFence() const -> vk::Fence {
    return _frameCommandBuffers[_frameIndex].executedFinishedFence;
}

}    // namespace vkgfx
