#include "overlay.h"

#include "util/log.h"

#include <fmt/format.h>

#include <wtypes.h>
#include <WinBase.h>

#include <memoryapi.h>
#include <handleapi.h>

#include <thread>
#include <chrono>

#define RENDER_TIMING 1000 / 120
#define DETECTION_TIMING 500

INITIALIZE_SINGLETON(COverlay);

bool COverlay::SetPixel(SPixel* pPixel) {
	if (!m_FrameInfo)
		return;

	if (pPixel->x > m_FrameInfo->m_Width || pPixel->y > m_FrameInfo->m_Height)
		return false;
	
	SColor* pPixelBuffer = reinterpret_cast<SColor*>(&m_FrameInfo->m_Pixels[0]);
	pPixelBuffer[pPixel->x * m_FrameInfo->m_Width + pPixel->y] = pPixel->col;

	return true;
}

bool COverlay::RenderThread() {
	while (!m_ShutdownQueued) {
		std::this_thread::sleep_for(std::chrono::milliseconds(RENDER_TIMING));

		if (!m_FrameInfo)
			continue;

		if (m_FrameInfo->m_Frame != m_LastFrameId) {
			// draw here
			m_FrameInfo->m_Frame++;
		}
		m_LastFrameId = m_FrameInfo->m_Frame;
	}
	return true;
}

bool COverlay::DetectionThread() {
	while (!m_ShutdownQueued) {
		std::this_thread::sleep_for(std::chrono::milliseconds(DETECTION_TIMING));

		HWND hWnd = GetForegroundWindow();
		if (!hWnd)
			continue;

		RECT WndRect;
		if (!GetWindowRect(hWnd, &WndRect))
			continue;

		DWORD dwStyles = static_cast<DWORD>(GetWindowLongPtrA(hWnd, GWL_STYLE));
		if ((dwStyles & WS_MAXIMIZE) != 0 && (dwStyles & WS_BORDER) != 0)
			continue;

		HMONITOR Monitor = MonitorFromRect(&WndRect, MONITOR_DEFAULTTONEAREST);
		if (!Monitor)
			continue;

		MONITORINFO MonInfo = { 0 };
		MonInfo.cbSize = sizeof(MonInfo);
		if (!GetMonitorInfo(Monitor, &MonInfo))
			continue;

		if (WndRect.left != MonInfo.rcMonitor.left ||
			WndRect.right != MonInfo.rcMonitor.right ||
			WndRect.bottom != MonInfo.rcMonitor.bottom ||
			WndRect.top != MonInfo.rcMonitor.top)
			continue;

		DWORD dwWndPID;
		GetWindowThreadProcessId(hWnd, &dwWndPID);

		if (!dwWndPID || m_TargetProcessId == dwWndPID)
			continue;

		if (!AdquireMap(dwWndPID))
			continue;

		GetWindowTextA(hWnd, m_TargetWindowName, sizeof(m_TargetWindowName));
		m_TargetProcessId = dwWndPID;
	}
	return true;
}

bool COverlay::AdquireMap(int iProcessId) {
	if (m_MapFile) CloseHandle(m_MapFile);
	if (m_MapView) UnmapViewOfFile(m_MapView);
	
	LogVerbose("Getting Framebuffer pointer for PID: {}", iProcessId);

	std::string sMapName = fmt::format("DiscordOverlay_Framebuffer_Memory_{}", iProcessId);

	m_MapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, false, sMapName.c_str());
	if (!m_MapFile) {
		LogError("Couldn't obtain framebuffer mapping for PID {}", iProcessId);
		return false;
	}

	m_MapView = MapViewOfFile(m_MapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (!m_MapView) {
		CloseHandle(m_MapFile);
		LogError("Couldn't obtain framebuffer view address for PID {}", iProcessId);
		return false;
	}

	m_FrameInfo = reinterpret_cast<SFrameInfo*>(m_MapView);
	m_LastFrameId = m_FrameInfo->m_Frame;

	LogVerbose("MapView obtained: {}", reinterpret_cast<void*>(m_MapView));
	return true;
}