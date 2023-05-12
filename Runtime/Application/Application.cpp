#include "Application.h"
#include "Foundation/Logger.h"
#include "VulkanUtils/utils.h"
#include <imgui.h>


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

	// create vulkan instance
	{
		const char *pAppName = "VulkanApp";
		const char *pEngineName = "Vulkan";

		vk::ApplicationInfo applicationInfo {
			pAppName, 
			1, 
			pEngineName, 
			1, 
			VK_VERSION_1_1
		};

        std::vector<vk::ExtensionProperties> properties = vk::enumerateInstanceExtensionProperties();
		auto compare = [](vk::ExtensionProperties const & ep) {
			return strcmp(ep.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0;
		};
		if (std::ranges::find_if(properties, compare) == properties.end()) {
			Exception::Throw(
				"Something went very wrong, cannot find {} {}", 
				VK_EXT_DEBUG_UTILS_EXTENSION_NAME, 
				"extension"
			);
		}

		std::vector<vk::LayerProperties> layersProperties = vk::enumerateInstanceLayerProperties();

		vk::InstanceCreateFlags flags = {};
		std::vector<const char *> layers;
		auto extensions = vkutils::GetRequiredInstanceExtensions();
#ifndef NODEBUG
        layers.push_back("VK_LAYER_KHRONOS_validation");
        extensions.push_back("VK_EXT_debug_report");
		flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

		vk::InstanceCreateInfo createInfo = {
		    flags,
		    &applicationInfo,
			static_cast<uint32_t>(layers.size()),
			layers.data(),
			static_cast<uint32_t>(extensions.size()),
			extensions.data(),
		};
		_vkInstance = createInstance(createInfo);
		vkutils::InitInstanceExtFunc(_vkInstance);

#ifndef NODEBUG
	{
        vkutils::LoadDebugUtilsMessengerFunc(_vkInstance);
        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                         vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
        vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | 
			                                               vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                                           vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
	    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo(
		    {}, 
		    severityFlags, 
		    messageTypeFlags, 
		    &vkutils::DebugMessageFunc 
	    );
		_vkDebugUtilsMessenger =  _vkInstance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfo);
	}
#endif
	}
}

void Application::DestroyVulkan() {
#if !defined(NDEBUG)
    _vkInstance.destroyDebugUtilsMessengerEXT(_vkDebugUtilsMessenger);
#endif
	_vkInstance.destroy();
}

void Application::GlfwErrorCallback(int error, const char *description) {
	gLogger->Error("GLFW Error {}: {}", error, description);
}
