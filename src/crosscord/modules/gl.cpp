#include "gl.h"

#include "util/log.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

INSTANTIATE_SINGLETON(CGLManager);

bool CGLManager::Initialize() {
	CModuleManager::Get()->RegisterFuture(std::move(std::async(std::launch::async, &CGLManager::Run, this)));
	return true;
}

bool CGLManager::Shutdown() {
	m_ShutdownQueued = true;
	return true;
}

bool CGLManager::Run() {
	/*
		glfw initialization
	*/
	if (!glfwInit()) {
		LogError("GLFW failed to initialize");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_VISIBLE, 0);

	m_Window = glfwCreateWindow(1, 1, "crosscord", nullptr, nullptr);
	if (!m_Window) {
		LogError("Failed to create glfw window");
		return false;
	}
	m_hWnd = glfwGetWin32Window(m_Window);

	glfwMakeContextCurrent(m_Window);
	glfwSwapInterval(1);
	g_CB_GLInit->Run(m_Window);

	/*
		wndproc hook
	*/
	auto WndProcCallback = +[](HWND hWnd, unsigned int uMsg, unsigned __int64 wParam, __int64 lParam) -> __int64 {
		CGLManager* pGLMan = CGLManager::Get();
		g_CB_WndProc->Run(hWnd, uMsg, wParam, lParam);
		return CallWindowProcA(reinterpret_cast<WNDPROC>(pGLMan->_m_OriginalWndProc), hWnd, uMsg, wParam, lParam);
	};
	_m_OriginalWndProc = SetWindowLongPtrA(reinterpret_cast<HWND>(m_hWnd), GWLP_WNDPROC, reinterpret_cast<long long>(WndProcCallback));

	/*
		draw loop
	*/
	while (!glfwWindowShouldClose(m_Window) && !m_ShutdownQueued) {
		glfwPollEvents();
		g_CB_GLDraw->Run();
	}

	/*
		cleanup
	*/
	g_CB_GLShutdown->Run();

	glfwSetWindowShouldClose(m_Window, 1);
	glfwDestroyWindow(m_Window);
	glfwTerminate();

	return true;
}