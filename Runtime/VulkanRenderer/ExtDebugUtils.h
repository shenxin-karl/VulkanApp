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
#pragma once

#include "vulkan/vulkan.h"
#include "InstanceProperties.h"
#include "Foundation/PreprocessorDirectives.h"

namespace vkgfx {

inline bool s_bCanUseDebugUtils = false;
inline std::mutex s_mutex = {};

Inline(2) bool ExtDebugUtilsCheckInstanceExtensions(InstanceProperties &instanceProperties) {
    s_bCanUseDebugUtils = instanceProperties.AddInstanceExtensionName("VK_EXT_debug_utils");
	return s_bCanUseDebugUtils;	
}

void SetResourceName(vk::Device device, vk::ObjectType objectType, uint64_t handle, std::string_view name);
void SetPerfMarkerBegin(vk::CommandBuffer cmd, std::string_view name);
void SetPerfMarkerEnd(vk::CommandBuffer cmd);

template<typename T>
void SetResourceName(vk::Device device, T object, std::string_view name) {
	using NativeType = typename T::NativeType;
	vk::ObjectType objectType = object.objectType;
	uint64_t handle = reinterpret_cast<uint64_t>(object.operator NativeType());
	SetResourceName(device, objectType, handle, name);
}

}    // namespace vkgfx