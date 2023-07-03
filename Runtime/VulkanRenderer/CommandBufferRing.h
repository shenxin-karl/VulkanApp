#pragma once
#include <cstdint>
#include <vulkan/vulkan.hpp>
#include "VKObject.h"
#include "Foundation/RuntimeStatic.h"

namespace vkgfx {

class Device;
class CommandBufferRing : public VKObject {
public:
    void OnCreate(Device *pDevice,
        uint32_t numberFrameOfBackBuffers,
        uint32_t commandBufferPreFrame,
        bool compute = false);
    void OnDestroy();
    void OnBeginFrame();
    auto GetNewCommandBuffer() -> vk::CommandBuffer;
    auto GetCommandPool() const -> vk::CommandPool;
    auto GetExecutedFinishedFence() const -> vk::Fence;
    auto GetRenderFinishedSemaphore() const -> const vk::Semaphore &;
    void WaitForRenderFinished();
private:
    struct CommandBuffersPreFrame {
        size_t currentAllocateIndex = 0;
        vk::CommandPool commandPool;
        vk::Fence executedFinishedFence;
        vk::Semaphore renderFinishedSemaphore;
        std::vector<vk::CommandBuffer> commandBuffers;
    };
private:
    uint32_t _frameIndex = 0;
    uint32_t _numberFrameOfAllocators = 0;
    uint32_t _commandBufferPreBackBuffer = 0;
    std::vector<CommandBuffersPreFrame> _frameCommandBuffers;
};

inline RuntimeStatic<CommandBufferRing> gCommandBufferRing;

}    // namespace vkgfx
