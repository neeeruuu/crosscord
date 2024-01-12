#pragma once

#include "util/macros.h"

#include <mutex>

enum ECrosshairType {
	CROSSHAIR_CROSS,
	CROSSHAIR_CIRCLE,
	CROSSHAIR_ARROW,
	CROSSHAIR_IMAGE
};
inline const char* cCrosshairTypes[]{ "Cross", "Circle", "Arrow", "Image" };

struct SCrosshairSettings {
	bool m_Enabled = true;
	bool m_CrossTStyle = false;
	bool m_CrossDot = true;
	bool m_CircleHollow = false;

	ECrosshairType m_Type = CROSSHAIR_CROSS;
	float m_Color[4] = { 1.f, 0.f, 0.f, .295f };
	int m_Offset[2] = { 0 };

	unsigned int m_CrossLength = 60;
	unsigned int m_CrossWidth = 3;
	unsigned int m_CrossGap = 0;

	unsigned int m_ArrowLength = 20;
	unsigned int m_ArrowWidth = 2;

	unsigned int m_CircleRadius = 5;
	unsigned int m_ImageHeight = 0;
	unsigned int m_ImageWidth = 0;

	float m_ImageSize = 100;
	float m_ImageAlpha = 1.f;
	void* m_ImageBuffer = nullptr;
};

class CCrosshair {
	DECLARE_SINGLETON(CCrosshair);
public:
	bool Initialize();
	void Shutdown();

	void _Draw(struct SFrameInfo* pFrameInfo, SCrosshairSettings* pSettings);
	void _SettingChanged();

	SCrosshairSettings m_Settings;
	SCrosshairSettings _m_PrevSettings;

	void SetImageBuffer(void* pBuffer, unsigned long long lSize, int iWidth, int iHeight);
	std::mutex ImageMutex;
private:
	CCrosshair() {};

	void DrawBox(unsigned int iX0, unsigned int iY0, unsigned int iX1, unsigned int iY1, struct SColor* Col);
	
	class CCallback* m_DrawCB = nullptr;
};