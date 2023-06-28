#include "SwapChain.h"
#include "Device.h"
#include "ExtDebugUtils.h"
#include "VKException.h"
#include "Foundation/Exception.h"

namespace vkgfx {

void SwapChain::OnCreate(Device *pDevice, uint32_t numBackBuffers, GLFWwindow *pWindow) {
    SetDevice(pDevice);
    _backBufferCount = numBackBuffers;
    _presentQueue = pDevice->GetPresentQueue();

    _swapChainFormat.format = vk::Format::eR8G8B8A8Unorm;
    _swapChainFormat.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;

    vk::Device device = pDevice->GetVKDevice();
    _cmdBufferExecutedFences.resize(numBackBuffers);
    _imageAvailableSemaphores.resize(numBackBuffers);
    _renderFinishedSemaphores.resize(numBackBuffers);
    for (uint32_t i = 0; i < numBackBuffers; ++i) {
        vk::FenceCreateInfo fenceCreateInfo;
        vk::SemaphoreCreateInfo semaphoreCreateInfo;
        fenceCreateInfo.sType = vk::StructureType::eFenceCreateInfo;
        fenceCreateInfo.flags = (i == 0 ? vk::FenceCreateFlagBits::eSignaled : vk::FenceCreateFlags{});
        semaphoreCreateInfo.sType = vk::StructureType::eSemaphoreCreateInfo;
        _cmdBufferExecutedFences[i] = device.createFence(fenceCreateInfo);
        _imageAvailableSemaphores[i] = device.createSemaphore(semaphoreCreateInfo);
        _renderFinishedSemaphores[i] = device.createSemaphore(semaphoreCreateInfo);
    }
    CreateRenderPass();
    SetIsCreate(true);
}

void SwapChain::OnDestroy() {
    DestroyRenderPass();
    vk::Device device = GetDevice()->GetVKDevice();
    for (size_t i = 0; i < _cmdBufferExecutedFences.size(); ++i) {
        device.destroy(_cmdBufferExecutedFences[i]);
        device.destroySemaphore(_imageAvailableSemaphores[i]);
        device.destroySemaphore(_renderFinishedSemaphores[i]);
    }
    _cmdBufferExecutedFences.clear();
    _imageAvailableSemaphores.clear();
    _renderFinishedSemaphores.clear();

    SetIsCreate(false);
    SetDevice(nullptr);
}

void SwapChain::GetSemaphores(vk::Semaphore &imageAvailableSemaphore,
    vk::Semaphore &renderFinishedSemaphore,
    vk::Fence &cmdBufferExecutedFences) const {

    imageAvailableSemaphore = _imageAvailableSemaphores[_prevSemaphoreIndex];
    renderFinishedSemaphore = _renderFinishedSemaphores[_semaphoreIndex];
    cmdBufferExecutedFences = _cmdBufferExecutedFences[_semaphoreIndex];
}

void SwapChain::Resize(uint32_t width, uint32_t height, bool bVSyncOn) {
    OnDestroyWindowDependentResources();
    OnCreateWindowDependentResources(width, height, bVSyncOn);
}

auto SwapChain::Present() -> vk::Result {
    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &_renderFinishedSemaphores[_semaphoreIndex];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &_swapChain;
    presentInfo.pImageIndices = &_imageIndex;
    presentInfo.pResults = nullptr;
    return _presentQueue.presentKHR(presentInfo);
}

auto SwapChain::WaitForSwapChain() -> uint32_t {
    vk::Device device = GetDevice()->GetVKDevice();
    VKException::Throw(device.acquireNextImageKHR(_swapChain,
        UINT64_MAX,
        _imageAvailableSemaphores[_semaphoreIndex],
        VK_NULL_HANDLE,
        &_imageIndex));

    _prevSemaphoreIndex = _semaphoreIndex;
    _semaphoreIndex = (_semaphoreIndex + 1 % _backBufferCount);
    VKException::Throw(device.waitForFences(1, &_cmdBufferExecutedFences[_prevSemaphoreIndex], VK_TRUE, UINT64_MAX));
    VKException::Throw(device.resetFences(1, &_cmdBufferExecutedFences[_prevSemaphoreIndex]));
    return _imageIndex;
}

auto SwapChain::GetCurrentBackBuffer() const -> vk::Image {
    return _images[_imageIndex];
}

auto SwapChain::GetCurrentBackBufferRTV() const -> vk::ImageView {
    return _imageViews[_imageIndex];
}

auto SwapChain::GetSwapChain() const -> vk::SwapchainKHR {
    return _swapChain;
}

auto SwapChain::GetFormat() const -> vk::Format {
    return _swapChainFormat.format;
}

auto SwapChain::GetRenderPass() const -> vk::RenderPass {
    return _renderPass;
}

auto SwapChain::GetFrameBuffer(size_t index) const -> vk::Framebuffer {
    return _framebuffers[index];
}

auto SwapChain::GetCurrentFrameBuffer() const -> vk::Framebuffer {
    return _framebuffers[_imageIndex];
}

void SwapChain::CreateRTV() {
    auto device = GetDevice()->GetVKDevice();
    _imageViews.resize(_images.size());
    for (size_t i = 0; i < _imageViews.size(); ++i) {
        vk::ImageViewCreateInfo imageViewCreateInfo;
        imageViewCreateInfo.format = _swapChainFormat.format;
        imageViewCreateInfo.components.r = vk::ComponentSwizzle::eR;
        imageViewCreateInfo.components.g = vk::ComponentSwizzle::eG;
        imageViewCreateInfo.components.b = vk::ComponentSwizzle::eB;
        imageViewCreateInfo.components.a = vk::ComponentSwizzle::eA;
        imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
        imageViewCreateInfo.image = _images[i];
        _imageViews[i] = device.createImageView(imageViewCreateInfo);
        std::string resourceName = fmt::format("SwapChain_ImageView_{}", i);
        SetResourceName(device, _imageViews[i], resourceName);
    }
}

void SwapChain::DestroyRTV() {
    auto device = GetDevice()->GetVKDevice();
    for (vk::ImageView imageView : _imageViews) {
        device.destroyImageView(imageView);
    }
    _imageViews.clear();
}

void SwapChain::CreateRenderPass() {
    vk::AttachmentDescription attachments[1];
    attachments[0].format = _swapChainFormat.format;
    attachments[0].samples = vk::SampleCountFlagBits::e1;
    attachments[0].loadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference colorReference;
    colorReference.attachment = 0;
    colorReference.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass;
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorReference;

    vk::SubpassDependency dep;
    dep.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    dep.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dep.srcAccessMask = {};
    dep.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dep.dstSubpass = VK_SUBPASS_EXTERNAL;

    vk::RenderPassCreateInfo renderPassCreateInfo;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = attachments;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dep;
    _renderPass = GetDevice()->GetVKDevice().createRenderPass(renderPassCreateInfo);
}

void SwapChain::DestroyRenderPass() {
    if (_renderPass) {
        GetDevice()->GetVKDevice().destroyRenderPass(_renderPass);
        _renderPass = nullptr;
    }
}

void SwapChain::CreateFrameBuffers(size_t width, size_t height) {
    vk::Device device = GetDevice()->GetVKDevice();
    _framebuffers.resize(_imageViews.size());
    for (size_t i = 0; i < _framebuffers.size(); ++i) {
        vk::ImageView attachments[] = {_imageViews[i]};
        vk::FramebufferCreateInfo framebufferCreateInfo;
        framebufferCreateInfo.renderPass = _renderPass;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = attachments;
        framebufferCreateInfo.width = width;
        framebufferCreateInfo.height = height;
        framebufferCreateInfo.layers = 1;
        _framebuffers[i] = device.createFramebuffer(framebufferCreateInfo);
        std::string resourceName = fmt::format("SwapChain_FrameBuffer_{}", i);
        SetResourceName(device, _framebuffers[i], resourceName);
    }
}

void SwapChain::DestroyFrameBuffers() {
    vk::Device device = GetDevice()->GetVKDevice();
    for (size_t i = 0; i < _framebuffers.size(); ++i) {
        device.destroyFramebuffer(_framebuffers[i]);
    }
    _framebuffers.clear();
}

void SwapChain::OnCreateWindowDependentResources(size_t width, size_t height, bool bVSyncOn) {
    _VSyncOn = bVSyncOn;
    CreateRenderPass();

    vk::PhysicalDevice physicalDevice = GetDevice()->GetPhysicalDevice();
    vk::SurfaceKHR surface = GetDevice()->GetSurface();
    vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);

    vk::Extent2D swapChainExtent;
    swapChainExtent.width = width;
    swapChainExtent.height = height;
    if (surfaceCapabilities.currentExtent.width == static_cast<uint32_t>(-1)) {
        if (swapChainExtent.width < surfaceCapabilities.minImageExtent.width) {
            swapChainExtent.width = surfaceCapabilities.minImageExtent.width;
        } else if (swapChainExtent.width > surfaceCapabilities.maxImageExtent.width) {
            swapChainExtent.width = surfaceCapabilities.maxImageExtent.width;
        }

        if (swapChainExtent.height < surfaceCapabilities.minImageExtent.height) {
            swapChainExtent.height = surfaceCapabilities.minImageExtent.height;
        } else if (swapChainExtent.height > surfaceCapabilities.maxImageExtent.height) {
            swapChainExtent.height = surfaceCapabilities.maxImageExtent.height;
        }
    }

    vk::SurfaceTransformFlagBitsKHR preTransform = (surfaceCapabilities.supportedTransforms &
                                                       vk::SurfaceTransformFlagBitsKHR::eIdentity)
                                                       ? vk::SurfaceTransformFlagBitsKHR::eIdentity
                                                       : surfaceCapabilities.currentTransform;
    vk::CompositeAlphaFlagBitsKHR compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    vk::CompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
        vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
        vk::CompositeAlphaFlagBitsKHR::eInherit,
    };
    for (vk::CompositeAlphaFlagBitsKHR flag : compositeAlphaFlags) {
        if (surfaceCapabilities.supportedCompositeAlpha & flag) {
            compositeAlpha = flag;
            break;
        }
    }

    // 不知道为什么调用这个, 也许是测试一下
    std::vector<vk::PresentModeKHR> presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

    vk::SwapchainCreateInfoKHR swapChainCreateInfo;
    swapChainCreateInfo.surface = surface;
    swapChainCreateInfo.imageFormat = _swapChainFormat.format;
    swapChainCreateInfo.minImageCount = _backBufferCount;
    swapChainCreateInfo.imageColorSpace = _swapChainFormat.colorSpace;
    swapChainCreateInfo.imageExtent = swapChainExtent;
    swapChainCreateInfo.preTransform = preTransform;
    swapChainCreateInfo.compositeAlpha = compositeAlpha;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.presentMode = (bVSyncOn ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eImmediate);
    swapChainCreateInfo.clipped = true;
    swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eColorAttachment;
    swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
    uint32_t queueFamilyIndices[2] = {GetDevice()->GetGraphicsQueueFamilyIndex(),
        GetDevice()->GetPresentQueueFamilyIndex()};
    if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
        swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }

    _swapChain = GetDevice()->GetVKDevice().createSwapchainKHR(swapChainCreateInfo);
    _images = GetDevice()->GetVKDevice().getSwapchainImagesKHR(_swapChain);
    ExceptionAssert(_backBufferCount == _images.size());

    CreateRTV();
    CreateFrameBuffers(width, height);
    _imageIndex = 0;
}

void SwapChain::OnDestroyWindowDependentResources() {
    DestroyRenderPass();
    DestroyFrameBuffers();
    DestroyRTV();
    GetDevice()->GetVKDevice().destroySwapchainKHR(_swapChain);
}

}    // namespace vkgfx
