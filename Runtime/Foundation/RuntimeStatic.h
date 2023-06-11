#pragma once
#include <cassert>
#include <cstddef>
#include <type_traits>
#include "Foundation/NonCopyable.h"

template<typename T>
class RuntimeStatic : NonCopyable {
public:
    RuntimeStatic() {
        static_assert(std::is_constructible_v<T>, "This Type not member func 'Initialize' or 'Destroy'");
        _pObject = new T;
    }
    ~RuntimeStatic() {
        delete _pObject;
        _pObject = nullptr;
    }
    T *Get() {
        return _pObject;
    }
    T *operator->() {
        return _pObject;
    }
    operator T*() const {
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