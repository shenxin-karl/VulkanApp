#include "InstanceProperties.h"
#include "Foundation/Exception.h"

namespace vkgfx {

bool InstanceProperties::IsLayerPresent(const char *pExtName) {
    auto iter = std::ranges::find_if(_instanceLayerProperties, [pExtName](const VkLayerProperties &layerProps) -> bool {
        return strcmp(layerProps.layerName, pExtName) == 0;
    });
    return iter != _instanceLayerProperties.end();
}

bool InstanceProperties::IsExtensionPresent(const char *pExtName) {
    return std::ranges::find_if(_instanceExtensionProperties,
               [pExtName](const VkExtensionProperties &extensionProps) -> bool {
                   return strcmp(extensionProps.extensionName, pExtName) == 0;
               }) != _instanceExtensionProperties.end();
}

bool InstanceProperties::Init() {
    _instanceLayerProperties = vk::enumerateInstanceLayerProperties();
    _instanceExtensionProperties = vk::enumerateInstanceExtensionProperties();
    return !_instanceLayerProperties.empty() && !_instanceLayerProperties.empty();
}

bool InstanceProperties::AddLayer(const char *pInstanceLayerName) {
    if (pInstanceLayerName == nullptr) {
        return false;
    }

    for (const char *pLayerName : _instanceLayerNames) {
        if (strcmp(pLayerName, pInstanceLayerName) == 0) {
            return true;
        }
    }

    if (IsLayerPresent(pInstanceLayerName)) {
        _instanceLayerNames.push_back(pInstanceLayerName);
        return true;
    }
    return false;
}

bool InstanceProperties::AddExtension(const char *pInstanceExtensionName) {
    if (pInstanceExtensionName == nullptr) {
        return false;
    }

    for (const char *pExtension : _instanceExtensionNames) {
        if (strcmp(pExtension, pInstanceExtensionName) == 0) {
            return true;
        }
    }

    if (IsExtensionPresent(pInstanceExtensionName)) {
        _instanceExtensionNames.push_back(pInstanceExtensionName);
        return true;
    }
    return false;
}

bool InstanceProperties::AddExtension(const std::vector<const char *> &extensions) {
    bool result = true;
    for (const char *pName : extensions) {
        if (!AddExtension(pName)) {
	        result = false;
        }
    }
    return result;
}

void InstanceProperties::GetLayersAndExtensions(std::vector<const char *> &pInstance_layer_names,
    std::vector<const char *> &pInstance_extension_names) const {
    for (auto &name : _instanceLayerNames) {
        pInstance_layer_names.push_back(name);
    }
    for (auto &name : _instanceExtensionNames) {
        pInstance_extension_names.push_back(name);
    }
}

}    // namespace vkgfx