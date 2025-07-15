#include "ToggleButtonToolItem.h"
#include "ThemeManager.h"
#include "Globals.h"

ToggleButtonToolItem::ToggleButtonToolItem(HWND parent, int commandId, const std::wstring& label)
	:ButtonToolItem(parent, commandId, label),m_IsToggled(false)
{
}

ToggleButtonToolItem::ToggleButtonToolItem(HWND parent, int commandId,
	const std::wstring& label, int iconId)
	:ButtonToolItem(parent, commandId, label, iconId), m_IsToggled(false)
{
}

ToggleButtonToolItem::ToggleButtonToolItem(HWND parent, int commandId, int iconId)
	:ButtonToolItem(parent, commandId, iconId), m_IsToggled(false)
{
}
void ToggleButtonToolItem::Paint(HDC hdc, RECT rect, bool isHovered)
{
	ButtonToolItem::Paint(hdc, rect, isHovered || m_IsToggled);
}
void ToggleButtonToolItem::OnClick()
{
	m_IsToggled = !m_IsToggled;
}

