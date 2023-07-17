#pragma once
#include "IViewport.h"

class InspectViewport : public IViewport {
public:
	InspectViewport();
	void OnGUI(GameTimer &gameTimer) override;
};
