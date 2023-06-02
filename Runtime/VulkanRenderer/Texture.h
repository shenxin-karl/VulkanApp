#pragma once
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>
#include "VKObject.h"

namespace vkgfx {

class Texture : public VKObject {
public:
    void OnCreate(Device *pDevice, const vk::ImageCreateInfo &createInfo, std::string_view name);
    void OnDestroy();
    auto GetWidth() const -> uint32_t {
        return _imageCreateInfo.extent.width;
    }
    auto GetHeight() const -> uint32_t {
        return _imageCreateInfo.extent.height;
    }
    auto GetMipCount() const -> uint32_t {
        return _imageCreateInfo.mipLevels;
    }
    auto GetArraySize() const -> uint32_t {
        return _imageCreateInfo.arrayLayers;
    }
    auto GetFormat() const -> vk::Format {
        return _imageCreateInfo.format;
    }
    auto GetName() const -> const std::string & {
        return _name;
    }
    auto GetImage() const -> vk::Image {
        return _image;
    }
    bool IsSupportRTV() const {
        return _isSupportRTV && _imageCreateInfo.usage & vk::ImageUsageFlagBits::eColorAttachment;
    }
    bool IsSupportDSV() const {
        return _isSupportDSV && _imageCreateInfo.usage & vk::ImageUsageFlagBits::eDepthStencilAttachment;
    }
    bool IsSupportSRV() const {
        return _isSupportSRV && _imageCreateInfo.usage & vk::ImageUsageFlagBits::eSampled;
    }
    bool IsSupportUAV() const {
        return _isSupportUAV && _imageCreateInfo.usage & vk::ImageUsageFlagBits::eStorage;
    }
    bool IsSupportUAVAtomic() const {
	    return _isSupportUAVAtomic && _imageCreateInfo.usage & vk::ImageUsageFlagBits::eStorage;
    }
private:
    void CheckViewSupport();
private:
    std::string _name;
    vk::Image _image;
    VmaAllocation _imageAlloc = VK_NULL_HANDLE;
    vk::ImageCreateInfo _imageCreateInfo;
    bool _isSupportRTV = false;
    bool _isSupportDSV = false;
    bool _isSupportSRV = false;
    bool _isSupportUAV = false;
    bool _isSupportUAVAtomic = false;
};

}    // namespace vkgfx
