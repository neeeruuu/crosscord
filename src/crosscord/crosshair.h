#pragma once

#include "util/macros.h"

enum ECrosshairType {
	CROSSHAIR_CROSS,
	CROSSHAIR_CIRCLE,
	CROSSHAIR_TRIANGLE,
	/*CROSSHAIR_IMAGE*/
};
inline const char* cCrosshairTypes[]{ "Cross", "Circle", "Triangle", /*"Image"*/ };

struct SCrosshairSettings {
	bool m_Enabled = true;

	bool m_SettingsInitialized = false;

	ECrosshairType m_Type = CROSSHAIR_CROSS;
	float m_Color[4] = { 1.f, 0.f, 0.f, 1.f };
	int m_Position[2] = { 0 };
	int m_Size = 15; // length for cross, radius for size, hipotenuse for triangle, multiplier for image

	// cross params
	int m_Width = 3;
	float m_Gap = 0.5f;
	bool m_TStyle = false;
	bool m_Dot = false;

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

	void DrawBox(int iX0, int iY0, int iX1, int iY1, struct SColor* Col);

	class CCallback* m_DrawCB;
};