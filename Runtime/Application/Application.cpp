#include "Application.h"
#include "Foundation/Logger.h"
#include "VulkanRenderer/Device.h"
#include "VulkanRenderer/CommandBufferRing.h"
#include "VulkanRenderer/DxcModule.h"
#include "VulkanRenderer/SwapChain.h"
#include "Utils/AssetProjectSetting.h"
#include "Shader/ShaderManager.h"
#include "VulkanRenderer/DefineList.h"
#include <imgui.h>
#include <glm/glm.hpp>

#include "VulkanRenderer/Utils.hpp"

Application::Application() {
}

void Application::Startup() {
    gLogger->Initialize();
    gAssetProjectSetting->Initialize();
    vkgfx::gDxcModule->Initialize();
    gShaderManager->Initialize();

    gLogger->StartLogging();
    SetupGlfw();
    SetupVulkan();

    Loading();
}

void Application::Cleanup() {
    gShaderManager->Destroy();
    vkgfx::gDxcModule->Destroy();
    gAssetProjectSetting->Destroy();
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
    vkgfx::gCommandBufferRing->OnBeginFrame();
    vk::CommandBuffer cmd = vkgfx::gCommandBufferRing->GetNewCommandBuffer();
    vk::CommandBufferBeginInfo beginInfo;
    cmd.begin(beginInfo);

    vkgfx::gSwapChain->WaitForSwapChain();
    vk::ClearValue clearColor = {};
    clearColor.color.float32 = std::array<float, 4>{0.f, 0.f, 0.f, 1.f};

    vk::RenderPassBeginInfo renderPassBeginInfo = vkgfx::gSwapChain->GetRenderPassBeginInfo();
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;
    cmd.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
    {
	    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, _graphicsPipeline);
        cmd.setViewport(0, vkgfx::gSwapChain->GetFullScreenViewport());
        cmd.setScissor(0, vkgfx::gSwapChain->GetFullScreenScissor());
        cmd.bindVertexBuffers(0, _triangleBufferInfo.buffer, _triangleBufferInfo.offset);
        cmd.draw(3, 1, 0, 0);
    }
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
    submitInfo.pSignalSemaphores = &vkgfx::gCommandBufferRing->GetRenderFinishedSemaphore();
    vkgfx::gDevice->GetGraphicsQueue().submit(submitInfo, vkgfx::gCommandBufferRing->GetExecutedFinishedFence());
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

    constexpr size_t kNumBackBuffer = 2;
    constexpr size_t kNumCommandBufferPreFrame = 3;
    vkgfx::gDevice->OnCreate("VulkanAPP", "Vulkan", true, _pWindow);
    vkgfx::gSwapChain->OnCreate(vkgfx::gDevice, kNumBackBuffer);
    vkgfx::gCommandBufferRing->OnCreate(vkgfx::gDevice, kNumBackBuffer, kNumCommandBufferPreFrame);
    vkgfx::gDevice->CreatePipelineCache();
}

void Application::DestroyVulkan() {
    vkgfx::gDevice->WaitGPUFlush();
    vkgfx::gDevice->DestroyPipelineCache();
    vkgfx::gCommandBufferRing->OnDestroy();
    vkgfx::gSwapChain->OnDestroy();
    vkgfx::gDevice->OnDestroy();
}

void Application::GlfwErrorCallback(int error, const char *description) {
    gLogger->Error("GLFW Error {}: {}", error, description);
}

void Application::Loading() {
    vk::Device device = vkgfx::gDevice->GetVKDevice();

    vk::PipelineShaderStageCreateInfo shaderStages[2];
    ShaderLoadInfo loadInfo = {"Assets/Shaders/Triangles.hlsl", "VSMain", vkgfx::ShaderType::kVS, {}};
    gShaderManager->LoadShaderStageCreateInfo(loadInfo, shaderStages[0]);

    loadInfo.entryPoint = "PSMain";
    loadInfo.shaderType = vkgfx::ShaderType::kPS;
    gShaderManager->LoadShaderStageCreateInfo(loadInfo, shaderStages[1]);

    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
    };

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
    attributeDescription[0].offset = offsetof(Vertex, color);

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
    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_TRUE;
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
    _graphicsPipeline = device.createGraphicsPipeline(nullptr, pipelineCreateInfo).value;

    const std::vector<Vertex> vertices = {
    	{{+0.0f, -0.5f, +0.f}, {1.0f, 0.0f, 0.0f}},
        {{+0.5f, +0.5f, +0.f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, +0.5f, +0.f}, {0.0f, 0.0f, 1.0f}}
    };

    constexpr size_t k128MB = 128 * 1024 * 1024;
    _uploadHeap.OnCreate(vkgfx::gDevice, k128MB);

    size_t memoryAllocSize = sizeof(Vertex) * vertices.size();
    _vertexBuffer.OnCreate(vkgfx::gDevice, memoryAllocSize, "Triangle Size");
    _triangleBufferInfo = _vertexBuffer.AllocBuffer(vertices).value();
    _vertexBuffer.UploadData(_uploadHeap.GetCommandBuffer());
    _vertexBuffer.FreeUploadHeap();
    _uploadHeap.Flush();
}
