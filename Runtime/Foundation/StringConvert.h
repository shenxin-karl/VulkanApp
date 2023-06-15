#pragma once
#include <string>

namespace nstd {

std::wstring to_wstring(const std::string &string);
std::string to_string(const std::wstring &string);

}
