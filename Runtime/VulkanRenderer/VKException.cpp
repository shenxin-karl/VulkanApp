#include "VKException.h"
#include <vulkan/vulkan_to_string.hpp>

void VKException::Throw(VkResult result, std::source_location sl) {
    if (result == VK_SUCCESS) {
        return;
    }
    std::string errorMessage = fmt::format("Vulkan error: The code: {}, The message: {}", 
        static_cast<int>(result),    
        vk::to_string(static_cast<vk::Result>(result))
    );
    VKException exception(errorMessage, sl);
    exception._errorCode = static_cast<VkResult>(result);
    ThrowException(exception);
}
