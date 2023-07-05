#include "CommandBufferRing.h"
#include "Device.h"
#include "ExtDebugUtils.h"
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

    vk::SemaphoreCreateInfo semaphoreCreateInfo;

    std::string name = compute ? "ComputeCommandBufferRing" : "GraphicsCommandBufferRing;";
    for (size_t i = 0; i < numberFrameOfBackBuffers; ++i) {
        CommandBuffersPreFrame &frame = _frameCommandBuffers[i];
        frame.commandPool = device.createCommandPool(commandPoolCreateInfo);
        cmdCreateInfo.commandPool = frame.commandPool;
        frame.commandBuffers = device.allocateCommandBuffers(cmdCreateInfo);
        frame.executedFinishedFence = device.createFence(fenceCreateInfo);
        frame.renderFinishedSemaphore = device.createSemaphore(semaphoreCreateInfo);

        std::string commandPoolName = fmt::format("{}_frame{}_CommandPool", name, i);
        std::string fenceName = fmt::format("{}_frame{}_executedFinishedFence", name, i);
        std::string semaphoreName = fmt::format("{}_frame{}_renderFinishedSemaphore", name, i);

        SetResourceName(device, frame.commandPool, commandPoolName);
        SetResourceName(device, frame.executedFinishedFence, fenceName);
        SetResourceName(device, frame.renderFinishedSemaphore, semaphoreName);
        for (size_t j = 0; j < commandBufferPreFrame; ++j) {
            std::string cmdName = fmt::format("{}_frame{}_CommandBuffer{}", name, i, j);
            SetResourceName(device, frame.commandBuffers[i], cmdName);
        }
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
        device.destroySemaphore(frame.renderFinishedSemaphore);
    }
    _frameCommandBuffers.clear();
    SetIsCreate(false);
    SetDevice(nullptr);
}

void CommandBufferRing::OnBeginFrame() {
    _frameIndex = (_frameIndex + 1) % _numberFrameOfAllocators;
    vk::Device device = GetDevice()->GetVKDevice();
    CommandBuffersPreFrame &currentFrame = _frameCommandBuffers[_frameIndex];
    currentFrame.currentAllocateIndex = 0;
    VKException::Throw(device.waitForFences(1, &currentFrame.executedFinishedFence, VK_TRUE, UINT64_MAX));
    VKException::Throw(device.resetFences(1, &currentFrame.executedFinishedFence));
}

auto CommandBufferRing::GetNewCommandBuffer() -> vk::CommandBuffer {
    CommandBuffersPreFrame &currentFrame = _frameCommandBuffers[_frameIndex];
    ExceptionAssert(currentFrame.currentAllocateIndex <= _commandBufferPreBackBuffer);
    return currentFrame.commandBuffers[currentFrame.currentAllocateIndex++];
}

auto CommandBufferRing::GetCommandPool() const -> vk::CommandPool {
    return _frameCommandBuffers[_frameIndex].commandPool;
}

auto CommandBufferRing::GetExecutedFinishedFence() const -> vk::Fence {
    return _frameCommandBuffers[_frameIndex].executedFinishedFence;
}

auto CommandBufferRing::GetRenderFinishedSemaphore() const -> const vk::Semaphore & {
    return _frameCommandBuffers[_frameIndex].renderFinishedSemaphore;
}

void CommandBufferRing::WaitForRenderFinished(vk::Queue queue) {
    queue.waitIdle();
    _frameIndex = 0;
    for (CommandBuffersPreFrame &currentFrame : _frameCommandBuffers) {
        currentFrame.currentAllocateIndex = 0;
    }
}

}    // namespace vkgfx
