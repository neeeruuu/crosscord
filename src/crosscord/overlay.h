#pragma once

#include "util/macros.h"
#include "util/callbacks.h"

#include <mutex>

#define RENDER_TIMING 1000 / 120
#define DETECTION_TIMING 500

struct SColor {
	unsigned char r, g, b, a;
};

struct SPixel {
	unsigned int x, y;
	SColor col;
};

struct SFrameInfo {
	unsigned int m_Header;
	unsigned int m_Frame;
	unsigned int _;
	unsigned int m_Width;
	unsigned int m_Height;
	SColor m_Pixels[1];
};

class COverlay {
	DECLARE_SINGLETON(COverlay);
public:
	inline void Shutdown() { m_ShutdownQueued = true; }

	bool RenderThread();
	bool DetectionThread();

	bool SetPixel(SPixel* pPixel);

	inline char* GetActiveWindowName() { return m_TargetWindowName; }
	inline SFrameInfo* GetFrameInfo() { return m_FrameInfo; }
private:
	unsigned int m_LastFrameId = 0;

	char m_TargetWindowName[64] = { 0 };
	unsigned long m_TargetProcessId = 0;

	void* m_MapFile = nullptr;
	void* m_MapView = nullptr;

	bool m_ShutdownQueued = false;

	SFrameInfo* m_FrameInfo = nullptr;

	std::mutex m_MapMutex;

	bool AdquireMap(int iProcessId);

	COverlay() {};
};

inline CCallbackEvent* g_CB_OverlayDraw = new CCallbackEvent();