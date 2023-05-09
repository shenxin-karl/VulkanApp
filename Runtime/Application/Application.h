#pragma once
#include "IApplication.h"

class Application : public IApplication {
public:
	void Startup() override;
	void Cleanup() override;
	bool IsDone() const override;
	void Update(std::shared_ptr<GameTimer> pGameTimer) override;
	void RenderScene() override;
	~Application() override;
};