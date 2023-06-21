#include "ShaderManager.h"
#include "VulkanRenderer/DefineList.h"
#include "Shader/ShaderDependency.h"
#include "Foundation/UUID128.h"
#include "Foundation/DebugBreak.h"
#include "Foundation/Logger.h"
#include "Utils/AssetProjectSetting.h"
#include "VulkanRenderer/ShaderCompiler.h"
#include "VulkanRenderer/Device.h"

#include <fstream>
#include <fmt/format.h>
#include <magic_enum.hpp>

#if defined(MODE_DEBUG)
static std::string_view sShaderCacheDirectory = "Debug";
#elif defined(MODE_RELEASE)
static std::string_view sShaderCacheDirectory = "Release";
#elif defined(MODE_RELWITHDEBINFO)
static std::string_view sShaderCacheDirectory = "RelWithDebInfo";
#endif

ShaderManager::ShaderManager() {
}

ShaderManager::~ShaderManager() {
}

void ShaderManager::Initialize() {
}

void ShaderManager::Destroy() {
    vk::Device device = vkgfx::gDevice->GetVKDevice();
    for (auto &shaderModule : _shaderModuleMap | std::views::values) {
	    device.destroyShaderModule(shaderModule);
    }
    _shaderModuleMap.clear();
}

auto ShaderManager::Load(stdfs::path path,
    std::string_view entryPoint,
    vkgfx::ShaderType type,
    const vkgfx::DefineList &defineList) -> vk::ShaderModule {

    if (!path.is_absolute()) {
        path = stdfs::absolute(path);
    }

    std::string keyString = fmt::format("{}_{}_{}_{}",
        path.string(),
        entryPoint.data(),
        magic_enum::enum_name(type).data(),
        defineList.ToString());

    UUID128 uuid = UUID128::New(keyString);
    if (auto iter = _shaderModuleMap.find(uuid); iter != _shaderModuleMap.end()) {
	    return iter->second;
    }

    std::string cacheFileName = fmt::format("{}.spir", uuid.ToString());
    stdfs::path shaderCachePath = gAssetProjectSetting->GetAssetCacheAbsolutePath() / sShaderCacheDirectory / cacheFileName;
    if (vk::ShaderModule shaderModule = LoadFromCache(uuid, path, shaderCachePath)) {
	    return shaderModule;
    }

	vkgfx::ShaderCompiler shaderCompiler;
    if (!shaderCompiler.Compile(path, entryPoint, type, defineList)) {
        Logger::Warning("Compile shader {} error: the error message: {}", path.string(), shaderCompiler.GetErrorMessage());
	    DEBUG_BREAK;
        return nullptr;
    }


}

auto ShaderManager::GetShaderDependency(stdfs::path path) -> ShaderDependency & {
    if (!path.is_absolute()) {
        path = stdfs::absolute(path);
    }

    auto iter = _shaderDependencyMap.find(path);
    if (iter != _shaderDependencyMap.end()) {
        return *iter->second;
    }

    auto item = std::make_pair(path, std::make_unique<ShaderDependency>(path));
    iter = _shaderDependencyMap.emplace_hint(iter, std::move(item));
    return *iter->second;
}

auto ShaderManager::LoadFromCache(UUID128 uuid, const stdfs::path &sourcePath, const stdfs::path &cachePath) -> vk::ShaderModule {
    vk::ShaderModule shaderModule = nullptr;
    stdfs::file_time_type cacheLastWriteTime = stdfs::last_write_time(cachePath);
    ShaderDependency &dependency = GetShaderDependency(sourcePath);
    if (dependency.GetLastWriteTime() <= cacheLastWriteTime) {
        std::ifstream fin(cachePath, std::ios::binary);
        fin.seekg(0, SEEK_END);
        auto fileSize  = fin.tellg();
        fin.seekg(0, SEEK_SET);
        std::vector<uint8_t> spirData;
        spirData.resize(fileSize, 0);
        fin.readsome(reinterpret_cast<char*>(spirData.data()), fileSize);

        vk::Device device = vkgfx::gDevice->GetVKDevice();
        vk::ShaderModuleCreateInfo shaderModuleCreateInfo;
        shaderModuleCreateInfo.codeSize = spirData.size() / sizeof(uint32_t);
        shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(spirData.data());
		shaderModule = device.createShaderModule(shaderModuleCreateInfo);
        _shaderModuleMap[uuid] = shaderModule;
        _shaderByteCodeMap[uuid] = std::move(spirData);
    }
    return shaderModule;
}
