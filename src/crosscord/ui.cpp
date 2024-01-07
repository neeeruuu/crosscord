#include "ui.h"
#include "gl.h"

#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_glfw.h>

INITIALIZE_SINGLETON(CInterface);

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

void CInterface::Draw() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (m_ShouldDraw) {
		ImGui::SetNextWindowSizeConstraints({ 300, 400 }, { 400, 500 });
		if (ImGui::Begin("crosscord", &m_ShouldDraw, ImGuiWindowFlags_NoCollapse)) {
			ImGui::Button("Hi");

			if (m_ShouldBringToFront) {
				// sometimes userdata is null, and imgui doesn't check that during SetFocus, so an access violation occurs, fix pls
				ImGuiViewport* pVP = ImGui::GetWindowViewport();
				if (pVP->PlatformUserData) {
					ImGui::GetPlatformIO().Platform_SetWindowFocus(ImGui::GetWindowViewport());;
					m_ShouldBringToFront = false;
				}
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