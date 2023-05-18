#include "Application.h"
#include "Foundation/Logger.h"
#include "VulkanUtils/utils.h"
#include "VulkanRenderer/Device.h"

#include <imgui.h>

Application::Application() {

}

void Application::Startup() {
	gLogger.Initialize();
	gLogger->StartLogging();

	SetupGlfw();
	SetupVulkan();
}

void Application::Cleanup() {
	gLogger.Destroy();

	DestroyVulkan();
	DestroyGlfw();
}

bool Application::IsDone() const {
	return true;
}

void Application::Update(std::shared_ptr<GameTimer> pGameTimer) {
}

void Application::RenderScene() {
}

Application::~Application() {
}

void Application::SetupGlfw() {
	glfwSetErrorCallback(GlfwErrorCallback);
	if (!glfwInit()) {
	    Exception::Throw("glfwInit error");
	}

	int width = 1280;
	int height = 720;
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	_pWindow = glfwCreateWindow(width, height, "Vulkan App", nullptr, nullptr);
}

void Application::DestroyGlfw() {
	glfwDestroyWindow(_pWindow);
	_pWindow = nullptr;
	glfwTerminate();
}

void Application::SetupVulkan() {
	if (!glfwVulkanSupported()) {
	    Exception::Throw("GLFW Vulkan Not Supported");
	}

	vkutils::InitDynamicLoader();
	_pDevice = std::make_unique<vkgfx::Device>();
	_pDevice->OnCreate("VulkanAPP", "Vulkan", true, true, _pWindow);
}

void Application::DestroyVulkan() {
	_pDevice->OnDestroy();
	_pDevice = nullptr;
}

void Application::GlfwErrorCallback(int error, const char *description) {
	gLogger->Error("GLFW Error {}: {}", error, description);
}
