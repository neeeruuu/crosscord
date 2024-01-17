#pragma once

#include <vector>
#include <string>

#include "util/macros.h"

class CInterface {
	DECLARE_SINGLETON(CInterface);
public:
	void _Init(struct GLFWwindow* pWindow);
	bool Initialize();
	void Shutdown();

	void QueueBringToFront() { m_ShouldBringToFront = true; m_ShouldDraw = true; }

	void Draw();

	std::string m_LastImagePath = "";
private:
	void DPIFix();

	void DrawCrossSettings(class CCrosshair* pCrosshair);
	void DrawCircleSettings(class CCrosshair* pCrosshair);
	void DrawArrowSettings(class CCrosshair* pCrosshair);
	void DrawImageSettings(class CCrosshair* pCrosshair);

	void LoadImageFromPath(const char* cPath);

	unsigned int m_ImagePreviewTex = 0;
	float m_ImageAspectRatio = 0;

	float m_MinSize[2] = { 0 };
	float m_MaxSize[2] = { 0 };

	bool m_ShouldDraw = true;
	bool m_ShouldBringToFront = false;

	std::vector<class CCallback*> m_Callbacks;

	CInterface() { };
};