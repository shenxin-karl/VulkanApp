#pragma once
#include <cassert>
#include <cstddef>
#include "Foundation/NonCopyable.h"

template<typename T>
class RuntimeStatic : NonCopyable {
public:
	void Initialize() {
		assert(_pObject == nullptr);
		_pObject = new T;
		_pObject->Initialize();
	}
	void Destroy() {
	    assert(_pObject != nullptr);
		_pObject->Destroy();
		delete _pObject;
		_pObject = nullptr;
	}
	T *Get() {
		return _pObject;
	}
	T *operator->() {
	    return _pObject;
	}
	explicit operator bool() const {
	    return _pObject != nullptr;
	}
	friend bool operator!=(const RuntimeStatic &lhs, std::nullptr_t) {
	    return lhs._pObject != nullptr;
	}
	friend bool operator!=(std::nullptr_t, const RuntimeStatic &rhs) {
	    return rhs._pObject != nullptr;
	}
	friend bool operator==(const RuntimeStatic &lhs, std::nullptr_t) {
	    return lhs._pObject == nullptr;
	}
	friend bool operator==(std::nullptr_t, const RuntimeStatic &rhs) {
	    return rhs._pObject == nullptr;
	}
private:
	T *_pObject = nullptr;
};