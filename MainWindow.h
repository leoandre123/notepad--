#pragma once
#include <wtypes.h>
#include "EditorWindow.h"
#include "ITextDocument.h"
#include "TabControl.h"
#include "ToolBar.h"
#include "TitleBar.h"


class MainWindow {
public:
	MainWindow();

	bool Create();
	void Show();
	void StartMessageLoop();

	//void SetDocument(ITextDocument* document);

	void CreateDocument();
	void OpenDocument(std::wstring path);

private:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	
	int OnCommand(WPARAM wParam, LPARAM lParam);
	int OnNotify(WPARAM wParam, LPARAM lParam);
	int OnResize(int newWidth, int newHeight);
	
	void CreateTitleBar();

	void CreateStatusBar();
	void UpdateStatusBar();
	void SetStatus(int part, const String text);

	void CreateTabControl();
	void SelectTab(int index, bool updateControl);
	void InsertTab(TabContext& context);
	void CloseTab(int index);
	void UpdateTabs();

	void CreateToolBar();


	std::wstring OpenFileDialog();

private:
	HWND m_WindowHandle;
	HMENU m_MenuHandle;
	HWND m_StatusBarHandle;


	EditorWindow m_EditorWindow;
	TabControl m_TabControl;
	ToolBar m_ToolBar;
	TitleBar m_TitleBar;


	bool m_IsTrackingMouse;


	std::vector<TabContext> m_Tabs;
	int m_SelectedTabIndex;
	//ITextDocument* m_ActiveDocument;
	
};