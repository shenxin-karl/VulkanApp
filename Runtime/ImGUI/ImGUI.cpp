#include "ImGUI.h"
#include <ShellScalingApi.h>

void ImGUI::OnCreate(vkgfx::Device *pDevice,
    vk::RenderPass renderPass,
    vkgfx::UploadHeap &uploadHeap,
    vkgfx::DynamicBufferRing *pConstantBuffer,
    float fontSize) {

    _pConstantBuffer = pConstantBuffer;
    _pDevice = pDevice;

    ImGuiIO &io = ImGui::GetIO();
    DEVICE_SCALE_FACTOR scaleFactor = GetScaleFactorForDevice(DEVICE_PRIMARY);
    float textScale = scaleFactor / 100.f;
    ImFontConfig fontConfig;
    fontConfig.SizePixels = fontSize * textScale;
    io.Fonts->AddFontDefault(&fontConfig);

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    size_t uploadSize = width * height * 4 * sizeof(char);

    vk::ImageCreateInfo imageCreateInfo;
    imageCreateInfo.imageType = vk::ImageType::e2D;
    imageCreateInfo.format = vk::Format::eR8G8B8A8Unorm;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
    imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
    imageCreateInfo.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
    imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
    imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;


}

void ImGUI::OnDestroy() {
}

void ImGUI::UpdateRenderPass(vk::RenderPass renderPass) {
}

void ImGUI::Draw(vk::CommandBuffer cmd) {
}
