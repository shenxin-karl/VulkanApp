#include "RenderDoc.h"
#include "Foundation/NamespeceAlias.h"
#include <renderdoc_app.h>
#include <Windows.h>

#include "Foundation/Exception.h"
#include "VulkanRenderer/Device.h"

#if PLATFORM_WIN
#include "Foundation/Platform/Win/RegistryUtils.h"
#endif

static RENDERDOC_API_1_0_0 *sRenderDocApi = nullptr;
static HINSTANCE sRenderDocDllModule = nullptr;

bool RenderDoc::IsLoaded() {
    return sRenderDocApi != nullptr;
}

bool RenderDoc::Load() {
    if (IsLoaded()) {
		Exception::Throw("RenderDoc Is Loaded!");
        return true;
    }

    auto InitRenderDocApi = [](const stdfs::path &modulePath) {
        if (sRenderDocDllModule != nullptr) {
	        return;
        }

	    sRenderDocDllModule = LoadLibrary(modulePath.wstring().c_str());
	    if (sRenderDocDllModule == nullptr) {
	        return;
	    }
	    pRENDERDOC_GetAPI RENDERDOC_GetAPI = reinterpret_cast<pRENDERDOC_GetAPI>(GetProcAddress(sRenderDocDllModule, "RENDERDOC_GetAPI"));
	    if (RENDERDOC_GetAPI) {
		    if (RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_0_0, reinterpret_cast<void **>(&sRenderDocApi))) {
				sRenderDocApi->MaskOverlayBits(0, 0);
				sRenderDocApi->SetFocusToggleKeys(nullptr, 0);
				sRenderDocApi->SetCaptureKeys(nullptr, 0);
				sRenderDocApi->UnloadCrashHandler();
			}
	    }
    };


#if PLATFORM_WIN
    std::wstring_view path = L"Software\\Classes\\CLSID\\{5D6BF029-A6BA-417A-8523-120492B1DCE3}\\InprocServer32";
	if (std::optional<std::string> pDllPath = Registry::GetString(HKEY_LOCAL_MACHINE, path.data(), L"")) {
		InitRenderDocApi(*pDllPath);
	}
#endif

	InitRenderDocApi("renderdoc.dll");
    return sRenderDocApi != nullptr;
}

void RenderDoc::Free() {
	if (sRenderDocDllModule) {
		FreeModule(sRenderDocDllModule);
		sRenderDocDllModule = nullptr;
	}
}

void RenderDoc::BeginFrameCapture(void *pNativeWindowHandle) {
    if (sRenderDocApi == nullptr) {
	    return;
    }
    VkInstance instance = vkgfx::gDevice->GetInstance();
    void *devicePointer = RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(instance);
    sRenderDocApi->StartFrameCapture(devicePointer, pNativeWindowHandle);
}

void RenderDoc::EndFrameCapture(void *pNativeWindowHandle) {
    if (sRenderDocApi == nullptr) {
	    return;
    }
    VkInstance instance = vkgfx::gDevice->GetInstance();
    void *devicePointer = RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(instance);
    sRenderDocApi->EndFrameCapture(devicePointer, pNativeWindowHandle);
    if (!sRenderDocApi->IsRemoteAccessConnected()) {
	    sRenderDocApi->LaunchReplayUI(true, "");
    }
}
