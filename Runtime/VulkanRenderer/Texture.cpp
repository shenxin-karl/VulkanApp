#include "Texture.h"
#include "Device.h"
#include "ExtDebugUtils.h"
#include "VKException.h"

namespace vkgfx {

void Texture::OnCreate(Device *pDevice, const vk::ImageCreateInfo &createInfo, std::string_view name) {
    SetDevice(pDevice);
    _name = name;

    VmaAllocator allocator = pDevice->GetAllocator();
    vk::Device device = GetDevice()->GetVKDevice();

    VmaAllocationInfo gpuImageAllocInfo = {};
    VmaAllocationCreateInfo imageAllocCreateInfo = {};
    imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    VkResult res = vmaCreateImage(allocator,
        reinterpret_cast<const VkImageCreateInfo *>(&createInfo),
        &imageAllocCreateInfo,
        reinterpret_cast<VkImage *>(&_image),
        &_imageAlloc,
        &gpuImageAllocInfo);
    VKException::Throw(res);

    SetResourceName(device, _image, name);
    CheckViewSupport();
    SetIsCreate(true);
}

void Texture::OnDestroy() {
    SetIsCreate(false);
    VmaAllocator allocator = GetDevice()->GetAllocator();
    vmaDestroyImage(allocator, _image, _imageAlloc);
    SetDevice(nullptr);
}

void Texture::CheckViewSupport() {
    vk::PhysicalDevice physicalDevice = GetDevice()->GetPhysicalDevice();
    vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(_imageCreateInfo.format);
    if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eColorAttachment) {
        _isSupportRTV = true;
    }
    if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
        _isSupportDSV = true;
    }
    if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage) {
        _isSupportSRV = true;
    }
    if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eStorageImage) {
        _isSupportUAV = true;
    }
    if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eStorageImageAtomic) {
        _isSupportUAVAtomic = true;
    }
}

}    // namespace vkgfx
