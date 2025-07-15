#pragma once
#include <wtypes.h>
#include <vector>
#include "ToolItem.h"

class ToolBar
{
public:
	ToolBar();
	bool Create(HWND parentHandle);
	void AddItem(ToolItem* item);
	HWND GetHandle() const { return m_Handle; }
	int GetPreferredHeight() const { return m_PreferredHeight; }


private:
	static LRESULT CALLBACK ToolProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void Paint();
	void OnMouseMove(int x, int y);
	void OnMouseClick(int x, int y);

private:
	HWND m_Handle;
	int m_PreferredHeight;

	std::vector<ToolItem*> m_Items;
	int m_HoveredItem;
	bool m_MouseTracking;
};

