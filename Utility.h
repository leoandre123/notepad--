#pragma once
#include <wtypes.h>
#include <string>

struct FontSpec
{
    std::wstring faceName;
    int pointSize;
    int weight;
    bool italic;
};

HFONT CreateFontFromSpec(const FontSpec& spec, int pointSize);