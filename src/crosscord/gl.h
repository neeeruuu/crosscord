#pragma once

#include "util/callbacks.h"
#include "util/macros.h"

class CGLManager {
	DECLARE_SINGLETON(CGLManager)
public:
	bool InitializeAndRun();
	void Shutdown();

	void* GetWindowHandle();

	__int64 _m_OriginalWndProc = 0;
private:
	void RenderLoop();

	struct GLFWwindow* m_Window = nullptr;
	void* m_hWnd = nullptr;
	bool m_ShutdownQueued = false;

	CGLManager() {};
};

inline CCallbackEvent* g_CB_GLInit = new CCallbackEvent();
inline CCallbackEvent* g_CB_GLShutdown = new CCallbackEvent();
inline CCallbackEvent* g_CB_GLRender = new CCallbackEvent();

inline CCallbackEvent* g_CB_WndProc = new CCallbackEvent();