#pragma once

#include <vector>

#include "util/macros.h"

class CInterface {
	DECLARE_SINGLETON(CInterface);
public:
	void _Init(struct GLFWwindow* pWindow);
	bool Initialize();
	void Shutdown();

	void QueueBringToFront() { m_ShouldBringToFront = true; m_ShouldDraw = true; }

	void Draw();
private:
	void DPIFix();

	void DrawCrossSettings(class CCrosshair* pCrosshair);
	void DrawCircleSettings(class CCrosshair* pCrosshair);
	void DrawArrowSettings(class CCrosshair* pCrosshair);

	bool m_ShouldDraw = true;
	bool m_ShouldBringToFront = false;

	float m_MinSize[2] = { 0 };
	float m_MaxSize[2] = { 0 };

	std::vector<class CCallback*> m_Callbacks;

	CInterface() { };
};