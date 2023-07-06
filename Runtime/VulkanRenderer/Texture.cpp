#include "Texture.h"
#include "Device.h"
#include "ExtDebugUtils.h"
#include "VKException.h"

namespace vkgfx {

VmaAllocationCreateInfo Texture::sDefaultAllocationCreateInfo = [] -> VmaAllocationCreateInfo {
    VmaAllocationCreateInfo result = {};
    result.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    return result;
}();

void Texture::OnCreate(std::string_view name,
    Device *pDevice,
    const vk::ImageCreateInfo &createInfo,
    VmaAllocationInfo gpuImageAllocInfo,
    const VmaAllocationCreateInfo &imageAllocCreateInfo) {

    SetDevice(pDevice);
    _name = name;
    _imageCreateInfo = createInfo;

    VmaAllocator allocator = pDevice->GetAllocator();
    vk::Device device = GetDevice()->GetVKDevice();

    VkResult res = vmaCreateImage(allocator,
        reinterpret_cast<const VkImageCreateInfo *>(&_imageCreateInfo),
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
