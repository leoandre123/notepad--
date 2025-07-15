#pragma once
#include <wtypes.h>
class ToolItem
{
public:
	virtual ~ToolItem() {}

	virtual int GetPreferredWidth(HDC hdc) const = 0;
	virtual void Paint(HDC hdc, RECT rect, bool isHovered) = 0;

	virtual void OnClick() {};
};

