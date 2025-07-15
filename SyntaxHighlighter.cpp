#include "SyntaxHighlighter.h"

std::vector<StringColorInfo> SyntaxHighlighter::HighlightLine(const String& line)
{
    std::vector<StringColorInfo> colorInfo;
    int length = line.GetByteCount();
    for (int i = 0; i < length;) {
        char c = line.ByteAt(i);
        char next = i + 1 < length ? line.ByteAt(i + 1) : ';';

        if (isspace(c)) {
            i++;
        }
        else if (isalpha(c) || c == '_') {

            int start = i;
            while (i < length && (isalnum(line.ByteAt(i)) || line.ByteAt(i) == '_')) i++;

            std::string word = line.Substring(start, i - start).ToString();

            HighlightTokenCategory cat = m_Keywords.count(word) ?
                HighlightTokenCategory::Keyword
                : HighlightTokenCategory::Identifier;

            colorInfo.push_back({ cat, start, i - start });
        }
        else if (isdigit(c)) {
            int start = i;
            while (i < length && (isdigit(line.ByteAt(i)) || line.ByteAt(i) == '.'
                || line.ByteAt(i) == 'f'
                )) i++;
            //std::string word = line.Substring(start, i - start).ToString();

            colorInfo.push_back({ HighlightTokenCategory::NumberLiteral, start, i - start });
        }
        else if (c == '"') {
            int start = i;
            while (i < length && line.ByteAt(i) != '"') i++;
            std::string word = line.Substring(start, i - start).ToString();

            colorInfo.push_back({ HighlightTokenCategory::StringLiteral, start, i - start });
        }
        else if (c == '\'') {
            int start = i;
            while (i < length && line.ByteAt(i) != '\'') i++;
            std::string word = line.Substring(start, i - start).ToString();

            colorInfo.push_back({ HighlightTokenCategory::StringLiteral, start, i - start });
        }
        else if (c == '/' && (next == '/' || next == '*')) {
            int start = i;

            if (next == '/') {
                while (i < length && line.ByteAt(i) != '\n') i++;
                std::string word = line.Substring(start, i - start).ToString();

                colorInfo.push_back({ HighlightTokenCategory::Comment, start, i - start });
            }
            else {
                while (i + 1 < length && line.ByteAt(i) != '*' && line.ByteAt(i + 1) == '/') i++;
                std::string word = line.Substring(start, i - start).ToString();

                colorInfo.push_back({ HighlightTokenCategory::Comment, start, i - start });
            }
        }
        else {
            int start = i++;
            colorInfo.push_back({ HighlightTokenCategory::Comment, start, 1});
        }
    }

    return colorInfo;
}

COLORREF SyntaxHighlighter::GetColor(HighlightTokenCategory category)
{
    switch (category) {
        case HighlightTokenCategory::Identifier     : return RGB(128,128,255);
        case HighlightTokenCategory::Keyword        : return RGB(0,0,255);
        case HighlightTokenCategory::Seperator      : return RGB(64,64,64);
        case HighlightTokenCategory::Operator       : return RGB(64,64,64);
        case HighlightTokenCategory::BooleanLiteral : return RGB(0,0,255);
        case HighlightTokenCategory::NumberLiteral  : return RGB(128,255,128);
        case HighlightTokenCategory::StringLiteral  : return RGB(255,127,0);
        case HighlightTokenCategory::Comment        : return RGB(0,255,0);
    }
}
