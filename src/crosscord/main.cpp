#include "util/log.h"

#include "gl.h"
#include "ui.h"
#include "tray.h"
#include "config.h"
#include "overlay.h"
#include "globals.h"
#include "crosshair.h"

#include <future>

#include <processenv.h>

#include <consoleapi.h>
#include <libloaderapi.h>

#define InitializeComponent(Name, ClassName) \
	LogInfo("Initializing " Name);\
	if (!ClassName::Get()->Initialize()) {\
		ReportError(Name " failed to initialize");\
		return 1;\
	}

// from: WinBase.h
#define STD_OUTPUT_HANDLE ((DWORD)-11)

// from: WinUser.h
#define MB_OK                       0x00000000L
#define MB_ICONHAND                 0x00000010L
#define MB_ICONERROR                MB_ICONHAND

#define THREAD_WAIT_RATE 1000 / 60

void ReportError(const char* cErrorMessage) {
	LogError("Fatal error ocurred during initialization - {}", cErrorMessage);

	typedef int (*tMessageBoxA) (void* hWnd, const char* lpText, const char* lpCaption, unsigned int uType);

	HMODULE hUser32 = LoadLibraryA("user32.dll");
	if (!hUser32)
		return;

	tMessageBoxA MessageBoxA = reinterpret_cast<tMessageBoxA>(GetProcAddress(hUser32, "MessageBoxA"));
	if (!MessageBoxA)
		return;
	MessageBoxA(0, cErrorMessage, "crosscord failed to load", MB_ICONERROR | MB_OK);
}

int WinMain(HINSTANCE, HINSTANCE, PSTR, int) {
	char cModuleName[MAX_PATH];
	GetModuleFileNameA(reinterpret_cast<HMODULE>(GetModuleHandleA(NULL)), cModuleName, MAX_PATH);
	std::string sModuleName(cModuleName);
	std::string sModulePath = sModuleName.substr(0, sModuleName.find_last_of('\\'));
	memcpy(g_cModulePath, sModulePath.c_str(), sModulePath.length() + 1);

#ifdef _DEBUG
	AllocConsole();

	FILE* pConOut;
	freopen_s(&pConOut, "CONOUT$", "w", stdout);

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(hConsole, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hConsole, dwMode);
#endif
	
	LogInit("crosscord", fmt::format("{}\\logs\\", sModulePath).c_str());

	LogInfo("CrossCord {}", g_cBuild);
	LogInfo("Initializing");

	LogInfo("Initializing components");
	InitializeComponent("UI", CInterface);
	InitializeComponent("Tray", CTray);
	InitializeComponent("Crosshair renderer", CCrosshair);
	InitializeComponent("Config", CConfigManager);

	LogInfo("Creating threads");
	std::vector<std::future<bool>> vFutures;
	vFutures.push_back(std::async(std::launch::async, &CGLManager::InitializeAndRun, CGLManager::Get()));
	vFutures.push_back(std::async(std::launch::async, &COverlay::RenderThread, COverlay::Get()));
	vFutures.push_back(std::async(std::launch::async, &COverlay::DetectionThread, COverlay::Get()));

	LogInfo("Ready");

	std::chrono::milliseconds ThreadWaitTime(THREAD_WAIT_RATE);
	bool bThreadHalted = false;
	while (!bThreadHalted) {
		for (std::future<bool>& Future : vFutures) {
			std::future_status Status = Future.wait_for(ThreadWaitTime);

			if (Status != std::future_status::ready)
				continue;

			if (!Future.get())
				LogWarning("Thread stopped with error");

			bThreadHalted = true;
			break;
		}
	}

	LogInfo("Shutting down");
	CInterface::Get()->Shutdown();
	CTray::Get()->Shutdown();
	COverlay::Get()->Shutdown();
	CGLManager::Get()->Shutdown();

	vFutures.clear();

	return 0;
}