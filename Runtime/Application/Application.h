#pragma once
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include "IApplication.h"

class Application : public IApplication {
public:
	void Startup() override;
	void Cleanup() override;
	bool IsDone() const override;
	void Update(std::shared_ptr<GameTimer> pGameTimer) override;
	void RenderScene() override;
	~Application() override;
public:
    void SetupGlfw();
	void DestroyGlfw();
	void SetupVulkan();
    void DestroyVulkan();
	static void GlfwErrorCallback(int error, const char *description);
private:
	bool						_isDone = false;
	GLFWwindow *				_pWindow = nullptr;
	vk::Instance				_vkInstance;
	vk::PhysicalDevice			_vkPhysicalDevice;
	vk::Device					_vkDevice;
#if !defined(NDEBUG)
	vk::DebugUtilsMessengerEXT	_vkDebugUtilsMessenger;
#endif
	uint32_t					_graphicsQueueFamilyIndex = -1;
	uint32_t					_presentQueueFamilyIndex  = -1;
	vk::Queue					_graphicsQueue;
	vk::Queue					_presentQueue;
	vk::CommandPool				_graphicsPool;
};