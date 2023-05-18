// AMD Cauldron code
//
// Copyright(c) 2018 Advanced Micro Devices, Inc.All rights reserved.
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "ExtDebugUtils.h"

namespace vkgfx {

void SetResourceName(vk::Device device, vk::ObjectType objectType, uint64_t handle, const char *name) {
    if (VULKAN_HPP_DEFAULT_DISPATCHER.vkSetDebugUtilsObjectNameEXT && handle && name) {
        std::unique_lock<std::mutex> lock(s_mutex);
        vk::DebugUtilsObjectNameInfoEXT nameInfo = {};
        nameInfo.sType = vk::StructureType::eDebugUtilsObjectNameInfoEXT;
        nameInfo.objectType = objectType;
        nameInfo.objectHandle = handle;
        nameInfo.pObjectName = name;
        device.setDebugUtilsObjectNameEXT(nameInfo);
    }
}

void SetPerfMarkerBegin(vk::CommandBuffer cmd, const char *name) {
    if (VULKAN_HPP_DEFAULT_DISPATCHER.vkCmdBeginDebugUtilsLabelEXT && name) {
        std::array<float, 4> color = {1.0f, 0.0f, 0.0f, 1.0f};
	    vk::DebugUtilsLabelEXT label = {
            .pLabelName = name,
            .color = color,
	    };
        cmd.beginDebugUtilsLabelEXT(label);
    }
}

void SetPerfMarkerEnd(vk::CommandBuffer cmd) {
    if (VULKAN_HPP_DEFAULT_DISPATCHER.vkCmdEndDebugUtilsLabelEXT) {
	    cmd.endDebugUtilsLabelEXT();
    }
}

}    // namespace vkgfx
