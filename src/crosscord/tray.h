#pragma once

#include "util/macros.h"

#include <vector>

#define TRAY_MESSAGE_ID 0x1337

class CTray {
	DECLARE_SINGLETON(CTray);
public:
	bool Initialize();
	void Shutdown();

	void _AddIcon();
	void _ProcessMessage(void* hWnd, unsigned int uMsg, unsigned __int64 wParam, __int64 lParam);
private:
	void ProcessCommand(int iCommandId);
	void DrawContextMenu(void* hWnd);

	CTray() {};

	std::vector<class CCallback*> m_Callbacks;
};