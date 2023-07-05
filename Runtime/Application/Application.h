#pragma once
#include <Windows.h>
#include <imgui.h>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include "IApplication.h"
#include "VulkanRenderer/Device.h"
#include "VulkanRenderer/StaticBufferPool.h"
#include "VulkanRenderer/UploadHeap.h"
#include "VulkanRenderer/CommandBufferRing.h"

class Application : public IApplication {
public:
    Application();
    void Startup() override;
    void Cleanup() override;
    bool IsDone() const override;
    bool IsPause() const override;
    void PollEvents() override;
    void Update(std::shared_ptr<GameTimer> pGameTimer) override;
    void RenderScene(std::shared_ptr<GameTimer> pGameTimer) override;
    void OnResize();
    ~Application() override;
public:
    void SetupGlfw();
    void DestroyGlfw();
    void SetupVulkan();
    void DestroyVulkan();
    void Loading();
    static void GlfwErrorCallback(int error, const char *description);
    static void FrameBufferResizeCallback(GLFWwindow *pWindow, int width, int height);
    static void WindowMinimizeCallback(GLFWwindow *pWindow, int minimized);
private:
    bool _pause = false;
    bool _needResize = true;
    GLFWwindow *_pWindow = nullptr;
    uint32_t _width = 0;
    uint32_t _height = 0;
private:
    vkgfx::CommandBufferRing _graphicsCmdRing;
private:
    vk::Pipeline _graphicsPipeline;
    vkgfx::UploadHeap _uploadHeap;
    vk::PipelineLayout _pipelineLayout;
    vkgfx::StaticBufferPool _vertexBuffer;
    vk::DescriptorBufferInfo _triangleBufferInfo = {};
};
