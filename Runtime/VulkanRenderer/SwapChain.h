#pragma once
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include "Foundation/NonCopyable.h"
#include "Foundation/RuntimeStatic.h"

namespace vkgfx {

class Device;
class SwapChain : private NonCopyable {
public:
    SwapChain();
    ~SwapChain();
    void OnCreate(Device *pDevice, uint32_t numBackBuffers, GLFWwindow *pWindow);
    void OnDestroy();
    void GetSemaphores(vk::Semaphore &imageAvailableSemaphore,
        vk::Semaphore &renderFinishedSemaphore,
        vk::Fence &cmdBufferExecutedFences) const;
    void Resize(uint32_t width, uint32_t height, bool bVSyncOn);
    auto Present() -> vk::Result;
    auto WaitForSwapChain() -> uint32_t;
    auto GetCurrentBackBuffer() const -> vk::Image;
    auto GetCurrentBackBufferRTV() const -> vk::ImageView;
    auto GetSwapChain() const -> vk::SwapchainKHR;
    auto GetFormat() const -> vk::Format;
    auto GetRenderPass() const -> vk::RenderPass;
    auto GetFrameBuffer(size_t index) const -> vk::Framebuffer;
    auto GetCurrentFrameBuffer() const -> vk::Framebuffer;
private:
    void CreateRTV();
    void DestroyRTV();
    void CreateRenderPass();
    void DestroyRenderPass();
    void CreateFrameBuffers(size_t width, size_t height);
    void DestroyFrameBuffers();
    void OnCreateWindowDependentResources(size_t width, size_t height, bool bVSyncOn);
	void OnDestroyWindowDependentResources();
private:
    bool _VSyncOn = false;
    Device *_pDevice = nullptr;
    vk::SwapchainKHR _swapChain;
    vk::SurfaceFormatKHR _swapChainFormat;
    vk::Queue _presentQueue;
    vk::RenderPass _renderPass;
    std::vector<vk::Image> _images;
    std::vector<vk::ImageView> _imageViews;
    std::vector<vk::Framebuffer> _framebuffers;
    std::vector<vk::Fence> _cmdBufferExecutedFences;
    std::vector<vk::Semaphore> _imageAvailableSemaphores;
    std::vector<vk::Semaphore> _renderFinishedSemaphores;
    uint32_t _imageIndex = 0;
    uint32_t _backBufferCount = 0;
    uint32_t _semaphoreIndex = 0;
    uint32_t _prevSemaphoreIndex = 0;
};

inline RuntimeStatic<SwapChain> gSwapChain;

}    // namespace vkgfx
