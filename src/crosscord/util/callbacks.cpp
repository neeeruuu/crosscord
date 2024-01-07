#include "callbacks.h"

/*
	CCallback implementation
*/
CCallback::CCallback(CCallbackEvent* pEvent, void* pFunction, bool bActive) {
	m_Active = bActive;
	m_Function = pFunction;
	m_Event = pEvent;
}

CCallback::~CCallback() {
	m_Active = false;
	if (m_Event != nullptr)
		m_Event->Unregister(this);
}

/*
	CCallbackEvent implementation
*/
//template <typename... T>
//void CCallbackEvent::Run(T... Arguments) {
//	if (!m_DeletionQueue.empty()) {
//		for (CCallback*& pCallback : m_Callbacks) {
//			std::vector<CCallback*>::iterator Iter = std::find(m_Callbacks.begin(), m_Callbacks.end(), pCallback);
//			if (Iter != m_Callbacks.end())
//				m_Callbacks.erase(Iter);
//		}
//		m_DeletionQueue.clear();
//	}
//
//	for (CCallback*& pCallback : m_Callbacks)
//		if (pCallback->m_Active)
//			reinterpret_cast<void(*)(T...)>(pCallback->m_Function)(std::format<T>(Arguments)...);
//}

CCallback* CCallbackEvent::Register(void* pFunction, bool bActive) {
	CCallback* pCallback = new CCallback(this, pFunction, bActive);
	m_Callbacks.push_back(pCallback);
	return pCallback;
}

void CCallbackEvent::Unregister(CCallback* pCallback) {
	pCallback->m_Active = false;
	pCallback->m_Event = nullptr;
	m_DeletionQueue.push_back(pCallback);
}