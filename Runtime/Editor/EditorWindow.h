#pragma once
#include <memory>
#include "Foundation/RuntimeStatic.h"

class GameTimer;
class MainBar;

class EditorWindow {
public:
	EditorWindow();
	~EditorWindow();
	void OnCreate();
	void OnDestroy();
	void OnGUI(GameTimer &gameTimer);
private:
	std::unique_ptr<MainBar> _pMainBar;
};

inline RuntimeStatic<EditorWindow> gEditorWindow;