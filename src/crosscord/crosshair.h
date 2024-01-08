#pragma once

#include "util/macros.h"

enum ECrosshairType {
	CROSSHAIR_CROSS,
	CROSSHAIR_CIRCLE,
	CROSSHAIR_ARROW,
	/*CROSSHAIR_IMAGE*/
};
inline const char* cCrosshairTypes[]{ "Cross", "Circle", "Arrow", /*"Image"*/ };

struct SCrosshairSettings {
	bool m_Enabled = true;
	bool m_CrossTStyle = false;
	bool m_CrossDot = true;
	bool m_CircleHollow = false;

	ECrosshairType m_Type = CROSSHAIR_CROSS;
	float m_Color[4] = { 1.f, 0.f, 0.f, .295f };
	int m_Offset[2] = { 0 };

	int m_CrossLength = 60;
	int m_CrossWidth = 3;
	int m_CrossGap = 0;

	int m_ArrowLength = 20;
	int m_ArrowWidth = 2;

	int m_CircleRadius = 5;
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
private:

	CCrosshair() {};

	void DrawBox(unsigned int iX0, unsigned int iY0, unsigned int iX1, unsigned int iY1, struct SColor* Col);
	
	class CCallback* m_DrawCB = nullptr;
};