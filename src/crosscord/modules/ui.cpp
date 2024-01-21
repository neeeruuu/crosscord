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

#include <imfilebrowser.h>

INSTANTIATE_SINGLETON(CUserInterface);

/*
	--------- module implementation ---------
*/
bool CUserInterface::Initialize() {
	m_Callbacks.push_back(
		g_CB_GLInit->Register([](GLFWwindow* pWindow) {
			CUserInterface::Get()->SetupGLObjects(pWindow);
		}, true)
	);

	m_Callbacks.push_back(
		g_CB_GLDraw->Register([]() {
			CUserInterface::Get()->Draw();
		}, true)
	);

	m_Callbacks.push_back(
		g_CB_GLShutdown->Register([]() {
			CUserInterface::Get()->CleanGLObjects();
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

void CUserInterface::DrawImageSettings(CCrosshair* /*pCrosshair*/) {

}