#pragma once
#include <vulkan/vulkan.hpp>

namespace vkgfx {

inline auto GetDynamicState_ViewportScissor() -> const vk::PipelineDynamicStateCreateInfo * {
    // clang-format off
    static vk::DynamicState sDynamicStates[2] = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor,
    };
    static vk::PipelineDynamicStateCreateInfo sDynamicState = {
    	.dynamicStateCount = 2,
        .pDynamicStates = sDynamicStates
    };
    // clang-format on
    return &sDynamicState;
}

inline auto GetInputAssemblyState_TriangleList() -> const vk::PipelineInputAssemblyStateCreateInfo * {
    // clang-format off
    static vk::PipelineInputAssemblyStateCreateInfo sAssemblyState {
    	.topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = false
    };
    // clang-format on
    return &sAssemblyState;
}

template<size_t ViewportCount, size_t ScissorCount>
auto GetViewportState() -> const vk::PipelineViewportStateCreateInfo * {
    static vk::PipelineViewportStateCreateInfo sStateCreateInfo{
        .viewportCount = ViewportCount,
        .scissorCount = ScissorCount,
    };
    return &sStateCreateInfo;
};

inline auto GetMultiSampleState_Disable() -> const vk::PipelineMultisampleStateCreateInfo * {
    static vk::PipelineMultisampleStateCreateInfo sStateCreateInfo{
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };
    return &sStateCreateInfo;
}

inline auto GetDepthStencilState_DepthStandard() -> const vk::PipelineDepthStencilStateCreateInfo * {
    static vk::PipelineDepthStencilStateCreateInfo sStateCreateInfo{
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = vk::CompareOp::eLessOrEqual,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
    };
    return &sStateCreateInfo;
}

inline auto GetDepthStencilState_DisableDepthStencil() -> const vk::PipelineDepthStencilStateCreateInfo * {
    static vk::PipelineDepthStencilStateCreateInfo sStateCreateInfo{
        .depthTestEnable = VK_FALSE,
        .depthWriteEnable = VK_FALSE,
        .depthCompareOp = vk::CompareOp::eAlways,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
    };
    return &sStateCreateInfo;
}

inline auto GetColorBlendAttachmentState_Opaque() -> const vk::PipelineColorBlendAttachmentState * {
    constexpr vk::Flags<vk::ColorComponentFlagBits> kWriteAll = vk::ColorComponentFlagBits::eR |
                                                                vk::ColorComponentFlagBits::eG |
                                                                vk::ColorComponentFlagBits::eB |
                                                                vk::ColorComponentFlagBits::eA;
    static vk::PipelineColorBlendAttachmentState sStateCreateInfo{
        .blendEnable = VK_FALSE,
        .colorWriteMask = kWriteAll,
    };
    return &sStateCreateInfo;
}

inline auto GetColorBlendAttachmentState_Blend() -> vk::PipelineColorBlendAttachmentState {
    constexpr vk::Flags<vk::ColorComponentFlagBits> kWriteAll = vk::ColorComponentFlagBits::eR |
                                                                vk::ColorComponentFlagBits::eG |
                                                                vk::ColorComponentFlagBits::eB |
                                                                vk::ColorComponentFlagBits::eA;
    vk::PipelineColorBlendAttachmentState state;
    state.blendEnable = VK_TRUE;
    state.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    state.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    state.colorBlendOp = vk::BlendOp::eAdd;
    state.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    state.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    state.alphaBlendOp = vk::BlendOp::eAdd;
    state.colorWriteMask = kWriteAll;
    return state;
}

inline auto GetComponentMapping_RGBA() -> vk::ComponentMapping {
    vk::ComponentMapping mapping;
    mapping.r = vk::ComponentSwizzle::eR;
    mapping.g = vk::ComponentSwizzle::eG;
    mapping.b = vk::ComponentSwizzle::eB;
    mapping.a = vk::ComponentSwizzle::eA;
    return mapping;
}

}    // namespace vkgfx
