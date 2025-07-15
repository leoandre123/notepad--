#include "TabControl.h"
#include "Globals.h"
#include <iostream>
#include <windowsx.h>
#include "ThemeManager.h"
#include "Math.h"
#include "resource.h"


const int TAB_HEIGHT = 20;
const int LEFT_MARGIN = 10;
const int SPACING = 5;
const int PADDING = 5;
const int CLOSE_BUTTON_SIZE = 12;

TabControl::TabControl() :
	m_TabHeight(TAB_HEIGHT),
	m_SelectedTab(0)
{
}

bool TabControl::Create(HWND parentHandle)
{
	std::cout << "Creating Tab Control...\n";
	WNDCLASS wc = {};
	wc.lpfnWndProc = TabProc;
	wc.hInstance = g_hInstance;
	wc.lpszClassName = L"CustomTabControl";


	ATOM atom = RegisterClass(&wc);
	std::cout << "1\n";
	if (!atom) {
		std::cerr << "Creating Tab Control RegisterClass failed: " << GetLastError() << "\n";
		//return false;
	}
	std::cout << "2\n";
	m_Handle = CreateWindowEx(
		0,
		TEXT("CustomTabControl"),
		NULL,
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		parentHandle,
		(HMENU)TAB_CONTROL_ID,
		g_hInstance,
		this
	);

	if (!m_Handle) {
		std::cerr << "Creating Tab Control CreateWindowEx failed: " << GetLastError() << "\n";
		return 0;
	}

	std::cout << "Creating Tab Control Done\n";
	return true;
}

void TabControl::AddTab(std::wstring name)
{
	m_Tabs.push_back({ name });
	InvalidateRect(m_Handle, NULL, true);
}

void TabControl::RemoveTab(int index)
{

}

void TabControl::SelectTab(int index, bool notifyParent)
{
	m_SelectedTab = index;

	NMTABSWITCH nm = {};
	nm.hwndFrom = m_Handle;
	nm.idFrom = TAB_CONTROL_ID;
	nm.code = TCN_TAB_SWITCH;
	nm.selectedIndex = index;

	SendMessage(GetParent(m_Handle), WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
}

LRESULT TabControl::TabProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TabControl* pThis = nullptr;
	if (uMsg == WM_NCCREATE) {
		CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
		pThis = (TabControl*)cs->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
		return true;
	}
	else {
		pThis = (TabControl*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	}

	if (pThis) {
		return pThis->HandleMessage(uMsg, wParam, lParam);
	}
	else {
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

LRESULT TabControl::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	default:
	{
	case WM_NCHITTEST:
	{
		return HTCLIENT;
	}
	case WM_ERASEBKGND:
	{
		return 1;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(m_Handle, &ps);
		RECT clientRect;
		GetClientRect(m_Handle, &clientRect);

		//std::cout << clientRect.bottom - clientRect.top << "\n";

		const Theme& theme = ThemeManager::Instance().GetTheme();

		HBRUSH bgBrush = CreateSolidBrush(theme.windowBackgroundColor);
		HBRUSH tabBgBrush = CreateSolidBrush(theme.windowBackgroundColor);
		HBRUSH tabBgBrushSelected = CreateSolidBrush(theme.editorBackgroundColor);

		HBRUSH closeBgBrush = CreateSolidBrush(RGB(230, 84, 122));

		COLORREF textColor = theme.secondaryTextColor;
		COLORREF textColorSelected = theme.primaryTextColor;

		HFONT font = CreateFontFromSpec(theme.primaryFont, 12);

		HFONT hOldFont = (HFONT)SelectObject(hdc, font);
		// Paint background
		FillRect(hdc, &clientRect, bgBrush);

		SetBkMode(hdc, TRANSPARENT);


		int x = DipToPixels(LEFT_MARGIN);
		int paddingPx = DipToPixels(PADDING);
		int closeButtonSizePx = DipToPixels(CLOSE_BUTTON_SIZE);
		int tabHeightPx = DipToPixels(m_TabHeight);
		int spacingPx = DipToPixels(SPACING);
		int iconSize = closeButtonSizePx / 2;

		HICON hCloseIcon = (HICON)LoadImage(
			g_hInstance,
			MAKEINTRESOURCE(IDI_WINDOW_CLOSE),
			IMAGE_ICON,
			iconSize, iconSize,
			LR_DEFAULTCOLOR);

		for (int i = 0; i < (int)m_Tabs.size(); i++)
		{
			std::wstring name = m_Tabs[i].name;
			SIZE textSize;
			GetTextExtentPoint32W(hdc, name.c_str(), name.size(), &textSize);

			int textWidthPx = textSize.cx;
			int tabWidthPx = textWidthPx + paddingPx + paddingPx + closeButtonSizePx + DipToPixels(10);

			m_Tabs[i].width = tabWidthPx;
			RECT tabRect = { x, 0, x + tabWidthPx, tabHeightPx };
			RECT textRect = { x + paddingPx, 0, x + paddingPx + textWidthPx, tabHeightPx };

			FillRect(hdc, &tabRect, i == m_SelectedTab ? tabBgBrushSelected : tabBgBrush);
			//DrawRect(hdc, &tabRect, tabBrush);

			
			if (i == m_SelectedTab) {
				HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255,0,255));
				HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
		
				MoveToEx(hdc, x, 0, nullptr);
				LineTo(hdc, x + tabWidthPx, 0);

				SelectObject(hdc, hOldPen);
				DeleteObject(hPen);
			}


			SetTextColor(hdc, i == m_SelectedTab ? textColorSelected : textColor);
			DrawTextW(hdc, name.c_str(), -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);



			RECT closeRect;
			closeRect.left = tabRect.right - closeButtonSizePx - paddingPx;
			closeRect.top = (tabRect.bottom - closeButtonSizePx) / 2;
			closeRect.right = tabRect.right - paddingPx;
			closeRect.bottom = (tabRect.bottom + closeButtonSizePx) / 2;

			FillRect(hdc, &closeRect, closeBgBrush);

			//MoveToEx(hdc, closeRect.left + 3, closeRect.top + 3, nullptr);
			//LineTo(hdc, closeRect.right - 3, closeRect.bottom - 3);
			//MoveToEx(hdc, closeRect.left + 3, closeRect.bottom - 3, nullptr);
			//LineTo(hdc, closeRect.right - 3, closeRect.top + 3);

			


			DrawIconEx(
				hdc,
				(closeRect.left + closeRect.right - iconSize) / 2,
				(closeRect.bottom + closeRect.top - iconSize) / 2,
				hCloseIcon,
				iconSize, iconSize,
				0, nullptr, DI_NORMAL);

			x += tabWidthPx + spacingPx;
		}

		DestroyIcon(hCloseIcon);
		SelectObject(hdc, hOldFont);
		DeleteFont(font);
		EndPaint(m_Handle, &ps);
		return 0;
	}
	case WM_LBUTTONDOWN: {
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);

		int startX = LEFT_MARGIN;
		for (int i = 0; i < (int)m_Tabs.size(); i++) {
			int width = m_Tabs[i].width;

			if (x >= startX && x < startX + width) {
				SelectTab(i);
				InvalidateRect(m_Handle, NULL, true);
				break;
			}

			startX += width + SPACING;
		}

		return 0;
	}
	case WM_SETCURSOR: {
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		return TRUE;
	}
					 return DefWindowProc(m_Handle, uMsg, wParam, lParam);
	}
	}
}
