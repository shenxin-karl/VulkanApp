#pragma once
#include "Foundation/NonCopyable.h"
#include "Foundation/Exception.h"

namespace vkgfx {

class Device;
class VKObject : public NonCopyable {
public:
    virtual ~VKObject() {
        ExceptionAssert(!_isCreate);
    }
	void SetIsCreate(bool bCreate) {
        ExceptionAssert(bCreate != _isCreate);
	    _isCreate = bCreate;
    }
    bool GetIsCreate() const {
	    return _isCreate;
    }
    void SetDevice(Device *pDevice) {
	    _pDevice = pDevice;
    }
    auto GetDevice() const -> Device * {
	    return _pDevice;
    }
private:
    Device *_pDevice = nullptr;
    bool _isCreate = false;
};

}    // namespace vkgfx
