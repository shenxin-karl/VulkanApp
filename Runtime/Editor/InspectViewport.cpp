#include "InspectViewport.h"
#include "ImGUI/Libary/imgui.h"

InspectViewport::InspectViewport() : IViewport("Inspect") {
}

void InspectViewport::OnGUI(GameTimer &gameTimer) {
	if (!isShow) {
		return;
	}
	ImGui::Begin("Inspect", &isShow);

	ImGui::End();
}
