#pragma once
#include <imgui.h>
#include "VulkanRenderer/Texture.h"
#include "VulkanRenderer/VKObject.h"
#include <glm/glm.hpp>

namespace vkgfx {

class Device;
class UploadHeap;
class DynamicBufferRing;

}    // namespace vkgfx

class ImGUI : vkgfx::VKObject {
    struct ConstantBuffer {
	    glm::mat4 mvp;
    };
public:
    void OnCreate(vkgfx::Device *pDevice,
        vk::RenderPass renderPass,
        vkgfx::UploadHeap &uploadHeap,
        vkgfx::DynamicBufferRing *pConstantBuffer,
        float fontSize);
    void OnDestroy();
    void UpdateRenderPass(vk::RenderPass renderPass);
    void Draw(vk::CommandBuffer cmd);
private:
    vkgfx::DynamicBufferRing *_pConstantBuffer = nullptr;
    vk::Sampler _sampler;
    vkgfx::Texture _texture;
    vk::ImageView _textureSRV;
    vk::Pipeline _pipeline;
    vk::DescriptorBufferInfo _geometry;
    vk::PipelineLayout _pipelineLayout;
    vk::DescriptorPool _descriptorPool;
    vk::DescriptorSet _descriptorSet[128];
    uint32_t _currentDescriptorIndex = 0;
    vk::DescriptorSetLayout _descriptorSetLayout;
};
