#pragma once
#include "ToolItem.h"
class SeparatorToolItem :
    public ToolItem
{
public:
private:
    int GetPreferredWidth(HDC hdc) const override { return 8; }
    void Paint(HDC hdc, RECT rect, bool isHovered) override;
};

