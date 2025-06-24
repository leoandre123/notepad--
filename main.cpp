#include <windows.h>
#include <string>
#include <vector>
#include <iostream>
#include "TextDocument.h"
#include <CommCtrl.h>
#include <bitset>
#include "ITextDocument.h"
#include "LineTextDocument.h"

int lineOffset(0);
ITextDocument* document = new LineTextDocument("");

HWND hwndEditor;
HWND hwndStatus;


const int LINE_HEIGHT = 16;
const int CHAR_WIDTH = 10;
const int GUTTER_WIDTH = 50;
const COLORREF BG_COLOR = RGB(20, 30, 30);
const COLORREF GUTTER_COLOR = RGB(30, 40, 40);
const COLORREF TEXT_COLOR = RGB(230, 230, 230);
const COLORREF CURSOR_COLOR = RGB(255, 140, 0);


int CountUTF8Codepoints(const std::string& str, int byteLimit = -1) {
	int count = 0;
	for (int i = 0; i < str.size();) {
		if (byteLimit != -1 && i >= byteLimit)
			break;

		unsigned char c = (unsigned char)str[i];

		int advance = 1;
		if ((c & 0b10000000) == 0b00000000)       advance = 1; // 1-byte ASCII
		else if ((c & 0b11100000) == 0b11000000)  advance = 2;
		else if ((c & 0b11110000) == 0b11100000)  advance = 3;
		else if ((c & 0b11111000) == 0b11110000)  advance = 4;

		i += advance;
		++count;
	}
	return count;
}

std::string WideCharToUTF8(wchar_t ch) {
	char buffer[5] = {};
	int len = WideCharToMultiByte(CP_UTF8, 0, &ch, 1, buffer, sizeof(buffer), NULL, NULL);
	return std::string(buffer, len);
}

std::wstring UTF8ToWide(const std::string& utf8) {
	int size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
	std::wstring wide(size, 0);
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &wide[0], size);
	wide.pop_back();
	return wide;
}

HWND CreateToolBar(HWND hwndParent) {
	return NULL;
}

HWND CreateStatusBar(HWND hwndParent, int idStatus, HINSTANCE
	hinst, int cParts)
{
	HWND hwndStatus;
	RECT rcClient;
	HLOCAL hloc;
	PINT paParts;
	int i, nWidth;

	// Ensure that the common control DLL is loaded.
	InitCommonControls();

	// Create the status bar.
	hwndStatus = CreateWindowEx(
		0,                       // no extended styles
		STATUSCLASSNAME,         // name of status bar class
		(PCTSTR)NULL,           // no text when first created
		SBARS_SIZEGRIP |         // includes a sizing grip
		WS_CHILD | WS_VISIBLE,   // creates a visible child window
		0, 0, 0, 0,              // ignores size and position
		hwndParent,              // handle to parent window
		(HMENU)idStatus,       // child window identifier
		hinst,                   // handle to application instance
		NULL);                   // no window creation data

	// Get the coordinates of the parent window's client area.
	GetClientRect(hwndParent, &rcClient);

	// Allocate an array for holding the right edge coordinates.
	hloc = LocalAlloc(LHND, sizeof(int) * cParts);
	paParts = (PINT)LocalLock(hloc);

	// Calculate the right edge coordinate for each part, and
	// copy the coordinates to the array.
	nWidth = rcClient.right / cParts;
	int rightEdge = nWidth;
	for (i = 0; i < cParts; i++) {
		paParts[i] = rightEdge;
		rightEdge += nWidth;
	}

	paParts[cParts - 1] = -1;

	// Tell the status bar to create the window parts.
	SendMessage(hwndStatus, SB_SETPARTS, (WPARAM)cParts, (LPARAM)
		paParts);

	// Free the array, and return.
	LocalUnlock(hloc);
	LocalFree(hloc);
	return hwndStatus;
}


void SetStatus(int part, const std::wstring& text) {
	SendMessage(hwndStatus, SB_SETTEXT, part, (LPARAM)(text.c_str()));
}


void UpdateStatusBar() {
	int pos = document->GetCursorPos() + 1;
	int row = document->GetRow() + 1;
	int col = document->GetColumn() + 1;
	int length = document->GetLength();
	int lines = document->GetLineCount();

	std::wstring lengthString = L"Length: " + std::to_wstring(length) + L" Lines: " + std::to_wstring(lines);
	std::wstring cursorString =
		L"Ln: " + std::to_wstring(row) + L" Col: " + std::to_wstring(col) + L" Pos: " + std::to_wstring(pos);

	SetStatus(0, lengthString);
	SetStatus(1, cursorString);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
	case WM_CREATE: {
		hwndStatus = CreateStatusBar(hwnd, 0, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), 4);
		hwndEditor = CreateWindowEx(
			0,
			TEXT("Editor"),
			NULL,
			WS_CHILD | WS_VISIBLE,
			0, 0, 0, 0,
			hwnd,
			(HMENU)1001,
			(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			NULL
		);
		UpdateStatusBar();
		return 0;
	}
	case WM_KEYDOWN: {
		std::cout << "KEY DOWN:" << ((char)wParam) << "\n";
		std::cout << "BITs: "
			<< std::bitset<8>(lParam >> 24) << " "
			<< std::bitset<8>(lParam >> 16) << " "
			<< std::bitset<8>(lParam >> 8) << " "
			<< std::bitset<8>(lParam) << " "
			"\n";
		char vk = (char)wParam;
		if (vk == VK_SPACE)
		{
			document->Write(" ");
		}
		else if (vk == VK_RETURN)
		{
			document->Write("\n");
		}
		else if (vk == VK_LEFT)
		{
			document->MoveCursor(CursorDirection::WEST, 1);
		}
		else if (vk == VK_RIGHT)
		{
			document->MoveCursor(CursorDirection::EAST, 1);
		}
		else if (vk == VK_UP)
		{
			document->MoveCursor(CursorDirection::NORTH, 1);
		}
		else if (vk == VK_DOWN)
		{
			document->MoveCursor(CursorDirection::SOUTH, 1);
		}
		else if (vk == VK_BACK)
		{
			document->DeleteAtCursor(true);
		}
		else if (vk == VK_DELETE)
		{
			document->DeleteAtCursor(false);
		}

		UpdateStatusBar();
		document->PrintDebugString();
		InvalidateRect(hwndEditor, NULL, true);
		return 0;
	}
	case WM_CHAR: {
		wchar_t vk = (wchar_t)wParam;
		std::cout << "CHAR:" << vk << " - " << (int)vk << "\n";

		if (vk > 32) {
			//document->Write(std::string(1, vk));
			document->Write(WideCharToUTF8(vk));
		}



		return 0;
	}
	case WM_CLOSE:
	{
		if (MessageBox(hwnd, L"Really quit?", L"My application", MB_OKCANCEL) == IDOK)
		{
			DestroyWindow(hwnd);
		}
		return 0;
	}
	case WM_SIZE:
	{
		SendMessage(hwndStatus, WM_SIZE, wParam, lParam);

		// Now get the updated size of the client area
		RECT rcClient;
		GetClientRect(hwnd, &rcClient);

		// Get height of the status bar
		RECT rcStatus;
		GetWindowRect(hwndStatus, &rcStatus);
		int statusHeight = rcStatus.bottom - rcStatus.top;

		// Resize editor to fill remaining space
		MoveWindow(
			hwndEditor,
			0,
			0,
			rcClient.right,
			rcClient.bottom - statusHeight,
			TRUE
		);
		return 0;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	default:
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	}
}

LRESULT CALLBACK EditorProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT)); // monospaced
		RECT clientRect;
		GetClientRect(hwnd, &clientRect);

		int textStartX = GUTTER_WIDTH + 5;
		int textStartY = 5;

		//Background
		HBRUSH bgBrush = CreateSolidBrush(BG_COLOR);
		FillRect(hdc, &clientRect, bgBrush);
		DeleteObject(bgBrush);

		//Gutter
		RECT gutterRect = { 0, 0, GUTTER_WIDTH, clientRect.bottom };
		HBRUSH gutterBrush = CreateSolidBrush(GUTTER_COLOR);
		FillRect(hdc, &gutterRect, gutterBrush);
		DeleteObject(gutterBrush);

		//TEXT COLOR
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, TEXT_COLOR);

		std::vector<std::string> lines = document->GetLines(0, 10);

		for (int i = 0; i < lines.size(); ++i) {
			const std::wstring& line = UTF8ToWide(lines[i]);

			wchar_t lineNum[8];
			swprintf_s(lineNum, L"%*d", 4, i + 1 + lineOffset);

			TextOutW(hdc, 5, textStartY + (int)i * LINE_HEIGHT, lineNum, lstrlenW(lineNum));
			TextOutW(hdc, textStartX, textStartY + (int)i * LINE_HEIGHT, line.c_str(), (int)line.size());
		}

		HPEN cursorPen = CreatePen(PS_SOLID, 2, CURSOR_COLOR);
		HPEN oldPen = (HPEN)SelectObject(hdc, cursorPen);

		int cursorLine = document->GetRow();
		int cursorCol = document->GetColumn();
		int charWidth = 8;
		int cursorX = textStartX + CountUTF8Codepoints(lines[cursorLine - lineOffset], cursorCol) * charWidth;
		int cursorY = textStartY + (cursorLine - lineOffset) * LINE_HEIGHT;
		std::cout << "cursorCol = " << cursorCol << ", visualCol = " << CountUTF8Codepoints(lines[cursorLine - lineOffset], cursorCol) << "\n";

		MoveToEx(hdc, cursorX, cursorY, NULL);
		LineTo(hdc, cursorX, cursorY + LINE_HEIGHT);

		SelectObject(hdc, oldPen);
		DeleteObject(cursorPen);

		EndPaint(hwnd, &ps);
		return 0;
	}
	default:
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	}
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {

	AllocConsole();

	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	freopen_s(&fp, "CONOUT$", "w", stdin);

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	//document = TextDocument("");
	//doc("");
	document->Write("HELLO WORLD!");

	std::string text = document->GetText();
	std::cout << text << "\n";
	HBRUSH bgBrush = CreateSolidBrush(RGB(255, 255, 255));




	WNDCLASS ewc = { };
	ewc.lpfnWndProc = EditorProc;
	ewc.hInstance = hInstance;
	ewc.lpszClassName = TEXT("Editor");
	RegisterClass(&ewc);

	const wchar_t CLASS_NAME[] = L"Notepad--";

	WNDCLASS wc = { };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hbrBackground = bgBrush;

	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(
		0,
		CLASS_NAME,
		L"Notepad--",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,

		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hwnd) return 0;




	ShowWindow(hwnd, nCmdShow);


	UpdateWindow(hwnd);

	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
