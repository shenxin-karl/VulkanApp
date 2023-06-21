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

struct ShaderLoadInfo {
	const stdfs::path &sourcePath;
    std::string_view entryPoint;
    vkgfx::ShaderType shaderType;
    const vkgfx::DefineList &defineList;
};

class ShaderDependency;
class ShaderManager : public NonCopyable {
public:
    ShaderManager();
    ~ShaderManager();
    void Initialize();
    void Destroy();
    auto LoadShaderModule(ShaderLoadInfo loadInfo) -> vk::ShaderModule;
    auto GetShaderDependency(stdfs::path path) -> ShaderDependency &;
    bool LoadShaderStageCreateInfo(const ShaderLoadInfo &loadInfo, vk::PipelineShaderStageCreateInfo &outputCreateInfo);
private:
    auto LoadFromCache(UUID128 uuid, const stdfs::path &sourcePath, const stdfs::path &cachePath) -> vk::ShaderModule;
    auto LoadFromByteCode(UUID128 uuid, std::span<const char> byteCode) -> vk::ShaderModule;
    using ShaderModuleMap = std::unordered_map<UUID128, vk::ShaderModule>;
    using ShaderByteCodeMap = std::unordered_map<UUID128, std::vector<char>>;
    using ShaderDependencyMap = std::unordered_map<stdfs::path, std::unique_ptr<ShaderDependency>>;
private:
    ShaderModuleMap _shaderModuleMap;
    ShaderByteCodeMap _shaderByteCodeMap;
    ShaderDependencyMap _shaderDependencyMap;
};

inline RuntimeStatic<ShaderManager> gShaderManager;