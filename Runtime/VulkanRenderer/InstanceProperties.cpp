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

#include "InstanceProperties.h"
#include "Foundation/Exception.h"

namespace vkgfx {

bool InstanceProperties::IsLayerPresent(const char *pExtName) {
    auto iter = std::ranges::find_if(_instanceLayerProperties, 
        [pExtName](const VkLayerProperties& layerProps) -> bool {
            return strcmp(layerProps.layerName, pExtName) == 0;
        });
    return iter != _instanceLayerProperties.end();
}

bool InstanceProperties::IsExtensionPresent(const char *pExtName) {
    return std::find_if(
        _instanceExtensionProperties.begin(),
        _instanceExtensionProperties.end(),
        [pExtName](const VkExtensionProperties& extensionProps) -> bool {
        return strcmp(extensionProps.extensionName, pExtName) == 0;
    }) != _instanceExtensionProperties.end();
}

bool InstanceProperties::Init() {   
    _instanceLayerProperties = vk::enumerateInstanceLayerProperties();
    _instanceExtensionProperties = vk::enumerateInstanceExtensionProperties();
    return !_instanceLayerProperties.empty() && !_instanceLayerProperties.empty();
}

bool InstanceProperties::AddInstanceLayerName(const char *instanceLayerName) {
    if (IsLayerPresent(instanceLayerName)) {
        _instanceLayerNames.push_back(instanceLayerName);
        return true;
    }
    Exception::Throw("The instance layer '{}' has not been found", instanceLayerName);
    return false;
}

bool InstanceProperties::AddInstanceExtensionName(const char *instanceExtensionName) {
    if (IsExtensionPresent(instanceExtensionName)) {
        _instanceExtensionNames.push_back(instanceExtensionName);
        return true;
    }
    Exception::Throw("The instance extension '{}' has not been found", instanceExtensionName);
    return false;
}

void  InstanceProperties::GetExtensionNamesAndConfigs(std::vector<const char *> &pInstance_layer_names, 
    std::vector<const char *> &pInstance_extension_names) const
{
    for (auto &name : _instanceLayerNames) {
        pInstance_layer_names.push_back(name);
    }
    for (auto &name : _instanceExtensionNames) {
        pInstance_extension_names.push_back(name);
    }
}

}