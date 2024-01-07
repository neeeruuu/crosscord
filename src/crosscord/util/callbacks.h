#pragma once

#include <vector>

/*
	Callback handler class
	Allows you to register an event, and attach functions to said event.

	TO-DO:
		Implement compiler checks to match the callback's signature
*/

class CCallbackEvent;

class CCallback {
	public:
		bool m_Active;
	
		~CCallback();
	private:
		CCallback(CCallbackEvent* pEvent, void* pFunction, bool bActive);

		void*			m_Function;
		CCallbackEvent* m_Event;

		friend class CCallbackEvent;
};

class CCallbackEvent {
	public:
		template <typename... T>
		inline void Run(T... Args) {
			if (!m_DeletionQueue.empty()) {
				for (CCallback*& pCallback : m_DeletionQueue) {
					std::vector<CCallback*>::iterator Iter = std::find(m_Callbacks.begin(), m_Callbacks.end(), pCallback);
					if (Iter != m_Callbacks.end())
						m_Callbacks.erase(Iter);
				}
				m_DeletionQueue.clear();
			}

			for (CCallback*& pCallback : m_Callbacks)
				if (pCallback->m_Active)
					reinterpret_cast<void(*)(T...)>(pCallback->m_Function)(std::forward<T>(Args)...);

		};

		CCallback* Register(void* pFunction, bool bActive = false);
		void Unregister(CCallback* pCallback);
	private:
		std::vector<CCallback*> m_Callbacks;
		std::vector<CCallback*> m_DeletionQueue;

		friend class CCallback;
};