#pragma once
#include "modules.h"

#include "util/macros.h"
#include "util/callbacks.h"

class CGLManager : public IModule {
	DECLARE_SINGLETON(CGLManager);
public:
	virtual bool Initialize() override;
	virtual bool Shutdown() override;
	virtual const char* GetName() override { return "GL Manager"; }

	bool Run();
	void* GetWindowHandle() { return m_hWnd; };

	__int64 _m_OriginalWndProc = 0;
private:
	CGLManager() {};

	struct GLFWwindow* m_Window = nullptr;
	void* m_hWnd = nullptr;
	bool m_ShutdownQueued = false;
};

// passes: GLFWWindow*
inline CCallbackEvent* g_CB_GLInit = new CCallbackEvent();
// passes: void
inline CCallbackEvent* g_CB_GLShutdown = new CCallbackEvent();
// passes: void
inline CCallbackEvent* g_CB_GLDraw = new CCallbackEvent();
// passes: HWND hWnd, unsigned int uMsg, unsigned __int64 wParam, __int64 lParam
inline CCallbackEvent* g_CB_WndProc = new CCallbackEvent();