#include "ShaderCompiler.h"
#include "DxcModule.h"
#include "Foundation/Exception.h"
#include "Foundation/StringConvert.h"
#include "DefineList.h"
#include "Foundation/PathUtils.h"
#include "Utils/AssetProjectSetting.h"
#include "DxcModule.h"

namespace vkgfx {

class CustomIncludeHandler : public IDxcIncludeHandler {
public:
    HRESULT STDMETHODCALLTYPE LoadSource(_In_ LPCWSTR pFilename,
        _COM_Outptr_result_maybenull_ IDxcBlob **ppIncludeSource) override {
        using Microsoft::WRL::ComPtr;
        ComPtr<IDxcBlobEncoding> pEncoding;

        stdfs::path filePath(nstd::to_string(std::wstring(pFilename)));
        stdfs::path assetAbsolutePath = gAssetProjectSetting->GetAssetAbsolutePath();
        std::optional<stdfs::path> pRelativePath = nstd::ToRelativePath(assetAbsolutePath, filePath);
        if (!pRelativePath) {
            return S_FALSE;
        }

        filePath = assetAbsolutePath / pRelativePath.value();
        std::wstring wFileName = nstd::to_wstring(filePath.string());
        HRESULT hr = gDxcModule->GetUtils()->LoadFile(wFileName.c_str(), nullptr, pEncoding.GetAddressOf());
        if (SUCCEEDED(hr)) {
            *ppIncludeSource = pEncoding.Detach();
            return S_OK;
        }
        return S_FALSE;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject) override {
        return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef(void) override {
        return 0;
    }
    ULONG STDMETHODCALLTYPE Release(void) override {
        return 0;
    }
};

bool ShaderCompiler::Compile(const stdfs::path &path,
    std::string_view entryPoint,
    ShaderType type,
    const DefineList &defineList,
    bool makeDebugInfo) {

    std::wstring fileName = nstd::to_wstring(path.string());
    CustomIncludeHandler includeHandler;
    Microsoft::WRL::ComPtr<IDxcBlob> pSourceBlob;
    _result = includeHandler.LoadSource(fileName.c_str(), pSourceBlob.GetAddressOf());
    if (FAILED(_result)) {
        _errorMessage = fmt::format("Can't open the file {}", path.string());
        return false;
    }

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

    std::wstring entryPointStr = nstd::to_wstring(entryPoint);
    std::vector<LPCWSTR> arguments = {fileName.c_str(), L"-E", entryPointStr.c_str(), L"-T", target.data(), L"-spirv"};

	if (makeDebugInfo) {
		arguments.push_back(L"-Zi");
        arguments.push_back(L"-O0");
	}

    std::vector<std::wstring> macros;
    for (auto &&[key, value] : defineList) {
        std::string arg = fmt::format("-D{}={}", key, value);
        macros.push_back(nstd::to_wstring(arg));
        arguments.push_back(macros.back().c_str());
    }

    // Compile shader
    DxcBuffer buffer{};
    buffer.Encoding = DXC_CP_ACP;
    buffer.Ptr = pSourceBlob->GetBufferPointer();
    buffer.Size = pSourceBlob->GetBufferSize();

    Microsoft::WRL::ComPtr<IDxcResult> pCompileResult;
    _result = gDxcModule->GetCompiler3()->Compile(&buffer,
        arguments.data(),
        static_cast<uint32_t>(arguments.size()),
        &includeHandler,
        IID_PPV_ARGS(&pCompileResult));

    if (pCompileResult == nullptr) {
	    return false;
    }

    pCompileResult->GetStatus(&_result);
    if (FAILED(_result)) {
	    Microsoft::WRL::ComPtr<IDxcBlobEncoding> pErrorBlob;
	    _result = pCompileResult->GetErrorBuffer(&pErrorBlob);
	    _errorMessage = static_cast<const char *>(pErrorBlob->GetBufferPointer());
	    return false;
    }

    _result = pCompileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&_pByteCode), nullptr);
    return SUCCEEDED(_result);
}

auto ShaderCompiler::GetErrorMessage() const -> const std::string & {
    return _errorMessage;
}

auto ShaderCompiler::GetByteCodePtr() const -> void * {
    if (_pByteCode != nullptr)
        return _pByteCode->GetBufferPointer();
    return nullptr;
}

auto ShaderCompiler::GetByteCodeSize() const -> size_t {
    if (_pByteCode != nullptr)
        return _pByteCode->GetBufferSize();
    return 0;
}
}    // namespace vkgfx
