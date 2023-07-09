#include "IApplication.h"
#include "Foundation/GameTimer.h"
#include "Foundation/Logger.h"

int RunApplication(IApplication &application) {
	std::shared_ptr<GameTimer> pGameTimer = std::make_shared<GameTimer>();
	stdchrono::microseconds sleepTime = stdchrono::microseconds(50);
	try {
		application.Startup();
		while (!application.IsDone()) {
			application.PollEvents();
			if (application.IsPause()) {
				pGameTimer->Stop();
				std::this_thread::sleep_for(sleepTime);
				continue;
			}
			if (pGameTimer->IsStopped()) {
				pGameTimer->Start();
			}

			pGameTimer->StartNewFrame();
			application.Update(pGameTimer);
			application.RenderScene(pGameTimer);
		}
		application.Cleanup();
	} catch (const std::exception &exception) {
		Logger::Error("Unresolved exception {}", exception.what());
		return -1;
	}
	return 0;
}
