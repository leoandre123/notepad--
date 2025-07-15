#pragma once
#include <wtypes.h>
#include <string>
#include "PopupMenu.h"

struct TitleBarMenuItem
{
	std::wstring label;
	PopupMenu menu;
	RECT labelRect;
};

class TitleBar
{
public:
	TitleBar(int height = 30);

	LRESULT OnNcHitTest(const POINT& pt);
	void OnPaint(HDC hdc, const RECT& clientArea);
	
	void OnMouseMove(const POINT& pt);
	void OnMouseClick(const POINT& pt);

	void AddMenu(const std::wstring& label, PopupMenu& popup);

	void SetParent(HWND parent) { m_ParentHandle = parent; }

	int GetHeight() const {return m_Height;}

private:
	void PaintWindowButtons(HDC hdc, RECT& titleArea);
	void PaintMenu(HDC hdc, RECT& titleArea);
	void PaintIcon(HDC hdc, RECT& titleArea);
	void PaintLabel(HDC hdc, RECT& titleArea);


	bool PointInRect(const POINT& pt, const RECT& rc);

private:
	HWND m_ParentHandle;
	int m_Height;
	
	int m_HoveredButton;

	RECT m_CloseRect;
	RECT m_MinimizeRect;
	RECT m_ExpandRect;


	std::vector<TitleBarMenuItem> m_MenuItems;
	int m_HoveredMenuItem;
};

