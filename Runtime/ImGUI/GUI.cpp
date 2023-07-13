#include "GUI.h"
#include "Libary/imgui.h"
#include "Libary/imgui_impl_glfw.h"

#include "Libary/imgui_impl_vulkan.h"
#include "VulkanRenderer/Device.h"
#include "VulkanRenderer/ExtDebugUtils.h"
#include "VulkanRenderer/SwapChain.h"

static constexpr uint32_t kMinImageCount = 2;

void GUI::OnCreate(vkgfx::Device *pDevice, GLFWwindow *pWindow, vk::RenderPass renderPass) {
	SetDevice(pDevice);

	vk::Device device = pDevice->GetVKDevice();
	vk::DescriptorPoolSize poolSizes[] = {
		{ vk::DescriptorType::eSampler, 1000 },
		{ vk::DescriptorType::eCombinedImageSampler, 1000 },
		{ vk::DescriptorType::eSampledImage, 1000 },
		{ vk::DescriptorType::eStorageImage, 1000 },
		{ vk::DescriptorType::eUniformTexelBuffer, 1000 },
		{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
		{ vk::DescriptorType::eUniformBuffer, 1000 },
		{ vk::DescriptorType::eStorageBuffer, 1000 },
		{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
		{ vk::DescriptorType::eStorageBufferDynamic, 1000 },
		{ vk::DescriptorType::eInputAttachment, 1000 },
	};

	vk::DescriptorPoolCreateInfo poolCreateInfo;
	poolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
	poolCreateInfo.maxSets = 1000 * static_cast<uint32_t>(std::size(poolSizes));
	poolCreateInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
	poolCreateInfo.pPoolSizes = poolSizes;
	_descriptorPool = device.createDescriptorPool(poolCreateInfo);


	 // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForVulkan(pWindow, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = pDevice->GetInstance();
    init_info.PhysicalDevice = pDevice->GetPhysicalDevice();
    init_info.Device = device;
    init_info.QueueFamily = pDevice->GetGraphicsQueueFamilyIndex();
    init_info.Queue = pDevice->GetGraphicsQueue();
    init_info.PipelineCache = pDevice->GetPipelineCache();
    init_info.DescriptorPool = _descriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = kMinImageCount;
    init_info.ImageCount = vkgfx::gSwapChain->GetBackBufferCount();
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = CheckImGuiVkResult;
    ImGui_ImplVulkan_Init(&init_info, renderPass);

    // upload font texture
    vk::CommandPoolCreateInfo commandPoolCreateInfo;
    commandPoolCreateInfo.queueFamilyIndex = pDevice->GetGraphicsQueueFamilyIndex();
    commandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    vk::CommandPool commandPool = device.createCommandPool(commandPoolCreateInfo);

    vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.commandBufferCount = 1;
    commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
    vk::CommandBuffer cmd = device.allocateCommandBuffers(commandBufferAllocateInfo).front();

    device.resetCommandPool(commandPool, {});
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    cmd.begin(beginInfo);
	{
		ImGui_ImplVulkan_CreateFontsTexture(cmd);
	}
    cmd.end();

    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    pDevice->GetGraphicsQueue().submit(submitInfo);
    device.waitIdle();

    ImGui_ImplVulkan_DestroyFontUploadObjects();

    device.freeCommandBuffers(commandPool, cmd);
    device.destroyCommandPool(commandPool);

	SetIsCreate(true);
}

void GUI::OnDestroy() {
    if (!GetIsCreate()) {
	    return;
    }

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    vk::Device device = GetDevice()->GetVKDevice();
    device.destroyDescriptorPool(_descriptorPool);
    _descriptorPool = VK_NULL_HANDLE;
}

void GUI::NewFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    // ´´½¨ docking ¿Õ¼ä
    //ImGui::DockSpaceOverViewport();
}

void GUI::Draw(vk::CommandBuffer cmd) {
    ImGui::Render();
    ImDrawData *pDrawData = ImGui::GetDrawData();
     
    vkgfx::PrefMarkerGuard guard(cmd, "ImGUI Draw Pass", Colors::Orange);
    ImGui_ImplVulkan_RenderDrawData(pDrawData, cmd);

    // Update and Render additional Platform Windows
    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void GUI::CheckImGuiVkResult(VkResult err) {
    if (err == 0)
        return;
    //fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
}
