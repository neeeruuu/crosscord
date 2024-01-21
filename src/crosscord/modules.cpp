#include "modules.h"
#include "defines.h"

#include <chrono>

#include "util/log.h"

INSTANTIATE_SINGLETON(CModuleManager);

void CModuleManager::_Register(IModule* pModule) {
	m_Mutex.lock();
	m_Modules.push_back(pModule);
	m_Mutex.unlock();
}

void CModuleManager::_Unregister(IModule* pModule) {
	m_Mutex.lock();
	std::vector<IModule*>::iterator Iter = std::find(m_Modules.begin(), m_Modules.end(), pModule);
	if (Iter != m_Modules.end())
		m_Modules.erase(Iter);
	m_Mutex.unlock();
}

bool CModuleManager::InitializeAll() {
	LogInfo("Initializing");

	m_Mutex.lock();
	for (IModule* Mod : m_Modules) {
		LogInfo("Starting module: {}", Mod->GetName());
		bool bResult = Mod->Initialize();
		if (!bResult) {
			LogError("Module {} failed to initialize", Mod->GetName());
			m_Mutex.unlock();
			return false;
		}
	}
	LogInfo("All modules started");
	m_Initialized = true;
	m_Mutex.unlock();
	return true;
}

bool CModuleManager::ShutdownAll() {
	bool bHadCleanShutdown = true;
	if (!m_Initialized)
		return bHadCleanShutdown;

	LogInfo("Shutting down");

	m_Mutex.lock();
	for (IModule* Mod : m_Modules) {
		LogInfo("Stopping module: {}", Mod->GetName());
		bool bResult = Mod->Shutdown();
		if (!bResult) {
			LogWarning("Module {} failed to shutdown", Mod->GetName());
			bHadCleanShutdown = false;
		}
	}
	LogInfo("All modules stopped");
	m_Initialized = false;
	m_Mutex.unlock();
	return bHadCleanShutdown;
}

void CModuleManager::RegisterFuture(std::future<bool> Future) {
	if (m_Initialized)
		return LogError("Attempt to register future after initialization");
	m_Futures.push_back(std::move(Future)); 
}

void CModuleManager::AwaitFutures() {
	if (!m_Initialized)
		return LogError("Attempt to await futures before initialization");

	std::chrono::milliseconds FutureInterval(FUTURE_WAIT_INTERVAL);
	bool bThreadHalted = false;
	
	size_t iTotalFutureCount = m_Futures.size();

	/*
		wait for at least one thread to stop
	*/
	while (!bThreadHalted && m_Initialized) {
		for (auto Future = m_Futures.rbegin(); Future != m_Futures.rend();) {
			std::future_status Status = Future->wait_for(FutureInterval);

			if (Status != std::future_status::ready) {
				++Future;
				continue;
			}

			if (!Future->get())
				LogWarning("Module futured stopped with error");

			Future = std::vector<std::future<bool>>::reverse_iterator(m_Futures.erase(--Future.base()));

			bThreadHalted = true;
			break;
		}
	}

	/*
		wait for other threads to stop or timeout
	*/
	auto StartTime = std::chrono::system_clock::now().time_since_epoch();

	size_t iFutureCount = m_Futures.size();
	bool bTimedOut = false;
	while (iFutureCount > 0 || bTimedOut) {
		/*
			remove futures as they finished
		*/
		for (auto Future = m_Futures.rbegin(); Future != m_Futures.rend();) {
			std::future_status Status = Future->wait_for(FutureInterval);

			if (Status != std::future_status::ready) {
				++Future;
				continue;
			}

			if (!Future->get())
				LogWarning("Module futured stopped with error");

			Future = std::vector<std::future<bool>>::reverse_iterator(m_Futures.erase(--Future.base()));
		}

		/*
			check if time elapsed is greater than timeout
		*/
		auto CurrTime = std::chrono::system_clock::now().time_since_epoch();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(CurrTime - StartTime).count() > FUTURE_TIMEOUT) {
			bTimedOut = true;
			break;
		}

		iFutureCount = m_Futures.size();
		std::this_thread::sleep_for(FutureInterval);
	}

	if (!bTimedOut)
		LogInfo("All futures stopped");
	else
		LogWarning("Not all of the futures stopped in time ({}/{})", m_Futures.size(), iTotalFutureCount);
}