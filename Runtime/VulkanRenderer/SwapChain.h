#pragma once
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include "Foundation/RuntimeStatic.h"
#include "VKObject.h"

namespace vkgfx {

class Device;
class SwapChain : private VKObject {
public:
    void OnCreate(Device *pDevice, uint32_t numBackBuffers);
    void OnDestroy();

    void Resize(uint32_t width, uint32_t height, bool bVSyncOn);
    auto Present(vk::Semaphore renderFinishedSemaphore) -> vk::Result;
    auto WaitForSwapChain() -> uint32_t;
    auto GetImageAvailableSemaphore() const -> const vk::Semaphore &;
    auto GetCurrentBackBuffer() const -> vk::Image;
    auto GetCurrentBackBufferRTV() const -> vk::ImageView;
    auto GetSwapChain() const -> vk::SwapchainKHR;
    auto GetFormat() const -> vk::Format;
    auto GetRenderPass() const -> vk::RenderPass;
    auto GetFrameBuffer(size_t index) const -> vk::Framebuffer;
    auto GetCurrentFrameBuffer() const -> vk::Framebuffer;
    auto GetRenderPassBeginInfo() const -> vk::RenderPassBeginInfo;
    auto GetFullScreenViewport() const -> vk::Viewport;
    auto GetFullScreenScissor() const -> vk::Rect2D;
private:
    void CreateRTV();
    void DestroyRTV();
    void CreateRenderPass();
    void DestroyRenderPass();
    void CreateFrameBuffers(size_t width, size_t height);
    void DestroyFrameBuffers();
    void OnCreateWindowDependentResources(size_t width, size_t height, bool bVSyncOn);
	void OnDestroyWindowDependentResources();

    struct SwapChainSupportDetails {
	    vk::SurfaceCapabilitiesKHR capabilities;
	    std::vector<vk::SurfaceFormatKHR> formats;
	    std::vector<vk::PresentModeKHR> presentModes;
	};
    auto QuerySwapChainSupport(vk::PhysicalDevice physicalDevice) const -> SwapChainSupportDetails;
    void ChooseSwapSurfaceFormat(const SwapChainSupportDetails &swapChainSupport);
private:
    bool _VSyncOn = false;
    vk::SwapchainKHR _swapChain;
    vk::SurfaceFormatKHR _swapChainFormat;
    vk::Queue _presentQueue;
    vk::RenderPass _renderPass;
    std::vector<vk::Image> _images;
    std::vector<vk::ImageView> _imageViews;
    std::vector<vk::Framebuffer> _framebuffers;
    std::vector<vk::Semaphore> _imageAvailableSemaphores;
    uint32_t _imageIndex = 0;
    uint32_t _backBufferCount = 0;
    uint32_t _semaphoreIndex = 0;
    uint32_t _prevSemaphoreIndex = 0;
    uint32_t _width = 0;
    uint32_t _height = 0;
};

inline RuntimeStatic<SwapChain> gSwapChain;

}    // namespace vkgfx
