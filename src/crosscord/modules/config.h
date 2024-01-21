#pragma once

#include "modules.h"

#include "util/macros.h"
#include "util/callbacks.h"

// from: minwindef.h
#define MAX_PATH 260

class CConfigManager : public IModule {
	DECLARE_SINGLETON(CConfigManager);
public:
	virtual bool Initialize() override;
	virtual bool Shutdown() override;

	virtual const char* GetName() override { return "Config Manager"; }

	bool LoadConfig(const char* cName);
	bool SaveConfig(const char* cName);
private:
	char m_Path[MAX_PATH + 1] = { 0 };
	CConfigManager() {};
};

// passes: nlohmann::json*
inline CCallbackEvent* g_CB_OnConfigLoad = new CCallbackEvent();
// passes: nlohmann::json*
inline CCallbackEvent* g_CB_OnConfigSave = new CCallbackEvent();