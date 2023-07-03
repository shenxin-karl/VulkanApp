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

#include "Foundation/PathUtils.h"
#include "VulkanRenderer/ExtDebugUtils.h"

#if defined(MODE_DEBUG)
static std::string_view sShaderCacheDirectory = "Shader/Debug";
#elif defined(MODE_RELEASE)
static std::string_view sShaderCacheDirectory = "Shader/Release";
#elif defined(MODE_RELWITHDEBINFO)
static std::string_view sShaderCacheDirectory = "Shader/RelWithDebInfo";
#endif

ShaderManager::ShaderManager() {
}

ShaderManager::~ShaderManager() {
}

void ShaderManager::Initialize() {
    stdfs::path shaderCacheDir = gAssetProjectSetting->GetAssetCacheAbsolutePath() / sShaderCacheDirectory;
    if (!stdfs::exists(shaderCacheDir)) {
        stdfs::create_directories(shaderCacheDir);
    }
    Exception::CondThrow(stdfs::is_directory(shaderCacheDir),
        "The cache path {} is occupied. Procedure",
        shaderCacheDir.string());
}

void ShaderManager::Destroy() {
    vk::Device device = vkgfx::gDevice->GetVKDevice();
    for (auto &shaderModule : _shaderModuleMap | std::views::values) {
        device.destroyShaderModule(shaderModule);
    }
    _shaderModuleMap.clear();
}

auto ShaderManager::LoadShaderModule(const ShaderLoadInfo &loadInfo) -> vk::ShaderModule {
    stdfs::path sourcePath = loadInfo.sourcePath;
    if (!sourcePath.is_absolute()) {
        sourcePath = stdfs::absolute(sourcePath);
    }

    Exception::CondThrow(nstd::IsSubPath(gAssetProjectSetting->GetAssetAbsolutePath(), sourcePath),
        "Only shaders under the Asset path can be loaded");

    std::string keyString = fmt::format("{}_{}_{}_{}",
        sourcePath.string(),
        loadInfo.entryPoint.data(),
        magic_enum::enum_name(loadInfo.shaderType).data(),
        loadInfo.defineList.ToString());

    UUID128 uuid = UUID128::New(keyString);
    if (auto iter = _shaderModuleMap.find(uuid); iter != _shaderModuleMap.end()) {
        return iter->second;
    }

    std::string cacheFileName = fmt::format("{}.spr", uuid.ToString());
    stdfs::path shaderCachePath = gAssetProjectSetting->GetAssetCacheAbsolutePath() / sShaderCacheDirectory /
                                  cacheFileName;
    if (vk::ShaderModule shaderModule = LoadFromCache(uuid, sourcePath, shaderCachePath)) {
        return shaderModule;
    }

    vkgfx::ShaderCompiler shaderCompiler;
    if (!shaderCompiler.Compile(sourcePath, loadInfo.entryPoint, loadInfo.shaderType, loadInfo.defineList)) {
        Logger::Warning("Compile shader {} error: the error message: {}",
            sourcePath.string(),
            shaderCompiler.GetErrorMessage());
        DEBUG_BREAK;
        return nullptr;
    }

    std::ofstream fileOutput(shaderCachePath, std::ios::binary);
    fileOutput.write(static_cast<const char *>(shaderCompiler.GetByteCodePtr()), shaderCompiler.GetByteCodeSize());
    fileOutput.close();

    std::vector<char> byteCode;
    byteCode.resize(shaderCompiler.GetByteCodeSize(), 0);
    std::memcpy(byteCode.data(), shaderCompiler.GetByteCodePtr(), shaderCompiler.GetByteCodeSize());

    vk::ShaderModule shaderModule = LoadFromByteCode(uuid, byteCode);
    if (shaderModule) {
        vkgfx::SetResourceName(vkgfx::gDevice->GetVKDevice(), shaderModule, keyString);
    }

    _shaderByteCodeMap[uuid] = std::move(byteCode);
    return shaderModule;
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

bool ShaderManager::LoadShaderStageCreateInfo(const ShaderLoadInfo &loadInfo,
    vk::PipelineShaderStageCreateInfo &outputCreateInfo) {

    if (vk::ShaderModule shaderModule = LoadShaderModule(loadInfo)) {
        outputCreateInfo = vk::PipelineShaderStageCreateInfo{};
        outputCreateInfo.module = shaderModule;
        outputCreateInfo.pName = loadInfo.entryPoint.data();
        switch (loadInfo.shaderType) {
        case vkgfx::ShaderType::kVS:
            outputCreateInfo.stage = vk::ShaderStageFlagBits::eVertex;
            break;
        case vkgfx::ShaderType::kHS:
            outputCreateInfo.stage = vk::ShaderStageFlagBits::eTessellationControl;
            break;
        case vkgfx::ShaderType::kDS:
            outputCreateInfo.stage = vk::ShaderStageFlagBits::eTessellationEvaluation;
            break;
        case vkgfx::ShaderType::kGS:
            outputCreateInfo.stage = vk::ShaderStageFlagBits::eGeometry;
            break;
        case vkgfx::ShaderType::kPS:
            outputCreateInfo.stage = vk::ShaderStageFlagBits::eFragment;
            break;
        case vkgfx::ShaderType::kCS:
            outputCreateInfo.stage = vk::ShaderStageFlagBits::eCompute;
            break;
        default:
            Exception::Throw("Invalid ShaderType {}", magic_enum::enum_name(loadInfo.shaderType).data());
        }
        return true;
    }
    DEBUG_BREAK;
    return false;
}

auto ShaderManager::LoadFromCache(UUID128 uuid, const stdfs::path &sourcePath, const stdfs::path &cachePath)
    -> vk::ShaderModule {

    if (!stdfs::exists(cachePath)) {
	    return nullptr;
    }

    vk::ShaderModule shaderModule = nullptr;
    stdfs::file_time_type cacheLastWriteTime = stdfs::last_write_time(cachePath);
    ShaderDependency &dependency = GetShaderDependency(sourcePath);
    if (dependency.GetLastWriteTime() <= cacheLastWriteTime) {
        std::ifstream fin(cachePath, std::ios::binary);
        fin.seekg(0, SEEK_END);
        uint32_t fileSize = fin.tellg();
        fin.seekg(0, SEEK_SET);
        std::vector<char> byteCode;
        byteCode.resize(fileSize, 0);
        fin.read(byteCode.data(), fileSize);
        shaderModule = LoadFromByteCode(uuid, byteCode);
        _shaderByteCodeMap[uuid] = std::move(byteCode);
    }
    return shaderModule;
}

auto ShaderManager::LoadFromByteCode(UUID128 uuid, std::span<const char> byteCode) -> vk::ShaderModule {
    vk::ShaderModule shaderModule = nullptr;
    vk::Device device = vkgfx::gDevice->GetVKDevice();
    vk::ShaderModuleCreateInfo shaderModuleCreateInfo;
    shaderModuleCreateInfo.codeSize = byteCode.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(byteCode.data());
    shaderModule = device.createShaderModule(shaderModuleCreateInfo);
    _shaderModuleMap[uuid] = shaderModule;
    return shaderModule;
}
