#include "MainBar.h"
#include "ImGUI/Libary/imgui.h"
#include "Editor/ViewportManager.h"

void MainBar::OnGUI(GameTimer &gameTimer) {
	if (!ImGui::BeginMainMenuBar())
		return;

	if (ImGui::BeginMenu("Windows")) {
		for (size_t i = 0; i < gViewportManager->GetViewportCount(); ++i) {
			auto *pViewport = gViewportManager->GetViewport(i);
			ImGui::MenuItem(pViewport->GetViewportName().c_str(), nullptr, &pViewport->isShow);
		}
		ImGui::EndMenu();
	}
    if (ImGui::BeginMenu("Tools")) {
		startRenderDocCapture = ImGui::Button("Capture Frame");
		ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}
