#pragma once
#include "ITextDocument.h"
struct TabContext
{
	std::wstring title;
	ITextDocument* document;
	int lineOffset;
};
