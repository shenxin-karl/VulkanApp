#pragma once
#include <string>
#include <vector>
#include <Windows.h>
#include <wrl/client.h>
#include <dxcapi.h>
#include "Foundation/NamespeceAlias.h"
#include "Foundation/NonCopyable.h"

namespace vkgfx {

class DefineList;
class ShaderCompiler : public NonCopyable {
public:
	enum class ShaderType {
		kVS = 1,
		kHS = 2,
		kDS = 3,
		kGS = 4,
		kPS = 5,
		kCS = 6,
	};
	bool Compile(const stdfs::path &path, std::string_view entryPoint, ShaderType type, const DefineList &defineList);
	auto GetErrorMessage() const -> const std::string &;
	auto GetByteCodePtr() const -> void *;
	auto GetByteCodeSize() const -> size_t;
private:
	HRESULT _result = 0;
	std::string _errorMessage;
	Microsoft::WRL::ComPtr<IDxcBlob> _pByteCode = nullptr;
};

}
