#pragma once
#include "ToolItem.h"
#include <string>

class ButtonToolItem :
    public ToolItem
{
public:
    ButtonToolItem(HWND parent, int commandId, const std::wstring& label);
    ButtonToolItem(HWND parent, int commandId, const std::wstring& label, int iconId);
    ButtonToolItem(HWND parent, int commandId, int iconId);

    int GetPreferredWidth(HDC hdc) const override;
    void Paint(HDC hdc, RECT rect, bool isHovered) override;

    void OnClick() override;

protected:
    HWND m_Parent;
    std::wstring m_Label;
    HICON m_Icon;
    int m_CommandId;
};

