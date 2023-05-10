#pragma once
#include <string>
#include <GLFW/glfw3.h>
#include "Foundation/NonCopyable.h"
#include "Foundation/RuntimeStatic.h"

class GlfwProxy : NonCopyable {
private:
	friend RuntimeStatic<GlfwProxy>;
	void Initialize();
	void Destroy();
public:
	void GetFrameBufferSize(int &width, int &height);
	bool IsWindowShouldClose() const;
	void PollEvents();
	void InitializeWindow(int width, int height, const std::string &title);
	void DestroyWindow();
	auto GetWindow() const -> GLFWwindow *;
	bool IsVulkanSupported() const;
	static auto GetErrorCallback() -> GLFWerrorfun;
	static void SetErrorCallback(GLFWerrorfun func);
private:
	GLFWwindow *			   _pWindow		   = nullptr;
	static inline GLFWerrorfun _pErrorCallback = nullptr;
};

inline RuntimeStatic<GlfwProxy> gGlfwProxy;