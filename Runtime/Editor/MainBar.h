#pragma once

class GameTimer;
class MainBar {
public:
	void OnGUI(GameTimer &gameTimer);
public:
	bool startRenderDocCapture = false;
};
