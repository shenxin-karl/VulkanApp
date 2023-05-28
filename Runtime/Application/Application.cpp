#include "Application.h"
#include "Foundation/Logger.h"
#include "VulkanRenderer/Device.h"

#include <imgui.h>

#include "VulkanRenderer/SwapChain.h"

Application::Application() {

}

void Application::Startup() {
	gLogger->Initialize();
	gLogger->StartLogging();

	SetupGlfw();
	SetupVulkan();
}

void Application::Cleanup() {
	gLogger->Destroy();

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

	constexpr size_t kNumBackBuffer = 3;
	vkgfx::gDevice->OnCreate("VulkanAPP", "Vulkan", true, true, _pWindow);
	vkgfx::gSwapChain->OnCreate(vkgfx::gDevice, kNumBackBuffer, _pWindow);
}

void Application::DestroyVulkan() {
	vkgfx::gSwapChain->OnDestroy();
	vkgfx::gDevice->OnDestroy();
}

void Application::GlfwErrorCallback(int error, const char *description) {
	gLogger->Error("GLFW Error {}: {}", error, description);
}
