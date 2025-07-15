#include "TitleBar.h"
#include "ThemeManager.h"
#include "Globals.h"
#include "resource.h"
#include "Math.h"
#include <windowsx.h>

#define BUTTON_CLOSE		0x01
#define BUTTON_MAXIMIZE		0x02
#define BUTTON_MINIMIZE		0x03


const int LEFT_MARGIN = 10;


TitleBar::TitleBar(int height)
	:
	//m_ParentHandle(parent),
	m_Height(height),
	m_HoveredButton(-1),
	m_HoveredMenuItem(-1)
{

}

LRESULT TitleBar::OnNcHitTest(const POINT& pt)
{
	if (PointInRect(pt, m_CloseRect)) return HTCLIENT;
	if (PointInRect(pt, m_MinimizeRect))  return HTCLIENT;
	if (PointInRect(pt, m_ExpandRect))  return HTCLIENT;
	for (auto& item : m_MenuItems)
		if (PointInRect(pt, item.labelRect))
			return HTCLIENT;



	if (pt.y < m_Height) return HTCAPTION;

	return HTCLIENT;
}

void TitleBar::OnPaint(HDC hdc, const RECT& clientArea)
{
	RECT rcTitle = clientArea;
	rcTitle.bottom = DipToPixels(m_Height);

	PaintWindowButtons(hdc, rcTitle);
	PaintIcon(hdc, rcTitle);
	PaintMenu(hdc, rcTitle);
	PaintLabel(hdc, rcTitle);
}

void TitleBar::OnMouseMove(const POINT& pt)
{
	int newHoveredButton = -1;
	if (PointInRect(pt, m_CloseRect))			newHoveredButton = BUTTON_CLOSE;
	else if (PointInRect(pt, m_MinimizeRect))	newHoveredButton = BUTTON_MINIMIZE;
	else if (PointInRect(pt, m_ExpandRect))		newHoveredButton = BUTTON_MAXIMIZE;

	if (newHoveredButton != m_HoveredButton) {
		m_HoveredButton = newHoveredButton;
		InvalidateRect(m_ParentHandle, NULL, true);
	}

	int newHoveredMenuIndex = -1;
	for (int i = 0; i < (int)m_MenuItems.size(); i++) {
		if (PointInRect(pt, m_MenuItems[i].labelRect)) {
			newHoveredMenuIndex = i;
			break;
		}
	}

	if (newHoveredMenuIndex != m_HoveredMenuItem) {
		if (m_HoveredMenuItem != -1 && newHoveredMenuIndex != -1
			&& m_MenuItems[m_HoveredMenuItem].menu.IsVisible()) {

			POINT menuPt = { m_MenuItems[newHoveredMenuIndex].labelRect.left,m_MenuItems[newHoveredMenuIndex].labelRect.bottom };
			ClientToScreen(m_ParentHandle, &menuPt);

			m_MenuItems[m_HoveredMenuItem].menu.Show(false);
			m_MenuItems[newHoveredMenuIndex].menu.Show(true, menuPt);
		}
		m_HoveredMenuItem = newHoveredMenuIndex;
		InvalidateRect(m_ParentHandle, NULL, true);
	}

}

void TitleBar::OnMouseClick(const POINT& pt)
{
	if (PointInRect(pt, m_CloseRect)) {
		SendMessage(m_ParentHandle, WM_SYSCOMMAND, SC_CLOSE, 0);
	}
	else if (PointInRect(pt, m_ExpandRect)) {
		if (IsZoomed(m_ParentHandle))
			SendMessage(m_ParentHandle, WM_SYSCOMMAND, SC_RESTORE, 0);
		else
			SendMessage(m_ParentHandle, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	}
	else if (PointInRect(pt, m_MinimizeRect)) {
		SendMessage(m_ParentHandle, WM_SYSCOMMAND, SC_MINIMIZE, 0);
	}


	for (auto& item : m_MenuItems) {

		if (PointInRect(pt, item.labelRect)) {
			POINT menuPt = { item.labelRect.left,item.labelRect.bottom };
			ClientToScreen(m_ParentHandle, &menuPt);
			item.menu.Show(true, menuPt);
		}
		else {
			//item.menu.Show(false);
		}
	}

}

void TitleBar::AddMenu(const std::wstring& label, PopupMenu& popup)
{

	TitleBarMenuItem item = {
		label,
		popup,
		{}
	};
	m_MenuItems.push_back(item);
}

void TitleBar::PaintWindowButtons(HDC hdc, RECT& titleArea)
{
	const Theme& theme = ThemeManager::Instance().GetTheme();
	HBRUSH bgBrush = CreateSolidBrush(theme.windowBackgroundColor);

	FillRect(hdc, &titleArea, bgBrush);
	DeleteObject(bgBrush);


	HFONT font = CreateFontFromSpec(theme.primaryFont, 12);
	HFONT hOldFont = (HFONT)SelectObject(hdc, font);

	

	const int buttonWidth = DipToPixels(60);

	m_CloseRect = { titleArea.right - buttonWidth, 0, titleArea.right, titleArea.bottom };
	m_ExpandRect = { m_CloseRect.left - buttonWidth, 0, m_CloseRect.right - buttonWidth, titleArea.bottom };
	m_MinimizeRect = { m_ExpandRect.left - buttonWidth, 0, m_ExpandRect.right - buttonWidth, titleArea.bottom };

	HBRUSH closeBrush = CreateSolidBrush(m_HoveredButton == BUTTON_CLOSE ? RGB(255, 0, 0) : theme.windowBackgroundColor);
	HBRUSH expandBrush = CreateSolidBrush(m_HoveredButton == BUTTON_MAXIMIZE ? theme.primaryBackgroundColor : theme.windowBackgroundColor);
	HBRUSH minimizeBrush = CreateSolidBrush(m_HoveredButton == BUTTON_MINIMIZE ? theme.primaryBackgroundColor : theme.windowBackgroundColor);

	FillRect(hdc, &m_CloseRect, closeBrush);
	HICON hCloseIcon = (HICON)LoadImage(
		g_hInstance,
		MAKEINTRESOURCE(IDI_WINDOW_CLOSE),
		IMAGE_ICON,
		14, 14,
		LR_DEFAULTCOLOR);

	DrawIconEx(
		hdc,
		(m_CloseRect.right + m_CloseRect.left) / 2 - 7,
		(m_CloseRect.bottom + m_CloseRect.top) / 2 - 7,
		hCloseIcon,
		14, 14,
		0, nullptr, DI_NORMAL);

	DestroyIcon(hCloseIcon);



	FillRect(hdc, &m_ExpandRect, expandBrush);
	HICON hMaximizeIcon = (HICON)LoadImage(
		g_hInstance,
		MAKEINTRESOURCE(IsZoomed(m_ParentHandle) ? IDI_WINDOW_UNMINIMIZE : IDI_WINDOW_MAXIMIZE),
		IMAGE_ICON,
		16, 16,
		LR_DEFAULTCOLOR);

	DrawIconEx(
		hdc,
		(m_ExpandRect.right + m_ExpandRect.left) / 2 - 8,
		(m_ExpandRect.bottom + m_ExpandRect.top) / 2 - 8,
		hMaximizeIcon,
		16, 16,
		0, nullptr, DI_NORMAL);

	DestroyIcon(hMaximizeIcon);

	FillRect(hdc, &m_MinimizeRect, minimizeBrush);
	HICON hMinmizeIcon = (HICON)LoadImage(
		g_hInstance,
		MAKEINTRESOURCE(IDI_WINDOW_MINIMIZE),
		IMAGE_ICON,
		16, 16,
		LR_DEFAULTCOLOR);

	DrawIconEx(
		hdc,
		(m_MinimizeRect.right + m_MinimizeRect.left) / 2 - 8,
		(m_MinimizeRect.bottom + m_MinimizeRect.top) / 2 - 8,
		hMinmizeIcon,
		16, 16,
		0, nullptr, DI_NORMAL);

	DestroyIcon(hMinmizeIcon);


	DeleteObject(closeBrush);
	DeleteObject(expandBrush);
	DeleteObject(minimizeBrush);

	SelectObject(hdc, hOldFont);
	DeleteFont(font);
}

void TitleBar::PaintMenu(HDC hdc, RECT& titleArea)
{
	const Theme& theme = ThemeManager::Instance().GetTheme();


	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, theme.primaryTextColor);

	HFONT font = CreateFontFromSpec(theme.primaryFont, 12);
	HFONT hOldFont = (HFONT)SelectObject(hdc, font);
	HBRUSH hoverBrush = CreateSolidBrush(theme.primaryBackgroundColor);

	int topBottomPadding = DipToPixels(4);
	int x = DipToPixels(m_Height);
	for (int i = 0; i < (int)m_MenuItems.size(); i++) {
		
		SIZE textSize;
		GetTextExtentPoint32W(hdc, m_MenuItems[i].label.c_str(), m_MenuItems[i].label.size(), &textSize);
		int width = textSize.cx + DipToPixels(16);
		if (!m_MenuItems[i].label.empty()) {
			RECT textRect = { x, topBottomPadding, x + width, titleArea.bottom- topBottomPadding };

			if (i == m_HoveredMenuItem) {
				FillRect(hdc, &textRect, hoverBrush);
			}

			DrawTextW(hdc, m_MenuItems[i].label.c_str(), -1, &textRect,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			m_MenuItems[i].labelRect = textRect;
		}

		x += width;
	}

	SelectObject(hdc, hOldFont);
	DeleteObject(hoverBrush);
	DeleteFont(font);
}

void TitleBar::PaintIcon(HDC hdc, RECT& titleArea)
{
	int iconSize = DipToPixels(m_Height/2);

	HICON hCloseIcon = (HICON)LoadImage(
		g_hInstance,
		MAKEINTRESOURCE(IDI_APP_ICON),
		IMAGE_ICON,
		iconSize, iconSize,
		LR_DEFAULTCOLOR);


	DrawIconEx(
		hdc,
		titleArea.left + DipToPixels(LEFT_MARGIN),
		(titleArea.bottom + m_CloseRect.top - iconSize) / 2,
		hCloseIcon,
		iconSize, iconSize,
		0, nullptr, DI_NORMAL);

	DestroyIcon(hCloseIcon);
}

void TitleBar::PaintLabel(HDC hdc, RECT& titleArea)
{
	const Theme& theme = ThemeManager::Instance().GetTheme();

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, theme.primaryTextColor);
	DrawTextW(hdc, L"Notepad - -", -1, &titleArea,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

bool TitleBar::PointInRect(const POINT& pt, const RECT& rc)
{
	return pt.x >= rc.left && pt.x <= rc.right
		&& pt.y >= rc.top && pt.y <= rc.bottom;
}
