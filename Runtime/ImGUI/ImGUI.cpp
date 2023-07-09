#include "ImGUI.h"
#include "VulkanRenderer/Device.h"
#include "VulkanRenderer/UploadHeap.h"
#include "VulkanRenderer/Utils.hpp"
#include "Shader/ShaderManager.h"
#include "Utils/AssetProjectSetting.h"
#include "VulkanRenderer/DefineList.h"
#include "VulkanRenderer/DynamicBufferRing.h"
#include "VulkanRenderer/ExtDebugUtils.h"
#include "VulkanRenderer/VKException.h"

void ImGUI::OnCreate(vkgfx::Device *pDevice,
                     vk::RenderPass renderPass,
                     vkgfx::UploadHeap &uploadHeap,
                     vkgfx::DynamicBufferRing *pConstantBuffer,
                     float fontSize) {

    _pConstantBuffer = pConstantBuffer;
    SetDevice(pDevice);

    ImGuiIO &io = ImGui::GetIO();
    //DEVICE_SCALE_FACTOR scaleFactor = GetScaleFactorForDevice(DEVICE_PRIMARY);
    //float textScale = scaleFactor / 100.f;
    float textScale = 1.0;
    ImFontConfig fontConfig;
    fontConfig.SizePixels = fontSize * textScale;
    io.Fonts->AddFontDefault(&fontConfig);

    unsigned char *pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    vk::Device device = pDevice->GetVKDevice();
	// create texture and srv
    {
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
	    _textureSRV = device.createImageView(imageViewCreateInfo);
    }

	// create sampler
    {
		vk::SamplerCreateInfo samplerCreateInfo;
		samplerCreateInfo.magFilter = vk::Filter::eNearest;
		samplerCreateInfo.minFilter = vk::Filter::eNearest;
		samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
		samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
		samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
		samplerCreateInfo.minLod = -1000;
		samplerCreateInfo.maxLod = +1000;
		samplerCreateInfo.maxAnisotropy = 1.f;
		_sampler = device.createSampler(samplerCreateInfo);
    }

    io.Fonts->TexID = static_cast<void *>(_textureSRV);
    std::optional<vk::DeviceSize> pOffset = uploadHeap.AllocBuffer(pixels, width * height * 4);

    vkgfx::UploadHeap::ImageUploadJob job;
    job.prevBarrier.dstAccessMask = vk::AccessFlagBits::eMemoryWrite;
    job.prevBarrier.oldLayout = vk::ImageLayout::eUndefined;
    job.prevBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
    job.prevBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    job.prevBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    job.prevBarrier.image = _texture.GetImage();
    job.prevBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    job.prevBarrier.subresourceRange.layerCount = 1;

    job.bufferImageCopy.bufferOffset = *pOffset;
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


    // create descriptor layout
    {
	    vk::DescriptorSetLayoutBinding layoutBinding[3];
	    layoutBinding[0].binding = 0;
	    layoutBinding[0].descriptorType = vk::DescriptorType::eUniformBufferDynamic;
	    layoutBinding[0].descriptorCount = 1;
	    layoutBinding[0].stageFlags = vk::ShaderStageFlagBits::eVertex;

	    layoutBinding[1].binding = 1;
	    layoutBinding[1].descriptorType = vk::DescriptorType::eSampledImage;
	    layoutBinding[1].descriptorCount = 1;
	    layoutBinding[1].stageFlags = vk::ShaderStageFlagBits::eFragment;

        layoutBinding[2].binding = 2;
        layoutBinding[2].descriptorType = vk::DescriptorType::eSampler;
	    layoutBinding[2].descriptorCount = 1;
	    layoutBinding[2].stageFlags = vk::ShaderStageFlagBits::eFragment;

	    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
	    descriptorSetLayoutCreateInfo.bindingCount = 2;
	    descriptorSetLayoutCreateInfo.pBindings = layoutBinding;

	    _descriptorSetLayout = device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
	    vkgfx::SetResourceName(device, _pipelineLayout, "ImGUI DescriptorSetLayout");
    }
    // create pipeline layout
    {
	    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
	    pipelineLayoutCreateInfo.setLayoutCount = 1;
	    pipelineLayoutCreateInfo.pSetLayouts = &_descriptorSetLayout;
	    _pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
	    vkgfx::SetResourceName(device, _pipelineLayout, "ImGUI PipelineLayout");
    }

    // create descriptor pool, allocate and update the descripties
    {
	    vk::DescriptorPoolSize poolTypeCount[] = {
            { vk::DescriptorType::eUniformBufferDynamic, 128 },
            { vk::DescriptorType::eSampledImage, 128 },
            { vk::DescriptorType::eSampler, 128 },
	    };
        vk::DescriptorPoolCreateInfo poolCreateInfo;
        poolCreateInfo.maxSets = 128 * 3;
        poolCreateInfo.poolSizeCount = 3;
        poolCreateInfo.pPoolSizes = poolTypeCount;
        _descriptorPool = device.createDescriptorPool(poolCreateInfo);

        vk::DescriptorSetAllocateInfo allocateInfo;
        allocateInfo.descriptorPool = _descriptorPool;
        allocateInfo.descriptorSetCount = 1;
        allocateInfo.pSetLayouts = &_descriptorSetLayout;
        for (size_t i = 0; i < 128; ++i) {
	        vk::Result res = device.allocateDescriptorSets(&allocateInfo, &_descriptorSet[i]);
            VKException::Throw(res);
			_pConstantBuffer->AttachBufferToDescriptorSet(_descriptorSet[i], 0, sizeof(ConstantBuffer));

            vk::DescriptorImageInfo descriptorImage[1];
            descriptorImage[0].sampler = _sampler;
            descriptorImage[0].imageView = _textureSRV;
            descriptorImage[0].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

            vk::WriteDescriptorSet writes[2];
            writes[0].dstSet = _descriptorSet[i];
            writes[0].descriptorCount = 1;
            writes[0].descriptorType = vk::DescriptorType::eSampledImage;
            writes[0].pImageInfo = descriptorImage;
            writes[0].dstArrayElement = 0;
            writes[0].dstBinding = 1;

            writes[1].descriptorCount = 1;
            writes[1].descriptorType = vk::DescriptorType::eSampler;
            writes[1].pImageInfo = descriptorImage;
            writes[1].dstArrayElement = 0;
            writes[1].dstBinding = 2;
            device.updateDescriptorSets(2, writes, 0, nullptr);
        }
    }

    UpdateRenderPass(renderPass);
    SetIsCreate(true);
}

void ImGUI::OnDestroy() {
    if (!GetIsCreate()) {
	    return;
    }

    vk::Device device = GetDevice()->GetVKDevice();
	_texture.OnDestroy();
	device.destroyImageView(_textureSRV);
	device.destroySampler(_sampler);
	device.destroyDescriptorSetLayout(_descriptorSetLayout);
    device.destroyPipelineLayout(_pipelineLayout);
    device.destroyDescriptorPool(_descriptorPool);

	_textureSRV = VK_NULL_HANDLE;
    _sampler = VK_NULL_HANDLE;
    _descriptorSetLayout = VK_NULL_HANDLE;
    _pipelineLayout = VK_NULL_HANDLE;
    _descriptorPool = VK_NULL_HANDLE;
}

void ImGUI::UpdateRenderPass(vk::RenderPass renderPass) {
	stdfs::path sourcePath = gAssetProjectSetting->GetAssetAbsolutePath() / "Shaders" / "ImGUI.hlsl";
    vk::PipelineShaderStageCreateInfo shaderStages[2];
    ShaderLoadInfo vertexLoadInfo = {
        sourcePath,
        "VSMain",
        vkgfx::ShaderType::kVS,
    };
    gShaderManager->LoadShaderStageCreateInfo(vertexLoadInfo, shaderStages[0]);

    ShaderLoadInfo pixelLoadInfo = {
        sourcePath,
        "PSMain",
        vkgfx::ShaderType::kPS,
    };
    gShaderManager->LoadShaderStageCreateInfo(pixelLoadInfo, shaderStages[1]);
}

void ImGUI::Draw(vk::CommandBuffer cmd) {
}
