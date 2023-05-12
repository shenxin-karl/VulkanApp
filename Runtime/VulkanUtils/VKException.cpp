#include "VKException.h"
#include <vulkan/vk_enum_string_helper.h>

void VKException::Throw(vk::ResultValue<uint32_t> result, std::source_location sl) {
    Throw(static_cast<VkResult>(result.result), sl);
}

void VKException::Throw(VkResult result, std::source_location sl) {
    if (result == VK_SUCCESS) {
        return;
    }
    std::string errorMessage = fmt::format("Vulkan error: The code: {}, The message: {}", 
        static_cast<int>(result),    
        string_VkResult(static_cast<VkResult>(result))
    );
    VKException exception(errorMessage, sl);
    exception._errorCode = static_cast<VkResult>(result);
    ThrowException(exception);
}
