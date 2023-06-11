#include "DefineList.h"
#include <algorithm>
#include <sstream>
#include <fmt/format.h>
#include <regex>

#include "Foundation/Exception.h"

namespace vkgfx {

void DefineList::Set(std::string_view key, int value) {
    auto iter = Find(key);
    if (iter != _macroItems.end()) {
        iter->value = value;
    }
    auto &back = _macroItems.emplace_back();
    back.key = key;
    back.value = value;
}

auto DefineList::Get(std::string_view key) const -> std::optional<int> {
    auto iter = Find(key);
    if (iter == _macroItems.end()) {
        return std::nullopt;
    }
    return std::make_optional(iter->value);
}

bool DefineList::Remove(std::string_view key) {
    auto iter = Find(key);
    if (iter == _macroItems.end()) {
        return false;
    }
    _macroItems.erase(iter);
    return true;
}

void DefineList::Clear() {
    _macroItems.clear();
}

auto DefineList::GetCount() const -> size_t {
    return _macroItems.size();
}

auto DefineList::operator[](std::string_view key) -> int & {
    auto iter = Find(key);
    if (iter != _macroItems.end()) {
        return iter->value;
    }
    auto &back = _macroItems.emplace_back();
    back.key = key;
    back.value = 1;
    return back.value;
}

auto DefineList::ToString() const -> std::string {
    std::ranges::stable_sort(_macroItems, [](const MacroItem &lhs, const MacroItem &rhs) {
        return lhs.key < rhs.key;
    });

    std::stringstream sbuf;
    for (const MacroItem &item : _macroItems) {
        sbuf << fmt::format("#{}={}", item.key.c_str(), item.value);
    }
    return sbuf.str();
}

static DefineList::MacroItem ParseMacroItem(std::string_view string) {
	DefineList::MacroItem item;
	size_t pos = string.find("=");
    ExceptionAssert(pos != std::string_view::npos);
    item.key = string.substr(0, pos);
    item.value = std::stoi(string.substr(pos+1).data());
    return item;
}

auto DefineList::FromString(std::string source) -> size_t {
    size_t count = 0;
    std::smatch match;
    std::regex pattern("[_a-zA-Z][a-zA-Z0-9_]*=[0-9]+");
    while (std::regex_search(source, match, pattern)) {
		_macroItems.push_back(ParseMacroItem(match.str()));
        ++count;
        source = match.suffix().str();
    }
    return count;
}

auto DefineList::Find(std::string_view key) const -> std::vector<MacroItem>::iterator {
    for (auto iter = _macroItems.begin(); iter != _macroItems.end(); ++iter) {
        if (iter->key == key) {
            return iter;
        }
    }
    return _macroItems.end();
}

}    // namespace vkgfx
