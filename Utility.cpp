#include "Utility.h"
#include "Globals.h"

HFONT CreateFontFromSpec(const FontSpec& spec, int pointSize) {

    UINT dpi = GetDpiForWindow(g_HandleMainWindow);

    LOGFONT lf = {};
    lf.lfHeight = -MulDiv(pointSize, dpi, 96);
    lf.lfWeight = spec.weight;
    lf.lfItalic = spec.italic ? TRUE : FALSE;
    wcsncpy_s(lf.lfFaceName, spec.faceName.c_str(), LF_FACESIZE);

    return CreateFontIndirect(&lf);
}