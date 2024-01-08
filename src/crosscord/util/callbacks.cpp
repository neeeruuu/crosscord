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