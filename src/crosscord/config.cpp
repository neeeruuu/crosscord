#include "config.h"
#include "crosshair.h"

#include "util/log.h"

#include <filesystem>
#include <fstream>

#include <fmt/format.h>

#include <nlohmann/json.hpp>

#define JSONGET(Name, Dest) if (Config.contains(Name)) Config.at(Name).get_to(Dest); else LogWarning("config doesn't contain value {}, using default", Name);

INITIALIZE_SINGLETON(CConfigManager);

bool CConfigManager::Initialize() {
	if (!LoadConfig("config"))
		return SaveConfig("config");
	return true;
}

bool CConfigManager::LoadConfig(const char* cName) {
	std::string sConfigPath = fmt::format("{}\\{}.json", m_Path, cName);
	if (!std::filesystem::exists(sConfigPath))
		return false;

	std::ifstream CfgStream(sConfigPath);
	nlohmann::json Config = nlohmann::json::parse(CfgStream);

	SCrosshairSettings* pSettings = &CCrosshair::Get()->m_Settings;
	JSONGET("enabled", pSettings->m_Enabled);

	JSONGET("type", pSettings->m_Type);
	JSONGET("color", pSettings->m_Color);
	JSONGET("offset", pSettings->m_Offset);

	JSONGET("crosslen", pSettings->m_CrossLength);
	JSONGET("crosswidth", pSettings->m_CrossWidth);
	JSONGET("crossgap", pSettings->m_CrossGap);
	JSONGET("crosst", pSettings->m_CrossTStyle);
	JSONGET("crossdot", pSettings->m_CrossDot);

	JSONGET("arrowlen", pSettings->m_ArrowLength);
	JSONGET("arrowwidth", pSettings->m_ArrowWidth);

	JSONGET("circlerad", pSettings->m_CircleRadius);
	JSONGET("circlehollow", pSettings->m_CircleHollow);

	JSONGET("imagealpha", pSettings->m_ImageAlpha);
	JSONGET("imagesize", pSettings->m_ImageSize);

	CfgStream.close();

	return true;
}

bool CConfigManager::SaveConfig(const char* cName) {
	std::string sConfigPath = fmt::format("{}\\{}.json", m_Path, cName);

	SCrosshairSettings* pSettings = &CCrosshair::Get()->m_Settings;

	nlohmann::json Config;
	Config["enabled"] = pSettings->m_Enabled;

	Config["type"] = pSettings->m_Type;
	Config["color"] = pSettings->m_Color;
	Config["offset"] = pSettings->m_Offset;

	Config["crosslen"] = pSettings->m_CrossLength;
	Config["crosswidth"] = pSettings->m_CrossWidth;
	Config["crossgap"] = pSettings->m_CrossGap;
	Config["crosst"] = pSettings->m_CrossTStyle;
	Config["crossdot"] = pSettings->m_CrossDot;

	Config["arrowlen"] = pSettings->m_ArrowLength;
	Config["arrowwidth"] = pSettings->m_ArrowWidth;
	Config["arrowwidth"] = pSettings->m_ArrowWidth;

	Config["circlerad"] = pSettings->m_CircleRadius;
	Config["circlehollow"] = pSettings->m_CircleHollow;

	Config["imagealpha"] = pSettings->m_ImageAlpha;
	Config["imagesize"] = pSettings->m_ImageSize;

	std::ofstream ConfigFile;
	ConfigFile.open(sConfigPath);
	if (!ConfigFile.is_open())
		return false;

	ConfigFile << Config.dump(4);
	ConfigFile.close();

	return true;
}

void CConfigManager::SetConfigPath(const char* cConfigPath) {
	memcpy(m_Path, cConfigPath, MAX_PATH + 1);
}