#include "Application.h"
#include "Foundation/Logger.h"
#include <imgui.h>
#include <vulkan/vulkan_extension_inspection.hpp>
#include <vulkan/vulkan_raii.hpp>

void Application::Startup() {
	gLogger.Initialize();
	gLogger->StartLogging();

	SetupVulkan();
}

void Application::Cleanup() {
	gLogger.Destroy();

	DestroyVulkan();
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

void Application::SetupVulkan() {
	glfwSetErrorCallback(GlfwErrorCallback);
	if (!glfwInit()) {
	    Exception::Throw("glfwInit error");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	_pWindow = glfwCreateWindow(1280, 720, "Vulkan App", nullptr, nullptr);
	if (!glfwVulkanSupported()) {
	    Exception::Throw("GLFW Vulkan Not Supported");
	}

	ImVector<const char *> extensions;
	uint32_t extensionCount = 0;
	const char **pGlfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
	for (int i = 0; i < extensionCount; ++i) {
	    extensions.push_back(pGlfwExtensions[i]);
	}

	// create vulkan instance
	{
		//_instance = vk::createInstance("VulkanApp", "Vulkan", {}, vk::getInstanceExtensions());
		
	}
}

void Application::DestroyVulkan() {
	_instance.destroy();
}

void Application::GlfwErrorCallback(int error, const char *description) {
	gLogger->Error("GLFW Error {}: {}", error, description);
}

bool Application::IsExtensionAvailable(const ImVector<VkExtensionProperties> &properties, const char *extension) {
    for (const VkExtensionProperties& p : properties) {
        if (strcmp(p.extensionName, extension) == 0) {
            return true;
        }
    }
    return false;
}
