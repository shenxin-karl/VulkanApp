#include "ViewportManager.h"
#include "HierarchyViewport.h"

void ViewportManager::OnCreate() {
    Add<HierarchyViewport>();
}

void ViewportManager::OnGUI(GameTimer &gameTimer) {
    for (ViewportItem &item : _viewports) {
        item.pViewport->OnGUI(gameTimer);
    }
}
