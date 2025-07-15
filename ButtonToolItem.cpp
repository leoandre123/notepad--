#include "ButtonToolItem.h"
#include "ThemeManager.h"
#include "Globals.h"
#include "Math.h"

const int PADDING = 4;
const int ICON_SIZE = 16;

ButtonToolItem::ButtonToolItem(HWND parent, int commandId, const std::wstring& label)
	:
	m_Parent(parent),
	m_Label(label),
	m_CommandId(commandId),
	m_Icon(NULL)
{

}

ButtonToolItem::ButtonToolItem(HWND parent, int commandId,
	const std::wstring& label, int iconId)
	:
	m_Parent(parent),
	m_Label(label),
	m_CommandId(commandId)
{
	m_Icon = (HICON)LoadImage(
		g_hInstance,
		MAKEINTRESOURCE(iconId),
		IMAGE_ICON,
		DipToPixels(ICON_SIZE), DipToPixels(ICON_SIZE),
		LR_DEFAULTCOLOR
	);

}

ButtonToolItem::ButtonToolItem(HWND parent, int commandId, int iconId)
	:
	m_Parent(parent),
	m_CommandId(commandId)
{
	m_Icon = (HICON)LoadImage(
		g_hInstance,
		MAKEINTRESOURCE(iconId),
		IMAGE_ICON,
		DipToPixels(ICON_SIZE), DipToPixels(ICON_SIZE),
		LR_DEFAULTCOLOR
	);
}

int ButtonToolItem::GetPreferredWidth(HDC hdc) const
{
	SIZE size;
	GetTextExtentPoint32W(hdc, m_Label.c_str(), (int)m_Label.size(), &size);
	return PADDING + (m_Icon ? DipToPixels(ICON_SIZE) : 0) + size.cx + PADDING;
}

void ButtonToolItem::Paint(HDC hdc, RECT rect, bool isHovered)
{
	const Theme& theme = ThemeManager::Instance().GetTheme();
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, theme.primaryTextColor);

	if (isHovered) {
		HBRUSH hoverBg = CreateSolidBrush(theme.primaryBackgroundColor);
		HBRUSH hoverOutline = CreateSolidBrush(theme.primaryOutlineColor);
		FillRect(hdc, &rect, hoverBg);
		FrameRect(hdc, &rect, hoverOutline);
	}

	if (!m_Label.empty()) {
		RECT textRect = { rect.left + (m_Icon ? 20 : 0), rect.top, rect.right, rect.bottom };
		DrawTextW(hdc, m_Label.c_str(), -1, &textRect,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}




	if (m_Icon) {
		int iconSizePx = DipToPixels(ICON_SIZE);
		DrawIconEx(
			hdc,
			(rect.left + rect.right - iconSizePx) / 2,
			(rect.top + rect.bottom - iconSizePx) / 2,
			m_Icon,
			iconSizePx,
			iconSizePx,
			0,
			NULL,
			DI_NORMAL
		);
	}
}

void ButtonToolItem::OnClick()
{
	SendMessage(m_Parent, WM_COMMAND, m_CommandId, 0);
}

