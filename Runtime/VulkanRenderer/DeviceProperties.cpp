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
#include "DeviceProperties.h"
#include "Foundation/Exception.h"

namespace vkgfx {

bool DeviceProperties::IsExtensionPresent(const char *pExtName) {
    return std::find_if(_deviceExtensionProperties.begin(),
                        _deviceExtensionProperties.end(),
                        [pExtName](const VkExtensionProperties &extensionProps) -> bool {
                            return strcmp(extensionProps.extensionName, pExtName) == 0;
                        }) != _deviceExtensionProperties.end();
}

bool DeviceProperties::Init(vk::PhysicalDevice physicalDevice) {
    _physicalDevice = physicalDevice;
    _deviceExtensionProperties = _physicalDevice.enumerateDeviceExtensionProperties();
    return _deviceExtensionProperties.size();
}

bool DeviceProperties::AddDeviceExtensionName(const char *deviceExtensionName) {
    if (IsExtensionPresent(deviceExtensionName)) {
        _deviceExtensionNames.push_back(deviceExtensionName);
        return true;
    }
    Exception::Throw("The device extension '{}' has not been found", deviceExtensionName);
    return false;
}

void DeviceProperties::GetExtensionNamesAndConfigs(
    std::vector<const char *> &deviceExtensionNames) const {
    for (auto &name : _deviceExtensionNames) {
        deviceExtensionNames.push_back(name);
    }
}

}    // namespace vkgfx