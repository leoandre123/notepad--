#include "Math.h"
#include "Globals.h"

bool Math::PointInRect(const POINT& pt, const RECT& rc)
{
	return pt.x >= rc.left && pt.x <= rc.right
		&& pt.y >= rc.top && pt.y <= rc.bottom;
}

int DipToPixels(int dip) {
	UINT dpi = GetDpiForWindow(g_HandleMainWindow);
	int scaledPx = MulDiv(dip, dpi, 96);
	return scaledPx;
}