#include "Application.h"
#include "Foundation/Logger.h"
#include <imgui.h>
#include <VKUtils/utils.hpp>

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

	int width = 1280;
	int height = 720;
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	_pWindow = glfwCreateWindow(width, height, "Vulkan App", nullptr, nullptr);
	if (!glfwVulkanSupported()) {
	    Exception::Throw("GLFW Vulkan Not Supported");
	}

	std::vector<std::string> extensions;
	uint32_t extensionCount = 0;
	const char **pGlfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
	for (int i = 0; i < extensionCount; ++i) {
	    extensions.push_back(pGlfwExtensions[i]);
	}


	// create vulkan instance
	{
		const char *pAppName = "VulkanApp";
		const char *pEngineName = "Vulkan";

		_vkInstance = vk::su::createInstance(
			pAppName, 
			pEngineName, 
			{}, 
			extensions, 
			VK_VERSION_1_1
		);

#if !defined(NDEBUG)
        _vkDebugUtilsMessenger = _vkInstance.createDebugUtilsMessengerEXT(vk::su::makeDebugUtilsMessengerCreateInfoEXT());
#endif
		_vkPhysicalDevice = _vkInstance.enumeratePhysicalDevices().front();
		vk::su::SurfaceData surfaceData(_vkInstance, pAppName, vk::Extent2D(width, height));

        std::pair<uint32_t, uint32_t> findRes = vk::su::findGraphicsAndPresentQueueFamilyIndex(
            _vkPhysicalDevice, 
			surfaceData.surface
		);

		_graphicsQueueFamilyIndex = findRes.first;
		_presentQueueFamilyIndex = findRes.second;

		_vkDevice = vk::su::createDevice(
			_vkPhysicalDevice, 
			_graphicsQueueFamilyIndex, 
			vk::su::getDeviceExtensions()
		);

		_graphicsQueue = _vkDevice.getQueue(_graphicsQueueFamilyIndex, 0);
		_presentQueue = _vkDevice.getQueue(_presentQueueFamilyIndex, 0);
		_graphicsPool = vk::su::createCommandPool(_vkDevice, _graphicsQueueFamilyIndex);
	}
}

void Application::DestroyVulkan() {
	_vkDevice.destroyCommandPool(_graphicsPool);
	_vkDevice.destroy();
#if !defined(NDEBUG)
    _vkInstance.destroyDebugUtilsMessengerEXT(_vkDebugUtilsMessenger);
#endif
	_vkInstance.destroy();
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
