#include "util/log.h"

#include "gl.h"
#include "ui.h"
#include "overlay.h"

#include <future>

#include <libloaderapi.h>


#define InitializeComponent(Name, ClassName) \
	LogInfo("Initializing " Name);\
	if (!ClassName::Get()->Initialize()) {\
		ReportError(Name " failed to initialize");\
		return 1;\
	}

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

int main(int, char**) {
	char cModuleName[MAX_PATH];
	GetModuleFileNameA(reinterpret_cast<HMODULE>(GetModuleHandleA(NULL)), cModuleName, MAX_PATH);

	std::string sModuleName(cModuleName);
	std::string sModulePath = sModuleName.substr(0, sModuleName.find_last_of('\\'));

	LogInit("crosscord", fmt::format("{}\\logs\\", sModulePath).c_str());

	LogInfo("Initializing");

	LogInfo("Creating threads");
	std::vector<std::future<bool>> vFutures;
	vFutures.push_back(std::async(std::launch::async, &CGLManager::InitializeAndRun, CGLManager::Get()));
	vFutures.push_back(std::async(std::launch::async, &COverlay::RenderThread, COverlay::Get()));
	vFutures.push_back(std::async(std::launch::async, &COverlay::DetectionThread, COverlay::Get()));

	LogInfo("Initializing components");
	InitializeComponent("UI", CInterface);
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
	vFutures.clear();

	LogInfo("Shutting down");
	CInterface::Get()->Shutdown();
	CGLManager::Get()->Shutdown();

	return 0;
}