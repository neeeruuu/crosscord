#include "tray.h"
#include "gl.h"
#include "ui.h"

#include <wtypes.h>
#include <shellapi.h>

/*
	TO-DO:
		add icon
*/

INITIALIZE_SINGLETON(CTray);

bool CTray::Initialize() {
	m_Callbacks.push_back(g_CB_GLInit->Register([](void* pWindow) { CTray::Get()->_AddIcon(); }, true));
	m_Callbacks.push_back(g_CB_WndProc->Register([](void* hWnd, unsigned int uMsg, unsigned __int64 wParam, __int64 lParam) { CTray::Get()->_ProcessMessage(hWnd, uMsg, wParam, lParam); }, true));
	return true;
}

void CTray::Shutdown() {
	for (CCallback* Callback : m_Callbacks) { delete Callback; }
	m_Callbacks.clear();
}

void CTray::_AddIcon() {
	void* hWnd = CGLManager::Get()->GetWindowHandle();

	NOTIFYICONDATA IconData;
	IconData.cbSize = sizeof(IconData);
	IconData.hWnd = reinterpret_cast<HWND>(hWnd);
	IconData.uFlags = NIF_TIP | NIF_MESSAGE;
	IconData.uCallbackMessage = TRAY_MESSAGE_ID;

	memcpy(IconData.szTip, "CrossCord", 128);

	Shell_NotifyIconA(NIM_ADD, &IconData);
}

void CTray::_ProcessMessage(void* hWnd, unsigned int uMsg, unsigned __int64 wParam, __int64 lParam) {
	if (uMsg != TRAY_MESSAGE_ID)
		return;

	switch (lParam) {
		case WM_LBUTTONDBLCLK:
			CInterface::Get()->QueueBringToFront();
			break;
		case WM_COMMAND:
			ProcessCommand(wParam);
			break;
		case WM_RBUTTONUP:
			DrawContextMenu(hWnd);
			break;
	}
}

void CTray::ProcessCommand(int iCommandId) {
	switch (iCommandId) {
	case 1:
		CGLManager::Get()->Shutdown();
		break;
	case 2:
		CInterface::Get()->QueueBringToFront();
		break;
	}
}

void CTray::DrawContextMenu(void* hWnd) {
	POINT Cursor;
	GetCursorPos(&Cursor);

	HMENU hPopup = CreatePopupMenu();
	InsertMenuA(hPopup, 0, MF_BYPOSITION | MF_STRING, 1, "Close");
	InsertMenuA(hPopup, 0, MF_BYPOSITION | MF_STRING, 2, "Open UI");

	int iSelectedOption = TrackPopupMenu(	hPopup,
											TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN | TPM_RETURNCMD,
											Cursor.x, Cursor.y, 0, reinterpret_cast<HWND>(hWnd), NULL);
	PostMessageA(reinterpret_cast<HWND>(hWnd), TRAY_MESSAGE_ID, iSelectedOption, WM_COMMAND);
}