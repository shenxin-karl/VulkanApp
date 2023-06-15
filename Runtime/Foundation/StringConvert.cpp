#include "StringConvert.h"
#include <locale>
#ifndef _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#endif
#ifndef _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#endif
#include <codecvt>

namespace nstd {

static thread_local std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
std::wstring to_wstring(const std::string &string) {
    return converter.from_bytes(string);
}

std::string to_string(const std::wstring &string) {
    return converter.to_bytes(string);
}

}    // namespace nstd
