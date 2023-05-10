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
	void SetupVulkan();
    void DestroyVulkan();
	static void GlfwErrorCallback(int error, const char *description);
    static bool IsExtensionAvailable(const ImVector<VkExtensionProperties> & properties, const char * str);
private:
	bool			_isDone = false;
	GLFWwindow *	_pWindow = nullptr;
	vk::Instance	_instance;
};