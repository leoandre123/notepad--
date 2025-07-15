#pragma once
#include <wtypes.h>
#include "Utility.h"

struct Theme {
	/*COLORS*/
	COLORREF windowBackgroundColor;
	COLORREF primaryBackgroundColor;
	COLORREF secondaryBackgroundColor;
	

	COLORREF primaryOutlineColor;

	COLORREF primaryTextColor;
	COLORREF secondaryTextColor;
	


	/*EDITOR*/
	COLORREF editorBackgroundColor;
	COLORREF editorTextColor;
	COLORREF editorSelectionColor;
	COLORREF editorCursorColor;

	/*FONTS*/
	FontSpec primaryFont;
	FontSpec editorFont;
};

class ThemeManager
{
public:
	static ThemeManager& Instance();

	const Theme& GetTheme() const { return m_ActiveTheme; }

private:
	ThemeManager() { LoadDefaultTheme(); }
	void LoadDefaultTheme();

private:
	Theme m_ActiveTheme;
};

