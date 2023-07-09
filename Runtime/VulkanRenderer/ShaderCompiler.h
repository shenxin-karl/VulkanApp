#pragma once
#include <string>
#include <Windows.h>
#include <wrl/client.h>
#include <dxcapi.h>
#include "EnumDefinition.h"
#include "Foundation/CompileEnvInfo.hpp"
#include "Foundation/NamespeceAlias.h"
#include "Foundation/NonCopyable.h"
#include "Foundation/ObjectView.hpp"

namespace vkgfx {
class DefineList;

class DefineList;
class ShaderCompiler : public NonCopyable {
public:
    bool Compile(const stdfs::path &path,
        std::string_view entryPoint,
        ShaderType type,
        ObjectView<const DefineList> pDefineList,
        bool makeDebugInfo = !CompileEnvInfo::IsModeRelease());
    auto GetErrorMessage() const -> const std::string &;
    auto GetByteCodePtr() const -> void *;
    auto GetByteCodeSize() const -> size_t;
private:
    HRESULT _result = 0;
    std::string _errorMessage;
    Microsoft::WRL::ComPtr<IDxcBlob> _pByteCode = nullptr;
};

}    // namespace vkgfx
