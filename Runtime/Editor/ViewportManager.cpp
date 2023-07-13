#include "ViewportManager.h"
#include "ConsoleViewport.h"
#include "HierarchyViewport.h"

void ViewportManager::OnCreate() {
    Add<HierarchyViewport>();
    Add<ConsoleViewport>();
}

void ViewportManager::OnGUI(GameTimer &gameTimer) {
    for (ViewportItem &item : _viewports) {
        item.pViewport->OnGUI(gameTimer);
    }
}
