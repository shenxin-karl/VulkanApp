#include "ImGUI.h"

#include "Application/Application.h"
#include "VulkanRenderer/Device.h"
#include "VulkanRenderer/UploadHeap.h"
#include "VulkanRenderer/Utils.hpp"
#include "Shader/ShaderManager.h"
#include "Utils/AssetProjectSetting.h"
#include "VulkanRenderer/DefineList.h"
#include "VulkanRenderer/DynamicBufferRing.h"
#include "VulkanRenderer/ExtDebugUtils.h"
#include "VulkanRenderer/VKException.h"

static float GetScaleFactorForDevice() {
    HDC hdc = GetDC(NULL);
    int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
    int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(NULL, hdc);
    float scaleFactorX = dpiX / 96.0f; // 96 dpi is the standard
    float scaleFactorY = dpiY / 96.0f;
    return (scaleFactorX + scaleFactorY) / 2.0f;
}

void ImGUI::OnCreate(vkgfx::Device *pDevice,
    vk::RenderPass renderPass,
    vkgfx::UploadHeap &uploadHeap,
    vkgfx::DynamicBufferRing *pConstantBuffer,
    float fontSize) {

    _pDynamicBuffer = pConstantBuffer;
    SetDevice(pDevice);

    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    float scaleFactor = GetScaleFactorForDevice();
    ImFontConfig fontConfig;
    fontConfig.SizePixels = fontSize * scaleFactor;
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
        imageViewCreateInfo.subresourceRange.levelCount = 1;
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
    job.prevBarrier.subresourceRange.levelCount = 1;

    job.bufferImageCopy.bufferOffset = *pOffset;
    job.bufferImageCopy.bufferRowLength = 0;
    job.bufferImageCopy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    job.bufferImageCopy.imageSubresource.layerCount = 1;
    job.bufferImageCopy.imageSubresource.mipLevel = 0;
    job.bufferImageCopy.imageSubresource.baseArrayLayer = 0;
    job.bufferImageCopy.imageOffset = {0, 0, 0};
    job.bufferImageCopy.imageExtent = {static_cast<uint32>(width), static_cast<uint32>(height), 1};

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
        descriptorSetLayoutCreateInfo.bindingCount = 3;
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
            {vk::DescriptorType::eUniformBufferDynamic, 128},
            {vk::DescriptorType::eSampledImage, 128},
            {vk::DescriptorType::eSampler, 128},
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
            _pDynamicBuffer->AttachBufferToDescriptorSet(_descriptorSet[i], 0, sizeof(ConstantBuffer));

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

            writes[1].dstSet = _descriptorSet[i];
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
    device.destroyPipeline(_pipeline);

    _textureSRV = VK_NULL_HANDLE;
    _sampler = VK_NULL_HANDLE;
    _descriptorSetLayout = VK_NULL_HANDLE;
    _pipelineLayout = VK_NULL_HANDLE;
    _descriptorPool = VK_NULL_HANDLE;
    _pipeline = VK_NULL_HANDLE;

    ImGui::DestroyContext();
}

void ImGUI::SetWindowSize(size_t width, size_t height) {
    _width = width;
    _height = height;
}

void ImGUI::UpdateRenderPass(vk::RenderPass renderPass) {
    vk::Device device = GetDevice()->GetVKDevice();
    if (_pipeline) {
        device.destroyPipeline(_pipeline);
    }

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

    vk::VertexInputBindingDescription viBinding;
    viBinding.binding = 0;
    viBinding.stride = sizeof(ImDrawVert);
    viBinding.inputRate = vk::VertexInputRate::eVertex;

    // clang-format off
    vk::VertexInputAttributeDescription viAttrs[] = {
    	{0, 0, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, pos)},
        {1, 0, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, uv)},
        {2, 0, vk::Format::eR8G8B8A8Unorm, offsetof(ImDrawVert, col)}
    };
    // clang-format on

    vk::PipelineVertexInputStateCreateInfo viCreateInfo;
    viCreateInfo.vertexBindingDescriptionCount = 1;
    viCreateInfo.pVertexBindingDescriptions = &viBinding;
    viCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32>(std::size(viAttrs));
    viCreateInfo.pVertexAttributeDescriptions = viAttrs;

    vk::PipelineRasterizationStateCreateInfo rasterizationCreateInfo;
    rasterizationCreateInfo.polygonMode = vk::PolygonMode::eFill;
    rasterizationCreateInfo.frontFace = vk::FrontFace::eClockwise;
    rasterizationCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizationCreateInfo.lineWidth = 1.0f;

    vk::PipelineColorBlendAttachmentState blendAttrs = vkgfx::GetColorBlendAttachmentState_Blend();
    vk::PipelineColorBlendStateCreateInfo colorBlendState;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &blendAttrs;
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.blendConstants = std::array{1.f, 1.f, 1.f, 1.f};

    vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &viCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = vkgfx::GetInputAssemblyState_TriangleList();
    pipelineCreateInfo.pTessellationState = nullptr;
    pipelineCreateInfo.pViewportState = vkgfx::GetViewportState<1, 1>();
    pipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
    pipelineCreateInfo.pMultisampleState = vkgfx::GetMultiSampleState_Disable();
    pipelineCreateInfo.pDepthStencilState = vkgfx::GetDepthStencilState_DisableDepthStencil();
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pDynamicState = vkgfx::GetDynamicState_ViewportScissor();
    pipelineCreateInfo.layout = _pipelineLayout;
    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = 0;
    _pipeline = device.createGraphicsPipeline(GetDevice()->GetPipelineCache(), pipelineCreateInfo).value;
}

void ImGUI::NewFrame() {
    ImGuiIO &io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    io.DisplaySize.x = static_cast<float>(_width);
    io.DisplaySize.y = static_cast<float>(_height);

    // Read keyboard modifiers inputs
    io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
    io.KeySuper = false;
    // io.KeysDown : filled by WM_KEYDOWN/WM_KEYUP events
    // io.MousePos : filled by WM_MOUSEMOVE events
    // io.MouseDown : filled by WM_*BUTTON* events
    // io.MouseWheel : filled by WM_MOUSEWHEEL events
    // Hide OS mouse cursor if ImGui is drawing it
    if (io.MouseDrawCursor)
        SetCursor(NULL);    // Start the frame
    ImGui::NewFrame();
}

void ImGUI::EndFrame() {
    ImGui::EndFrame();
}

void ImGUI::Draw(vk::CommandBuffer cmd) {
    vkgfx::PrefMarkerGuard prefMarkerGuard(cmd, "ImGUI Pass");
    ImGui::Render();
    if (!_pipeline) {
        return;
    }

    ImDrawData *pDrawData = ImGui::GetDrawData();
    if (pDrawData->CmdListsCount == 0) {
	    return;
    }

    ImDrawVert *pVertexBuffer = nullptr;
    ImDrawIdx *pIndicesBuffer = nullptr;
    size_t totalVertexSize = pDrawData->TotalVtxCount * sizeof(ImDrawVert);
    size_t totalIndicesSize = pDrawData->TotalIdxCount * sizeof(ImDrawIdx);
    vk::DescriptorBufferInfo vbv = _pDynamicBuffer->AllocBuffer(totalVertexSize, &pVertexBuffer).value();
    vk::DescriptorBufferInfo ibv = _pDynamicBuffer->AllocBuffer(totalIndicesSize, &pIndicesBuffer).value();
    for (int i = 0; i < pDrawData->CmdListsCount; ++i) {
        const ImDrawList *pCmdList = pDrawData->CmdLists[i];
        memcpy(pVertexBuffer, pCmdList->VtxBuffer.Data, pCmdList->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(pIndicesBuffer, pCmdList->IdxBuffer.Data, pCmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
        pVertexBuffer += pCmdList->VtxBuffer.Size;
        pIndicesBuffer += pCmdList->IdxBuffer.Size;
    }

    vk::DescriptorBufferInfo cbv;
    {
        float L = 0.0f;
        float R = ImGui::GetIO().DisplaySize.x;
        float B = ImGui::GetIO().DisplaySize.x;
        float T = 0.0f;

        ConstantBuffer cbuffer;
        // clang-format off
        cbuffer.mvp = 
	    {
		    { 2.0f / (R-L),     0.0f,               0.0f,       0.0f },
		    { 0.0f,             2.0f / (T-B),       0.0f,       0.0f },
		    { 0.0f,             0.0f,               0.5f,       0.0f },
		    { (R+L) / (L-R),    (T+B) / (B-T),      0.5f,       1.0f },
	    };
        // clang-format on
        cbv = _pDynamicBuffer->AllocBuffer(cbuffer).value();
    }

    vk::Viewport viewport;
    viewport.x = 0;
    viewport.y = ImGui::GetIO().DisplaySize.y;
    viewport.width = ImGui::GetIO().DisplaySize.x;
    viewport.height = -ImGui::GetIO().DisplaySize.y;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    cmd.setViewport(0, viewport);
    cmd.bindVertexBuffers(0, 1, &vbv.buffer, &vbv.offset);
    cmd.bindIndexBuffer(ibv.buffer, ibv.offset, vk::IndexType::eUint16);
    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);

    int vtxOffset = 0;
    int idxOffset = 0;
    ImTextureID pTextureID = nullptr;
    vk::Device device = GetDevice()->GetVKDevice();
    uint32_t uniformOffsets[1] = {static_cast<uint32_t>(cbv.offset)};

    for (int i = 0; i < pDrawData->CmdListsCount; ++i) {
        const ImDrawList *pCmdList = pDrawData->CmdLists[i];
        for (int j = 0; j < pCmdList->CmdBuffer.size(); ++j) {
            const ImDrawCmd *pCmd = &pCmdList->CmdBuffer[j];
            if (pCmd->UserCallback != nullptr) {
                pCmd->UserCallback(pCmdList, pCmd);
                continue;
            }

            vk::Rect2D scissor;
            ImVec2 rectMin = pCmdList->GetClipRectMin();
            ImVec2 rectMax = pCmdList->GetClipRectMax();
            scissor.offset.x = rectMin.x;
            scissor.offset.y = rectMin.y;
            scissor.extent.width = rectMax.x;
            scissor.extent.height = rectMax.y;
            cmd.setScissor(0, scissor);

            if (pTextureID != pCmd->GetTexID()) {
                pTextureID = pCmd->GetTexID();
                vk::DescriptorImageInfo desc_image[1] = {};
                desc_image[0].sampler = _sampler;
                desc_image[0].imageView = static_cast<VkImageView>(pCmd->TextureId);
                desc_image[0].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

                vk::WriteDescriptorSet writes[2];
                writes[0].dstSet = _descriptorSet[_currentDescriptorIndex];
                writes[0].descriptorCount = 1;
                writes[0].descriptorType = vk::DescriptorType::eSampledImage;
                writes[0].pImageInfo = desc_image;
                writes[0].dstArrayElement = 0;
                writes[0].dstBinding = 1;

                writes[1].dstSet = _descriptorSet[_currentDescriptorIndex];
                writes[1].descriptorCount = 1;
                writes[1].descriptorType = vk::DescriptorType::eSampler;
                writes[1].pImageInfo = desc_image;
                writes[1].dstArrayElement = 0;
                writes[1].dstBinding = 2;

                device.updateDescriptorSets(2, writes, 0, nullptr);
                cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                    _pipelineLayout,
                    0,
                    1,
                    &_descriptorSet[_currentDescriptorIndex],
                    1,
                    uniformOffsets);
                ++_currentDescriptorIndex;
                _currentDescriptorIndex &= 127;
            }
            cmd.drawIndexed(pCmd->ElemCount, 1, idxOffset, vtxOffset, 0);
            idxOffset += pCmd->ElemCount;
        }
        vtxOffset += pCmdList->VtxBuffer.size();
    }
}
