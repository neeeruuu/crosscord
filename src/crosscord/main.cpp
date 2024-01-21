#include "util/log.h"

#include "modules.h"
#include "defines.h"
#include "globals.h"

#include <synchapi.h>
#include <libloaderapi.h>
#include <errhandlingapi.h>

// from: WinUser.h
#define MB_OK                       0x00000000L
#define MB_ICONHAND                 0x00000010L
#define MB_ICONERROR                MB_ICONHAND

// from: winerror.h
#define ERROR_ALREADY_EXISTS		183L

#ifdef _DEBUG
	#include <consoleapi.h>
	#include <processenv.h>

	// from: WinBase.h
	#define STD_OUTPUT_HANDLE ((DWORD)-11)

	void CreateConsole() {
		/*
			alloc console and open stdout
		*/
		AllocConsole();
		FILE* pConOut;
		freopen_s(&pConOut, "CONOUT$", "w", stdout);

		/*
			enable ansi escape sequences
		*/
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD dwMode = 0;
		GetConsoleMode(hConsole, &dwMode);
		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(hConsole, dwMode);
	}
#else
	#define CreateConsole()
#endif

int ReportError(const char* cErrorMessage) {
	LogError("Fatal error ocurred during initialization - {}", cErrorMessage);

	/*
		get address and call MessageBoxA
	*/
	typedef int (*tMessageBoxA) (void* hWnd, const char* lpText, const char* lpCaption, unsigned int uType);

	HMODULE hUser32 = LoadLibraryA("user32.dll");
	if (!hUser32)
		return 1;

	tMessageBoxA MessageBoxA = reinterpret_cast<tMessageBoxA>(GetProcAddress(hUser32, "MessageBoxA"));
	if (!MessageBoxA)
		return 1;
	MessageBoxA(0, cErrorMessage, "crosscord failed to load", MB_ICONERROR | MB_OK);

	return 1;
}

int WinMain(void*, void*, char*, int) {
	/*
		prevent program from running twice
	*/
	HANDLE hMutex = CreateMutex(NULL, TRUE, "CROSSCORD_MTX");
	if (GetLastError() == ERROR_ALREADY_EXISTS || !hMutex)
		return ReportError("Crosscord is already running!");

	/*
		get program's path
	*/
	char cModuleName[MAX_PATH];
	GetModuleFileNameA(reinterpret_cast<HMODULE>(GetModuleHandleA(NULL)), cModuleName, MAX_PATH);
	std::string sModuleName(cModuleName);
	std::string sModulePath = sModuleName.substr(0, sModuleName.find_last_of('\\'));

	memcpy(g_ModulePath, sModulePath.c_str(), sModulePath.length());
	g_ModulePath[sModulePath.length()] = '\0';

	/*
		alloc and configure console on dbg builds
	*/
	CreateConsole();

	/*
		initialize log and report version
	*/
	LogInit("crosscord", fmt::format("{}\\logs\\", sModulePath).c_str());
	LogInfo("CrossCord" CROSSCORD_VER);

	/*
		initialize everything
	*/
	if (!CModuleManager::Get()->InitializeAll())
		return ReportError("Error ocurred during initialization");

	/*
		wait for module's futures
	*/
	CModuleManager::Get()->AwaitFutures();

	/*
		shutdown everything
	*/
	CModuleManager::Get()->ShutdownAll();

	return 0;
}