#pragma once
#include "IViewport.h"
#include <vector>
#include <memory>
#include <typeindex>
#include "Foundation/Exception.h"
#include "Foundation/RuntimeStatic.h"

class ViewportManager {
public:
    void OnCreate();
    void OnDestroy();

    void OnGUI(GameTimer &gameTimer);

    template<typename T, typename... Args>
        requires(std::is_base_of_v<IViewport, T>)
    auto Add(Args &&...args) -> T * {
        ExceptionAssert(Get<T>() == nullptr);
        ViewportItem item = {};
        item.typeIndex = typeid(T);
        item.pViewport = std::make_unique<T>(std::forward<Args>(args)...);
        _viewports.push_back(std::move(item));
        return static_cast<T *>(_viewports.back().pViewport.get());
    }

    template<typename T>
    auto Get() const -> T * {
        for (const ViewportItem &item : _viewports) {
            if (item.typeIndex == typeid(T)) {
                return static_cast<T *>(item.pViewport.get());
            }
        }
        return nullptr;
    }

    template<typename T>
    bool Remove() {
        for (auto iter = _viewports.begin(); iter != _viewports.end(); ++iter) {
            if (iter->typeIndex == typeid(T)) {
                _viewports.erase(iter);
                return true;
            }
        }
        return false;
    }
    auto GetViewportCount() const -> size_t {
	    return _viewports.size();
    }
    auto GetViewport(size_t i) const -> IViewport * {
	    return _viewports[i].pViewport.get();
    }
private:
    struct ViewportItem {
        std::type_index typeIndex = typeid(void);
        std::unique_ptr<IViewport> pViewport = {};
    };
    using ViewportList = std::vector<ViewportItem>;
private:
    ViewportList _viewports;
};


inline RuntimeStatic<ViewportManager> gViewportManager;