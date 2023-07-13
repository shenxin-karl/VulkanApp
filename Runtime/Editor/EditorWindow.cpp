#include "EditorWindow.h"
#include "ViewportManager.h"
#include "MainBar.h"

EditorWindow::EditorWindow() = default;
EditorWindow::~EditorWindow() = default;

void EditorWindow::OnCreate() {
    _pMainBar = std::make_unique<MainBar>();
    gViewportManager->OnCreate();
}

void EditorWindow::OnDestroy() {
}

void EditorWindow::OnGUI(GameTimer &gameTimer) {
    _pMainBar->OnGUI(gameTimer);
    gViewportManager->OnGUI(gameTimer);
}
