#pragma once

#include <mutex>

#include "modules.h"
#include "defines.h"

#include "util/macros.h"
#include "util/callbacks.h"

struct SColor {
	unsigned char r, g, b, a;
};

struct SFrameInfo {
	unsigned int m_Header;
	unsigned int m_Frame;
	unsigned int _;
	unsigned int m_Width;
	unsigned int m_Height;
	SColor m_Pixels[1];
};

class COverlay : public IModule {
	DECLARE_SINGLETON(COverlay);
public:
	virtual bool Initialize() override;
	virtual bool Shutdown() override;
	virtual const char* GetName() override { return "Overlay"; }
	const char* GetProcessName() { return m_TargetProcessName; }
	inline SFrameInfo* GetFrameInfo() { return m_FrameInfo; }
private:
	bool Detect();
	bool Render();
	bool AdquireMap(unsigned long dwPID);
	void GetProcessName(unsigned long dwPID);

	COverlay() {};

	void* m_MapFile = nullptr;
	void* m_MapView = nullptr;

	SFrameInfo* m_FrameInfo = nullptr;
	unsigned int m_LastFrameId = 0;

	bool m_ShouldShutdown = false;

	char m_TargetProcessName[64] = { 0 };
	unsigned long m_TargetProcessId = 0;
};

// passes: SFrameInfo*
inline CCallbackEvent* g_CB_OverlayDraw = new CCallbackEvent();