#pragma once
#include <cstdint>
#include <vulkan/vulkan.hpp>

namespace vkgfx {

class Device;
class CommandBufferRing {
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
private:
    struct CommandBuffersPreFrame {
        size_t currentAllocateIndex = 0;
        vk::CommandPool commandPool;
        vk::Fence executedFinishedFence;
        std::vector<vk::CommandBuffer> commandBuffers;
    };
private:
    Device *_pDevice = nullptr;
    uint32_t _frameIndex = 0;
    uint32_t _numberFrameOfAllocators = 0;
    uint32_t _commandBufferPreBackBuffer = 0;
    std::vector<CommandBuffersPreFrame> _frameCommandBuffers;
};

}    // namespace vkgfx
