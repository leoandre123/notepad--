#pragma once
#include <vector>
#include "String.h"
#include <unordered_set>
#include <regex>
#include "ITextDocument.h"

namespace SyntaxHighlighter {

	struct StringColorInfo {
		int start;
		int length;
		COLORREF color;

        StringColorInfo(int s, int l, COLORREF c) : start(s), length(l), color(c) {}
	};

	enum RuleType {
		Comment,
		Keyword,
		Operator,
		StringLiteral
	};

	struct SyntaxRule {
		std::regex regex;
		RuleType ruleType;
	};

	std::vector<StringColorInfo> HighlightLine(const String& line, DocumentType type);
	std::vector<SyntaxRule> GetRules(const DocumentType& type);
}



