#pragma comment (lib, "Dwmapi")
#include "MainWindow.h"
#include "Globals.h"
#include <CommCtrl.h>
#include <iostream>
#include <bitset>
#include <fstream>
#include "LineTextDocument.h"
#include "TabContext.h"
#include <dwmapi.h>
#include "ButtonToolItem.h"
#include "ToggleButtonToolItem.h"
#include "SeparatorToolItem.h"
#include "resource.h"
#include <windowsx.h>
#include "ThemeManager.h"
#include "String.h"
#include "Math.h"

#define STATUS_ID 0


#define STATUS_BAR_PART_COUNT 4


#define ID_FILE_NEW		101
#define ID_FILE_OPEN	102
#define ID_FILE_SAVE	103
#define ID_FILE_SAVE_AS	104
#define ID_FILE_EXIT	105

#define ID_EDIT_UNDO	201
#define ID_EDIT_REDO	202
#define ID_EDIT_COPY	203
#define ID_EDIT_PASTE	204
#define ID_EDIT_CUT		205




MainWindow::MainWindow():
	m_IsTrackingMouse(false)
{
}

bool MainWindow::Create()
{
	if (m_Tabs.empty()) {
		TabContext context;
		context.document = new LineTextDocument("");
		context.title = L"new 1";
		context.lineOffset = 0;

		m_Tabs.push_back(context);
	}

	m_SelectedTabIndex = 0;
	m_EditorWindow.SetTabContext(&m_Tabs[0]);


	std::cout << "Creating Main Window...\n";

	if (g_hInstance == NULL) {
		std::cerr << "Failed: g_hInstance is NULL" << "\n";
		return false;
	}

	HBRUSH bgBrush = CreateSolidBrush(RGB(255, 255, 255));
	const wchar_t CLASS_NAME[] = L"Notepad--";

	WNDCLASS wc = { };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = g_hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hbrBackground = bgBrush;

	ATOM atom = RegisterClass(&wc);

	if (!atom) {
		std::cerr << "MainWindow Creation RegisterClass failed: " << GetLastError() << "\n";
		return false;
	}

	m_WindowHandle = CreateWindowEx(
		0,
		CLASS_NAME,
		L"Notepad--",
		WS_POPUP,


		CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768,
		NULL,
		NULL,
		g_hInstance,
		this
	);

	if (!m_WindowHandle) {
		std::cerr << "MainWindow Creation CreateWindowEx failed: " << GetLastError() << "\n";
		return 0;
	}

	

	BOOL disable = TRUE;
	DwmSetWindowAttribute(
		m_WindowHandle,
		DWMWA_NCRENDERING_POLICY,
		&disable,
		sizeof(disable)
	);


	DWORD radius = DWM_WINDOW_CORNER_PREFERENCE::DWMWCP_ROUND;
	DwmSetWindowAttribute(
		m_WindowHandle,
		DWMWA_WINDOW_CORNER_PREFERENCE,
		&radius,
		sizeof(radius)
	);

	std::cout << "MainWindow Creation Done\n";
	return true;
}

void MainWindow::Show()
{
	ShowWindow(m_WindowHandle, SW_SHOWDEFAULT);
	UpdateWindow(m_WindowHandle);
}

void MainWindow::StartMessageLoop()
{
	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MainWindow* pThis = nullptr;
	if (uMsg == WM_NCCREATE) {
		CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
		pThis = (MainWindow*)cs->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
		pThis->m_WindowHandle = hwnd;
		g_HandleMainWindow = hwnd;
		return true;
	}
	else {
		pThis = (MainWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	}

	if (pThis) {
		return pThis->HandleMessage(uMsg, wParam, lParam);
	}
	else {
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE: {
		

		CreateTitleBar();
		SetMenu(m_WindowHandle, m_MenuHandle);
		CreateStatusBar();
		CreateTabControl();
		m_EditorWindow.Create(m_WindowHandle);
		m_TabControl.Create(m_WindowHandle);
		CreateToolBar();

		UpdateStatusBar();
		return 0;
	}
	case WM_NCCALCSIZE: {
		return 0;
	}
	case WM_ERASEBKGND:
	{
		return 1;
	}
	case WM_NCHITTEST:
	{
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		ScreenToClient(m_WindowHandle, &pt);

		LRESULT result = m_TitleBar.OnNcHitTest(pt);

		if (result != HTCLIENT)
			return result;

		break;
	}
	case WM_SETCURSOR: {
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		return TRUE;
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
			m_Tabs[m_SelectedTabIndex].document->Write(" ");
		}
		else if (vk == VK_RETURN)
		{
			m_Tabs[m_SelectedTabIndex].document->Write("\n");
		}
		else if (vk == VK_LEFT)
		{
			m_Tabs[m_SelectedTabIndex].document->MoveCursor(CursorDirection::WEST, 1);
			m_EditorWindow.ScrollToCursor();
		}
		else if (vk == VK_RIGHT)
		{
			m_Tabs[m_SelectedTabIndex].document->MoveCursor(CursorDirection::EAST, 1);
			m_EditorWindow.ScrollToCursor();
		}
		else if (vk == VK_UP)
		{
			m_Tabs[m_SelectedTabIndex].document->MoveCursor(CursorDirection::NORTH, 1);
			m_EditorWindow.ScrollToCursor();
		}
		else if (vk == VK_DOWN)
		{
			m_Tabs[m_SelectedTabIndex].document->MoveCursor(CursorDirection::SOUTH, 1);
			m_EditorWindow.ScrollToCursor();
		}
		else if (vk == VK_BACK)
		{
			m_Tabs[m_SelectedTabIndex].document->DeleteAtCursor(true);
		}
		else if (vk == VK_DELETE)
		{
			m_Tabs[m_SelectedTabIndex].document->DeleteAtCursor(false);
		}
		else if (vk == VK_TAB)
		{
			m_Tabs[m_SelectedTabIndex].document->Write("\t");
		}

		UpdateStatusBar();
		UpdateTabs();
		m_Tabs[m_SelectedTabIndex].document->PrintDebugString();
		InvalidateRect(m_EditorWindow.GetHandle(), NULL, true);
		return 0;
	}
	case WM_CHAR: {
		wchar_t vk = (wchar_t)wParam;
		std::cout << "CHAR:" << vk << " - " << (int)vk << "\n";

		if (vk > 32) {
			//document->Write(std::string(1, vk));

			std::wstring wstr(1, vk);
			m_Tabs[m_SelectedTabIndex].document->Write(wstr.c_str());
		}



		return 0;
	}
	case WM_COMMAND: {
		return OnCommand(wParam, lParam);
	}
	case WM_NOTIFY: {
		return OnNotify(wParam, lParam);
	}
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(m_WindowHandle, &ps);

		RECT rc;
		GetClientRect(m_WindowHandle, &rc);

		m_TitleBar.OnPaint(hdc, rc);

		EndPaint(m_WindowHandle, &ps);
		return 0;
	}
	case WM_NCPAINT: {
		HDC hdc = GetWindowDC(m_WindowHandle);
		if (hdc)
		{
			RECT rc;
			GetWindowRect(m_WindowHandle, &rc);
			OffsetRect(&rc, -rc.left, -rc.top);

			m_TitleBar.OnPaint(hdc, rc);

			ReleaseDC(m_WindowHandle, hdc);
		}
		return 0;
	}
	case WM_MOUSEMOVE:
	{
		if (!m_IsTrackingMouse)
		{
			TRACKMOUSEEVENT tme = { sizeof(tme) };
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = m_WindowHandle;
			TrackMouseEvent(&tme);
			m_IsTrackingMouse = true;
		}
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
		//ScreenToClient(m_WindowHandle, &pt);
		m_TitleBar.OnMouseMove(pt);
		return 0;
	}
	case WM_MOUSELEAVE:
	{
		m_IsTrackingMouse = false;
		POINT pt = {-1, -1 };
		m_TitleBar.OnMouseMove(pt);
		return 0;
	}
	case WM_LBUTTONDOWN: {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		m_TitleBar.OnMouseClick(pt);
		return 0;
	}
	case WM_NCMOUSEMOVE:
	{
		//POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		//ScreenToClient(m_WindowHandle, &pt);
		//m_TitleBar.OnMouseMove(pt);
		return 0;
	}
	case WM_CLOSE:
	{
		if (MessageBox(m_WindowHandle, L"Really quit?", L"My application", MB_OKCANCEL) == IDOK)
		{
			DestroyWindow(m_WindowHandle);
		}
		return 0;
	}
	case WM_SIZE: {
		SendMessage(m_StatusBarHandle, WM_SIZE, wParam, lParam);
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);
		return OnResize(width, height);
	}
	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;

		HMONITOR hMon = MonitorFromWindow(m_WindowHandle, MONITOR_DEFAULTTONEAREST);

		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hMon, &mi);

		lpMMI->ptMaxPosition.x = mi.rcWork.left - mi.rcMonitor.left;
		lpMMI->ptMaxPosition.y = mi.rcWork.top - mi.rcMonitor.top;

		lpMMI->ptMaxSize.x = mi.rcWork.right - mi.rcWork.left;
		lpMMI->ptMaxSize.y = mi.rcWork.bottom - mi.rcWork.top;

		return 0;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	default:
	{
		return DefWindowProc(m_WindowHandle, uMsg, wParam, lParam);
	}
	}
}

int MainWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam)) {
	case ID_FILE_NEW: {
		CreateDocument();
		break;
	}
	case ID_FILE_OPEN:
	{
		std::wstring path = OpenFileDialog();
		std::wcout << path << "\n";
		OpenDocument(path);


		break;
	}
	case ID_FILE_SAVE:
		MessageBox(m_WindowHandle, L"Save clicked", L"Menu", MB_OK);
		break;
	case ID_FILE_EXIT:
		DestroyWindow(m_WindowHandle);
		break;
	case ID_EDIT_UNDO:
		m_Tabs[m_SelectedTabIndex].document->Undo();
		InvalidateRect(m_EditorWindow.GetHandle(), NULL, true);
		break;
	case ID_EDIT_REDO:
		m_Tabs[m_SelectedTabIndex].document->Redo();
		InvalidateRect(m_EditorWindow.GetHandle(), NULL, true);
		break;
	}
	return 0;
}

int MainWindow::OnNotify(WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmhdr = (LPNMHDR)lParam;

	if (pnmhdr->code == TCN_TAB_SWITCH)
	{
		NMTABSWITCH* pnm = (NMTABSWITCH*)pnmhdr;
		SelectTab(pnm->selectedIndex, false);
		return 0;
	}

	return 1;
}

int MainWindow::OnResize(int newWidth, int newHeight)
{
	RECT rcClient;
	GetClientRect(m_WindowHandle, &rcClient);

	RECT rcStatus;
	GetWindowRect(m_StatusBarHandle, &rcStatus);
	int statusHeightPx = rcStatus.bottom - rcStatus.top;


	int toolBarHeightPx = DipToPixels(m_ToolBar.GetPreferredHeight());
	int tabHeightPx = DipToPixels(m_TabControl.GetPreferredHeight());
	int titleHeightPx = DipToPixels(30);


	MoveWindow(
		m_EditorWindow.GetHandle(),
		0,
		titleHeightPx + toolBarHeightPx + tabHeightPx,
		rcClient.right,
		rcClient.bottom - statusHeightPx - tabHeightPx - toolBarHeightPx - titleHeightPx,
		TRUE
	);

	MoveWindow(
		m_TabControl.GetHandle(),
		0,
		titleHeightPx + toolBarHeightPx,
		rcClient.right,
		tabHeightPx,
		TRUE
	);

	MoveWindow(
		m_ToolBar.GetHandle(),
		0,
		titleHeightPx,
		rcClient.right,
		toolBarHeightPx,
		TRUE
	);

	return 0;
}

void MainWindow::CreateTitleBar()
{

	m_TitleBar.SetParent(m_WindowHandle);

	PopupMenu* fileMenu = new PopupMenu(m_WindowHandle, m_WindowHandle);

	fileMenu->AppendString(L"New", L"Ctrl+N", ID_FILE_NEW);
	fileMenu->AppendString(L"Open", ID_FILE_OPEN);
	fileMenu->AppendSeparator();
	fileMenu->AppendString(L"Save", L"Ctrl+S", ID_FILE_SAVE);
	fileMenu->AppendString(L"Save As", ID_FILE_SAVE_AS);
	fileMenu->AppendSeparator();
	fileMenu->AppendString(L"Exit", L"Alt+F4", ID_FILE_EXIT);

	PopupMenu* editMenu = new PopupMenu(m_WindowHandle, m_WindowHandle);

	editMenu->AppendString(L"Undo", L"Ctrl+Z", ID_EDIT_UNDO);
	editMenu->AppendString(L"Redo", L"Ctrl+Y", ID_EDIT_REDO);
	editMenu->AppendSeparator();
	editMenu->AppendString(L"Copy", L"Ctrl+C", ID_EDIT_COPY);
	editMenu->AppendString(L"Paste", L"Ctrl+V", ID_EDIT_PASTE);
	editMenu->AppendString(L"Cut", L"Ctrl+X", ID_EDIT_CUT);

	PopupMenu* nestedMenu = new PopupMenu(m_WindowHandle, editMenu->GetHandle());
	nestedMenu->AppendString(L"Nest 1", ID_EDIT_UNDO);
	nestedMenu->AppendString(L"Nest 2", ID_EDIT_REDO);
	editMenu->AppendPopup(L"Nested", nestedMenu);


	PopupMenu* settingsMenu = new PopupMenu(m_WindowHandle, m_WindowHandle);
	settingsMenu->AppendString(L"Settings...", 0);
	settingsMenu->AppendString(L"Theme", 0);
	settingsMenu->AppendString(L"Edit shortcuts", 0);



	m_TitleBar.AddMenu(L"File", *fileMenu);
	m_TitleBar.AddMenu(L"Edit", *editMenu);
	m_TitleBar.AddMenu(L"Settings", *settingsMenu);
}

void MainWindow::CreateStatusBar()
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
		m_WindowHandle,              // handle to parent window
		(HMENU)STATUS_ID,       // child window identifier
		g_hInstance,                   // handle to application instance
		NULL);                   // no window creation data

	// Get the coordinates of the parent window's client area.
	GetClientRect(m_WindowHandle, &rcClient);

	// Allocate an array for holding the right edge coordinates.
	hloc = LocalAlloc(LHND, sizeof(int) * STATUS_BAR_PART_COUNT);
	paParts = (PINT)LocalLock(hloc);

	// Calculate the right edge coordinate for each part, and
	// copy the coordinates to the array.
	nWidth = rcClient.right / STATUS_BAR_PART_COUNT;
	int rightEdge = nWidth;
	for (i = 0; i < STATUS_BAR_PART_COUNT; i++) {
		paParts[i] = rightEdge;
		rightEdge += nWidth;
	}

	paParts[STATUS_BAR_PART_COUNT - 1] = -1;

	// Tell the status bar to create the window parts.
	SendMessage(hwndStatus, SB_SETPARTS, (WPARAM)STATUS_BAR_PART_COUNT, (LPARAM)
		paParts);

	// Free the array, and return.
	LocalUnlock(hloc);
	LocalFree(hloc);
	m_StatusBarHandle = hwndStatus;
}

void MainWindow::CreateTabControl()
{
	for (int i = 0; i < (int)m_Tabs.size(); i++) {
		m_TabControl.AddTab(m_Tabs[i].title);
	}
}

void MainWindow::CreateDocument()
{
	TabContext context;
	context.document = new LineTextDocument("");
	context.title = L"";
	context.lineOffset = 0;
	InsertTab(context);
}

void MainWindow::OpenDocument(std::wstring path)
{
	std::ifstream file(path);
	std::string str((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());

	TabContext context;
	context.document = new LineTextDocument(str);
	context.title = path;
	context.lineOffset = 0;

	InsertTab(context);
}


void MainWindow::SelectTab(int index, bool updateControl)
{
	m_SelectedTabIndex = index;
	m_EditorWindow.SetTabContext(&m_Tabs[m_SelectedTabIndex]);
	InvalidateRect(m_EditorWindow.GetHandle(), NULL, true);
	if (updateControl) {
		m_TabControl.SelectTab(index, false);
	}
}

void MainWindow::InsertTab(TabContext& context)
{
	if (context.title.empty()) {

		int titleNum = 1;
		std::wstring name;
		bool hasFound = false;
		while (!hasFound)
		{
			hasFound = true;
			name = L"new " + std::to_wstring(titleNum);
			for (int i = 0; i < (int)m_Tabs.size(); i++) {
				if (m_Tabs[i].title == name) {
					hasFound = false;
					break;
				}
			}
			titleNum++;
		}
		context.title = name;
	}
	//TCITEM tie = {};
	//tie.mask = TCIF_TEXT;
	//tie.pszText = (LPWSTR)context.title.c_str();
	//
	//TabCtrl_InsertItem(m_TabControlHandle, (int)m_Tabs.size(), &tie);
	m_TabControl.AddTab(context.title);
	m_Tabs.push_back(context);

	m_EditorWindow.SetTabContext(&m_Tabs[m_SelectedTabIndex]);
}

void MainWindow::CloseTab(int index)
{
}

void MainWindow::UpdateTabs()
{
	//for (int i = 0; i < (int)m_Tabs.size(); i++) {
	//	TCITEM tie = {};
	//	tie.mask = TCIF_TEXT;
	//
	//	std::wstring title = m_Tabs[i].title + (m_Tabs[i].document->GetLength() ? L"*" : L"");
	//
	//	tie.pszText = (LPWSTR)title.c_str();
	//
	//	TabCtrl_SetItem(m_TabControlHandle, i, &tie);
	//}
}

void MainWindow::CreateToolBar()
{
	m_ToolBar.AddItem(new ButtonToolItem(m_WindowHandle, ID_FILE_NEW, IDI_NEW_DOCUMENT));
	m_ToolBar.AddItem(new ButtonToolItem(m_WindowHandle, ID_FILE_SAVE, IDI_SAVE));
	m_ToolBar.AddItem(new SeparatorToolItem());
	m_ToolBar.AddItem(new ButtonToolItem(m_WindowHandle, ID_EDIT_UNDO, IDI_UNDO));
	m_ToolBar.AddItem(new ButtonToolItem(m_WindowHandle, ID_EDIT_REDO, IDI_REDO));
	m_ToolBar.AddItem(new SeparatorToolItem());
	m_ToolBar.AddItem(new ButtonToolItem(m_WindowHandle, ID_EDIT_COPY, IDI_COPY));
	m_ToolBar.AddItem(new ButtonToolItem(m_WindowHandle, ID_EDIT_PASTE, IDI_CLIPBOARD));
	m_ToolBar.Create(m_WindowHandle);
}

void MainWindow::UpdateStatusBar()
{
	int pos = m_Tabs[m_SelectedTabIndex].document->GetCursorPos() + 1;
	int row = m_Tabs[m_SelectedTabIndex].document->GetRow() + 1;
	int col = m_Tabs[m_SelectedTabIndex].document->GetColumn() + 1;
	int length = m_Tabs[m_SelectedTabIndex].document->GetLength();
	int lines = m_Tabs[m_SelectedTabIndex].document->GetLineCount();



	String lengthString = "Length: " + std::to_string(length) + " Lines: " + std::to_string(lines);
	String cursorString =
		"Ln: " + std::to_string(row) + " Col: " + std::to_string(col) + " Pos: " + std::to_string(pos);

	SetStatus(0, lengthString);
	SetStatus(1, cursorString);
}

void MainWindow::SetStatus(int part, const String text) {
	SendMessage(m_StatusBarHandle, SB_SETTEXT, part, (LPARAM)(text.ToWString().c_str()));
}

std::wstring MainWindow::OpenFileDialog()
{
	OPENFILENAME ofn = { 0 };
	wchar_t szFile[MAX_PATH] = { 0 };

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_WindowHandle;
	ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = L"txt";

	if (GetOpenFileName(&ofn)) {
		return std::wstring(ofn.lpstrFile);  // File successfully selected
	}

	return L""; // User canceled or error
}
