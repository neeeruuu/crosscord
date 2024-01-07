#include "util/log.h"

#include <libloaderapi.h>

// from: WinUser.h
#define MB_OK                       0x00000000L
#define MB_ICONHAND                 0x00000010L
#define MB_ICONERROR                MB_ICONHAND

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

	return 0;
}