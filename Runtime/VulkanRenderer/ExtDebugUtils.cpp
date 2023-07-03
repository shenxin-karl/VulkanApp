#include "ExtDebugUtils.h"

namespace vkgfx {

static std::mutex sMutex = {};

auto ExtDebugUtils::Attach(InstanceProperties &instanceProperties) -> std::unique_ptr<ExtDebugUtils> {
    if (instanceProperties.AddExtension("VK_EXT_debug_utils")) {
        return std::make_unique<ExtDebugUtils>();
    }
    return nullptr;
}

void ExtDebugUtils::OnCreate() {
}

void ExtDebugUtils::OnDestroy() {
}

void ExtDebugUtils::SetResourceName(vk::Device device,
    vk::ObjectType objectType,
    uint64_t handle,
    std::string_view name) {

    if (VULKAN_HPP_DEFAULT_DISPATCHER.vkSetDebugUtilsObjectNameEXT && handle && !name.empty()) {
        std::unique_lock<std::mutex> lock(sMutex);
        vk::DebugUtilsObjectNameInfoEXT nameInfo = {};
        nameInfo.sType = vk::StructureType::eDebugUtilsObjectNameInfoEXT;
        nameInfo.objectType = objectType;
        nameInfo.objectHandle = handle;
        nameInfo.pObjectName = name.data();
        device.setDebugUtilsObjectNameEXT(nameInfo);
    }
}

void ExtDebugUtils::SetPrefMarkerBegin(vk::CommandBuffer cmd, std::string_view name) {
    if (VULKAN_HPP_DEFAULT_DISPATCHER.vkCmdBeginDebugUtilsLabelEXT && !name.empty()) {
        std::array<float, 4> color = {1.0f, 0.0f, 0.0f, 1.0f};
        vk::DebugUtilsLabelEXT label = {
            .pLabelName = name.data(),
            .color = color,
        };
        cmd.beginDebugUtilsLabelEXT(label);
    }
}

void ExtDebugUtils::SetPrefMarkerEnd(vk::CommandBuffer cmd) {
    if (VULKAN_HPP_DEFAULT_DISPATCHER.vkCmdEndDebugUtilsLabelEXT) {
        cmd.endDebugUtilsLabelEXT();
    }
}

}    // namespace vkgfx
