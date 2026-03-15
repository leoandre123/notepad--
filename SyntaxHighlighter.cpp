#include "SyntaxHighlighter.h"
#include <regex>
#include <iostream>
#include <Windows.h>


using namespace SyntaxHighlighter;

std::vector<StringColorInfo> SyntaxHighlighter::HighlightLine(const String& line, DocumentType type)
{

	std::vector<StringColorInfo> colorInfo;
	std::vector<SyntaxRule> rules = GetRules(type);
	std::string str = line.ToString();
	std::string::const_iterator searchStart(str.cbegin());


	int pos = 0;
	while (pos < (int)str.length()) {

		std::smatch match;
		std::string sub = str.substr(pos);


		RuleType bestType = (RuleType)-1;
		for (auto& rule : rules) {

			if (std::regex_search(sub, match, rule.regex) && match.position() == 0) {
				bestType = rule.ruleType;
				break;
			}
		}

		if (bestType != -1) {
			std::cout << "[" << match.str() << "]";
			colorInfo.emplace_back(pos, match.length(), RGB(255, 255, 0));
			pos += match.length();
		}
		else {
			colorInfo.emplace_back(pos, 1, RGB(255, 255, 255));
			std::cout << str[pos];
			pos += 1;
		}


	}

	return colorInfo;
}

std::vector<SyntaxRule> SyntaxHighlighter::GetRules(const DocumentType& type)
{
	if (type.name == "C++") {
		return {
			{std::regex(R"(/\/\*[^*]*\*+([^\/][^*]*\*+)*\//g)"), RuleType::Comment},
			{std::regex(R"(/\/\/[^\n\r]+(?:[\n\r]|\*\))/g)"), RuleType::Comment},
			{std::regex(R"(/alignas|alignof|and|and_eq|asm|auto|bitand|bitor|bool|break|case|catch|char|char8_t|char16_t|char32_t|class|compl|concept|const|const_cast|consteval|constexpr|constinit|continue|co_await|co_return|co_yield|decltype|default|delete|do|double|dynamic_cast|else|enum|explicit|export|extern|false|float|for|friend|goto|if|inline|int|long|mutable|namespace|new|noexcept|not|not_eq|nullptr|operator|or|or_eq|private|protected|public|register|reinterpret_cast|requires|return|short|signed|sizeof|static|static_assert|static_cast|struct|switch|template|this|thread_local|throw|true|try|typedef|typeid|typename|union|unsigned|using declaration|using directive|virtual|void|volatile|wchar_t|while|xor|xor_eq/g)"), RuleType::Keyword},
		};
	}

	return {};
}

//COLORREF SyntaxHighlighter::GetColor(HighlightTokenCategory category)
//{
//    switch (category) {
//        case HighlightTokenCategory::Identifier     : return RGB(128,128,255);
//        case HighlightTokenCategory::Keyword        : return RGB(0,0,255);
//        case HighlightTokenCategory::Seperator      : return RGB(64,64,64);
//        case HighlightTokenCategory::Operator       : return RGB(64,64,64);
//        case HighlightTokenCategory::BooleanLiteral : return RGB(0,0,255);
//        case HighlightTokenCategory::NumberLiteral  : return RGB(128,255,128);
//        case HighlightTokenCategory::StringLiteral  : return RGB(255,127,0);
//        case HighlightTokenCategory::Comment        : return RGB(0,255,0);
//    }
//}
