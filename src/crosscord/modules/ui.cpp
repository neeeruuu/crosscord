#include "ui.h"
#include "gl.h"
#include "config.h"
#include "overlay.h"
#include "crosshair.h"

#include "defines.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_glfw.h>

#include <GLFW/glfw3.h>

#include <imfilebrowser.h>

INSTANTIATE_SINGLETON(CUserInterface);

/*
	--------- module implementation ---------
*/
bool CUserInterface::Initialize() {
	m_Callbacks.push_back(
		g_CB_GLInit->Register(+[](GLFWwindow* pWindow) {
			CUserInterface::Get()->SetupGLObjects(pWindow);
		}, true)
	);

	m_Callbacks.push_back(
		g_CB_GLDraw->Register(+[]() {
			CUserInterface::Get()->Draw();
		}, true)
	);

	m_Callbacks.push_back(
		g_CB_GLShutdown->Register(+[]() {
			CUserInterface::Get()->CleanGLObjects();
		}, true)
	);

	m_Callbacks.push_back(
		g_CB_CrosshairImageLoaded->Register(+[](void* pImageBuffer, unsigned int iWidth, unsigned int iHeight) {
			CUserInterface::Get()->LoadImageFromBuffer(pImageBuffer, iWidth, iHeight);
		}, true)
	);

	return true;
}

bool CUserInterface::Shutdown() {
	if (m_Initialized)
		m_ShutdownQueued = true;
	return true;
}

/*
	--------- gl callbacks ---------
*/
void CUserInterface::SetupGLObjects(GLFWwindow* pWindow) {
	ImGui::CreateContext();
	ImGuiIO& IO = ImGui::GetIO();
	IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	IO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	IO.IniFilename = NULL;
	IO.LogFilename = NULL;

	const char* cGLSLVer = "#version 130";
	ImGui_ImplGlfw_InitForOpenGL(pWindow, true);
	ImGui_ImplOpenGL3_Init(cGLSLVer);

	m_MinSize[0] = WND_DEF_X * MIN_MULT;
	m_MinSize[1] = WND_DEF_Y * MIN_MULT;

	m_MaxSize[0] = WND_DEF_X * MAX_MULT;
	m_MaxSize[1] = WND_DEF_Y * MAX_MULT;

	/*
		initialize the custom theme (dracula styled)
	*/
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 1.0f };
		colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

		// Border
		colors[ImGuiCol_Border] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.29f };
		colors[ImGuiCol_BorderShadow] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.24f };

		// Text
		colors[ImGuiCol_Text] = ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f };
		colors[ImGuiCol_TextDisabled] = ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.13f, 0.13f, 0.17f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.13f, 0.13f, 0.17f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
		colors[ImGuiCol_CheckMark] = ImVec4{ 0.74f, 0.58f, 0.98f, 1.0f };

		// Popups
		colors[ImGuiCol_PopupBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 0.92f };

		// Slider
		colors[ImGuiCol_SliderGrab] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.54f };
		colors[ImGuiCol_SliderGrabActive] = ImVec4{ 0.74f, 0.58f, 0.98f, 0.54f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.13f, 0.13f, 0.17f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.24f, 0.24f, 0.32f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.2f, 0.22f, 0.27f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

		// Scrollbar
		colors[ImGuiCol_ScrollbarBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 1.0f };
		colors[ImGuiCol_ScrollbarGrab] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{ 0.24f, 0.24f, 0.32f, 1.0f };

		// Seperator
		colors[ImGuiCol_Separator] = ImVec4{ 0.44f, 0.37f, 0.61f, 1.0f };
		colors[ImGuiCol_SeparatorHovered] = ImVec4{ 0.74f, 0.58f, 0.98f, 1.0f };
		colors[ImGuiCol_SeparatorActive] = ImVec4{ 0.84f, 0.58f, 1.0f, 1.0f };

		// Resize Grip
		colors[ImGuiCol_ResizeGrip] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.29f };
		colors[ImGuiCol_ResizeGripHovered] = ImVec4{ 0.74f, 0.58f, 0.98f, 0.29f };
		colors[ImGuiCol_ResizeGripActive] = ImVec4{ 0.84f, 0.58f, 1.0f, 0.29f };

		// Docking
		colors[ImGuiCol_DockingPreview] = ImVec4{ 0.44f, 0.37f, 0.61f, 1.0f };

		auto& style = ImGui::GetStyle();
		style.TabRounding = 4;
		style.ScrollbarRounding = 9;
		style.WindowRounding = 7;
		style.GrabRounding = 3;
		style.FrameRounding = 3;
		style.PopupRounding = 4;
		style.ChildRounding = 4;
		style.WindowMenuButtonPosition = ImGuiDir_Right;
	}

	m_Initialized = true;
}

void CUserInterface::Draw() {
	/*
		begin new frame
	*/
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	/*
		draw menu
	*/
	if (m_ShouldDraw) {
		ImGui::SetNextWindowSizeConstraints({ m_MinSize[0], m_MinSize[1] }, { m_MaxSize[0], m_MaxSize[1] });
		if (ImGui::Begin("CrossCord " CROSSCORD_VER, &m_ShouldDraw, ImGuiWindowFlags_NoCollapse)) {
			/*
				handle DPI changes and set window focus if needed
			*/
			DPIFix();
			if (m_ShouldBringToFront) {
				ImGuiViewport* pVP = ImGui::GetWindowViewport();
				if (pVP && pVP->PlatformUserData) {
					ImGui::GetPlatformIO().Platform_SetWindowFocus(pVP);
					m_ShouldBringToFront = false;
				}
			}

			DrawContents();
		}
		ImGui::End();
	}

	/*
		render frame
	*/
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	/*
		handle shutdowns on draw thread if possible
	*/
	if (m_ShutdownQueued)
		CleanGLObjects();
}

void CUserInterface::CleanGLObjects() {
	if (!m_Initialized)
		return;

	m_ShutdownQueued = false;
	m_Initialized = false;
	
	for (CCallback* Callback : m_Callbacks) { delete Callback; }
	m_Callbacks.clear();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

/*
	--------- menu functions ---------
*/

/*
	fixes rendering issues if window is dragged to a monitor with different dpi
*/
void CUserInterface::DPIFix() {
	static ImVec2 vWindowSize = { 0.f, 0.f };
	vWindowSize = ImGui::GetWindowSize();

	static float fDPIScale = -1.f;
	float fCurrentDPI = ImGui::GetViewportPlatformMonitor(ImGui::GetWindowViewport())->DpiScale;

	if (fDPIScale != fCurrentDPI) {
		float fUnscaledSize[2] = { 0, 0 };
		if (fDPIScale == -1.f) {
			fUnscaledSize[0] = WND_DEF_X;
			fUnscaledSize[1] = WND_DEF_Y;
		}
		else {
			fUnscaledSize[0] = vWindowSize[0] / fDPIScale;
			fUnscaledSize[1] = vWindowSize[1] / fDPIScale;
		}

		ImGui::SetWindowSize({ fUnscaledSize[0] * fCurrentDPI, fUnscaledSize[1] * fCurrentDPI });

		m_MinSize[0] = (fUnscaledSize[0] * fCurrentDPI) * MIN_MULT;
		m_MinSize[1] = (fUnscaledSize[1] * fCurrentDPI) * MIN_MULT;

		m_MaxSize[0] = (fUnscaledSize[0] * fCurrentDPI) * MAX_MULT;
		m_MaxSize[1] = (fUnscaledSize[1] * fCurrentDPI) * MAX_MULT;

		fDPIScale = fCurrentDPI;
	}
}

void CUserInterface::DrawContents() {
	const char* cProcessName = COverlay::Get()->GetProcessName();
	if (strlen(cProcessName) != 0)
		ImGui::Text("Current process: %s", cProcessName);
	else
		ImGui::Text("Waiting for game...");

	ImGui::Separator();

	CCrosshair* pCrosshair = CCrosshair::Get();
	ImGuiStyle& Style = ImGui::GetStyle();

	SCrosshairSettings SettingsCopy = pCrosshair->m_Settings;

	ImGui::Checkbox("Enabled", &pCrosshair->m_Settings.m_Enabled);
	
	ImGui::SameLine();

	ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x + ImGui::CalcTextSize("Close").x + Style.FramePadding.x * 2);
	if (ImGui::Button("Close"))
		CModuleManager::Get()->ShutdownAll();

	ImGui::Text("Type:");

	ImGui::SameLine();
	ImGui::SetNextItemWidth((ImGui::GetContentRegionAvail().x - (Style.WindowPadding.x * 2)) / 2);
	ImGui::Combo("##TypeCombo", reinterpret_cast<int*>(&pCrosshair->m_Settings.m_Type), cCrosshairTypes, IM_ARRAYSIZE(cCrosshairTypes));

	ImGui::SameLine();
	ImGui::Text("Color:");
	
	ImGui::SameLine();
	ImGui::ColorEdit4("##Color", pCrosshair->m_Settings.m_Color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar);

	ImGui::Separator();

	ImGui::Text("Offset (x, y)");
	if (ImGui::BeginTable("##PositionTable", 3, ImGuiTableFlags_NoBordersInBody)) {
		ImGui::TableSetupColumn("1", ImGuiTableColumnFlags_WidthStretch, -1.f);
		ImGui::TableSetupColumn("2", ImGuiTableColumnFlags_WidthStretch, -1.f);
		ImGui::TableSetupColumn("3", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("Center").x + Style.FramePadding.x * 2);

		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
		ImGui::InputInt("X", &pCrosshair->m_Settings.m_Offset[0]);

		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
		ImGui::InputInt("Y", &pCrosshair->m_Settings.m_Offset[1]);

		ImGui::TableNextColumn();
		if (ImGui::Button("Center")) {
			pCrosshair->m_Settings.m_Offset[0] = 0;
			pCrosshair->m_Settings.m_Offset[1] = 0;
		}

		ImGui::EndTable();
	}

	ImGui::Separator();

	switch (pCrosshair->m_Settings.m_Type) {
		case CROSSHAIR_CROSS:
			DrawCrossSettings(pCrosshair);
			break;
		case CROSSHAIR_CIRCLE:
			DrawCircleSettings(pCrosshair);
			break;
		case CROSSHAIR_ARROW:
			DrawArrowSettings(pCrosshair);
			break;
		case CROSSHAIR_IMAGE:
			DrawImageSettings(pCrosshair);
			break;
	}

	if (memcmp(&SettingsCopy, &pCrosshair->m_Settings, sizeof(SCrosshairSettings))) {
		pCrosshair->Refresh();
		CConfigManager::Get()->SaveConfig("last");
	}
}

void CUserInterface::DrawCrossSettings(CCrosshair* pCrosshair) {
	ImGui::SliderInt("Length", reinterpret_cast<int*>(&pCrosshair->m_Settings.m_CrossSettings.m_Length), 1, 256);
	ImGui::SliderInt("Width", reinterpret_cast<int*>(&pCrosshair->m_Settings.m_CrossSettings.m_Width), 0, 32);
	ImGui::SliderInt("Gap", reinterpret_cast<int*>(&pCrosshair->m_Settings.m_CrossSettings.m_Gap), 0, 64);

	ImGui::Checkbox("T Style", &pCrosshair->m_Settings.m_CrossSettings.m_TStyle);
	ImGui::Checkbox("Center dot", &pCrosshair->m_Settings.m_CrossSettings.m_Dot);
}

void CUserInterface::DrawCircleSettings(CCrosshair* pCrosshair) {
	ImGui::SliderInt("Radius", reinterpret_cast<int*>(&pCrosshair->m_Settings.m_CircleSettings.m_Radius), 1, 64);
	ImGui::Checkbox("Hollow", &pCrosshair->m_Settings.m_CircleSettings.m_Hollow);

}

void CUserInterface::DrawArrowSettings(CCrosshair* pCrosshair) {
	ImGui::SliderInt("Length", reinterpret_cast<int*>(&pCrosshair->m_Settings.m_ArrowSettings.m_Length), 1, 64);
	ImGui::SliderInt("Width", reinterpret_cast<int*>(&pCrosshair->m_Settings.m_ArrowSettings.m_Width), 0, 64);
}

void CUserInterface::DrawImageSettings(CCrosshair* pCrosshair) {
	static ImGui::FileBrowser ImageDialog;
	static bool bInit = false;
	if (!bInit) {
		ImageDialog.SetTitle("Select an image");
		ImageDialog.SetTypeFilters({ ".png" });
		//ImageDialog.SetPwd(m_LastImagePath.substr(0, m_LastImagePath.length() - 1));
		bInit = true;
	}

	ImVec2 RegionAvail = ImGui::GetContentRegionAvail();

	ImGui::SetCursorPosX(RegionAvail.x / 2 - ((ImGui::CalcTextSize("Open image").x + ImGui::GetStyle().FramePadding.x * 2) / 2));
	if (ImGui::Button("Open image")) {
		ImageDialog.Open();
	}

	ImageDialog.Display();

	if (ImageDialog.HasSelected()) {
		std::string sFilePath = ImageDialog.GetSelected().string();
		CCrosshair::Get()->LoadImg(sFilePath.c_str());

		//m_LastImagePath = ImageDialog.GetSelected().remove_filename().string();
		ImageDialog.ClearSelected();
	}

	float fWidth = RegionAvail.x / 3;
	if (m_ImagePreviewTex) {
		#pragma warning(push)
		#pragma warning(disable: 4312)
				ImGui::Image(reinterpret_cast<void*>(m_ImagePreviewTex), { fWidth, fWidth / m_ImageAspectRatio });
		#pragma warning(pop)
	}
	else
		ImGui::Image(0, { fWidth, fWidth });

	ImGui::SameLine();
	ImVec2 vCursorPos = ImGui::GetCursorPos();
	ImGui::SetNextItemWidth(fWidth * 1.5f);
	ImGui::SliderFloat("Size", &pCrosshair->m_Settings.m_ImageSettings.m_Size, 0.1f, 1.f);

	ImGui::SetNextItemWidth(fWidth * 1.5f);
	ImGui::SetCursorPos({ vCursorPos.x, vCursorPos.y + ImGui::GetTextLineHeight() * 2 });
	ImGui::SliderFloat("Alpha", &pCrosshair->m_Settings.m_ImageSettings.m_Alpha, 0.f, 1.f);
}

void CUserInterface::LoadImageFromBuffer(void* pImageBuffer, unsigned int iWidth, unsigned int iHeight) {
	if (m_ImagePreviewTex)
		glDeleteTextures(1, &m_ImagePreviewTex);

	GLint iPrevTexture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPrevTexture);

	glGenTextures(1, &m_ImagePreviewTex);
	glBindTexture(GL_TEXTURE_2D, m_ImagePreviewTex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
		static_cast<GLsizei>(iWidth), static_cast<GLsizei>(iHeight), 0,
		GL_RGBA, GL_UNSIGNED_BYTE,
		reinterpret_cast<GLvoid*>(pImageBuffer));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	m_ImageAspectRatio = static_cast<float>(iWidth) / static_cast<float>(iHeight);
}