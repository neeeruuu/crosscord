#include "ui.h"
#include "gl.h"
#include "overlay.h"
#include "crosshair.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_glfw.h>

INITIALIZE_SINGLETON(CInterface);

#define WND_DEF_X 350.f
#define WND_DEF_Y 225.f

#define MIN_MULT 0.95f
#define MAX_MULT 1.05f

bool CInterface::Initialize() {
	m_Callbacks.push_back(g_CB_GLInit->Register([](GLFWwindow* pWindow) { CInterface::Get()->_Init(pWindow); }, true));
	m_Callbacks.push_back(g_CB_GLRender->Register([]() { CInterface::Get()->Draw(); }, true));
	m_Callbacks.push_back(g_CB_GLShutdown->Register([]() { CInterface::Get()->Shutdown(); }, true));
	return true;
}

void CInterface::_Init(GLFWwindow* pWindow) {
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
}

void CInterface::Shutdown() {
	for (CCallback* Callback : m_Callbacks) { delete Callback; }
	m_Callbacks.clear();

	ImGui::DestroyContext();
	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
}

/*
	drawing functions / helpers
*/
#define CROSSHAIR_SETTING(Widget) if (Widget) pCrosshair->_SettingChanged();

bool SliderUInt(const char* cLabel, unsigned int* pValue, unsigned int iMin, unsigned int iMax, const char* cFmt = "%d", ImGuiSliderFlags Flags = 0) {
	return ImGui::SliderScalar(cLabel, ImGuiDataType_U32, reinterpret_cast<void*>(pValue),
		reinterpret_cast<const void*>(&iMin), reinterpret_cast<const void*>(&iMax),
		cFmt, Flags);
}

void CInterface::DrawCrossSettings(CCrosshair* pCrosshair) {
	CROSSHAIR_SETTING(SliderUInt("Length", &pCrosshair->m_Settings.m_CrossLength, 1, 256));
	CROSSHAIR_SETTING(SliderUInt("Width", &pCrosshair->m_Settings.m_CrossWidth, 0, 32));
	CROSSHAIR_SETTING(SliderUInt("Gap", &pCrosshair->m_Settings.m_CrossGap, 0, 64));
	CROSSHAIR_SETTING(ImGui::Checkbox("T Style", &pCrosshair->m_Settings.m_CrossTStyle));
	CROSSHAIR_SETTING(ImGui::Checkbox("Center dot", &pCrosshair->m_Settings.m_CrossDot));
}

void CInterface::DrawCircleSettings(CCrosshair* pCrosshair) {
	CROSSHAIR_SETTING(SliderUInt("Radius", &pCrosshair->m_Settings.m_CircleRadius, 1, 64));
	CROSSHAIR_SETTING(ImGui::Checkbox("Hollow", &pCrosshair->m_Settings.m_CircleHollow));
}

void CInterface::DrawArrowSettings(CCrosshair* pCrosshair) {
	CROSSHAIR_SETTING(SliderUInt("Length", &pCrosshair->m_Settings.m_ArrowLength, 1, 64));
	CROSSHAIR_SETTING(SliderUInt("Width", &pCrosshair->m_Settings.m_ArrowWidth, 0, 64));
}

void CInterface::DPIFix() {
	static ImVec2 vWindowSize = {0.f, 0.f};
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

void CInterface::Draw() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (m_ShouldDraw) {
		ImGui::SetNextWindowSizeConstraints({ m_MinSize[0], m_MinSize[1] }, { m_MaxSize[0], m_MaxSize[1] });
		if (ImGui::Begin("CrossCord " CROSSCORD_VER, &m_ShouldDraw, ImGuiWindowFlags_NoCollapse)) {
			DPIFix();

			char* cWindowName = COverlay::Get()->GetActiveWindowName();

			if (strlen(cWindowName) != 0)
				ImGui::Text("Current process: %s", COverlay::Get()->GetActiveWindowName());
			else
				ImGui::Text("Waiting for game...");

			ImGui::Separator();

			CCrosshair* pCrosshair = CCrosshair::Get();
			ImGuiStyle& Style = ImGui::GetStyle();

			CROSSHAIR_SETTING(ImGui::Checkbox("Enabled", &pCrosshair->m_Settings.m_Enabled));
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x + ImGui::CalcTextSize("Close").x + Style.FramePadding.x * 2);
			if (ImGui::Button("Close"))
				CGLManager::Get()->Shutdown();

			ImGui::Text("Type:");
			ImGui::SetNextItemWidth((ImGui::GetContentRegionAvail().x - (Style.WindowPadding.x * 2)) / 2);
			ImGui::SameLine();
			CROSSHAIR_SETTING(ImGui::Combo("##Type", reinterpret_cast<int*>(&pCrosshair->m_Settings.m_Type), cCrosshairTypes, IM_ARRAYSIZE(cCrosshairTypes)));
			ImGui::SameLine();
			ImGui::Text("Color:");
			ImGui::SameLine();
			CROSSHAIR_SETTING(ImGui::ColorEdit4("Color", pCrosshair->m_Settings.m_Color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel));

			// is there any way i could make this compact? seems like a lot of clutter for how little it's doing
			ImGui::Text("Offset (x, y)");
			if (ImGui::BeginTable("##PositionTable", 3, ImGuiTableFlags_NoBordersInBody)) {
				ImGui::TableSetupColumn("1", ImGuiTableColumnFlags_WidthStretch, -1.f);
				ImGui::TableSetupColumn("2", ImGuiTableColumnFlags_WidthStretch, -1.f);
				ImGui::TableSetupColumn("3", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("Center").x + Style.FramePadding.x * 2);

				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
				CROSSHAIR_SETTING(ImGui::InputInt("X", &pCrosshair->m_Settings.m_Offset[0]));

				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
				CROSSHAIR_SETTING(ImGui::InputInt("Y", &pCrosshair->m_Settings.m_Offset[1]));

				ImGui::TableNextColumn();
				if (ImGui::Button("Center")) {
					pCrosshair->m_Settings.m_Offset[0] = 0;
					pCrosshair->m_Settings.m_Offset[1] = 0;
					pCrosshair->_SettingChanged();
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
			}
		}
		ImGui::End();
	}

	ImGui::Render();

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}