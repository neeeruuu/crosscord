#include "crosshair.h"
#include "config.h"
#include "overlay.h"

#include "util/log.h"

#include <spng.h>

INSTANTIATE_SINGLETON(CCrosshair);

/*
	--------- module implementation ---------
*/
bool CCrosshair::Initialize() {
	m_Callbacks.push_back(
		g_CB_OverlayDraw->Register([](SFrameInfo* pFrameInfo) {
			CCrosshair::Get()->Clear(pFrameInfo);
			CCrosshair::Get()->Draw(pFrameInfo);
		}, true)
	);

	m_Callbacks.push_back(
		g_CB_OnConfigLoad->Register([](nlohmann::json* pJSON) {
			CCrosshair::Get()->LoadConfig(pJSON);
		}, true)
	);

	m_Callbacks.push_back(
		g_CB_OnConfigSave->Register([](nlohmann::json* pJSON) {
			CCrosshair::Get()->SaveConfig(*pJSON);
			}, true)
	);

	m_PrevSettings.m_Type = CROSSHAIR_UNINITIALIZED;

	return true;
}

bool CCrosshair::Shutdown() {
	SFrameInfo* pFrameInfo = COverlay::Get()->GetFrameInfo();
	if (!pFrameInfo) return false;
	CCrosshair::Get()->Clear(pFrameInfo);
	pFrameInfo->m_Frame++;

	for (CCallback* Callback : m_Callbacks) { delete Callback; }
	m_Callbacks.clear();
	return true;
}

/*
	--------- crosshair implementation ---------
*/
void CCrosshair::Draw(SFrameInfo* pFrameInfo) {
	if (!m_Settings.m_Enabled)
		return;

	SColor Color{
		static_cast<unsigned char>(m_Settings.m_Color[2] * 255),
		static_cast<unsigned char>(m_Settings.m_Color[1] * 255),
		static_cast<unsigned char>(m_Settings.m_Color[0] * 255),
		static_cast<unsigned char>(m_Settings.m_Color[3] * 255)
	};

	unsigned int iPosX = (pFrameInfo->m_Width / 2) + m_Settings.m_Offset[0];
	unsigned int iPosY = (pFrameInfo->m_Height / 2) + m_Settings.m_Offset[1];

	if (iPosX > pFrameInfo->m_Width || iPosY > pFrameInfo->m_Height)
		return;

	switch (m_PrevSettings.m_Type) {
		case CROSSHAIR_CROSS:
			DrawCross(pFrameInfo, &m_PrevSettings, iPosX, iPosY, &Color);
			break;
		case CROSSHAIR_CIRCLE:
			DrawCircle(pFrameInfo, &m_PrevSettings, iPosX, iPosY, &Color);
			break;
		case CROSSHAIR_ARROW:
			DrawArrow(pFrameInfo, &m_PrevSettings, iPosX, iPosY, &Color);
			break;
		case CROSSHAIR_IMAGE:
			DrawImage(pFrameInfo, &m_PrevSettings, iPosX, iPosY, &Color);
			break;
	}
}

void CCrosshair::Clear(SFrameInfo* pFrameInfo) {
	if (m_PrevSettings.m_Type == CROSSHAIR_UNINITIALIZED) {
		memcpy(&m_PrevSettings, &m_Settings, sizeof(SCrosshairSettings));
		return;
	}

	SColor Color{ 0 };

	m_PrevSettings.m_ImageSettings.m_Buffer = 0;

	unsigned int iPosX = (pFrameInfo->m_Width / 2) + m_PrevSettings.m_Offset[0];
	unsigned int iPosY = (pFrameInfo->m_Height / 2) + m_PrevSettings.m_Offset[1];
	
	if (iPosX < pFrameInfo->m_Width && iPosY < pFrameInfo->m_Height && m_PrevSettings.m_Enabled) {
		switch (m_PrevSettings.m_Type) {
			case CROSSHAIR_CROSS:
				DrawCross(pFrameInfo, &m_PrevSettings, iPosX, iPosY, &Color);
				break;
			case CROSSHAIR_CIRCLE:
				DrawCircle(pFrameInfo, &m_PrevSettings, iPosX, iPosY, &Color);
				break;
			case CROSSHAIR_ARROW:
				DrawArrow(pFrameInfo, &m_PrevSettings, iPosX, iPosY, &Color);
				break;
			case CROSSHAIR_IMAGE:
				DrawImage(pFrameInfo, &m_PrevSettings, iPosX, iPosY, &Color);
				break;
		}
	}

	memcpy(&m_PrevSettings, &m_Settings, sizeof(SCrosshairSettings));
}

void CCrosshair::Refresh() {
	SFrameInfo* pFrameInfo = COverlay::Get()->GetFrameInfo();
	if (!pFrameInfo) return;
	pFrameInfo->m_Frame++;
}

void CCrosshair::DrawRect(SFrameInfo* pFrameInfo, unsigned int iStartX, unsigned int iStartY, unsigned int iEndX, unsigned int iEndY, SColor* pColor) {
	if (iStartX >= pFrameInfo->m_Width || iEndX > pFrameInfo->m_Width || 
		iStartY >= pFrameInfo->m_Height || iEndY > pFrameInfo->m_Height)
		return;

	SColor* pPixelBuffer = reinterpret_cast<SColor*>(pFrameInfo->m_Pixels);
	for (unsigned int iX = iStartX; iX < iEndX; iX++) {
		for (unsigned int iY = iStartY; iY < iEndY; iY++) {
			pPixelBuffer[iY * pFrameInfo->m_Width + iX] = *pColor;
		}
	}
}

void CCrosshair::DrawCross(SFrameInfo* pFrameInfo, SCrosshairSettings* pSettings, unsigned int iCenterX, unsigned int iCenterY, SColor* pColor) {
	unsigned int iStartX, iStartY;
	unsigned int iEndX, iEndY;

	// horizontal lines
	iStartY = iCenterY - pSettings->m_CrossSettings.m_Width;
	iEndY = iCenterY + 1 + pSettings->m_CrossSettings.m_Width;

	iStartX = iCenterX - pSettings->m_CrossSettings.m_Gap - (pSettings->m_CrossSettings.m_Length / 2);
	iEndX = iCenterX - pSettings->m_CrossSettings.m_Gap;
	DrawRect(pFrameInfo, iStartX, iStartY, iEndX, iEndY, pColor); // left

	iStartX = iCenterX + 1 + pSettings->m_CrossSettings.m_Gap;
	iEndX = iCenterX + 1 + pSettings->m_CrossSettings.m_Gap + (pSettings->m_CrossSettings.m_Length / 2);
	DrawRect(pFrameInfo, iStartX, iStartY, iEndX, iEndY, pColor); // right

	// vertical lines
	iStartX = iCenterX - pSettings->m_CrossSettings.m_Width;
	iEndX = iCenterX + 1 + pSettings->m_CrossSettings.m_Width;

	if (!pSettings->m_CrossSettings.m_TStyle) {
		iStartY = iCenterY - pSettings->m_CrossSettings.m_Gap - (pSettings->m_CrossSettings.m_Length / 2);
		iEndY = iCenterY - pSettings->m_CrossSettings.m_Gap;
		DrawRect(pFrameInfo, iStartX, iStartY, iEndX, iEndY, pColor); // top
	}

	iStartY = iCenterY + 1 + pSettings->m_CrossSettings.m_Gap;
	iEndY = iCenterY + 1 + pSettings->m_CrossSettings.m_Gap + (pSettings->m_CrossSettings.m_Length / 2);
	DrawRect(pFrameInfo, iStartX, iStartY, iEndX, iEndY, pColor); // bottom

	if (pSettings->m_CrossSettings.m_Dot) {
		iStartX = iCenterX - pSettings->m_CrossSettings.m_Width;
		iStartY = iCenterY - pSettings->m_CrossSettings.m_Width;
		iEndX = iCenterX + pSettings->m_CrossSettings.m_Width + 1;
		iEndY = iCenterY + pSettings->m_CrossSettings.m_Width + 1;
		DrawRect(pFrameInfo, iStartX, iStartY, iEndX, iEndY, pColor);
	}
}

void CCrosshair::DrawCircle(SFrameInfo* pFrameInfo, SCrosshairSettings* pSettings, unsigned int iCenterX, unsigned int iCenterY, SColor* pColor) {
	SColor* pPixelBuffer = reinterpret_cast<SColor*>(pFrameInfo->m_Pixels);

	if (pSettings->m_CircleSettings.m_Hollow) {
		unsigned int iXCoord = pSettings->m_CircleSettings.m_Radius;
		unsigned int iYCoord = 0;
		int iDecision = 1 - pSettings->m_CircleSettings.m_Radius;

		while (iXCoord >= iYCoord) {
			pPixelBuffer[(iCenterY + iYCoord) * pFrameInfo->m_Width + (iCenterX + iXCoord)] = *pColor;
			pPixelBuffer[(iCenterY + iYCoord) * pFrameInfo->m_Width + (iCenterX - iXCoord)] = *pColor;
			pPixelBuffer[(iCenterY - iYCoord) * pFrameInfo->m_Width + (iCenterX + iXCoord)] = *pColor;
			pPixelBuffer[(iCenterY - iYCoord) * pFrameInfo->m_Width + (iCenterX - iXCoord)] = *pColor;

			pPixelBuffer[(iCenterY + iXCoord) * pFrameInfo->m_Width + (iCenterX + iYCoord)] = *pColor;
			pPixelBuffer[(iCenterY + iXCoord) * pFrameInfo->m_Width + (iCenterX - iYCoord)] = *pColor;
			pPixelBuffer[(iCenterY - iXCoord) * pFrameInfo->m_Width + (iCenterX + iYCoord)] = *pColor;
			pPixelBuffer[(iCenterY - iXCoord) * pFrameInfo->m_Width + (iCenterX - iYCoord)] = *pColor;


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
		for (int iY = -static_cast<int>(pSettings->m_CircleSettings.m_Radius); iY <= static_cast<int>(pSettings->m_CircleSettings.m_Radius); ++iY) {
			for (int iX = -static_cast<int>(pSettings->m_CircleSettings.m_Radius); iX <= static_cast<int>(pSettings->m_CircleSettings.m_Radius); ++iX) {
				unsigned int iDistSquared = static_cast<unsigned int>(iX * iX + iY * iY);

				if (iDistSquared <= pSettings->m_CircleSettings.m_Radius * pSettings->m_CircleSettings.m_Radius) {
					unsigned int iDrawX = iCenterX + iX;
					unsigned int iDrawY = iCenterY + iY;

					if (iDrawX < pFrameInfo->m_Width && iDrawY < pFrameInfo->m_Height)
						pPixelBuffer[iDrawY * pFrameInfo->m_Width + iDrawX] = *pColor;
				}
			}
		}
	}
}

void CCrosshair::DrawArrow(SFrameInfo* pFrameInfo, SCrosshairSettings* pSettings, unsigned int iCenterX, unsigned int iCenterY, SColor* pColor) {
	SColor* pPixelBuffer = reinterpret_cast<SColor*>(pFrameInfo->m_Pixels);
	if (iCenterX + pSettings->m_ArrowSettings.m_Length > pFrameInfo->m_Width || iCenterY + pSettings->m_ArrowSettings.m_Length + pSettings->m_ArrowSettings.m_Width > pFrameInfo->m_Height)
		return;

	for (unsigned int iDiag = 0; iDiag < pSettings->m_ArrowSettings.m_Length; iDiag++) {
		unsigned int iBaseX, iY;
		iY = iCenterY + iDiag;

		iBaseX = iCenterX + iDiag;
		for (int iX = -static_cast<int>(pSettings->m_ArrowSettings.m_Width); iX < static_cast<int>(pSettings->m_ArrowSettings.m_Width + 1); iX++)
			pPixelBuffer[iY * pFrameInfo->m_Width + iBaseX + iX] = *pColor;

		iBaseX = iCenterX - iDiag;
		for (int iX = -static_cast<int>(pSettings->m_ArrowSettings.m_Width); iX < static_cast<int>(pSettings->m_ArrowSettings.m_Width + 1); iX++)
			pPixelBuffer[iY * pFrameInfo->m_Width + iBaseX + iX] = *pColor;
	}
}

void CCrosshair::DrawImage(SFrameInfo* pFrameInfo, SCrosshairSettings* pSettings, unsigned int iCenterX, unsigned int iCenterY, SColor* pColor) {
	if (!pSettings->m_ImageSettings.m_Buffer) {
		unsigned int iStartX = iCenterX - (pSettings->m_ImageSettings.m_Width / 2);
		unsigned int iStartY = iCenterY - (pSettings->m_ImageSettings.m_Height / 2);

		unsigned int iEndX = iCenterX + (pSettings->m_ImageSettings.m_Width / 2);
		unsigned int iEndY = iCenterY + (pSettings->m_ImageSettings.m_Height / 2);

		DrawRect(pFrameInfo, iStartX, iStartY, iEndX, iEndY, pColor);
		return;
	}

	SColor* pPixelBuffer = reinterpret_cast<SColor*>(pFrameInfo->m_Pixels);

	float fScale = pSettings->m_ImageSettings.m_Size / 1.f;
	unsigned int iUnscaledY, iUnscaledX = 0;

	ImageLock.lock();
	unsigned int iScaledWidth = static_cast<unsigned int>(static_cast<float>(pSettings->m_ImageSettings.m_Width) * fScale);
	unsigned int iScaledHeight = static_cast<unsigned int>(static_cast<float>(pSettings->m_ImageSettings.m_Height) * fScale);
	for (unsigned int iY = 0; iY < iScaledHeight; ++iY) {
		for (unsigned int iX = 0; iX < iScaledWidth; ++iX) {
			unsigned int iDestX = iCenterX + (iX - iScaledWidth / 2);
			unsigned int iDestY = iCenterY + (iY - iScaledHeight / 2);

			if (iDestX <= pFrameInfo->m_Width && iDestY <= pFrameInfo->m_Height) {
				iUnscaledY = static_cast<unsigned int>(static_cast<float>(iY) / fScale);
				iUnscaledX = static_cast<unsigned int>(static_cast<float>(iX) / fScale);

				SColor pImageCol = reinterpret_cast<SColor*>(pSettings->m_ImageSettings.m_Buffer)[iUnscaledY * pSettings->m_ImageSettings.m_Width + iUnscaledX];
				SColor* pPixel = &pPixelBuffer[iDestY * pFrameInfo->m_Width + iDestX];
				pPixel->r = pImageCol.b;
				pPixel->g = pImageCol.g;
				pPixel->b = pImageCol.r;
				pPixel->a = static_cast<unsigned char>(pImageCol.a * pSettings->m_ImageSettings.m_Alpha);
			}
		}
	}
	ImageLock.unlock();
}

void CCrosshair::LoadImg(const char* cPath) {
	FILE* pFile = nullptr;
	size_t lImageSize = 0;
	void* pImageBuffer = nullptr;

	if (fopen_s(&pFile, cPath, "rb") || !pFile) {
		LogError("Unable to open file: {}", cPath);
		return;
	}

	spng_ctx* pContext = spng_ctx_new(0);
	if (spng_set_png_file(pContext, pFile)) {
		LogError("Failed to set decoder png for file: {}", cPath);
		spng_ctx_free(pContext);
		return;
	}

	if (spng_decoded_image_size(pContext, SPNG_FMT_RGBA8, &lImageSize)) {
		LogError("Failed to get decoded image size for file: {}", cPath);
		spng_ctx_free(pContext);
		return;
	}
	pImageBuffer = malloc(lImageSize);

	if (!pImageBuffer) {
		LogError("Failed to allocate memory for png");
		spng_ctx_free(pContext);
		return;
	}

	if (spng_decode_image(pContext, pImageBuffer, lImageSize, SPNG_FMT_RGBA8, 0)) {
		LogError("Failed to decode image for file: {}", cPath);
		spng_ctx_free(pContext);
		return;
	}

	spng_ihdr ImgHeader;
	if (spng_get_ihdr(pContext, &ImgHeader)) {
		LogError("Failed to get header for file: {}", cPath);
		spng_ctx_free(pContext);
		return;
	}

	ImageLock.lock();
	if (m_Settings.m_ImageSettings.m_Buffer)
		free(m_Settings.m_ImageSettings.m_Buffer);
	m_Settings.m_ImageSettings.m_Buffer = pImageBuffer;
	m_Settings.m_ImageSettings.m_Height = ImgHeader.height;
	m_Settings.m_ImageSettings.m_Width = ImgHeader.width;
	ImageLock.unlock();

	g_CB_CrosshairImageLoaded->Run(pImageBuffer, ImgHeader.width, ImgHeader.height);

	spng_ctx_free(pContext);
	fclose(pFile);
}

void CCrosshair::LoadConfig(nlohmann::json* pJSON) {
	JSONGET("enabled", m_Settings.m_Enabled, pJSON);
	JSONGET("type", m_Settings.m_Type, pJSON);
	JSONGET("color", m_Settings.m_Color, pJSON);
	JSONGET("offset", m_Settings.m_Offset, pJSON);

	JSONGET("cross_tstyle", m_Settings.m_CrossSettings.m_TStyle, pJSON);
	JSONGET("cross_dot", m_Settings.m_CrossSettings.m_Dot, pJSON);
	JSONGET("cross_length", m_Settings.m_CrossSettings.m_Length, pJSON);
	JSONGET("cross_width", m_Settings.m_CrossSettings.m_Width, pJSON);
	JSONGET("cross_gap", m_Settings.m_CrossSettings.m_Gap, pJSON);

	JSONGET("circle_hollow", m_Settings.m_CircleSettings.m_Hollow, pJSON);
	JSONGET("circle_radius", m_Settings.m_CircleSettings.m_Radius, pJSON);

	JSONGET("arrow_length", m_Settings.m_ArrowSettings.m_Length, pJSON);
	JSONGET("arrow_width", m_Settings.m_ArrowSettings.m_Width, pJSON);
}

void CCrosshair::SaveConfig(nlohmann::json &pJSON) {
	pJSON["enabled"] = m_Settings.m_Enabled;
	pJSON["type"] = m_Settings.m_Type;
	pJSON["color"] = m_Settings.m_Color;
	pJSON["offset"] = m_Settings.m_Offset;

	pJSON["cross_tstyle"] = m_Settings.m_CrossSettings.m_TStyle;
	pJSON["cross_dot"] = m_Settings.m_CrossSettings.m_Dot;
	pJSON["cross_length"] = m_Settings.m_CrossSettings.m_Length;
	pJSON["cross_width"] = m_Settings.m_CrossSettings.m_Width;
	pJSON["cross_gap"] = m_Settings.m_CrossSettings.m_Gap;

	pJSON["circle_hollow"] = m_Settings.m_CircleSettings.m_Hollow;
	pJSON["circle_radius"] = m_Settings.m_CircleSettings.m_Radius;

	pJSON["arrow_length"] = m_Settings.m_ArrowSettings.m_Length;
	pJSON["arrow_width"] = m_Settings.m_ArrowSettings.m_Width;
}