#pragma once
#include <cassert>
#include <cstddef>
#include "Foundation/NonCopyable.h"

template<typename T>
class RuntimeStatic : NonCopyable {
public:
	void Startup() {
	    assert(!_isInit);
		new(&_value) T;
		
	}

	T *Get() {
	    
	}
private:
	bool	  _isInit = false;
	std::byte _value[sizeof(T)];
};