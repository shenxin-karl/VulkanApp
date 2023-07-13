#pragma once
#include "Foundation/NonCopyable.h"
#include "Foundation/GameTimer.h"

class IViewport : public NonCopyable {
public:
    explicit IViewport(std::string &&viewportName) : _viewportName(std::move(viewportName)) {
    }
    virtual ~IViewport() {
    }
    virtual void OnGUI(GameTimer &gameTimer) = 0;
    auto GetViewportName() const -> const std::string & {
	    return _viewportName;
    }
public:
    bool isShow = true;
protected:
    std::string _viewportName;
};