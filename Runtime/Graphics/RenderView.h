#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include <optional>

// clang-format off
struct RenderView {
	glm::vec4					cameraPosition;
	glm::mat4					matProj;
	glm::mat4					matView;
	glm::mat4					matViewProj;
	glm::mat4					matInvProj;
	glm::mat4					matInvView;
	glm::mat4					matInvViewProj;
	float						depth;
	uint32_t					cullMask;
	vk::Viewport				viewport;
	std::optional<vk::Rect2D>	pScissor;
	
};
// clang-format on