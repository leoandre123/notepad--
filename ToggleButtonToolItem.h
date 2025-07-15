#pragma once
#include "ButtonToolItem.h"
#include <string>
class ToggleButtonToolItem :
    public ButtonToolItem
{
public:
    ToggleButtonToolItem(HWND parent, int commandId, const std::wstring& label);
    ToggleButtonToolItem(HWND parent, int commandId, const std::wstring& label, int iconId);
    ToggleButtonToolItem(HWND parent, int commandId, int iconId);
    //ToggleButtonToolItem(const std::wstring& label);
    // 
    //
    //int GetPreferredWidth(HDC hdc) const override;
    void Paint(HDC hdc, RECT rect, bool isHovered) override;
    //
    void OnClick() override;

private:
    //std::wstring m_Label;
    bool m_IsToggled;
};

