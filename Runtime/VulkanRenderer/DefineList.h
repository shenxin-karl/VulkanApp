#pragma once
#include <optional>
#include <string>
#include <vector>
#include "Foundation/NonCopyable.h"

namespace vkgfx {

class DefineList : public NonCopyable {
public:
    struct MacroItem {
        std::string key;
        int value;
    };
    using iterator = std::vector<MacroItem>::iterator;
    using const_iterator = std::vector<MacroItem>::const_iterator;
public:
    void Set(std::string_view key, int value = 1);
    auto Get(std::string_view key) const -> std::optional<int>;
    bool Remove(std::string_view key);
    void Clear();
    auto GetCount() const -> size_t;
    auto operator[](std::string_view key) -> int &;
    auto ToString() const -> std::string;
    auto FromString(std::string source) -> size_t;

    auto begin() const -> const_iterator {
        return _macroItems.begin();
    }
    auto end() const -> const_iterator {
        return _macroItems.end();
    }
private:
    auto Find(std::string_view key) const -> iterator;
private:
    mutable std::vector<MacroItem> _macroItems;
};

}    // namespace vkgfx
