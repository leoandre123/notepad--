#include "ToolBar.h"
#include <iostream>
#include "Globals.h"
#include <windowsx.h>
#include "ThemeManager.h"
#include "Math.h"

const int PADDING = 5;
const int MARGIN_LEFT = 10;

ToolBar::ToolBar() :
	m_PreferredHeight(30),
	m_HoveredItem(-1),
	m_MouseTracking(false)
{
}

bool ToolBar::Create(HWND parentHandle)
{
	std::cout << "Creating Tab Control...\n";
	WNDCLASS wc = {};
	wc.lpfnWndProc = ToolProc;
	wc.hInstance = g_hInstance;
	wc.lpszClassName = L"CustomToolBarControl";


	ATOM atom = RegisterClass(&wc);
	std::cout << "1\n";
	if (!atom) {
		std::cerr << "Creating Tab Control RegisterClass failed: " << GetLastError() << "\n";
		//return false;
	}
	std::cout << "2\n";
	m_Handle = CreateWindowEx(
		0,
		TEXT("CustomToolBarControl"),
		NULL,
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		parentHandle,
		(HMENU)1001,
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

void ToolBar::AddItem(ToolItem* item)
{
	m_Items.push_back(item);
}

LRESULT ToolBar::ToolProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ToolBar* pThis = nullptr;
	if (uMsg == WM_NCCREATE) {
		CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
		pThis = (ToolBar*)cs->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
		return true;
	}
	else {
		pThis = (ToolBar*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	}

	if (pThis) {
		return pThis->HandleMessage(uMsg, wParam, lParam);
	}
	else {
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

LRESULT ToolBar::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	default:
	{
	case WM_NCHITTEST:
	{
		return HTCLIENT;
	}
	case WM_PAINT:
	{
		Paint();
		return 0;
	}
	case WM_LBUTTONDOWN: {
		OnMouseClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}
	case WM_MOUSEMOVE: {
		OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}
	case WM_MOUSELEAVE: {
		m_HoveredItem = -1;
		m_MouseTracking = false;
		InvalidateRect(m_Handle, NULL, true);
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

void ToolBar::Paint()
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(m_Handle, &ps);
	RECT clientRect;
	GetClientRect(m_Handle, &clientRect);


	const Theme& theme = ThemeManager::Instance().GetTheme();
	HBRUSH bgBrush = CreateSolidBrush(theme.windowBackgroundColor);

	FillRect(hdc, &clientRect, bgBrush);

	int paddingPx = DipToPixels(PADDING);

	int x = DipToPixels(MARGIN_LEFT);
	for (int i = 0; i < (int)m_Items.size(); i++) {

		int widthPx = DipToPixels(m_Items[i]->GetPreferredWidth(hdc));
		RECT itemRect = { x, paddingPx, x + widthPx, clientRect.bottom - paddingPx };
		m_Items[i]->Paint(hdc, itemRect, i == m_HoveredItem);

		x += widthPx;
	}

	EndPaint(m_Handle, &ps);
}

void ToolBar::OnMouseMove(int x, int y)
{
	HDC hdc = GetDC(m_Handle);
	RECT clientRect;
	GetClientRect(m_Handle, &clientRect);

	m_HoveredItem = -1;

	int paddingPx = DipToPixels(PADDING);

	int startX = DipToPixels(MARGIN_LEFT);
	for (int i = 0; i < (int)m_Items.size(); i++) {

		int widthPx = DipToPixels(m_Items[i]->GetPreferredWidth(hdc));
		if (x >= startX && x < startX + widthPx) {
			m_HoveredItem = i;
			break;
		}
		startX += widthPx;
	}

	if (!m_MouseTracking) {
		TRACKMOUSEEVENT tme = { sizeof(tme) };
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_Handle;
		TrackMouseEvent(&tme);
		m_MouseTracking = true;
	}

	InvalidateRect(m_Handle, NULL, true);
}

void ToolBar::OnMouseClick(int x, int y)
{
	HDC hdc = GetDC(m_Handle);
	RECT clientRect;
	GetClientRect(m_Handle, &clientRect);

	int paddingPx = DipToPixels(PADDING);

	int startX = DipToPixels(MARGIN_LEFT);
	for (auto& item : m_Items) {

		int widthPx = DipToPixels(item->GetPreferredWidth(hdc));

		if (x >= startX && x < startX + widthPx) {
			item->OnClick();
			break;
		}

		startX += widthPx;
	}

	InvalidateRect(m_Handle, NULL, true);
}
