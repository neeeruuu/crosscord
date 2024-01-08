#include "ui.h"
#include "gl.h"
#include "overlay.h"
#include "crosshair.h"

#include "fmt/format.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_glfw.h>

//#include <imfilebrowser.h>

/*
	TO-DO:
		Refactor menu code
*/

INITIALIZE_SINGLETON(CInterface);

#define CROSSHAIR_SETTING(Widget) if (Widget) pCrosshair->_SettingChanged();

bool CInterface::Initialize() {
	m_Callbacks.push_back(g_CB_GLInit->Register([](GLFWwindow* pWindow) { CInterface::Get()->_Init(pWindow); }, true));
	m_Callbacks.push_back(g_CB_GLRender->Register([]() { CInterface::Get()->Draw(); }, true));
	m_Callbacks.push_back(g_CB_GLShutdown->Register([]() { CInterface::Get()->Shutdown(); }, true));
	return true;
}

void CInterface::Shutdown() {
	for (CCallback* Callback : m_Callbacks) { delete Callback; }
	m_Callbacks.clear();
}

void CInterface::_Init(GLFWwindow* pWindow) {
	ImGui::CreateContext();
	ImGuiIO& IO = ImGui::GetIO();
	IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	IO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	const char* cGLSLVer = "#version 130";
	ImGui_ImplGlfw_InitForOpenGL(pWindow, true);
	ImGui_ImplOpenGL3_Init(cGLSLVer);
}

bool SliderUInt(const char* cLabel, unsigned int* pValue, unsigned int iMin, unsigned int iMax, const char* cFmt = "%d", ImGuiSliderFlags Flags = 0) {
	return ImGui::SliderScalar(	cLabel, ImGuiDataType_U32, reinterpret_cast<void*>(pValue),
								reinterpret_cast<const void*>(&iMin), reinterpret_cast<const void*>(&iMax),
								cFmt, Flags);
}

void CInterface::Draw() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	static float fDPIScale = 1.f;

	if (m_ShouldDraw) {
		char* cWindowName = COverlay::Get()->GetActiveWindowName();
		bool bHasGame = strlen(cWindowName) != 0;

		static ImGuiCond WndSizeCond = ImGuiCond_Once;
		ImGui::SetNextWindowSize({350 * fDPIScale, 450 * fDPIScale }, WndSizeCond);
		WndSizeCond = ImGuiCond_Once;
		ImGui::SetNextWindowSizeConstraints({ 300 * fDPIScale, 400 * fDPIScale }, { 400 * fDPIScale, 500 * fDPIScale });
		if (ImGui::Begin("CrossCord " CROSSCORD_VER, &m_ShouldDraw, ImGuiWindowFlags_NoCollapse)) {
			ImGuiStyle* pStyle = &ImGui::GetStyle();
			CCrosshair* pCrosshair = CCrosshair::Get();

			if (bHasGame)
				ImGui::Text("Current process: %s", COverlay::Get()->GetActiveWindowName());
			else
				ImGui::Text("Waiting for game...");
			
			ImGui::Separator();

			CROSSHAIR_SETTING(ImGui::Checkbox("Enabled", &pCrosshair->m_Settings.m_Enabled));
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x + ImGui::CalcTextSize("Close").x + pStyle->FramePadding.x * 2);
			if (ImGui::Button("Close"))
				CGLManager::Get()->Shutdown();

			ImGui::Text("Type:");
			ImGui::SetNextItemWidth((ImGui::GetContentRegionAvail().x - (pStyle->WindowPadding.x * 2)) / 3);
			ImGui::SameLine();
			CROSSHAIR_SETTING(ImGui::Combo("##Type", reinterpret_cast<int*>(&pCrosshair->m_Settings.m_Type), cCrosshairTypes, IM_ARRAYSIZE(cCrosshairTypes)));
			ImGui::SameLine();
			ImGui::Text("Color:");
			ImGui::SameLine();
			CROSSHAIR_SETTING(ImGui::ColorEdit4("Color", pCrosshair->m_Settings.m_Color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel));

			ImGui::Separator();

			ImGui::Text("Offset (x, y)");
			
			if (ImGui::BeginTable("##PositionTable", 3, ImGuiTableFlags_NoBordersInBody)) {

				ImGui::TableSetupColumn("1", ImGuiTableColumnFlags_WidthStretch, -1.f);
				ImGui::TableSetupColumn("2", ImGuiTableColumnFlags_WidthStretch, -1.f);
				ImGui::TableSetupColumn("3", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("Center").x + pStyle->FramePadding.x * 2);

				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
				CROSSHAIR_SETTING(ImGui::InputInt("X", &pCrosshair->m_Settings.m_Offset[0]));

				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
				CROSSHAIR_SETTING(ImGui::InputInt("Y", &pCrosshair->m_Settings.m_Offset[1]));

				ImGui::TableNextColumn();
				if (!bHasGame)
					ImGui::BeginDisabled();
				if (ImGui::Button("Center")) {
					pCrosshair->m_Settings.m_Offset[0] = 0;
					pCrosshair->m_Settings.m_Offset[1] = 0;
					pCrosshair->_SettingChanged();
				}
				if (!bHasGame)
					ImGui::EndDisabled();

				ImGui::EndTable();
			}

			ImGui::Separator();

			ImGui::Text("Type specific settings");

			switch (pCrosshair->m_Settings.m_Type) {
				case CROSSHAIR_CROSS:
					CROSSHAIR_SETTING(SliderUInt("Length", &pCrosshair->m_Settings.m_CrossLength, 1, 256));
					CROSSHAIR_SETTING(SliderUInt("Width", &pCrosshair->m_Settings.m_CrossWidth, 0, 32));
					CROSSHAIR_SETTING(SliderUInt("Gap", &pCrosshair->m_Settings.m_CrossGap, 0, 64));
					CROSSHAIR_SETTING(ImGui::Checkbox("T Style", &pCrosshair->m_Settings.m_CrossTStyle));
					CROSSHAIR_SETTING(ImGui::Checkbox("Center dot", &pCrosshair->m_Settings.m_CrossDot));
					break;
				case CROSSHAIR_CIRCLE:
					CROSSHAIR_SETTING(SliderUInt("Radius", &pCrosshair->m_Settings.m_CircleRadius, 1, 64));
					CROSSHAIR_SETTING(ImGui::Checkbox("Hollow", &pCrosshair->m_Settings.m_CircleHollow));
					break;
				case CROSSHAIR_ARROW:
					CROSSHAIR_SETTING(SliderUInt("Length", &pCrosshair->m_Settings.m_ArrowLength, 1, 64));
					CROSSHAIR_SETTING(SliderUInt("Width", &pCrosshair->m_Settings.m_ArrowWidth, 0, 64));
					break;
				//case CROSSHAIR_IMAGE:
				//	CROSSHAIR_SETTING(ImGui::SliderFloat("Size", &pCrosshair->m_Size, 0.1f, 15.f));
				//	break;
			}


			/*
				ImGui workarounds
			*/
			if (m_ShouldBringToFront) {
				// sometimes userdata is null, and imgui doesn't check that during SetFocus, so an access violation occurs, fix pls
				ImGuiViewport* pVP = ImGui::GetWindowViewport();
				if (pVP->PlatformUserData) {
					ImGui::GetPlatformIO().Platform_SetWindowFocus(ImGui::GetWindowViewport());;
					m_ShouldBringToFront = false;
				}
			}
			if (fDPIScale != ImGui::GetViewportPlatformMonitor(ImGui::GetWindowViewport())->DpiScale) {
				WndSizeCond = ImGuiCond_Always;
			}
			fDPIScale = ImGui::GetViewportPlatformMonitor(ImGui::GetWindowViewport())->DpiScale;
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