#pragma once
#include <imgui.h>
#include <GLFW/glfw3.h>
#include "VulkanRenderer/Texture.h"
#include "VulkanRenderer/VKObject.h"
#include <glm/glm.hpp>
#include "Foundation/RuntimeStatic.h"

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
    void SetWindowSize(size_t width, size_t height);
    void UpdateRenderPass(vk::RenderPass renderPass);
    void NewFrame();
    void EndFrame();
    void Draw(vk::CommandBuffer cmd);
private:
    vkgfx::DynamicBufferRing *_pDynamicBuffer = nullptr;
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
    size_t _width = 0;
    size_t _height = 0;
};

inline RuntimeStatic<ImGUI> gImGUI;