#include "SwapChain.h"
#include "Device.h"
#include "ExtDebugUtils.h"
#include "VKException.h"
#include "Foundation/Exception.h"

namespace vkgfx {

void SwapChain::OnCreate(Device *pDevice, uint32_t numBackBuffers) {
    SetDevice(pDevice);
    _backBufferCount = numBackBuffers;
    _presentQueue = pDevice->GetPresentQueue();

    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(pDevice->GetPhysicalDevice());
    ChooseSwapSurfaceFormat(swapChainSupport);

    vk::Device device = pDevice->GetVKDevice();
    _imageAvailableSemaphores.resize(numBackBuffers);
    for (uint32_t i = 0; i < numBackBuffers; ++i) {
        vk::SemaphoreCreateInfo semaphoreCreateInfo;
        _imageAvailableSemaphores[i] = device.createSemaphore(semaphoreCreateInfo);
    }
    CreateRenderPass();
    SetIsCreate(true);
}

void SwapChain::OnDestroy() {
    DestroyRenderPass();
    vk::Device device = GetDevice()->GetVKDevice();
    for (size_t i = 0; i < _imageAvailableSemaphores.size(); ++i) {
        device.destroySemaphore(_imageAvailableSemaphores[i]);
    }
    _imageAvailableSemaphores.clear();

    SetIsCreate(false);
    SetDevice(nullptr);
}

void SwapChain::Resize(uint32_t width, uint32_t height, bool bVSyncOn) {
    OnDestroyWindowDependentResources();
    OnCreateWindowDependentResources(width, height, bVSyncOn);
}

auto SwapChain::Present(vk::Semaphore renderFinishedSemaphore) -> vk::Result {
    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
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
    return _imageIndex;
}

auto SwapChain::GetImageAvailableSemaphore() const -> vk::Semaphore {
    return _imageAvailableSemaphores[_prevSemaphoreIndex];
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
    //vk::AttachmentDescription attachments[1];
    //attachments[0].format = _swapChainFormat.format;
    //attachments[0].samples = vk::SampleCountFlagBits::e1;
    //attachments[0].loadOp = vk::AttachmentLoadOp::eDontCare;
    //attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
    //attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    //attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    //attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;
    //attachments[0].flags = {};

    //vk::AttachmentReference colorReference;
    //colorReference.attachment = 0;
    //colorReference.layout = vk::ImageLayout::eColorAttachmentOptimal;

    //vk::SubpassDescription subpass;
    //subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    //subpass.colorAttachmentCount = 1;
    //subpass.pColorAttachments = &colorReference;

    //vk::SubpassDependency dep;
    //dep.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    //dep.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    //dep.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    //dep.dstSubpass = VK_SUBPASS_EXTERNAL;

    //vk::RenderPassCreateInfo renderPassCreateInfo;
    //renderPassCreateInfo.attachmentCount = 1;
    //renderPassCreateInfo.pAttachments = attachments;
    //renderPassCreateInfo.subpassCount = 1;
    //renderPassCreateInfo.pSubpasses = &subpass;
    //renderPassCreateInfo.dependencyCount = 1;
    //renderPassCreateInfo.pDependencies = &dep;
    //_renderPass = GetDevice()->GetVKDevice().createRenderPass(renderPassCreateInfo);

         // color RT
        VkAttachmentDescription attachments[1];
        attachments[0].format = (VkFormat)_swapChainFormat.format;
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachments[0].flags = 0;

        VkAttachmentReference color_reference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.flags = 0;
        subpass.inputAttachmentCount = 0;
        subpass.pInputAttachments = NULL;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_reference;
        subpass.pResolveAttachments = NULL;
        subpass.pDepthStencilAttachment = NULL;
        subpass.preserveAttachmentCount = 0;
        subpass.pPreserveAttachments = NULL;

        VkSubpassDependency dep = {};
        dep.dependencyFlags = 0;
        dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dep.dstSubpass = 0;
        dep.srcAccessMask = 0;
        dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dep.srcSubpass = VK_SUBPASS_EXTERNAL;

        VkRenderPassCreateInfo rp_info = {};
        rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        rp_info.pNext = NULL;
        rp_info.attachmentCount = 1;
        rp_info.pAttachments = attachments;
        rp_info.subpassCount = 1;
        rp_info.pSubpasses = &subpass;
        rp_info.dependencyCount = 1;
        rp_info.pDependencies = &dep;

        VkRenderPass m_render_pass_swap_chain = VK_NULL_HANDLE;
        VkResult res = vkCreateRenderPass(GetDevice()->GetVKDevice(), &rp_info, NULL, &m_render_pass_swap_chain);
        assert(res == VK_SUCCESS);
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

auto SwapChain::QuerySwapChainSupport(vk::PhysicalDevice physicalDevice) const -> SwapChainSupportDetails {
    SwapChainSupportDetails details;
    details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(GetDevice()->GetSurface());
    details.formats = physicalDevice.getSurfaceFormatsKHR(GetDevice()->GetSurface());
    details.presentModes = physicalDevice.getSurfacePresentModesKHR(GetDevice()->GetSurface());
    return details;
}

void SwapChain::ChooseSwapSurfaceFormat(const SwapChainSupportDetails &swapChainSupport) {
    for (const vk::SurfaceFormatKHR &surfaceFormat : swapChainSupport.formats) {
        const vk::Format format = surfaceFormat.format;
        if (format == vk::Format::eR8G8B8A8Unorm || format == vk::Format::eB8G8R8A8Unorm ||
            format == vk::Format::eA2B10G10R10UnormPack32 || format == vk::Format::eA2R10G10B10UnormPack32 ||
            format == vk::Format::eR16G16B16A16Sfloat) {
            _swapChainFormat = surfaceFormat;
            return;
        }
    }
    Exception::Throw("There is no suitable swap chain format");
}

}    // namespace vkgfx
