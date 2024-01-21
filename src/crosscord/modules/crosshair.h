#pragma once
#include "modules.h"

#include "util/macros.h"
#include "util/callbacks.h"

#include <vector>
#include <mutex>

/*
	TO-DO:
		don't include the whole json header just for a class member
		do fwd declaration instead
*/
#include <nlohmann/json.hpp>

struct SFrameInfo;
struct SColor;

enum ECrosshairType {
	CROSSHAIR_CROSS,
	CROSSHAIR_CIRCLE,
	CROSSHAIR_ARROW,
	CROSSHAIR_IMAGE,
	CROSSHAIR_UNINITIALIZED
};
inline const char* cCrosshairTypes[]{ "Cross", "Circle", "Arrow", "Image" };

struct SCrosshairSettings {
	bool m_Enabled = true;
	ECrosshairType m_Type = CROSSHAIR_CROSS;
	float m_Color[4] = { 1.f, 0.f, 0.f, 1.f };
	int m_Offset[2] = { 0 };

	struct SCrossSettings {
		bool m_TStyle = false;
		bool m_Dot = true;
		unsigned int m_Length = 60;
		unsigned int m_Width = 3;
		unsigned int m_Gap = 0;
	} m_CrossSettings;

	struct SCircleSettings {
		bool m_Hollow = false;
		unsigned int m_Radius = 5;
	} m_CircleSettings;

	struct SArrowSettings {
		unsigned int m_Length = 20;
		unsigned int m_Width = 2;
	} m_ArrowSettings;

	struct SImageSettings {
		unsigned int m_Height = 0;
		unsigned int m_Width = 0;

		float m_Size = 1.f;
		float m_Alpha = 1.f;
		void* m_Buffer = nullptr;
	} m_ImageSettings;
};

class CCrosshair : public IModule {
	DECLARE_SINGLETON(CCrosshair);
public:
	virtual bool Initialize() override;
	virtual bool Shutdown() override;
	virtual const char* GetName() override { return "Crosshair renderer"; }

	void Draw(SFrameInfo* pFrameInfo);
	void Clear(SFrameInfo* pFrameInfo);

	void Refresh();

	SCrosshairSettings m_Settings;
	SCrosshairSettings m_PrevSettings;

	void LoadConfig(nlohmann::json* pJSON);
	void SaveConfig(nlohmann::json& pJSON);

	void LoadImg(const char* cPath);
private:
	std::mutex ImageLock;

	void DrawRect(SFrameInfo* pFrameInfo, unsigned int iStartX, unsigned int iStartY, unsigned int iEndX, unsigned int iEndY, SColor* pColor);

	void DrawCross(SFrameInfo* pFrameInfo, SCrosshairSettings* pSettings, unsigned int iCenterX, unsigned int iCenterY, SColor* pColor);
	void DrawCircle(SFrameInfo* pFrameInfo, SCrosshairSettings* pSettings, unsigned int iCenterX, unsigned int iCenterY, SColor* pColor);
	void DrawArrow(SFrameInfo* pFrameInfo, SCrosshairSettings* pSettings, unsigned int iCenterX, unsigned int iCenterY, SColor* pColor);
	void DrawImage(SFrameInfo* pFrameInfo, SCrosshairSettings* pSettings, unsigned int iCenterX, unsigned int iCenterY, SColor* pColor);

	std::vector<class CCallback*> m_Callbacks;
	CCrosshair() {};
};

// passes: void* pImageBuffer, unsigned int iWidth, unsigned int iHeight
inline CCallbackEvent* g_CB_CrosshairImageLoaded = new CCallbackEvent();