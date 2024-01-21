#include "overlay.h"

#include "util/log.h"

#include <wtypes.h>
#include <WinBase.h>

#include <thread>
#include <chrono>

#include <TlHelp32.h>

#include <fmt/format.h>

INSTANTIATE_SINGLETON(COverlay);

/*
	--------- module implementation ---------
*/
bool COverlay::Initialize() {
	CModuleManager::Get()->RegisterFuture(std::move(std::async(std::launch::async, &COverlay::Render, this)));
	CModuleManager::Get()->RegisterFuture(std::move(std::async(std::launch::async, &COverlay::Detect, this)));
	return true;
}

bool COverlay::Shutdown() {
	m_ShouldShutdown = true;
	return true;
}

/* 
	--------- callback functions ---------
*/
bool COverlay::Render() {
	auto LastDraw = std::chrono::system_clock::now().time_since_epoch();
	auto RenderInterval = std::chrono::milliseconds(RENDER_INTERVAL);

	bool bShouldRedraw = false;

	/*
		detect if discord drew a new frame, or if enough time passed
		then invoke draw callback and update framecount to force redraw
	*/
	while (!m_ShouldShutdown) {
		if (!m_FrameInfo)
			continue;

		auto CurrTime = std::chrono::system_clock::now().time_since_epoch();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(CurrTime - LastDraw).count() > FORCE_RENDER_INTERVAL)
			bShouldRedraw = true;

		if (m_FrameInfo->m_Frame != m_LastFrameId || bShouldRedraw) {
			LastDraw = CurrTime;
			bShouldRedraw = false;

			g_CB_OverlayDraw->Run(m_FrameInfo);
			m_FrameInfo->m_Frame++;
			m_LastFrameId = m_FrameInfo->m_Frame;
		}

		std::this_thread::sleep_for(RenderInterval);
	}

	return true;
}

bool COverlay::Detect() {
	auto Interval = std::chrono::milliseconds(DETECTION_INTERVAL);
	while (!m_ShouldShutdown) {
		std::this_thread::sleep_for(Interval);

		/*
			iterate every window until one that has discord's overlay is found
		*/
		unsigned long dwWndPID = 0;
		HWND hWnd = GetTopWindow(GetDesktopWindow());
		bool bHasMap = false;
		while (hWnd && !bHasMap) {
			GetWindowThreadProcessId(hWnd, &dwWndPID);
			if (dwWndPID && m_TargetProcessId != dwWndPID && AdquireMap(static_cast<int>(dwWndPID)))
				bHasMap = true;
			hWnd = GetWindow(hWnd, GW_HWNDNEXT);
		}
		if (!bHasMap) continue;

		m_TargetProcessId = dwWndPID;
		GetProcessName(dwWndPID);

		LogInfo("Detected overlay for process: {} - PID: {}", m_TargetProcessName, m_TargetProcessId);
	}
	return true;
}

/*
	--------- overlay functions ---------
*/
bool COverlay::AdquireMap(unsigned long dwPID) {
	std::string sMapName = fmt::format("DiscordOverlay_Framebuffer_Memory_{}", dwPID);
	void* pMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, false, sMapName.c_str());
	if (!pMapFile)
		return false;

	void* pMapView = MapViewOfFile(pMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (!pMapView) {
		CloseHandle(pMapFile);
		return false;
	}

	if (reinterpret_cast<SFrameInfo*>(pMapView)->m_Frame < 1) {
		CloseHandle(pMapFile);
		UnmapViewOfFile(pMapView);
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

void COverlay::GetProcessName(unsigned long dwPID) {
	HANDLE hTH32Snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hTH32Snap == INVALID_HANDLE_VALUE)
		return;

	PROCESSENTRY32 ProcEntry32;
	ProcEntry32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hTH32Snap, &ProcEntry32)) {
		CloseHandle(hTH32Snap);
		return;
	}

	bool bHadProcessName = false;
	do {
		if (ProcEntry32.th32ProcessID == dwPID) {
			memcpy(m_TargetProcessName, &ProcEntry32.szExeFile, sizeof(m_TargetProcessName));
			m_TargetProcessName[sizeof(m_TargetProcessName) - 1] = '\0';
			bHadProcessName = true;
			break;
		}
	} while (Process32Next(hTH32Snap, &ProcEntry32) && !bHadProcessName);

	CloseHandle(hTH32Snap);

	if (!bHadProcessName)
		memcpy(m_TargetProcessName, "unknown", 8);
}