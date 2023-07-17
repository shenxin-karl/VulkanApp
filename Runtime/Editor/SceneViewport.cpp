#include "SceneViewport.h"
#include "ImGUI/Libary/imgui.h"
#include "ImGUI/Libary/imgui_impl_vulkan.h"
#include "VulkanRenderer/Device.h"

SceneViewport::SceneViewport() : IViewport("Scene") {
    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.minLod = -1000;
    samplerInfo.maxLod = 1000;
    samplerInfo.maxAnisotropy = 1.0f;
    _sampler = vkgfx::gDevice->GetVKDevice().createSampler(samplerInfo);
}

SceneViewport::~SceneViewport() {
    vk::Device device = vkgfx::gDevice->GetVKDevice();
    if (_sampler) {
        device.destroySampler(_sampler);
        _sampler = VK_NULL_HANDLE;
    }
}

void SceneViewport::OnGUI(GameTimer &gameTimer) {
    if (!isShow) {
        return;
    }
    ImGui::Begin("Scene", &isShow);
    {
	    ImVec2 size = ImGui::GetWindowSize();
	    if (_textureId) {
		    ImGui::Image(_textureId, size);
		}
    }
    ImGui::End();
}

void SceneViewport::SetSceneRenderTargetView(vk::ImageView view) {
    if (view == _sceneRenderTargetView) {
        return;
    }
    RemoveImGuiTexture();
    AddImGuiTexture(view);
}

void SceneViewport::RemoveImGuiTexture() {
    if (_textureId) {
        ImGui_ImplVulkan_RemoveTexture(_textureId);
        _textureId = VK_NULL_HANDLE;
    }
    _sceneRenderTargetView = VK_NULL_HANDLE;
}

void SceneViewport::AddImGuiTexture(vk::ImageView view) {
    _sceneRenderTargetView = view;
    _textureId = ImGui_ImplVulkan_AddTexture(_sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
