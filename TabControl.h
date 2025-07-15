#pragma once
#include <wtypes.h>
#include <vector>
#include <string>

#define TAB_CONTROL_ID 1001
#define TCN_TAB_SWITCH (WM_USER + 1)

struct Tab {
	std::wstring name;
	int width;
};

struct NMTABSWITCH : public NMHDR
{
	int selectedIndex;
};

class TabControl
{
public:
	TabControl();

	bool Create(HWND parentHandle);
	HWND GetHandle() const { return m_Handle; }

	int GetPreferredHeight() const {return m_TabHeight;}

	void AddTab(std::wstring name);	
	void RemoveTab(int index);

	void SelectTab(int index, bool notifyParent = true);

private:
	static LRESULT CALLBACK TabProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);


private:
	HWND m_Handle;
	int m_TabHeight;

	std::vector<Tab> m_Tabs;
	int m_SelectedTab;
};

