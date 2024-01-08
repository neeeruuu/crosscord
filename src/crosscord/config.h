#pragma once

#include "util/macros.h"

// from: minwindef.h
#define MAX_PATH 260

class CConfigManager {
	DECLARE_SINGLETON(CConfigManager);
public:
	bool Initialize();

	bool LoadConfig(const char* cName);
	bool SaveConfig(const char* cName);

	void SetConfigPath(const char* m_Path);
private:
	char m_Path[MAX_PATH + 1] = { 0 };
	CConfigManager() {};
};