#include "crosshair.h"
#include "overlay.h"
#include "config.h"

INITIALIZE_SINGLETON(CCrosshair);

bool CCrosshair::Initialize() {
	m_DrawCB = g_CB_OverlayDraw->Register([](SFrameInfo* pFrameInfo) {
		CCrosshair* pCrosshair = CCrosshair::Get();

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

void CCrosshair::_Draw(SFrameInfo* pFrameInfo, SCrosshairSettings* pSettings) {
	if (!pSettings->m_Enabled)
		return;

	SColor TargetColor{
		static_cast<unsigned char>(pSettings->m_Color[2] * 255),
		static_cast<unsigned char>(pSettings->m_Color[1] * 255),
		static_cast<unsigned char>(pSettings->m_Color[0] * 255),
		static_cast<unsigned char>(pSettings->m_Color[3] * 255)
	};

	int iPosX = (pFrameInfo->m_Width / 2) + pSettings->m_Offset[0];
	int iPosY = (pFrameInfo->m_Height / 2) + pSettings->m_Offset[1];

	switch (pSettings->m_Type) {
		case CROSSHAIR_CROSS: {
			int iStartX, iStartY;
			int iEndX, iEndY;

			// horizontal lines
			iStartY = iPosY - pSettings->m_CrossWidth;
			iEndY = iPosY + 1 + pSettings->m_CrossWidth;

			iStartX = iPosX - pSettings->m_CrossGap - (pSettings->m_CrossLength / 2);
			iEndX = iPosX - pSettings->m_CrossGap;
			DrawBox(iStartX, iStartY, iEndX, iEndY, &TargetColor); // left

			iStartX = iPosX + 1 + pSettings->m_CrossGap;
			iEndX = iPosX + 1 + pSettings->m_CrossGap + (pSettings->m_CrossLength / 2);
			DrawBox(iStartX, iStartY, iEndX, iEndY, &TargetColor); // right

			// vertical lines
			iStartX = iPosX - pSettings->m_CrossWidth;
			iEndX = iPosX + 1 + pSettings->m_CrossWidth;

			if (!pSettings->m_CrossTStyle) {
				iStartY = iPosY - pSettings->m_CrossGap - (pSettings->m_CrossLength / 2);
				iEndY = iPosY - pSettings->m_CrossGap;
				DrawBox(iStartX, iStartY, iEndX, iEndY, &TargetColor); // top
			}

			iStartY = iPosY + 1 + pSettings->m_CrossGap;
			iEndY = iPosY + 1 + pSettings->m_CrossGap + (pSettings->m_CrossLength / 2);
			DrawBox(iStartX, iStartY, iEndX, iEndY, &TargetColor); // bottom

			if (pSettings->m_CrossDot) {
				iStartX = iPosX - pSettings->m_CrossWidth;
				iStartY = iPosY - pSettings->m_CrossWidth;
				iEndX = iPosX + pSettings->m_CrossWidth + 1;
				iEndY = iPosY + pSettings->m_CrossWidth + 1;
				DrawBox(iStartX, iStartY, iEndX, iEndY, &TargetColor);
			}
			break;
		}
		case CROSSHAIR_ARROW: {
			SColor* pPixelBuffer = reinterpret_cast<SColor*>(pFrameInfo->m_Pixels);
			for (int iDiag = 0; iDiag < pSettings->m_ArrowLength; iDiag++) {
				int iBaseX, iBaseY;
				iBaseY = iPosY + iDiag;

				iBaseX = iPosX + iDiag;
				for (int iX = -pSettings->m_ArrowWidth; iX < pSettings->m_ArrowWidth + 1; iX++)
					pPixelBuffer[iBaseY * pFrameInfo->m_Width + iBaseX + iX] = TargetColor;

				iBaseX = iPosX - iDiag;
				for (int iX = -pSettings->m_ArrowWidth; iX < pSettings->m_ArrowWidth + 1; iX++)
					pPixelBuffer[iBaseY * pFrameInfo->m_Width + iBaseX + iX] = TargetColor;
			}

			break;
		}
		case CROSSHAIR_CIRCLE: {
			break;
		}
	}
}

void CCrosshair::_SettingChanged() {
	SFrameInfo* pFrameInfo = COverlay::Get()->GetFrameInfo();
	if (!pFrameInfo)
		return;
	CConfigManager::Get()->SaveConfig("config");
	pFrameInfo->m_Frame++;
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
