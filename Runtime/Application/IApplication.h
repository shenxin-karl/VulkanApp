#pragma once
#include "Foundation/NonCopyable.h"
#include <memory>

class GameTimer;
class IApplication : public NonCopyable {
public:
	virtual void Startup() = 0;
    virtual void Cleanup() = 0;
    virtual bool IsDone() const = 0;
    virtual bool IsPause() const = 0;
    virtual void PollEvents() = 0;
    virtual void Update(std::shared_ptr<GameTimer> pGameTimer) = 0;
    virtual void RenderScene(std::shared_ptr<GameTimer> pGameTimer) = 0;
    virtual ~IApplication() = default;
};

int RunApplication(IApplication &application);