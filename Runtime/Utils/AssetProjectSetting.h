#pragma once
#include "Foundation/NamespeceAlias.h"
#include "Foundation/NonCopyable.h"
#include "Foundation/RuntimeStatic.h"
#include "Foundation/TypeBase.hpp"
#include "Serializer/TransferBase.hpp"

class AssetProjectSetting : public TypeBase {
    DECLARE_CLASS(AssetProjectSetting)
    DECLARE_SERIALIZER(AssetProjectSetting)
public:
    void Initialize();
    void Destroy();
    void SetAssetRelativePath(stdfs::path path);
    void SetAssetAbsolutePath(stdfs::path path);
    void SetAssetCacheRelativePath(stdfs::path path);
    void SetAssetCacheAbsolutePath(stdfs::path path);
    bool SerializeToFile();

    auto GetAssetRelativePath() const -> const stdfs::path & {
        return _assetRelativePath;
    }
    auto GetAssetAbsolutePath() const -> const stdfs::path & {
        return _assetAbsolutePath;
    }
    auto GetAssetCacheRelativePath() const -> const stdfs::path & {
        return _assetCacheRelativePath;
    }
    auto GetAssetCacheAbsolutePath() const -> const stdfs::path & {
        return _assetCacheAbsolutePath;
    }
private:
	constexpr static std::string_view sSerializePath = "AssetProjectSetting.json";
    static void RepairPath(stdfs::path &relativePath, stdfs::path &absolutePath);
private:
    stdfs::path _assetRelativePath;
    stdfs::path _assetAbsolutePath;
    stdfs::path _assetCacheRelativePath;
    stdfs::path _assetCacheAbsolutePath;
};

inline RuntimeStatic<AssetProjectSetting> gAssetProjectSetting;
