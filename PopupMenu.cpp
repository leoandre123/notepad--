#include "PopupMenu.h"
#include "ThemeManager.h"
#include <iostream>
#include "Globals.h"
#include <Windowsx.h>
#include "Math.h"

PopupMenu::PopupMenu(HWND mainWindow, HWND owner) :
	m_IsVisible(false), 
	m_HoveredItem(-1),
	m_MainWindowHandle(mainWindow)
{


	HBRUSH bgBrush = CreateSolidBrush(RGB(255, 255, 255));
	const wchar_t CLASS_NAME[] = L"Popup";

	WNDCLASS wc = { };
	wc.lpfnWndProc = PopupProc;
	wc.hInstance = g_hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hbrBackground = bgBrush;

	ATOM atom = RegisterClass(&wc);

	if (!atom) {
		std::cerr << "Popup Creation RegisterClass failed: " << GetLastError() << "\n";
		//return;
	}

	m_Handle = CreateWindowEx(
		0,
		CLASS_NAME,
		L"",
		WS_POPUP,


		CW_USEDEFAULT, CW_USEDEFAULT, 100, 100,
		owner,
		NULL,
		g_hInstance,
		this
	);

	if (!m_Handle) {
		std::cerr << "Popup Creation CreateWindowEx failed: " << GetLastError() << "\n";
	}
}

void PopupMenu::AppendString(const std::wstring& name, int command)
{
	m_Items.push_back({ PopupItemType::Button, name, L"", command, { }, nullptr });
	CalculateSize();
}
void PopupMenu::AppendString(const std::wstring& name, const std::wstring& shortcut, int command)
{
	m_Items.push_back({ PopupItemType::Button, name, shortcut, command, { }, nullptr });
	CalculateSize();
}

void PopupMenu::AppendSeparator()
{
	m_Items.push_back({ PopupItemType::Separator, L"", L"", -1, {},nullptr });
	CalculateSize();
}

void PopupMenu::AppendPopup(const std::wstring& name, PopupMenu* popup)
{
	//PopupMenu* popup = new PopupMenu(m_Handle);
	m_Items.push_back({ PopupItemType::Popup, name, L"", -1, {}, popup });
	CalculateSize();
}


void PopupMenu::Show(bool show, POINT pos)
{
	m_IsVisible = show;
	ShowWindow(m_Handle, show ? SW_SHOW : SW_HIDE);

	if (show) {
		//SetCapture(m_Handle);
		SetWindowPos(m_Handle, NULL, pos.x, pos.y, 0, 0, SWP_NOSIZE);
	}

}


LRESULT PopupMenu::PopupProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PopupMenu* pThis = nullptr;
	if (uMsg == WM_NCCREATE) {
		CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
		pThis = (PopupMenu*)cs->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
		return true;
	}
	else {
		pThis = (PopupMenu*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	}

	if (pThis) {
		return pThis->HandleMessage(uMsg, wParam, lParam);
	}
	else {
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

LRESULT PopupMenu::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(m_Handle, &ps);
		OnPaint(hdc);
		EndPaint(m_Handle, &ps);
		return 0;
	}
	case WM_LBUTTONDOWN: {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		OnMouseClick(pt);
		return 0;
	}
	case WM_MOUSEMOVE: {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		OnMouseMove(pt);
		return 0;
	}
	case WM_ACTIVATE: {
		if (LOWORD(wParam) == WA_INACTIVE) {
			HWND newHandle = (HWND)lParam;
			HWND owner = GetWindow(newHandle, GW_OWNER);
			if (m_Handle != owner)
			{
				Show(false);
			}

		}
	}
	case WM_SETCURSOR: {
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		return TRUE;
	}

	default:
	{
		return DefWindowProc(m_Handle, uMsg, wParam, lParam);
	}
	}
}

void PopupMenu::OnPaint(HDC hdc)
{
	const Theme& theme = ThemeManager::Instance().GetTheme();
	HBRUSH bg = CreateSolidBrush(theme.primaryBackgroundColor);
	HBRUSH outline = CreateSolidBrush(theme.primaryOutlineColor);
	HBRUSH hoveredBrush = CreateSolidBrush(theme.secondaryBackgroundColor);

	HFONT font = CreateFontFromSpec(theme.primaryFont, 14);
	HFONT hOldFont = (HFONT)SelectObject(hdc, font);


	RECT popupRc;
	GetClientRect(m_Handle, &popupRc);


	FillRect(hdc, &popupRc, bg);
	FrameRect(hdc, &popupRc, outline);



	int y = popupRc.top;

	for (int i = 0; i < (int)m_Items.size(); i++) {
		auto& item = m_Items[i];
		if (item.type == PopupItemType::Separator) {


			HPEN hPen = CreatePen(PS_SOLID, 1, theme.primaryOutlineColor);
			HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

			MoveToEx(hdc, popupRc.left + 32, y + 4, NULL);
			LineTo(hdc, popupRc.right - 32, y + 4);

			SelectObject(hdc, hOldPen);
			DeleteObject(hPen);

			y += 8;
			continue;
		}
		else {
			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, theme.primaryTextColor);
			RECT itemRect = { popupRc.left, y, popupRc.right, y + 24 };
			item.rect = itemRect;

			if (i == m_HoveredItem) {
				FillRect(hdc, &itemRect, hoveredBrush);
			}

			if (!item.label.empty()) {
				RECT textRect = { popupRc.left + 32, y, popupRc.right - 32, y + 24 };
				DrawTextW(hdc, item.label.c_str(), -1, &textRect,
					DT_LEFT | DT_VCENTER | DT_SINGLELINE);

				if (!item.shortcut.empty()) {
					DrawTextW(hdc, item.shortcut.c_str(), -1, &textRect,
						DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
				}
			}

			if (item.type == PopupItemType::Popup) {
				RECT rc = { popupRc.right - 32, y, popupRc.right, y + 24 };
				DrawTextW(hdc, L">", 1, &rc,
					DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			}

			y += 24;
		}

	}

	SelectObject(hdc, hOldFont);
	DeleteObject(&bg);
	DeleteObject(&outline);
	DeleteFont(font);
}

void PopupMenu::OnMouseMove(const POINT& pt) {
	RECT popupRc;
	GetClientRect(m_Handle, &popupRc);

	int newHoveredIndex = -1;
	for (int i = 0; i < (int)m_Items.size(); i++) {
		if (Math::PointInRect(pt, m_Items[i].rect)) {
			newHoveredIndex = i;
			break;
		}
	}
	if (newHoveredIndex != m_HoveredItem) {

		if (m_HoveredItem != -1 && m_Items[m_HoveredItem].type == PopupItemType::Popup) {
			((PopupMenu*)m_Items[m_HoveredItem].data)->Show(false);
		}
		if (newHoveredIndex != -1 && m_Items[newHoveredIndex].type == PopupItemType::Popup) {

			POINT menuPt = {
				m_Items[newHoveredIndex].rect.right,
				m_Items[newHoveredIndex].rect.top
			};
			ClientToScreen(m_Handle, &menuPt);

			((PopupMenu*)m_Items[newHoveredIndex].data)->Show(true, menuPt);
		}

		m_HoveredItem = newHoveredIndex;
		InvalidateRect(m_Handle, NULL, true);
	}
}
void PopupMenu::OnMouseClick(const POINT& pt) {
	for (auto& item : m_Items) {
		if (item.type == PopupItemType::Button) {
			if (Math::PointInRect(pt, item.rect)) {
				SendMessage(m_MainWindowHandle, WM_COMMAND, item.command, 0);
				Show(false);
				return;
			}
		}
	}
}


void PopupMenu::CalculateSize()
{
	HDC hdc = CreateCompatibleDC(NULL);

	int popupX = 0;
	int popupY = 0;
	int widthLabel = 32;
	int widthShortcut = 0;
	int height = 0;
	for (auto& item : m_Items) {
		if (item.type == PopupItemType::Separator) {
			height += 8;

		}
		else {
			height += 24;

			SIZE textSize;
			GetTextExtentPoint32W(hdc, item.label.c_str(), item.label.size(), &textSize);
			int textWidth = textSize.cx;
			if (textWidth > widthLabel)
				widthLabel = textWidth;

			if (!item.shortcut.empty())
			{
				GetTextExtentPoint32W(hdc, item.shortcut.c_str(), item.shortcut.size(), &textSize);
				textWidth = textSize.cx;
				if (textWidth > widthShortcut)
					widthShortcut = textWidth;
			}

		}
	}

	MoveWindow(m_Handle, 0, 0, widthLabel + widthShortcut + 64 + 64, height, true);

	DeleteDC(hdc);
}
