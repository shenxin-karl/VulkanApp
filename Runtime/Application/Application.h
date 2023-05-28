#pragma once
#include <Windows.h>
#include <imgui.h>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include "IApplication.h"
#include "VulkanRenderer/Device.h"

class Application : public IApplication {
public:
    Application();
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
    bool _isDone = false;
    GLFWwindow *_pWindow = nullptr;
};