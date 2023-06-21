#pragma once
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "Foundation/RuntimeStatic.h"
#include "Foundation/NonCopyable.h"
#include "Foundation/NamespeceAlias.h"
#include "Foundation/UUID128.h"
#include "VulkanRenderer/EnumDefinition.h"

namespace vkgfx {
class DefineList;
}

class ShaderDependency;
class ShaderManager : public NonCopyable {
public:
    ShaderManager();
    ~ShaderManager();
    void Initialize();
    void Destroy();
    auto Load(stdfs::path path,
        std::string_view entryPoint,
        vkgfx::ShaderType type,
        const vkgfx::DefineList &defineList) -> vk::ShaderModule;
    auto GetShaderDependency(stdfs::path path) -> ShaderDependency &;
private:
    auto LoadFromCache(UUID128 uuid, const stdfs::path &sourcePath, const stdfs::path &cachePath) -> vk::ShaderModule;
    //auto LoadByByteCode(UUID128 uuid)
    using ShaderModuleMap = std::unordered_map<UUID128, vk::ShaderModule>;
    using ShaderByteCodeMap = std::unordered_map<UUID128, std::vector<uint8_t>>;
    using ShaderDependencyMap = std::unordered_map<stdfs::path, std::unique_ptr<ShaderDependency>>;
private:
    ShaderModuleMap _shaderModuleMap;
    ShaderByteCodeMap _shaderByteCodeMap;
    ShaderDependencyMap _shaderDependencyMap;
};

inline RuntimeStatic<ShaderManager> gShaderManager;