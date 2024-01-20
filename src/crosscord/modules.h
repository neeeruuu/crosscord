#pragma once

#include "util/macros.h"

#include <future>
#include <vector>
#include <mutex>

class CModuleManager {
	DECLARE_SINGLETON(CModuleManager);
public:
	void _Register(class IModule* pModule);
	void _Unregister(class IModule* pModule);

	bool InitializeAll();
	bool ShutdownAll();

	void RegisterFuture(std::future<bool> Future);
	void AwaitFutures();
private:
	bool m_Initialized = false;

	CModuleManager() {};
	std::vector<class IModule*> m_Modules;
	std::vector<std::future<bool>> m_Futures;
	std::mutex m_Mutex;
};

class IModule {
public:
	virtual bool Initialize() = 0;
	virtual bool Shutdown() = 0;

	virtual const char* GetName() = 0;

	IModule() { CModuleManager::Get()->_Register(this); }
	~IModule() { CModuleManager::Get()->_Unregister(this); }
};
