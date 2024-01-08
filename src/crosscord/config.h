#pragma once

#include "util/macros.h"

class CConfigManager {
	DECLARE_SINGLETON(CConfigManager);
public:
	bool Initialize();

	bool LoadConfig(const char* cName);
	bool SaveConfig(const char* cName);

private:
	CConfigManager() {};
};