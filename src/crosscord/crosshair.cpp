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
		pCrosshair->_m_PrevSettings.m_ImageBuffer = nullptr;
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

	unsigned int iPosX = (pFrameInfo->m_Width / 2) + pSettings->m_Offset[0];
	unsigned int iPosY = (pFrameInfo->m_Height / 2) + pSettings->m_Offset[1];

	switch (pSettings->m_Type) {
		case CROSSHAIR_CROSS: {
			unsigned int iStartX, iStartY;
			unsigned int iEndX, iEndY;

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
			if (static_cast<int>(iPosX) - static_cast<int>(pSettings->m_ArrowLength) < 0)
				return;

			for (unsigned int iDiag = 0; iDiag < pSettings->m_ArrowLength; iDiag++) {
				unsigned int iBaseX, iY;
				iY = iPosY + iDiag;

				iBaseX = iPosX + iDiag;
				for (int iX = -static_cast<int>(pSettings->m_ArrowWidth); iX < static_cast<int>(pSettings->m_ArrowWidth + 1); iX++)
					pPixelBuffer[iY * pFrameInfo->m_Width + iBaseX + iX] = TargetColor;

				iBaseX = iPosX - iDiag;
				for (int iX = -static_cast<int>(pSettings->m_ArrowWidth); iX < static_cast<int>(pSettings->m_ArrowWidth + 1); iX++)
					pPixelBuffer[iY * pFrameInfo->m_Width + iBaseX + iX] = TargetColor;
			}

			break;
		}
		case CROSSHAIR_CIRCLE: {
			SColor* pPixelBuffer = reinterpret_cast<SColor*>(pFrameInfo->m_Pixels);
			if (pSettings->m_CircleHollow) {
				unsigned int iXCoord = pSettings->m_CircleRadius;
				unsigned int iYCoord = 0;
				int iDecision = 1 - pSettings->m_CircleRadius;

				while (iXCoord >= iYCoord) {

					pPixelBuffer[(iPosY + iYCoord) * pFrameInfo->m_Width + (iPosX + iXCoord)] = TargetColor;
					pPixelBuffer[(iPosY + iYCoord) * pFrameInfo->m_Width + (iPosX - iXCoord)] = TargetColor;
					pPixelBuffer[(iPosY - iYCoord) * pFrameInfo->m_Width + (iPosX + iXCoord)] = TargetColor;
					pPixelBuffer[(iPosY - iYCoord) * pFrameInfo->m_Width + (iPosX - iXCoord)] = TargetColor;

					pPixelBuffer[(iPosY + iXCoord) * pFrameInfo->m_Width + (iPosX + iYCoord)] = TargetColor;
					pPixelBuffer[(iPosY + iXCoord) * pFrameInfo->m_Width + (iPosX - iYCoord)] = TargetColor;
					pPixelBuffer[(iPosY - iXCoord) * pFrameInfo->m_Width + (iPosX + iYCoord)] = TargetColor;
					pPixelBuffer[(iPosY - iXCoord) * pFrameInfo->m_Width + (iPosX - iYCoord)] = TargetColor;


					iYCoord++;

					if (iDecision <= 0)
						iDecision = iDecision + 2 * static_cast<int>(iYCoord) + 1;
					else {
						iXCoord--;
						iDecision = iDecision + 2 * static_cast<int>(iYCoord - iXCoord) + 1;
					}
				}
			}
			else {
				for (int iY = -static_cast<int>(pSettings->m_CircleRadius); iY <= static_cast<int>(pSettings->m_CircleRadius); ++iY) {
					for (int iX = -static_cast<int>(pSettings->m_CircleRadius); iX <= static_cast<int>(pSettings->m_CircleRadius); ++iX) {
						unsigned int iDistSquared = static_cast<unsigned int>(iX * iX + iY * iY);

						if (iDistSquared <= pSettings->m_CircleRadius * pSettings->m_CircleRadius) {
							unsigned int iDrawX = iPosX + iX;
							unsigned int iDrawY = iPosY + iY;

							if (iDrawX < pFrameInfo->m_Width && iDrawY < pFrameInfo->m_Height)
								pPixelBuffer[iDrawY * pFrameInfo->m_Width + iDrawX] = TargetColor;
						}
					}
				}
			}
			break;
		}
		case CROSSHAIR_IMAGE: {
			if (!pSettings->m_ImageBuffer) {
				unsigned int iStartX = iPosX - (pSettings->m_ImageHeight / 2);
				unsigned int iStartY = iPosY - (pSettings->m_ImageWidth / 2);

				unsigned int iEndX = iPosX + (pSettings->m_ImageHeight / 2);
				unsigned int iEndY = iPosY + (pSettings->m_ImageWidth / 2);

				DrawBox(iStartX, iStartY, iEndX, iEndY, &TargetColor);
			}
			else {
				SColor* pPixelBuffer = reinterpret_cast<SColor*>(pFrameInfo->m_Pixels);
				ImageMutex.lock();

				float fScale = pSettings->m_ImageSize / 1.f;
				
				unsigned int iScaledWidth = static_cast<unsigned int>(static_cast<float>(pSettings->m_ImageWidth) * fScale);
				unsigned int iScaledHeight = static_cast<unsigned int>(static_cast<float>(pSettings->m_ImageHeight) * fScale);

				unsigned int iUnscaledY, iUnscaledX = 0;

				for (unsigned int iY = 0; iY < iScaledHeight; ++iY) {
					for (unsigned int iX = 0; iX < iScaledWidth; ++iX) {
						unsigned int iDestX = iPosX + (iX - iScaledWidth / 2);
						unsigned int iDestY = iPosY + (iY - iScaledHeight / 2);

						if (iDestX <= pFrameInfo->m_Width && iDestY <= pFrameInfo->m_Height) {
							iUnscaledY = static_cast<unsigned int>(static_cast<float>(iY) / fScale);
							iUnscaledX = static_cast<unsigned int>(static_cast<float>(iX) / fScale);

							SColor pImageCol = reinterpret_cast<SColor*>(pSettings->m_ImageBuffer)[iUnscaledY * pSettings->m_ImageWidth + iUnscaledX];
							SColor* pPixel = &pPixelBuffer[iDestY * pFrameInfo->m_Width + iDestX];
							pPixel->r = pImageCol.b;
							pPixel->g = pImageCol.g;
							pPixel->b = pImageCol.r;
							pPixel->a = static_cast<unsigned char>(pImageCol.a * pSettings->m_ImageAlpha);
						}
					}
				}
				ImageMutex.unlock();
			}
			break;
		}
	}
}

void CCrosshair::_SettingChanged() {
	SFrameInfo* pFrameInfo = COverlay::Get()->GetFrameInfo();
	CConfigManager::Get()->SaveConfig("config");
	if (!pFrameInfo)
		return;
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

void CCrosshair::SetImageBuffer(void* pBuffer, unsigned long long lSize, unsigned int iWidth, unsigned int iHeight) {
	ImageMutex.lock();
	if (m_Settings.m_ImageBuffer)
		free(m_Settings.m_ImageBuffer);
	m_Settings.m_ImageBuffer = malloc(lSize);
	if (!m_Settings.m_ImageBuffer) {
		ImageMutex.unlock();
		return;
	}

	memcpy(m_Settings.m_ImageBuffer, pBuffer, lSize);

	m_Settings.m_ImageWidth = iWidth;
	m_Settings.m_ImageHeight = iHeight;
	ImageMutex.unlock();
}