#pragma once
#include <vector>
#include "String.h"
#include <unordered_set>

enum HighlightTokenCategory {
	Identifier,
	Keyword,
	Seperator,
	Operator,

	BooleanLiteral,
	NumberLiteral,
	StringLiteral,

	Comment,
};
struct StringColorInfo {
	HighlightTokenCategory category;
	int start;
	int length;
	//COLORREF color;
};
struct Token {
	HighlightTokenCategory category;
	std::string value;
};

class SyntaxHighlighter
{
public:
	std::vector<StringColorInfo> HighlightLine(const String& line);

	COLORREF GetColor(HighlightTokenCategory category);

private:

private:
	std::unordered_set<std::string> m_Keywords;
};

