#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include "Foundation/RuntimeStatic.h"
#include "VulkanRenderer/VKObject.h"

namespace vkgfx {
class Device;
}

class GUI : public vkgfx::VKObject {
public:
	void OnCreate(vkgfx::Device *pDevice, GLFWwindow *pWindow, vk::RenderPass renderPass);
	void OnDestroy();
	void NewFrame();
	void Draw(vk::CommandBuffer cmd);
private:
	static void CheckImGuiVkResult(VkResult err);
private:
	vk::DescriptorPool _descriptorPool;
};

inline RuntimeStatic<GUI> gGui;
