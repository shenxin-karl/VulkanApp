#include "ShaderCompiler.h"
#include "DxcModule.h"
#include "Foundation/Exception.h"

bool vkgfx::ShaderCompiler::Compile(const stdfs::path &path,
                                    std::string_view entryPoint,
                                    ShaderType type,
                                    const DefineList &defineList) {

    std::wstring_view target;
    switch (type) {
    case ShaderType::kVS:
        target = L"vs_6_1";
        break;
    case ShaderType::kHS:
        target = L"hs_6_1";
        break;
    case ShaderType::kDS:
        target = L"ds_6_1";
        break;
    case ShaderType::kGS:
        target = L"gs_6_1";
        break;
    case ShaderType::kPS:
        target = L"ps_6_1";
        break;
    case ShaderType::kCS:
        target = L"cs_6_1";
        break;
    default:
        Exception::Throw("Error ShaderType");
        break;  
    }

    std::wstring fileName;
    std::to_wstring()
    std::vector<LPCWSTR> arguments = {
        
    };
}

auto vkgfx::ShaderCompiler::GetErrorMessage() const -> const std::string & {
    return _errorMessage;
}

auto vkgfx::ShaderCompiler::GetByteCodePtr() const -> void * {
    return (_pByteCode != nullptr) ? _pByteCode->GetBufferPointer() : nullptr;
}

auto vkgfx::ShaderCompiler::GetByteCodeSize() const -> size_t {
    return (_pByteCode != nullptr) ? _pByteCode->GetBufferSize() : 0;
}
