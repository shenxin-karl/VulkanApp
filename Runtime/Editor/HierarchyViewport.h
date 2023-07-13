#pragma once
#include "IViewport.h"

class HierarchyViewport : public IViewport {
public:
	HierarchyViewport();
	void OnGUI(GameTimer &gameTimer) override;
	
};
