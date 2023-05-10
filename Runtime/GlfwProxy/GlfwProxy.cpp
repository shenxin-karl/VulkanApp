#include "GlfwProxy.h"
#include "Foundation/Exception.h"

void GlfwProxy::Initialize() {
	Exception::Throw(glfwInit(), "GLFWInit error!");
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void GlfwProxy::Destroy() {
    glfwTerminate();
}

void GlfwProxy::GetFrameBufferSize(int &width, int &height) {
	glfwGetFramebufferSize(_pWindow, &width, &height);
}

bool GlfwProxy::IsWindowShouldClose() const {
	return glfwWindowShouldClose(_pWindow);
}

void GlfwProxy::PollEvents() {
	PollEvents();
}

void GlfwProxy::InitializeWindow(int width, int height, const std::string &title) {
	_pWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}

void GlfwProxy::DestroyWindow() {
	glfwDestroyWindow(_pWindow);
	_pWindow = nullptr;
}

auto GlfwProxy::GetWindow() const -> GLFWwindow * {
	return _pWindow;
}

bool GlfwProxy::IsVulkanSupported() const {
	return glfwVulkanSupported();
}

auto GlfwProxy::GetErrorCallback() -> GLFWerrorfun {
	return _pErrorCallback;
}

void GlfwProxy::SetErrorCallback(GLFWerrorfun func) {
	_pErrorCallback = func;
	glfwSetErrorCallback(_pErrorCallback);
}