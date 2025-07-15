#include "SeparatorToolItem.h"
#include "ThemeManager.h"

void SeparatorToolItem::Paint(HDC hdc, RECT rect, bool isHovered)
{
    const Theme& theme = ThemeManager::Instance().GetTheme();

    HPEN hPen = CreatePen(PS_SOLID, 1, theme.primaryOutlineColor);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    int midX = (rect.left + rect.right) / 2;
    MoveToEx(hdc, midX, rect.top, NULL);
    LineTo(hdc, midX, rect.bottom);

    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}
