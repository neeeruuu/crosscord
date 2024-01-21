#include "config.h"
#include "globals.h"

#include <filesystem>
#include <fstream>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

INSTANTIATE_SINGLETON(CConfigManager);

bool CConfigManager::Initialize() {
	/*
		determine config path and ensure it exists
	*/
	std::string sConfigPath = fmt::format("{}/config", g_ModulePath);
	std::filesystem::create_directories(sConfigPath);

	memcpy(m_Path, sConfigPath.c_str(), sConfigPath.length());
	m_Path[sConfigPath.length()] = '\0';

	/*
		attempt to load latest config, if it doesn't exist, create one
	*/
	if (!LoadConfig("last"))
		return SaveConfig("last");
	return true;
}

bool CConfigManager::Shutdown() { return true; }

bool CConfigManager::LoadConfig(const char* cName) {
	std::string sConfigPath = fmt::format("{}\\{}.json", m_Path, cName);
	if (!std::filesystem::exists(sConfigPath))
		return false;

	std::ifstream CfgStream(sConfigPath);
	
	nlohmann::json Config = nlohmann::json::parse(CfgStream);
	g_CB_OnConfigLoad->Run(&Config);

	CfgStream.close();
	return true;
}

bool CConfigManager::SaveConfig(const char* cName) {
	std::string sConfigPath = fmt::format("{}\\{}.json", m_Path, cName);

	nlohmann::json Config;
	g_CB_OnConfigSave->Run(&Config);

	std::ofstream ConfigFile;
	ConfigFile.open(sConfigPath);
	if (!ConfigFile.is_open())
		return false;

	ConfigFile << Config.dump(4);
	ConfigFile.close();

	return true;
}