#include "Application.h"
#include "Foundation/Logger.h"

void Application::Startup() {
	gLogger.Initialize();
	gLogger->StartLogging();
}

void Application::Cleanup() {
	gLogger.Destroy();
}

bool Application::IsDone() const {
	return true;
}

void Application::Update(std::shared_ptr<GameTimer> pGameTimer) {
}

void Application::RenderScene() {
}

Application::~Application() {
}
