#include "ThemeManager.h"
#include "Math.h"

ThemeManager& ThemeManager::Instance()
{
    static ThemeManager instance;
    return instance;
}

void ThemeManager::LoadDefaultTheme()
{
	m_ActiveTheme.windowBackgroundColor = RGB(24, 24, 24);
	m_ActiveTheme.primaryBackgroundColor = RGB(61, 61, 61);
	m_ActiveTheme.secondaryBackgroundColor = RGB(41, 41, 41);
	

	m_ActiveTheme.primaryOutlineColor = RGB(112,112,112);

	m_ActiveTheme.primaryTextColor = RGB(255,255,255);
	m_ActiveTheme.secondaryTextColor = RGB(200, 200, 200);

	m_ActiveTheme.editorBackgroundColor = RGB(35, 35, 35);
	m_ActiveTheme.editorTextColor = RGB(255, 255, 255);
	m_ActiveTheme.editorSelectionColor = RGB(100, 100, 100);
	m_ActiveTheme.editorCursorColor = RGB(255,255, 0);


	m_ActiveTheme.primaryFont = {
		L"Segoe UI",
		12,
		FW_NORMAL,
		false
	};
	m_ActiveTheme.editorFont = {
		L"Consolas",
		12,
		FW_NORMAL,
		false
	};
}


