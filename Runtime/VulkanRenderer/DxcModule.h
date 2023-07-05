#pragma once
#include <Windows.h>
#include <dxcapi.h>
#include <wrl/client.h>
#include "Foundation/NonCopyable.h"
#include "Foundation/RuntimeStatic.h"

namespace vkgfx {

class DxcModule : public NonCopyable {
public:
    void OnCreate();
    void OnDestroy();
    auto GetCompiler3() const -> IDxcCompiler3 *;
    auto GetLinker() const -> IDxcLinker *;
    auto GetUtils() const -> IDxcUtils *;
    auto GetLibrary() const -> IDxcLibrary *;
private:
    Microsoft::WRL::ComPtr<IDxcUtils> _pUtils;
    Microsoft::WRL::ComPtr<IDxcLinker> _pLinker;
    Microsoft::WRL::ComPtr<IDxcLibrary> _pLibrary;
    Microsoft::WRL::ComPtr<IDxcCompiler3> _pCompiler;
};

inline RuntimeStatic<DxcModule> gDxcModule;

}    // namespace vkgfx
