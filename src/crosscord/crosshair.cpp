#include "crosshair.h"
#include "overlay.h"

INITIALIZE_SINGLETON(CCrosshair);

bool CCrosshair::Initialize() {
	m_DrawCB = g_CB_OverlayDraw->Register([](SFrameInfo* pFrameInfo) {
		CCrosshair* pCrosshair = CCrosshair::Get();

		if (pCrosshair->_m_PrevSettings.m_SettingsInitialized)
			pCrosshair->_Draw(pFrameInfo, &pCrosshair->_m_PrevSettings);
		pCrosshair->_Draw(pFrameInfo, &pCrosshair->m_Settings);

		memcpy(&pCrosshair->_m_PrevSettings, &pCrosshair->m_Settings, sizeof(SCrosshairSettings));
		memset(&pCrosshair->_m_PrevSettings.m_Color, 0, sizeof(int) * 4);
		}, true);
	return true;
}

void CCrosshair::Shutdown() {
	delete m_DrawCB;
}

void CCrosshair::DrawBox(int iStartX, int iStartY, int iEndX, int iEndY, SColor* Col) {
	SFrameInfo* pFrameInfo = COverlay::Get()->GetFrameInfo();
	SColor* pPixelBuffer = reinterpret_cast<SColor*>(COverlay::Get()->GetFrameInfo()->m_Pixels);
	for (int iX = iStartX; iX < iEndX; iX++) {
		for (int iY = iStartY; iY < iEndY; iY++) {
			pPixelBuffer[iY * pFrameInfo->m_Width + iX] = *Col;
		}
	}
}

void CCrosshair::_Draw(SFrameInfo* pFrameInfo, SCrosshairSettings* pSettings) {
	if (!pSettings->m_Position[0])
		pSettings->m_Position[0] = pFrameInfo->m_Width / 2;

	if (!pSettings->m_Position[1])
		pSettings->m_Position[1] = pFrameInfo->m_Height / 2;

	pSettings->m_SettingsInitialized = true;

	SColor TargetColor{
		static_cast<unsigned char>(pSettings->m_Color[2] * 255),
		static_cast<unsigned char>(pSettings->m_Color[1] * 255),
		static_cast<unsigned char>(pSettings->m_Color[0] * 255),
		static_cast<unsigned char>(pSettings->m_Color[3] * 255),
	};

	switch (pSettings->m_Type) {
		case CROSSHAIR_CROSS: {
			int iStartX, iStartY;
			int iEndX, iEndY;


			// horizontal line
			iStartX = pSettings->m_Position[0] + 1 - (pSettings->m_Size / 2);
			iStartY = pSettings->m_Position[1] + 1 - pSettings->m_Width;

			iEndX = pSettings->m_Position[0] + (pSettings->m_Size / 2);
			iEndY = pSettings->m_Position[1] + pSettings->m_Width;
			DrawBox(iStartX, iStartY, iEndX, iEndY, &TargetColor);

			// vertical line
			iStartX = pSettings->m_Position[0] + 1 - pSettings->m_Width;
			if (!pSettings->m_TStyle)
				iStartY = pSettings->m_Position[1] + 1 - (pSettings->m_Size / 2);
			else
				iStartY = pSettings->m_Position[1];


			iEndX = pSettings->m_Position[0] + pSettings->m_Width;
			iEndY = pSettings->m_Position[1] + (pSettings->m_Size / 2);

			DrawBox(iStartX, iStartY, iEndX, iEndY, &TargetColor);
		}
			//DrawBox(pSettings->m_Position[0] - (pSettings->m_Size / 2),
			//	pSettings->m_Position[1] - (pSettings->m_Width / 2),
			//	pSettings->m_Position[0] + (pSettings->m_Size / 2),
			//	pSettings->m_Position[1] + (pSettings->m_Width / 2),
			//	&TargetColor);
			//DrawBox(pSettings->m_Position[0] - (pSettings->m_Width / 2),
			//	pSettings->m_Position[1] - (pSettings->m_Size / 2),
			//	pSettings->m_Position[0] + (pSettings->m_Width / 2),
			//	pSettings->m_Position[1] + (pSettings->m_Size / 2),
			//	&TargetColor);
	}
}

void CCrosshair::_SettingChanged() {
	SFrameInfo* pFrameInfo = COverlay::Get()->GetFrameInfo();
	if (!pFrameInfo)
		return;
	pFrameInfo->m_Frame++;
}