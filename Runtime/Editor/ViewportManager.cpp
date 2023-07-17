#include "ViewportManager.h"
#include "ConsoleViewport.h"
#include "HierarchyViewport.h"
#include "InspectViewport.h"
#include "SceneViewport.h"

void ViewportManager::OnCreate() {
    Add<HierarchyViewport>();
    Add<ConsoleViewport>();
    Add<InspectViewport>();
    Add<SceneViewport>();
}

void ViewportManager::OnDestroy() {
    _viewports.clear();
}

void ViewportManager::OnGUI(GameTimer &gameTimer) {
    for (ViewportItem &item : _viewports) {
        item.pViewport->OnGUI(gameTimer);
    }
}
