

#include "stdafx.h"
#include "overlayMarqueeText.h"
#include "resource.h"
#include<iostream>
#include<fstream>
#include <sys/stat.h>

// add your own path for PlayClaw5.lib
#ifdef _DEBUG
#pragma comment(lib, "../playclaw5.lib")
#else
#pragma comment(lib, "../playclaw5.lib")
#endif

HINSTANCE GetDllInstance();

DWORD m_dwPluginID = -1;		// unique (realtime) plugin ID

PluginOverlayGdiPlusHelper	*pRenderHelper = 0;

Gdiplus::SolidBrush*	pTextBrush = 0;
Gdiplus::SolidBrush*	pBackBrush = 0;
Gdiplus::Font*			pTextFont = 0;
Gdiplus::Pen*			pGraphPen = 0;
Gdiplus::Pen*			pAxisPen = 0;

Gdiplus::Font*			pFPSFont = 0;
Gdiplus::SolidBrush*	pFPSNormalBrush = 0;
Gdiplus::SolidBrush*	pFPSRecordBrush = 0;
Gdiplus::SolidBrush*	pFPSBackBrush = 0;
Gdiplus::SolidBrush*	pFPSScreenIndicatorBrush = 0;

BOOL					bShowBackground = true;
wchar_t trig[256];
BOOL					bFirstAttach = true;
BOOL					bTriggered = false;

int						cycleCount = 0;
int						scroll = 0;
int						textLength = 0;
time_t					oldModifyTime;
#define SCREENSHOT_INDICATOR_FPS_COLOR 1

//
//	Init plugin
//
PLUGIN_EXPORT DWORD PluginInit(DWORD dwPluginID)
{
	m_dwPluginID = dwPluginID;

	// Initialize GDI+.
	pRenderHelper = new PluginOverlayGdiPlusHelper(dwPluginID);

	pBackBrush = new Gdiplus::SolidBrush(Gdiplus::Color(100, 0, 0, 0));
	pAxisPen = new Gdiplus::Pen(Gdiplus::Color(100, 0, 170, 70));
	pGraphPen = new Gdiplus::Pen(Gdiplus::Color(255, 0, 200, 70));
	pTextBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, 255, 255, 255));
	pTextFont = new Gdiplus::Font(L"System", 8, Gdiplus::FontStyleBold);

	pFPSBackBrush = new Gdiplus::SolidBrush(Gdiplus::Color(200, 0, 0, 0));
	return PC_PLUGIN_FLAG_IS_ACTIVE | PC_PLUGIN_FLAG_IS_OVERLAY;	// plugin is active and has overlay features
}

void startRecording()
{
	while (bFirstAttach) {
		if (bFirstAttach && PC_GetConfirmedProcessID() != -1) {
			bFirstAttach = false;
			if (!PC_IsRecording()) {
				PC_StartRecording();
				bTriggered = true;
			}
		}
	}

}

PLUGIN_EXPORT const char* PluginGetTitle()
{
	return "TEXT_MARQUEE_PLUGIN";
}

//
//	Set default variables
//
PLUGIN_EXPORT void PluginSetDefaultVars()
{
	PC_SetPluginVar(m_dwPluginID, VAR_TEXT_COLOR, RGB(255, 255, 255));
	PC_SetPluginVar(m_dwPluginID, VAR_BACKGROUND_COLOR, 0);
	PC_SetPluginVar(m_dwPluginID, VAR_FPS_FONT_FAMILY, L"Consolas");
	PC_SetPluginVar(m_dwPluginID, VAR_FPS_FONT_SIZE, 24);
	PC_SetPluginVar(m_dwPluginID, VAR_FPS_FONT_STYLE, (int)0);	// flags: 1 - bold, 2 - italic
	PC_SetPluginVar(m_dwPluginID, VAR_SHOW_BACKGROUND, 1);
}

//
//	Update variables (after profile loading for example)
//
PLUGIN_EXPORT void PluginUpdateVars()
{
	DWORD clr;

	SAFE_DELETE(pFPSNormalBrush);
	clr = PC_GetPluginVarInt(m_dwPluginID, VAR_TEXT_COLOR);
	pFPSNormalBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, GetRValue(clr), GetGValue(clr), GetBValue(clr)));

	SAFE_DELETE(pFPSRecordBrush);
	clr = PC_GetPluginVarInt(m_dwPluginID, VAR_BACKGROUND_COLOR);
	pFPSRecordBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, GetRValue(clr), GetGValue(clr), GetBValue(clr)));


	bShowBackground = PC_GetPluginVarInt(m_dwPluginID, VAR_SHOW_BACKGROUND);
	SAFE_DELETE(pFPSFont);
	pFPSFont = new Gdiplus::Font(
		PC_GetPluginVarStr(m_dwPluginID, VAR_FPS_FONT_FAMILY),
		(float)PC_GetPluginVarInt(m_dwPluginID, VAR_FPS_FONT_SIZE),
		PC_GetPluginVarInt(m_dwPluginID, VAR_FPS_FONT_STYLE));


	struct stat attrib;
	stat("C:/Users/Koosemose/Desktop/Snip/Snip.txt", &attrib);
	time_t modifyTime = attrib.st_mtime;
	
	oldModifyTime = modifyTime;
	string line;
	ifstream myfile;
	myfile.open("C:/Users/Koosemose/Desktop/Snip/Snip.txt");
	string textContents = "";
	if (myfile.is_open())
	{
		while (getline(myfile, line))
		{
			textContents += line;
			//cout << line << '\n';
		}
		myfile.close();
	}
	textLength = textContents.length();
	trig[textLength] = 0;
	std::copy(textContents.begin(), textContents.end(), trig);
	


}

//
//	Shutdown plugin
//
PLUGIN_EXPORT void PluginShutdown()
{
	SAFE_DELETE(pFPSFont);
	SAFE_DELETE(pFPSNormalBrush);
	SAFE_DELETE(pFPSRecordBrush);
	SAFE_DELETE(pFPSBackBrush);
	SAFE_DELETE(pFPSScreenIndicatorBrush);

	SAFE_DELETE(pBackBrush);
	SAFE_DELETE(pAxisPen);
	SAFE_DELETE(pGraphPen);
	SAFE_DELETE(pTextBrush);
	SAFE_DELETE(pTextFont);

	SAFE_DELETE(pRenderHelper);
}

int max_fps = 110;
int min_fps = 5;
std::vector<int>	fps_array;

void GetFPSBounds(int max_fps, int &max_bound)
{
	max_bound = 1;
	while (max_fps > 9)
	{
		max_fps /= 10;
		max_bound *= 10;
	}
	max_fps++;	// next number
	max_bound *= max_fps;
}

//
//	Draw overlay 
//
PLUGIN_EXPORT void PluginUpdateOverlay()
{

	if (!pRenderHelper)
		return;

	// lock overlay image
	auto pLock = pRenderHelper->BeginFrame();
	if (!pLock)
		return;

	int w = pLock->dwWidth;
	int h = pLock->dwHeight;

	Gdiplus::Graphics *pGraphics = pRenderHelper->GetGraphics();

	//-----------------------------------------
	// draw Text Overlay

	// set options
	pGraphics->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);

	// clear back
	pGraphics->Clear(Gdiplus::Color(0, 0, 0, 0));

	WCHAR s[128];
	_itow_s(PC_GetConfirmedProcessID(), s, 10);
	struct stat attrib;
	stat("C:/Users/Koosemose/Desktop/Snip/Snip.txt", &attrib);
	time_t modifyTime = attrib.st_mtime;
	if (modifyTime > oldModifyTime) {
		oldModifyTime = modifyTime;
		string line;
		ifstream myfile;
		myfile.open("C:/Users/Koosemose/Desktop/Snip/Snip.txt");
		string textContents = "";
		if (myfile.is_open())
		{
			while (getline(myfile, line))
			{
				textContents += line;
			}
			myfile.close();
		}
		textLength = textContents.length();
		trig[textLength] = 0;
		std::copy(textContents.begin(), textContents.end(), trig);
	}
	// draw back if required
	if (bShowBackground)
	{
		Gdiplus::RectF bound;
		pRenderHelper->GetTextExtent(trig, pFPSFont, &bound);
		pGraphics->FillRectangle(pFPSRecordBrush, bound);
	}

	// draw text

	Gdiplus::RectF bound;
	pRenderHelper->GetTextExtent(trig, pFPSFont, &bound);

	if (bound.Width > w) {
		wchar_t newtrig[256];
		newtrig[textLength * 2 + 5] = 0;

		swprintf_s(newtrig, L"%s     %s", trig, trig);
		Gdiplus::RectF newbound;
		pRenderHelper->GetTextExtent(newtrig, pFPSFont, &newbound);
		if (PC_IsRecording())
			pRenderHelper->DrawString(newtrig, pFPSFont, Gdiplus::Point(0 - scroll, 0), pFPSRecordBrush, pFPSBackBrush);
		else
			pRenderHelper->DrawString(newtrig, pFPSFont, Gdiplus::Point(0 - scroll, 0), pFPSNormalBrush, pFPSBackBrush);
		scroll+=2;
		if (scroll > newbound.Width - bound.Width)
			scroll = 0;
	}
	else {
		if (PC_IsRecording())
			pRenderHelper->DrawString(trig, pFPSFont, Gdiplus::Point(0, 0), pFPSRecordBrush, pFPSBackBrush);
		else
			pRenderHelper->DrawString(trig, pFPSFont, Gdiplus::Point(0, 0), pFPSNormalBrush, pFPSBackBrush);
	}

	// fill overlay image
	pRenderHelper->EndFrame();

}

//////////////////////////////////////////////////////////////////////////

DWORD dwTextColor, dwBackgroundColor;
BOOL bItalic, bBold;
wstring szFontFamily;
DWORD dwFontSize;

static void SetFontButtonText(HWND dlg)
{
	WCHAR fontstr[128];
	wsprintf(fontstr, L"%s %d %s %s",
		szFontFamily.c_str(),
		dwFontSize,
		bBold ? L"bold" : L"",
		bItalic ? L"italic" : L""
		);
	SetWindowText(GetDlgItem(dlg, IDC_FPS_FONT), fontstr);
}

static void InitSettingsDialog(HWND hwnd)
{
	//	HWND ctrl;

	PC_LocalizeDialog(m_dwPluginID, hwnd);

	dwTextColor = PC_GetPluginVarInt(m_dwPluginID, VAR_TEXT_COLOR);
	dwBackgroundColor = PC_GetPluginVarInt(m_dwPluginID, VAR_BACKGROUND_COLOR);
	bItalic = (PC_GetPluginVarInt(m_dwPluginID, VAR_FPS_FONT_STYLE) & 2) != 0;
	bBold = (PC_GetPluginVarInt(m_dwPluginID, VAR_FPS_FONT_STYLE) & 1) != 0;
	szFontFamily = PC_GetPluginVarStr(m_dwPluginID, VAR_FPS_FONT_FAMILY);
	dwFontSize = PC_GetPluginVarInt(m_dwPluginID, VAR_FPS_FONT_SIZE);

	Button_SetCheck(GetDlgItem(hwnd, IDC_SHOW_BACKGROUND), PC_GetPluginVarInt(m_dwPluginID, VAR_SHOW_BACKGROUND) != 0 ? BST_CHECKED : BST_UNCHECKED);
	SetFontButtonText(hwnd);
}

static void DrawColorButton(LPDRAWITEMSTRUCT lpDIS, COLORREF clr)
{
	HDC dc = lpDIS->hDC;
	UINT flag = DFCS_BUTTONPUSH;
	if (lpDIS->itemState & ODS_SELECTED)
		flag |= DFCS_PUSHED;
	DrawFrameControl(dc, &lpDIS->rcItem, DFC_BUTTON, flag);

	InflateRect(&lpDIS->rcItem, -3, -3);
	auto br = CreateSolidBrush(clr);
	FillRect(dc, &lpDIS->rcItem, br);
	DeleteObject(br);
}

static INT_PTR CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DWORD id;
	//	NMHDR *p;


	switch (uMsg)
	{
	case WM_COMMAND:
		id = LOWORD(wParam);
		if (id == IDOK || id == IDCANCEL)
		{
			if (id == IDOK)
			{
				PC_SetPluginVar(m_dwPluginID, VAR_TEXT_COLOR, dwTextColor);
				PC_SetPluginVar(m_dwPluginID, VAR_BACKGROUND_COLOR, dwBackgroundColor);

				PC_SetPluginVar(m_dwPluginID, VAR_FPS_FONT_FAMILY, szFontFamily.c_str());
				PC_SetPluginVar(m_dwPluginID, VAR_FPS_FONT_SIZE, dwFontSize);
				PC_SetPluginVar(m_dwPluginID, VAR_SHOW_BACKGROUND, Button_GetCheck(GetDlgItem(hwnd, IDC_SHOW_BACKGROUND)) == BST_CHECKED);
				int style = (bBold ? 1 : 0) | (bItalic ? 2 : 0);
				PC_SetPluginVar(m_dwPluginID, VAR_FPS_FONT_STYLE, style);
			}

			EndDialog(hwnd, id);
		}

		if (id == IDC_TEXT_COLOR_BTN || id == IDC_BACKGROUND_COLOR_BTN)
		{
			DWORD &color_var = (id == IDC_TEXT_COLOR_BTN) ? dwTextColor : dwBackgroundColor;
			COLORREF custColors[16];
			CHOOSECOLOR cc = { 0 };
			cc.lStructSize = sizeof(cc);
			cc.hwndOwner = hwnd;
			cc.rgbResult = color_var;
			cc.Flags = CC_FULLOPEN | CC_RGBINIT;
			cc.lpCustColors = custColors;
			if (ChooseColor(&cc))
			{
				color_var = cc.rgbResult;
				InvalidateRect(hwnd, 0, 0);
			}
		}

		if (id == IDC_FPS_FONT)
		{
			LOGFONT lf = { 0 };
			lf.lfItalic = bItalic;
			wcscpy_s(lf.lfFaceName, szFontFamily.c_str());
			HDC dc = GetDC(hwnd);
			lf.lfHeight = -MulDiv(dwFontSize, GetDeviceCaps(dc, LOGPIXELSY), 72);
			lf.lfWeight = 400;

			CHOOSEFONT cf = { 0 };
			cf.lStructSize = sizeof(cf);
			cf.hwndOwner = hwnd;
			cf.lpLogFont = &lf;
			cf.iPointSize = dwFontSize * 10;
			cf.Flags = CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT | CF_NOSCRIPTSEL;
			if (ChooseFont(&cf))
			{
				bItalic = lf.lfItalic != 0;
				bBold = lf.lfWeight > 500;
				szFontFamily = lf.lfFaceName;
				dwFontSize = (-lf.lfHeight * 72) / GetDeviceCaps(dc, LOGPIXELSY);
				SetFontButtonText(hwnd);
			}
			ReleaseDC(hwnd, dc);

		}

		break;

	case WM_NOTIFY:
		break;

	case WM_DRAWITEM:
	{
		auto lpDIS = (LPDRAWITEMSTRUCT)lParam;
		if (lpDIS->CtlID == IDC_TEXT_COLOR_BTN)
			DrawColorButton(lpDIS, dwTextColor);
		if (lpDIS->CtlID == IDC_BACKGROUND_COLOR_BTN)
			DrawColorButton(lpDIS, dwBackgroundColor);
		break;
	}


	case WM_INITDIALOG:
		InitSettingsDialog(hwnd);
		return true;
	}

	return false;
}

//
//	Show plugin settings dialog
//
PLUGIN_EXPORT void PluginConfigure(HWND parent)
{
	DialogBox(GetDllInstance(), MAKEINTRESOURCE(IDD_FPS_CONFIG_DLG), parent, DlgProc);
}