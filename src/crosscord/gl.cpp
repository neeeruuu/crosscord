#include "gl.h"

#include "util/log.h"

#include <functional>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

INITIALIZE_SINGLETON(CGLManager);

bool CGLManager::InitializeAndRun() {
	if (!glfwInit()) {
		LogError("failed to initialize glfw");
		return false;
	}

	const char* cGLSLVer = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_VISIBLE, 0);

	m_Window = glfwCreateWindow(1, 1, "crosscord", nullptr, nullptr);
	if (!m_Window) {
		LogError("failed to create glfw window");
		return false;
	}

	m_hWnd = glfwGetWin32Window(m_Window);
	
	auto WndProcCallback = +[](HWND hWnd, unsigned int uMsg, unsigned __int64 wParam, __int64 lParam) -> __int64 {
		CGLManager* pGLMan = CGLManager::Get();
		g_CB_WndProc->Run(hWnd, uMsg, wParam, lParam);
		return CallWindowProcA(reinterpret_cast<WNDPROC>(pGLMan->_m_OriginalWndProc), hWnd, uMsg, wParam, lParam);
	};

	_m_OriginalWndProc = SetWindowLongPtrA(reinterpret_cast<HWND>(m_hWnd), GWLP_WNDPROC, reinterpret_cast<long long>(WndProcCallback));

	glfwMakeContextCurrent(m_Window);
	glfwSwapInterval(1);

	g_CB_GLInit->Run(m_Window);

	RenderLoop();

	return true;
}

void CGLManager::Shutdown() { m_ShutdownQueued = true; }

void* CGLManager::GetWindowHandle() { return m_hWnd; }

void CGLManager::RenderLoop() {
	while (!glfwWindowShouldClose(m_Window) && !m_ShutdownQueued) {
		glfwPollEvents();
		g_CB_GLRender->Run();
	}

	g_CB_GLShutdown->Run();

	glfwSetWindowShouldClose(m_Window, 1);
	glfwDestroyWindow(m_Window);
	glfwTerminate();

	m_Window = nullptr;
}