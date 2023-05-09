#include "IApplication.h"
#include "Foundation/GameTimer.h"

int RunApplication(IApplication &application) {
	std::shared_ptr<GameTimer> pGameTimer = std::make_shared<GameTimer>();
	try {
		application.Startup();
		while (!application.IsDone()) {
			pGameTimer->StartNewFrame();
			application.Update(pGameTimer);
			application.RenderScene();
		}
		application.Cleanup();
	} catch (const std::exception &exception) {
		
	}
}
