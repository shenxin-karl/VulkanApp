#pragma once
#include "Foundation/PreprocessorDirectives.h"

namespace vkgfx {

template<typename T>
Inline(2) T AlignUp(T val, T alignment) {
    return (val + alignment - static_cast<T>(1)) & ~(alignment - static_cast<T>(1));
}

template<typename T>
Inline(2) T AlignDown(T val, T alignment) {
    return val & ~(alignment - static_cast<T>(1));
}

template<typename T>
Inline(2) T DivideRoundingUp(T a, T b) {
    return (a + b - static_cast<T>(1)) / b;
}

}    // namespace vkgfx
