#pragma once
#include <string>
#include <vector>
#include <Windows.h>
#include "Foundation/NamespeceAlias.h"

namespace vkgfx {

class DefineList;
class ShaderCompiler {
public:
	enum class ShaderType {
		kVS = 1,
		kHS = 2,
		kDS = 3,
		kGS = 4,
		kPS = 5,
		kCS = 6,
	};
	void Compile(const stdfs::path &path, std::string_view entryPoint, ShaderType type, const DefineList &defineList);
private:
	HRESULT _result = 0;
	std::string _errorMessage;
	
};

}
