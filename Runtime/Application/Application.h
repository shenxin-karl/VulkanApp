#pragma once
#include <Windows.h>
#include <imgui.h>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include "IApplication.h"
#include "VulkanRenderer/Device.h"
#include "VulkanRenderer/StaticBufferPool.h"
#include "VulkanRenderer/UploadHeap.h"

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
    void Loading();
private:
    bool _isDone = false;
    GLFWwindow *_pWindow = nullptr;
private:
    vk::Pipeline _graphicsPipeline;
    vk::PipelineLayout _pipelineLayout;
    vkgfx::StaticBufferPool _vertexBuffer;
    vkgfx::UploadHeap _uploadHeap;
};
