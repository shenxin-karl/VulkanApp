#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include "Foundation/Exception.h"

class VKException : public Exception {
public:
    using Exception::Exception;
    static void Throw(vk::ResultValue<uint32_t> result, std::source_location sl = std::source_location::current());
    static void Throw(VkResult result, std::source_location sl = std::source_location::current());
private:
    VkResult _errorCode;
};