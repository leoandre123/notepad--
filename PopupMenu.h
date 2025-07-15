#pragma once
#include <wtypes.h>
#include <string>
#include <vector>

enum PopupItemType {
	Button,
	Popup,
	Separator,
};

struct PopupItem {
	PopupItemType type;
	std::wstring label;
	std::wstring shortcut;
	int command;
	RECT rect;
	void* data;
};

class PopupMenu
{
public:
	PopupMenu(HWND mainWindow, HWND owner);

	void AppendString(const std::wstring& name, int command);
	void AppendString(const std::wstring& name, const std::wstring& shortcut, int command);
	void AppendSeparator();
	void AppendPopup(const std::wstring& name, PopupMenu* popup);

	

	void Show(bool show, POINT pos = {0,0});

	void SetLabel(const std::wstring& label) { m_Label = label; }
	std::wstring GetLabel() const { return m_Label; }

	HWND GetHandle() { return m_Handle; }

	bool IsVisible() { return m_IsVisible; }

private:
	static LRESULT CALLBACK PopupProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnPaint(HDC hdc);
	void OnMouseMove(const POINT& pt);
	void OnMouseClick(const POINT& pt);

	void CalculateSize();

private:

	HWND m_MainWindowHandle;
	HWND m_Handle;

	std::vector<PopupItem> m_Items;

	int m_HoveredItem;

	std::wstring m_Label;

	bool m_IsVisible;
};

