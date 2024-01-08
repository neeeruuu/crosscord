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

void CCrosshair::DrawBox(unsigned int iStartX, unsigned int iStartY, unsigned int iEndX, unsigned int iEndY, SColor* Col) {
	SFrameInfo* pFrameInfo = COverlay::Get()->GetFrameInfo();

	if (iStartX > pFrameInfo->m_Width || iEndX > pFrameInfo->m_Width)
		return;
	if (iStartY > pFrameInfo->m_Height || iEndY > pFrameInfo->m_Height)
		return;

	SColor* pPixelBuffer = reinterpret_cast<SColor*>(pFrameInfo->m_Pixels);
	for (unsigned int iX = iStartX; iX < iEndX; iX++) {
		for (unsigned int iY = iStartY; iY < iEndY; iY++) {
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

			// horizontal lines
			iStartY = pSettings->m_Position[1] - pSettings->m_Width;
			iEndY = pSettings->m_Position[1] + 1 + pSettings->m_Width;

			iStartX = pSettings->m_Position[0] - pSettings->m_Gap - (pSettings->m_Size / 2);
			iEndX = pSettings->m_Position[0] - pSettings->m_Gap;
			DrawBox(iStartX, iStartY, iEndX, iEndY, &TargetColor); // left

			iStartX = pSettings->m_Position[0] + 1 + pSettings->m_Gap;
			iEndX = pSettings->m_Position[0] + 1 + pSettings->m_Gap + (pSettings->m_Size / 2);
			DrawBox(iStartX, iStartY, iEndX, iEndY, &TargetColor); // right

			// vertical lines
			iStartX = pSettings->m_Position[0] - pSettings->m_Width;
			iEndX = pSettings->m_Position[0] + 1 + pSettings->m_Width;
			
			if (!pSettings->m_TStyle) {
				iStartY = pSettings->m_Position[1] - pSettings->m_Gap - (pSettings->m_Size / 2);
				iEndY = pSettings->m_Position[1] - pSettings->m_Gap;
				DrawBox(iStartX, iStartY, iEndX, iEndY, &TargetColor); // top
			}

			iStartY = pSettings->m_Position[1] + 1 + pSettings->m_Gap;
			iEndY = pSettings->m_Position[1] + 1 + pSettings->m_Gap + (pSettings->m_Size / 2);
			DrawBox(iStartX, iStartY, iEndX, iEndY, &TargetColor); // bottom

			if (pSettings->m_Dot) {
				iStartX = pSettings->m_Position[0] - pSettings->m_Width;
				iStartY = pSettings->m_Position[1] - pSettings->m_Width;
				iEndX = pSettings->m_Position[0] + pSettings->m_Width + 1;
				iEndY = pSettings->m_Position[1] + pSettings->m_Width + 1;
				DrawBox(iStartX, iStartY, iEndX, iEndY, &TargetColor);
			}
			break;
		}
		case CROSSHAIR_ARROW: {
			SColor* pPixelBuffer = reinterpret_cast<SColor*>(pFrameInfo->m_Pixels);
			for (int iDiag = 0; iDiag < pSettings->m_Size; iDiag++) {
				int iBaseX, iBaseY;

				iBaseX = pSettings->m_Position[0] + iDiag;
				iBaseY = pSettings->m_Position[1] + iDiag;
				for (int iX = -pSettings->m_Width; iX < pSettings->m_Width + 1; iX++)
					pPixelBuffer[iBaseY * pFrameInfo->m_Width + iBaseX + iX] = TargetColor;

				iBaseX = pSettings->m_Position[0] - iDiag;
				iBaseY = pSettings->m_Position[1] + iDiag;
				for (int iX = -pSettings->m_Width; iX < pSettings->m_Width + 1; iX++)
					pPixelBuffer[iBaseY * pFrameInfo->m_Width + iBaseX + iX] = TargetColor;

			}
		}
	}
}

void CCrosshair::_SettingChanged() {
	SFrameInfo* pFrameInfo = COverlay::Get()->GetFrameInfo();
	if (!pFrameInfo)
		return;
	pFrameInfo->m_Frame++;
}