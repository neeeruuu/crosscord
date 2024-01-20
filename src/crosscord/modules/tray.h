#pragma once

#include "modules.h"
#include "globals.h"

#include "util/macros.h"

#include <vector>

class CTray : public IModule {
	DECLARE_SINGLETON(CTray);
public:
	virtual bool Initialize() override;
	virtual bool Shutdown() override;
	virtual const char* GetName() override { return "Tray"; }

	void _AddIcon();
	void _ProcessMessage(void* hWnd, unsigned int uMsg, unsigned __int64 wParam, __int64 lParam);
private:
	void ProcessCommand(unsigned __int64 iCommandId);
	void DrawContextMenu(void* hWnd);

	std::vector<class CCallback*> m_Callbacks;
	CTray() {};
};