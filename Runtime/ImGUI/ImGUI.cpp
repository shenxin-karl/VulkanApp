#include "ImGUI.h"
#include "VulkanRenderer/Device.h"
#include "VulkanRenderer/UploadHeap.h"
#include "VulkanRenderer/Utils.hpp"
#include <ShellScalingApi.h>

void ImGUI::OnCreate(vkgfx::Device *pDevice,
    vk::RenderPass renderPass,
    vkgfx::UploadHeap &uploadHeap,
    vkgfx::DynamicBufferRing *pConstantBuffer,
    float fontSize) {

    _pConstantBuffer = pConstantBuffer;
    _pDevice = pDevice;

    ImGuiIO &io = ImGui::GetIO();
    DEVICE_SCALE_FACTOR scaleFactor = GetScaleFactorForDevice(DEVICE_PRIMARY);
    float textScale = scaleFactor / 100.f;
    ImFontConfig fontConfig;
    fontConfig.SizePixels = fontSize * textScale;
    io.Fonts->AddFontDefault(&fontConfig);

    unsigned char *pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    size_t uploadSize = width * height * 4 * sizeof(char);

    vk::ImageCreateInfo imageCreateInfo;
    imageCreateInfo.imageType = vk::ImageType::e2D;
    imageCreateInfo.format = vk::Format::eR8G8B8A8Unorm;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
    imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
    imageCreateInfo.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
    imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
    imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;

    VmaAllocationCreateInfo imageAllocCreateInfo = {};
    imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    imageAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    _texture.OnCreate("ImGUI Tex", pDevice, imageCreateInfo, {}, imageAllocCreateInfo);

    vk::ImageViewCreateInfo imageViewCreateInfo;
    imageViewCreateInfo.image = _texture.GetImage();
    imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
    imageViewCreateInfo.format = _texture.GetFormat();
    imageViewCreateInfo.components = vkgfx::GetComponentMapping_RGBA();
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    _textureSRV = _pDevice->GetVKDevice().createImageView(imageViewCreateInfo);

    io.Fonts->TexID = static_cast<void *>(_textureSRV);
    uploadHeap.AllocBuffer(pixels, width * height * 4);

    vkgfx::UploadHeap::ImageUploadJob job;
    job.prevBarrier.dstAccessMask = vk::AccessFlagBits::eMemoryWrite;
    job.prevBarrier.oldLayout = vk::ImageLayout::eUndefined;
    job.prevBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
    job.prevBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    job.prevBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    job.prevBarrier.image = _texture.GetImage();
    job.prevBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    job.prevBarrier.subresourceRange.layerCount = 1;

    job.bufferImageCopy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    job.bufferImageCopy.imageSubresource.layerCount = 1;
    job.bufferImageCopy.imageExtent.width = width;
    job.bufferImageCopy.imageExtent.height = height;
    job.bufferImageCopy.imageExtent.depth = 1;

    job.postBarrier = job.prevBarrier;
    job.postBarrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
    job.postBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    job.postBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    uploadHeap.AddImageJob(job);

    uploadHeap.Flush();
}

void ImGUI::OnDestroy() {
    _texture.OnDestroy();
    _pDevice->GetVKDevice().destroyImageView(_textureSRV);
    _textureSRV = VK_NULL_HANDLE;
}

void ImGUI::UpdateRenderPass(vk::RenderPass renderPass) {
}

void ImGUI::Draw(vk::CommandBuffer cmd) {
}
