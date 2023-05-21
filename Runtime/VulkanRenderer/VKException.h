#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include "Foundation/Exception.h"
#include "Foundation/PreprocessorDirectives.h"

class VKException : public Exception {
public:
    using Exception::Exception;
    static void Throw(VkResult result, std::source_location sl = std::source_location::current());

    Inline(2) static void Throw(vk::ResultValue<uint32_t> result,
        const std::source_location &sl = std::source_location::current()) {

        Throw(static_cast<VkResult>(result.result), sl);
    }

    Inline(2) static void Throw(vk::Result result, const std::source_location &sl = std::source_location::current()) {
        Throw(static_cast<VkResult>(result), sl);
    }
private:
    VkResult _errorCode;
};
