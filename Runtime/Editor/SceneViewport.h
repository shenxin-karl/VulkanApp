#pragma once
#include <vulkan/vulkan.hpp>
#include "IViewport.h"

class SceneViewport : public IViewport {
public:
	SceneViewport();
	~SceneViewport() override;
	void OnGUI(GameTimer &gameTimer) override;
	void SetSceneRenderTargetView(vk::ImageView view);
private:
	void RemoveImGuiTexture();
	void AddImGuiTexture(vk::ImageView view);
private:
	vk::ImageView _sceneRenderTargetView;
	vk::Sampler _sampler;
	vk::DescriptorSet _textureId;
	size_t _width = 0;
	size_t _height = 0;
};
