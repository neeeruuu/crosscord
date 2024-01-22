#pragma once

#include "modules.h"

#include <vector>

class CUserInterface : public IModule {
	DECLARE_SINGLETON(CUserInterface);
public:
	// module implementatino
	virtual bool Initialize() override;
	virtual bool Shutdown() override;
	virtual const char* GetName() override { return "UI"; }

	void LoadImageFromBuffer(void* pImageBuffer, unsigned int iWidth, unsigned int iHeight);

	// menu functions
	void QueueBringToFront() { m_ShouldBringToFront = true; m_ShouldDraw = true; }
private:
	std::vector<class CCallback*> m_Callbacks;

	bool m_Initialized = false;
	bool m_ShutdownQueued = false;
	bool m_ShouldDraw = true;
	bool m_ShouldBringToFront = true;
	bool m_NotClosed = true;

	float m_MinSize[2] = { 0 };
	float m_MaxSize[2] = { 0 };

	unsigned int m_ImagePreviewTex = 0;
	float m_ImageAspectRatio = 0.f;

	// callback functions
	void SetupGLObjects(struct GLFWwindow* pWindow);
	void Draw();
	void CleanGLObjects();

	// menu functions
	void DPIFix();
	void DrawContents();

	void DrawCrossSettings(class CCrosshair* pCrosshair);
	void DrawCircleSettings(class CCrosshair* pCrosshair);
	void DrawArrowSettings(class CCrosshair* pCrosshair);
	void DrawImageSettings(class CCrosshair* pCrosshair);

	CUserInterface() {};
};