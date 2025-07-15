#include "EditorWindow.h"
#include "Globals.h"
#include <vector>
#include <iostream>
#include "SyntaxHighlighter.h"
#include <windowsx.h>
#include "ThemeManager.h"
#include "Math.h"

#define MBUTTON_1 0x01
#define MBUTTON_2 0x02
#define MBUTTON_3 0x03
#define MBUTTON_4 0x04
#define MBUTTON_5 0x05

const int LEFT_MARGIN = 10;
const int LINE_HEIGHT = 16;
const int CHAR_WIDTH = 8;
const int GUTTER_WIDTH = 50;
const int PADDING = 5;


EditorWindow::EditorWindow():
	m_IsSelecting(false),
	m_VisibleLineCount(0),
	m_ActiveTabContext(nullptr)
{
}

bool EditorWindow::Create(HWND parentHandle)
{
	std::cout << "Creating Editor Window...\n";
	WNDCLASS ewc = { };
	ewc.lpfnWndProc = EditorProc;
	ewc.hInstance = g_hInstance;
	ewc.lpszClassName = TEXT("Editor");

	ATOM atom = RegisterClass(&ewc);

	if (!atom) {
		std::cerr << "Creating Editor Window RegisterClass failed: " << GetLastError() << "\n";
		return false;
	}

	m_WindowHandle = CreateWindowEx(
		0,
		TEXT("Editor"),
		NULL,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL,
		0, 0, 0, 0,
		parentHandle,
		(HMENU)1001,
		g_hInstance,
		this
	);

	if (!m_WindowHandle) {
		std::cerr << "Creating Editor Window CreateWindowEx failed: " << GetLastError() << "\n";
		return 0;
	}

	std::cout << "Creating Editor Window Done\n";
	return true;
}


void EditorWindow::SetTabContext(TabContext* tabContext)
{
	m_ActiveTabContext = tabContext;
}
LRESULT EditorWindow::EditorProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	EditorWindow* pThis = nullptr;
	if (uMsg == WM_NCCREATE) {
		CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
		pThis = (EditorWindow*)cs->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
		return true;
	}
	else {
		pThis = (EditorWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	}

	if (pThis) {
		return pThis->HandleMessage(uMsg, wParam, lParam);
	}
	else {
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

LRESULT EditorWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT: PaintWindow(); return 0;
	case WM_SETCURSOR: {
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(m_WindowHandle, &p);

		if (p.x > GUTTER_WIDTH)
		{
			SetCursor(LoadCursor(NULL, IDC_IBEAM));
			return TRUE;
		}
		else {
			SetCursor(LoadCursor(NULL, IDC_UPARROW));
			return TRUE;
		}

		break;
	}
	case WM_ERASEBKGND:
	{
		return 1;
	}
	case WM_VSCROLL:
		OnScroll(wParam, lParam); return 0;
	case WM_LBUTTONDOWN:
		OnMouseButtonDown(MBUTTON_1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
	case WM_MBUTTONDOWN:
		OnMouseButtonDown(MBUTTON_2, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
	case WM_RBUTTONDOWN:
		OnMouseButtonDown(MBUTTON_3, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
	case WM_XBUTTONDOWN:
		OnMouseButtonDown(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? MBUTTON_4 : MBUTTON_5, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
	case WM_LBUTTONUP:
		OnMouseButtonUp(MBUTTON_1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
	case WM_MBUTTONUP:
		OnMouseButtonUp(MBUTTON_2, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
	case WM_RBUTTONUP:
		OnMouseButtonUp(MBUTTON_3, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
	case WM_XBUTTONUP:
		OnMouseButtonUp(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? MBUTTON_4 : MBUTTON_5, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
	case WM_SIZE: {
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);
		OnResize(width, height);
		return 0;
	}
	default:
	{
		return DefWindowProc(m_WindowHandle, uMsg, wParam, lParam);
	}
	}
}

void EditorWindow::PaintWindow()
{
	if (!m_ActiveTabContext)return;

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(m_WindowHandle, &ps);
	RECT clientRect;
	GetClientRect(m_WindowHandle, &clientRect);


	const Theme& theme = ThemeManager::Instance().GetTheme();

	
	HFONT editorFont = CreateFontFromSpec(theme.editorFont, LINE_HEIGHT);
	HFONT hOldFont = (HFONT)SelectObject(hdc, editorFont);

	int lineHeightPx = DipToPixels(LINE_HEIGHT);
	int gutterWidthPx = DipToPixels(GUTTER_WIDTH);
	int paddingPx = DipToPixels(PADDING);
	int leftMarginPx = DipToPixels(LEFT_MARGIN);

	int textStartX = gutterWidthPx + paddingPx;
	int textStartY = paddingPx;
	int cursorLine = m_ActiveTabContext->document->GetRow();
	int cursorCol = m_ActiveTabContext->document->GetColumn();
	bool hasSelection = m_ActiveTabContext->document->HasSelection();
	int selectionStartLine = m_ActiveTabContext->document->GetSelectionRow();
	int selectionStartCol = m_ActiveTabContext->document->GetSelectionColumn();



	int lineCount = abs(clientRect.bottom - clientRect.top) / lineHeightPx;

	int selectionRowMin = min(cursorLine, selectionStartLine);
	int selectionColMin = min(cursorCol, selectionStartCol);

	int selectionRowMax = max(cursorLine, selectionStartLine);
	int selectionColMax = max(cursorCol, selectionStartCol);

	

	//Background
	HBRUSH windowBgBrush = CreateSolidBrush(theme.windowBackgroundColor);
	FillRect(hdc, &clientRect, windowBgBrush);
	DeleteObject(windowBgBrush);

	//Gutter
	RECT gutterRect = { leftMarginPx, 0, gutterWidthPx, clientRect.bottom };
	HBRUSH gutterBrush = CreateSolidBrush(theme.primaryBackgroundColor);
	FillRect(hdc, &gutterRect, gutterBrush);
	DeleteObject(gutterBrush);

	//Background
	RECT editorRect = { gutterRect.right, 0, clientRect.right, clientRect.bottom };
	HBRUSH bgBrush = CreateSolidBrush(theme.editorBackgroundColor);
	FillRect(hdc, &editorRect, bgBrush);
	DeleteObject(bgBrush);

	

	std::vector<String> lines = m_ActiveTabContext->document->GetLines(m_ActiveTabContext->lineOffset, lineCount);

	for (int i = 0; i < lines.size(); ++i) {
		wchar_t lineNum[8];
		swprintf_s(lineNum, L"%*d", 4, i + 1 + m_ActiveTabContext->lineOffset);

		int row = i + m_ActiveTabContext->lineOffset;
		int selectionStart = -1;
		int selectionEnd = -1;

		if (hasSelection) {
			if (row >= selectionRowMin && row <= selectionRowMax) {
				selectionStart = max(row == selectionRowMin ? selectionColMin : 0, 0);
				int lineLength = lines[i].GetColumnCount();
				selectionEnd = min(row == selectionRowMax ? selectionColMax : lineLength, lineLength);
			}
		}

		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, theme.editorTextColor);

		TextOutW(hdc, 5, textStartY + (int)i * lineHeightPx, lineNum, lstrlenW(lineNum));
		DrawString(hdc, textStartX, textStartY + (int)i * lineHeightPx,
			lines[i],
			cursorLine == i + m_ActiveTabContext->lineOffset ? cursorCol : -1,
			selectionStart, selectionEnd);
	}

	SelectObject(hdc, hOldFont);
	DeleteFont(editorFont);

	EndPaint(m_WindowHandle, &ps);
}

void EditorWindow::OnScroll(WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO si = { sizeof(si) };
	si.fMask = SIF_ALL;
	GetScrollInfo(m_WindowHandle, SB_VERT, &si);

	int pos = si.nPos;
	switch (LOWORD(wParam)) {
	case SB_LINEUP:   pos -= 1; break;
	case SB_LINEDOWN: pos += 1; break;
	case SB_PAGEUP:   pos -= si.nPage; break;
	case SB_PAGEDOWN: pos += si.nPage; break;
	case SB_THUMBTRACK: pos = HIWORD(wParam); break;
	}

	si.nPos = max(0, min(pos, si.nMax));
	SetScrollInfo(m_WindowHandle, SB_VERT, &si, TRUE);
	m_ActiveTabContext->lineOffset = si.nPos;

	InvalidateRect(m_WindowHandle, NULL, TRUE);
}

void EditorWindow::OnResize(int newWidth, int newHeight)
{
	if (!m_ActiveTabContext)return;
	RECT clientRect;
	GetClientRect(m_WindowHandle, &clientRect);
	m_VisibleLineCount = abs(clientRect.bottom - clientRect.top) / DipToPixels(LINE_HEIGHT);

	SCROLLINFO si = { sizeof(si), SIF_RANGE | SIF_PAGE | SIF_POS };
	si.nMin = 0;
	si.nMax = m_ActiveTabContext->document->GetLineCount();
	si.nPage = m_VisibleLineCount;
	si.nPos = m_ActiveTabContext->lineOffset;

	SetScrollInfo(m_WindowHandle, SB_VERT, &si, TRUE);
}

void EditorWindow::OnMouseButtonDown(int button, int x, int y)
{
	if (button == MBUTTON_1) {
		SetCapture(m_WindowHandle);
		POINT cursorPos = MouseToCursorCoordinates(x, y);

		m_ActiveTabContext->document->SetCursorCoordinates(cursorPos.x, cursorPos.y);
		m_ActiveTabContext->document->SetSelectionStart(cursorPos.x, cursorPos.y);

		m_IsSelecting = true;
		InvalidateRect(m_WindowHandle, NULL, TRUE);
	}
}

void EditorWindow::OnMouseButtonUp(int button, int x, int y)
{
	if (button == MBUTTON_1) {
		ReleaseCapture();
		m_IsSelecting = false;
		InvalidateRect(m_WindowHandle, NULL, TRUE);
	}
}

void EditorWindow::OnMouseMove(int x, int y)
{
	if (m_IsSelecting) {
		POINT cursorPos = MouseToCursorCoordinates(x, y);
		m_ActiveTabContext->document->SetCursorCoordinates(cursorPos.x, cursorPos.y);
		InvalidateRect(m_WindowHandle, NULL, TRUE);
	}
}


void EditorWindow::DrawString(HDC hdc, int x, int y, String str, int cursorPos,
	int selectionStart, int selectionEnd)
{
	//std::vector<StringColorInfo> highlightingInfo = highlighter.HighlightLine(str);

	const Theme& theme = ThemeManager::Instance().GetTheme();


	if (selectionStart != -1) {
		std::cout << selectionStart << " - " << selectionEnd << "\n";

		const String string1 = str.SubstringColumn(0, selectionStart);
		const String string2 = str.SubstringColumn(selectionStart, selectionEnd - selectionStart);
		const String string3 = str.SubstringColumn(selectionEnd);


		int size1 = string1.GetVisualWidth(hdc);
		int size2 = string2.GetVisualWidth(hdc);


		const std::wstring wstr1 = string1.ToWStringWithExpandedTabs();
		const std::wstring wstr2 = string2.ToWStringWithExpandedTabs();
		const std::wstring wstr3 = string3.ToWStringWithExpandedTabs();

		std::cout << string1.ToString() << " - " << string2.ToString() << " - " << string3.ToString() << "\n";
		std::wcout << wstr1 << " - " << wstr2 << " - " << wstr3 << "\n";


		SetBkColor(hdc, theme.editorSelectionColor);

		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, theme.editorTextColor);
		TextOutW(hdc, x, y, wstr1.c_str(), (int)wstr1.size());

		SetBkMode(hdc, OPAQUE);
		SetTextColor(hdc, theme.editorSelectionColor);
		TextOutW(hdc, x + size1, y, wstr2.c_str(), (int)wstr2.size());

		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, theme.editorTextColor);
		TextOutW(hdc, x + size1 + size2, y, wstr3.c_str(), (int)wstr3.size());
	}
	else {
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, theme.editorTextColor);
		const std::wstring& expandedStr = str.ToWStringWithExpandedTabs();
		TextOutW(hdc, x, y, expandedStr.c_str(), (int)expandedStr.size());
	}


	if (cursorPos != -1) {

		HPEN cursorPen = CreatePen(PS_SOLID, 2, theme.editorCursorColor);
		HPEN oldPen = (HPEN)SelectObject(hdc, cursorPen);
		//int cursorX = x + GetVisualWidth(hdc, str, cursorPos);
		int cursorX = x + str.GetVisualWidth(hdc, cursorPos);
		MoveToEx(hdc, cursorX, y, NULL);
		LineTo(hdc, cursorX, y + DipToPixels(LINE_HEIGHT));

		SelectObject(hdc, oldPen);
		DeleteObject(cursorPen);

	}

}



void EditorWindow::ScrollToCursor() {
	int cursorLine = m_ActiveTabContext->document->GetRow();
	if (cursorLine < m_ActiveTabContext->lineOffset)
		m_ActiveTabContext->lineOffset = cursorLine;
	else if (cursorLine >= m_ActiveTabContext->lineOffset + m_VisibleLineCount)
		m_ActiveTabContext->lineOffset = cursorLine - m_VisibleLineCount + 1;


	SCROLLINFO si = { sizeof(si), SIF_RANGE | SIF_PAGE | SIF_POS };
	si.nMin = 0;
	si.nMax = m_ActiveTabContext->document->GetLineCount();
	si.nPage = m_VisibleLineCount;
	si.nPos = m_ActiveTabContext->lineOffset;

	SetScrollInfo(m_WindowHandle, SB_VERT, &si, TRUE);
}

POINT EditorWindow::MouseToCursorCoordinates(int x, int y)
{
	return { (x - GUTTER_WIDTH - 5 + CHAR_WIDTH / 2) / CHAR_WIDTH, m_ActiveTabContext->lineOffset + (y - 5) / LINE_HEIGHT };
}
