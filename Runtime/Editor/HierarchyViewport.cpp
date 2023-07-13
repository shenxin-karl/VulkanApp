#include "HierarchyViewport.h"
#include "ImGUI/Libary/imgui.h"

static constexpr std::string_view sViewportName = "Hierarchy";

HierarchyViewport::HierarchyViewport() : IViewport(sViewportName.data()) {

}

void HierarchyViewport::OnGUI(GameTimer &gameTimer) {
	if (!isShow) {
		return;
	}
	ImGui::Begin(sViewportName.data(), &isShow);

	ImGui::End();
}
