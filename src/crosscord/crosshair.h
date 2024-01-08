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

	bool m_SettingsInitialized = false;

	ECrosshairType m_Type = CROSSHAIR_CROSS;
	float m_Color[4] = { 1.f, 0.f, 0.f, .295f };
	int m_Position[2] = { 0 };
	int m_Size = 60; // length for cross, radius for size, hipotenuse for triangle, multiplier for image

	int m_Width = 3; // used on cross and arrow

	// cross params
	int m_Gap = 0;
	bool m_TStyle = false;
	bool m_Dot = true;

	// circle / triangle params
	bool m_Hollow = true;
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
	
	class CCallback* m_DrawCB;
};