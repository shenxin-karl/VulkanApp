#include "Application.h"
#include "Foundation/Logger.h"
#include "VulkanRenderer/Device.h"
#include "VulkanRenderer/CommandBufferRing.h"
#include "VulkanRenderer/DxcModule.h"
#include "VulkanRenderer/SwapChain.h"
#include "Utils/AssetProjectSetting.h"
#include "Shader/ShaderManager.h"
#include "VulkanRenderer/DefineList.h"
#include <glm/glm.hpp>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Editor/EditorWindow.h"
#include "Editor/MainBar.h"
#include "ImGUI/GUI.h"
#include "ImGUI/Libary/imgui.h"
#include "RenderDoc/RenderDoc.h"
#include "VulkanRenderer/ExtDebugUtils.h"
#include "VulkanRenderer/Utils.hpp"


struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
};

Application::Application() {
}

void Application::Startup() {
    gLogger->Initialize();
    RenderDoc::Load();

    gLogger->StartLogging();
    gAssetProjectSetting->Initialize();
    vkgfx::gDxcModule->OnCreate();

    SetupGlfw();
    SetupVulkan();

    gShaderManager->Initialize();

    constexpr size_t k128MB = 128 * 1024 * 1024;
    _uploadHeap.OnCreate("GlobalUploadHeap", vkgfx::gDevice, k128MB);

    vkgfx::DynamicBufferRing::BufferType dynamicBufferType = vkgfx::DynamicBufferRing::Constant |
                                                             vkgfx::DynamicBufferRing::Index |
                                                             vkgfx::DynamicBufferRing::Vertex;

    constexpr size_t k32MB = 32 * 1024 * 1024;
    _dynamicBufferRing.OnCreate("DynamicBuffer", vkgfx::gDevice, dynamicBufferType, kNumBackBuffer, k32MB);

    gGui->OnCreate(vkgfx::gDevice, _pWindow, vkgfx::gSwapChain->GetRenderPass());
    gEditorWindow->OnCreate();

    Loading();
}

void Application::Cleanup() {
    vkgfx::gDevice->WaitGPUFlush();
    gEditorWindow->OnDestroy();
    gGui->OnDestroy();
    gShaderManager->Destroy();
    CleanUpVulkan();
    CleanUpGlfw();
    vkgfx::gDxcModule->OnDestroy();
    gAssetProjectSetting->Destroy();
    RenderDoc::Free();
    gLogger->Destroy();
}

bool Application::IsDone() const {
    return glfwWindowShouldClose(_pWindow);
}

bool Application::IsPause() const {
    return _pause;
}

void Application::PollEvents() {
    glfwPollEvents();
    if (_pause) {
        return;
    }
    if (_needResize) {
        _needResize = false;
        vkgfx::gDevice->WaitGPUFlush();
        OnResize();
    }
}

void Application::Update(std::shared_ptr<GameTimer> pGameTimer) {
    gGui->NewFrame();
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    gEditorWindow->OnGUI(*pGameTimer);
}

void Application::RenderScene(std::shared_ptr<GameTimer> pGameTimer) {
    if (gEditorWindow->GetMainBar()->startRenderDocCapture) {
        HWND hwnd = glfwGetWin32Window(_pWindow);
	    RenderDoc::BeginFrameCapture(hwnd);
    }

    _graphicsCmdRing.OnBeginFrame();
    _dynamicBufferRing.OnBeginFrame();

    vk::CommandBuffer cmd = _graphicsCmdRing.GetNewCommandBuffer();
    vk::CommandBufferBeginInfo beginInfo;
    cmd.begin(beginInfo);

    vkgfx::gSwapChain->WaitForSwapChain();

    cmd.setViewport(0, vkgfx::gSwapChain->GetFullScreenViewport());
    cmd.setScissor(0, vkgfx::gSwapChain->GetFullScreenScissor());

    vk::ClearValue clearColor = {};
    clearColor.color.float32 = std::array{0.f, 0.f, 0.f, 1.f};
    vk::RenderPassBeginInfo renderPassBeginInfo = vkgfx::gSwapChain->GetRenderPassBeginInfo();
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;
    cmd.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
    if (vkgfx::PrefMarkerGuard profile(cmd, "OpaquePass"); profile.Sample()) {

        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, _graphicsPipeline);
        cmd.bindVertexBuffers(0, _triangleBufferInfo.buffer, _triangleBufferInfo.offset);
        cmd.draw(3, 1, 0, 0);
    }
    gGui->Draw(cmd);

    cmd.endRenderPass();
    cmd.end();

    vk::PipelineStageFlags waitDstMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submitInfo;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &vkgfx::gSwapChain->GetImageAvailableSemaphore();
    submitInfo.pWaitDstStageMask = &waitDstMask;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &_graphicsCmdRing.GetRenderFinishedSemaphore();
    vkgfx::gDevice->GetGraphicsQueue().submit(submitInfo, _graphicsCmdRing.GetExecutedFinishedFence());

    if (gEditorWindow->GetMainBar()->startRenderDocCapture) {
        HWND hwnd = glfwGetWin32Window(_pWindow);
	    RenderDoc::EndFrameCapture(hwnd);
        gEditorWindow->GetMainBar()->startRenderDocCapture = false;
    }

    vkgfx::gSwapChain->Present(_graphicsCmdRing.GetRenderFinishedSemaphore());
}

void Application::OnResize() {
    _graphicsCmdRing.WaitForRenderFinished(vkgfx::gDevice->GetGraphicsQueue());
    vkgfx::gSwapChain->Resize(_width, _height, false);

    vk::Device device = vkgfx::gDevice->GetVKDevice();
    if (_graphicsPipeline) {
        device.destroyPipeline(_graphicsPipeline);
        _graphicsPipeline = VK_NULL_HANDLE;
    }
    if (_pipelineLayout) {
        device.destroyPipelineLayout(_pipelineLayout);
        _pipelineLayout = VK_NULL_HANDLE;
    }

    vk::PipelineShaderStageCreateInfo shaderStages[2];
    ShaderLoadInfo loadInfo = {"Assets/Shaders/Triangles.hlsl", "VSMain", vkgfx::ShaderType::kVS};
    gShaderManager->LoadShaderStageCreateInfo(loadInfo, shaderStages[0]);

    loadInfo.entryPoint = "PSMain";
    loadInfo.shaderType = vkgfx::ShaderType::kPS;
    gShaderManager->LoadShaderStageCreateInfo(loadInfo, shaderStages[1]);

    vk::VertexInputBindingDescription bindingDescription[1];
    bindingDescription[0].binding = 0;
    bindingDescription[0].stride = sizeof(Vertex);
    bindingDescription[0].inputRate = vk::VertexInputRate::eVertex;

    vk::VertexInputAttributeDescription attributeDescription[2];
    attributeDescription[0].location = 0;
    attributeDescription[0].binding = 0;
    attributeDescription[0].format = vk::Format::eR32G32B32Sfloat;
    attributeDescription[0].offset = offsetof(Vertex, position);

    attributeDescription[1].location = 1;
    attributeDescription[1].binding = 0;
    attributeDescription[1].format = vk::Format::eR32G32B32Sfloat;
    attributeDescription[1].offset = offsetof(Vertex, color);

    vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo;
    vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputCreateInfo.pVertexBindingDescriptions = bindingDescription;
    vertexInputCreateInfo.vertexAttributeDescriptionCount = 2;
    vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescription;

    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo;
    rasterizationStateCreateInfo.polygonMode = vk::PolygonMode::eFill;
    rasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eBack;
    rasterizationStateCreateInfo.frontFace = vk::FrontFace::eClockwise;
    rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizationStateCreateInfo.depthBiasConstantFactor = 0;
    rasterizationStateCreateInfo.depthBiasClamp = 0;
    rasterizationStateCreateInfo.depthBiasSlopeFactor = 0;
    rasterizationStateCreateInfo.lineWidth = 1.0;

    vk::PipelineColorBlendStateCreateInfo colorBlendCreateInfo;
    colorBlendCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendCreateInfo.attachmentCount = 1;
    colorBlendCreateInfo.pAttachments = vkgfx::GetColorBlendAttachmentState_Opaque();
    colorBlendCreateInfo.blendConstants = std::array<float, 4>{0.f};

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    pipelineLayoutCreateInfo.setLayoutCount = 0;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    _pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);

    vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = vkgfx::GetInputAssemblyState_TriangleList();
    pipelineCreateInfo.pTessellationState = nullptr;
    pipelineCreateInfo.pViewportState = vkgfx::GetViewportState<1, 1>();
    pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    pipelineCreateInfo.pMultisampleState = vkgfx::GetMultiSampleState_Disable();
    pipelineCreateInfo.pDepthStencilState = vkgfx::GetDepthStencilState_DepthStandard();
    pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
    pipelineCreateInfo.pDynamicState = vkgfx::GetDynamicState_ViewportScissor();
    pipelineCreateInfo.layout = _pipelineLayout;
    pipelineCreateInfo.renderPass = vkgfx::gSwapChain->GetRenderPass();
    pipelineCreateInfo.basePipelineHandle = nullptr;
    pipelineCreateInfo.basePipelineIndex = -1;
    _graphicsPipeline = device.createGraphicsPipeline(vkgfx::gDevice->GetPipelineCache(), pipelineCreateInfo).value;
}

Application::~Application() {
}

void Application::SetupGlfw() {
    glfwSetErrorCallback(GlfwErrorCallback);
    if (!glfwInit()) {
        Exception::Throw("glfwInit error");
    }

    _width = 1280;
    _height = 720;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    _pWindow = glfwCreateWindow(_width, _height, "Vulkan App", nullptr, nullptr);
    glfwSetWindowUserPointer(_pWindow, this);
    glfwSetFramebufferSizeCallback(_pWindow, FrameBufferResizeCallback);
    glfwSetWindowIconifyCallback(_pWindow, WindowMinimizeCallback);
}

void Application::CleanUpGlfw() {
    glfwDestroyWindow(_pWindow);
    _pWindow = nullptr;
    glfwTerminate();
}

void Application::SetupVulkan() {
    if (!glfwVulkanSupported()) {
        Exception::Throw("GLFW Vulkan Not Supported");
    }

    constexpr size_t kNumCommandBufferPreFrame = 3;
    vkgfx::gDevice->OnCreate("VulkanAPP", "Vulkan", true, _pWindow);
    vkgfx::gSwapChain->OnCreate(vkgfx::gDevice, kNumBackBuffer);
    _graphicsCmdRing.OnCreate(vkgfx::gDevice, kNumBackBuffer, kNumCommandBufferPreFrame);
    vkgfx::gDevice->CreatePipelineCache();
}

void Application::CleanUpVulkan() {
    vkgfx::gDevice->WaitGPUFlush();
    vkgfx::gDevice->DestroyPipelineCache();
    _graphicsCmdRing.OnDestroy();
    _vertexBuffer.OnDestroy();
    _uploadHeap.OnDestroy();
    if (_pipelineLayout) {
	    vkgfx::gDevice->GetVKDevice().destroyPipelineLayout(_pipelineLayout);
        _pipelineLayout = VK_NULL_HANDLE;
    }
    if (_graphicsPipeline) {
	    vkgfx::gDevice->GetVKDevice().destroyPipeline(_graphicsPipeline);
	    _graphicsPipeline = VK_NULL_HANDLE;
    }

    _dynamicBufferRing.OnDestroy();
    vkgfx::gSwapChain->OnDestroy();
    vkgfx::gDevice->OnDestroy();
}

void Application::GlfwErrorCallback(int error, const char *description) {
    Logger::Error("GLFW Error {}: {}", error, description);
}

void Application::FrameBufferResizeCallback(GLFWwindow *pWindow, int width, int height) {
    Application *pApplication = static_cast<Application *>(glfwGetWindowUserPointer(pWindow));
    if (pApplication->_width != width || pApplication->_height != height) {
        pApplication->_width = width;
        pApplication->_height = height;
        pApplication->_needResize = true;
    }
}

void Application::WindowMinimizeCallback(GLFWwindow *pWindow, int minimized) {
    Application *pApplication = static_cast<Application *>(glfwGetWindowUserPointer(pWindow));
    pApplication->_pause = minimized;
}

void Application::Loading() {
    // clang-format off
    const std::vector<Vertex> vertices = {
    	{{+0.0f, -0.5f, +0.f}, {1.0f, 0.0f, 0.0f}},
        {{+0.5f, +0.5f, +0.f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, +0.5f, +0.f}, {0.0f, 0.0f, 1.0f}}
    };
    // clang-format on

    size_t memoryAllocSize = sizeof(Vertex) * vertices.size();
    _vertexBuffer.OnCreate("TriangleBuffer", vkgfx::gDevice, memoryAllocSize);
    _triangleBufferInfo = _vertexBuffer.AllocBuffer(vertices).value();
    _vertexBuffer.UploadData(_uploadHeap);
    _vertexBuffer.FreeUploadHeap();
    _uploadHeap.Flush();
}
