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

#include <vector>
#include <vulkan/vulkan.hpp>

namespace vkgfx {

class InstanceProperties {
public:
    bool Init();
    bool AddLayer(const char *pInstanceLayerName);
    bool AddExtension(const char *pInstanceExtensionName);
    bool AddExtension(const std::vector<const char *> &extensions);
    auto GetNext() const -> void * {
        return _pNext;
    }
    void SetNewNext(void *pNext) {
        _pNext = pNext;
    }
    void GetLayersAndExtensions(std::vector<const char *> &pInstance_layer_names,
        std::vector<const char *> &pInstance_extension_names) const;
private:
    bool IsLayerPresent(const char *pExtName);
    bool IsExtensionPresent(const char *pExtName);
private:
    friend class Device;
    std::vector<vk::LayerProperties> _instanceLayerProperties;
    std::vector<vk::ExtensionProperties> _instanceExtensionProperties;
    std::vector<const char *> _instanceLayerNames;
    std::vector<const char *> _instanceExtensionNames;
    void *_pNext = nullptr;
};

}    // namespace vkgfx
