#include "overlay.h"

#include "util/log.h"

#include <fmt/format.h>

#include <wtypes.h>
#include <WinBase.h>

#include <memoryapi.h>
#include <handleapi.h>

#include <thread>
#include <chrono>

INITIALIZE_SINGLETON(COverlay);

bool COverlay::SetPixel(SPixel* pPixel) {
	if (!m_FrameInfo || pPixel->x > m_FrameInfo->m_Width || pPixel->y > m_FrameInfo->m_Height)
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
			g_CB_OverlayDraw->Run(m_FrameInfo);
			m_FrameInfo->m_Frame++;
		}
		m_LastFrameId = m_FrameInfo->m_Frame;
	}
	return true;
}

bool COverlay::DetectionThread() {
	while (!m_ShutdownQueued) {
		std::this_thread::sleep_for(std::chrono::milliseconds(DETECTION_TIMING));

		HWND hWnd = GetTopWindow(GetDesktopWindow());
		DWORD dwWndPID = 0;
		bool bHasMap = false;

		while (hWnd && !bHasMap) {
			GetWindowThreadProcessId(hWnd, &dwWndPID);

			if (dwWndPID && m_TargetProcessId != dwWndPID) {
				if (AdquireMap(dwWndPID))
					bHasMap = true;
			}

			hWnd = GetWindow(hWnd, GW_HWNDNEXT);
		}

		if (!hWnd || !bHasMap || !dwWndPID)
			continue;

		GetWindowTextA(hWnd, m_TargetWindowName, sizeof(m_TargetWindowName));
		m_TargetProcessId = dwWndPID;

		std::this_thread::sleep_for(std::chrono::milliseconds(DETECTION_TIMING * 5));
	}
	return true;
}

bool COverlay::AdquireMap(int iProcessId) {	
	std::string sMapName = fmt::format("DiscordOverlay_Framebuffer_Memory_{}", iProcessId);

	void* pMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, false, sMapName.c_str());
	if (!pMapFile)
		return false;

	void* pMapView;
	pMapView = MapViewOfFile(pMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (!pMapView) {
		CloseHandle(pMapFile);
		return false;
	}

	if (m_MapFile) CloseHandle(m_MapFile);
	if (m_MapView) UnmapViewOfFile(m_MapView);
	m_MapFile = pMapFile;
	m_MapView = pMapView;

	m_FrameInfo = reinterpret_cast<SFrameInfo*>(m_MapView);
	m_LastFrameId = m_FrameInfo->m_Frame;

	LogVerbose("MapView obtained: {}", reinterpret_cast<void*>(m_MapView));

	m_FrameInfo->m_Frame++; 

	return true;
}