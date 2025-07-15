#include "String.h"
#include <iostream>

void String::Clear()
{
	m_Data.clear();
}

void String::Insert(int pos, const String& text)
{
	std::string str = text.ToString();



	std::cout << "Pos: " << pos << " Str: " << str << " Size: " << str.size() << "\n";
	std::cout << "Before: " << m_Data << "\n";
	m_Data.insert(pos, str);
	std::cout << "After: " << m_Data << "\n";
}

void String::Delete(int pos, int length)
{
	m_Data.erase(pos, length);
}

void String::operator+=(const String& rhs)
{
	m_Data += rhs.ToString();
}

void String::operator+=(const char& rhs)
{
	m_Data += rhs;
}

int String::GetVisualWidth(HDC hdc, int columnLimit) const
{
	SIZE spaceSize;
	GetTextExtentPoint32W(hdc, L" ", 1, &spaceSize);
	int spaceWidth = spaceSize.cx;

	std::wstring wstr = ToWString();

	int visualWidth = 0;
	int column = 0;

	for (int i = 0; i < wstr.size();) {

		if (column == columnLimit)break;

		wchar_t wc = wstr[i];

		int advance = 1;
		if (wc >= 0xD800 && wc <= 0xDBFF) {
			if (i + 1 < (int)wstr.size()) {
				wchar_t next = wstr[i + 1];
				if (next >= 0xDC00 && next <= 0xDFFF) {
					advance = 2;
				}
			}
		}

		if (wc == L'\t') {
			int spaces = m_TabWidth - (column % m_TabWidth);
			visualWidth += spaceWidth * spaces;
			column += spaces;
		}
		else {
			SIZE size;
			GetTextExtentPoint32W(hdc, wstr.c_str() + i, advance, &size);
			visualWidth += size.cx;
			column += 1;
		}

		i += advance;
	}

	return visualWidth;
}

std::wstring String::ToWStringWithExpandedTabs() const {
	std::wstring line = ToWString();
	std::wstring result;
	int visualCol = 0;
	for (wchar_t ch : line) {
		if (ch == L'\t') {
			int spacesToNextTab = m_TabWidth - (visualCol % m_TabWidth);
			result.append(spacesToNextTab, L' ');
			visualCol += spacesToNextTab;
		}
		else {
			result.push_back(ch);
			visualCol++;
		}
	}
	return result;
}

String String::Substring(int pos, int count) const
{
	return String(m_Data.substr(pos, count).c_str());
}

String String::Substring(int pos) const
{
	return String(m_Data.substr(pos).c_str());
}

String String::SubstringColumn(int column, int count) const
{
	int byteOffset = GetByteCount(column);
	int byteCount = GetByteCount(column, count);

	return Substring(byteOffset, byteCount);
}

String String::SubstringColumn(int column) const
{
	int byteOffset = GetByteCount(column);
	return Substring(byteOffset);
}

String String::Join(const String seperator, const String* values, int count)
{
	String result;
	if (count > 0) {
		result = values[0];
	}
	for (int i = 1; i < count; i++) {
		result += seperator + values[i];
	}


	return result;
}

String String::operator+(const String& rhs) const
{
	return String((m_Data + rhs.ToString()).c_str());
}

std::string String::WideToUTF8(const std::wstring str) const
{

	int len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size(),
		nullptr, 0, nullptr, nullptr);

	std::string result(len, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size(), &result[0], len, nullptr, nullptr);

	return result;
}

std::wstring String::UTF8ToWide(const std::string str) const
{
	int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
	std::wstring wide(size, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wide[0], size);
	return wide;
}



int String::GetByteCount(int columnLimit) const
{
	if (columnLimit == -1)
		return (int)m_Data.size();

	int cols = 0;
	for (int i = 0; i < (int)m_Data.size();) {
		if (columnLimit != -1 && cols >= columnLimit)
			return i;

		unsigned char c = (unsigned char)m_Data[i];

		if (c == '\t') {
			int spacesToNextTab = m_TabWidth - (cols % m_TabWidth);
			cols += spacesToNextTab;
			i++;
		}
		else {
			int advance = 1;
			if ((c & 0b10000000) == 0b00000000)       advance = 1;
			else if ((c & 0b11100000) == 0b11000000)  advance = 2;
			else if ((c & 0b11110000) == 0b11100000)  advance = 3;
			else if ((c & 0b11111000) == 0b11110000)  advance = 4;
			i += advance;
			cols++;
		}

	}
	return (int)m_Data.size();
}

int String::GetByteCount(int columnStart, int columnCount) const
{
	int byteStart = 0;
	int cols = 0;
	for (int i = 0; i < (int)m_Data.size();) {
		if (cols == columnStart)
			byteStart = i;

		if (cols >= columnStart + columnCount)
			return i - byteStart;


		unsigned char c = (unsigned char)m_Data[i];

		if (c == '\t') {
			int spacesToNextTab = m_TabWidth - (cols % m_TabWidth);
			cols += spacesToNextTab;
			i++;
		}
		else {
			int advance = 1;
			if ((c & 0b10000000) == 0b00000000)       advance = 1;
			else if ((c & 0b11100000) == 0b11000000)  advance = 2;
			else if ((c & 0b11110000) == 0b11100000)  advance = 3;
			else if ((c & 0b11111000) == 0b11110000)  advance = 4;
			i += advance;
			cols++;
		}

	}
	return (int)m_Data.size() - byteStart;
}

int String::GetColumnCount(int byteLimit) const
{

	int cols = 0;
	for (int i = 0; i < (int)m_Data.size();) {
		if (byteLimit != -1 && i >= byteLimit)
			break;

		unsigned char c = (unsigned char)m_Data[i];

		if (c == '\t') {
			int spacesToNextTab = m_TabWidth - (cols % m_TabWidth);
			cols += spacesToNextTab;
			i++;
		}
		else {
			int advance = 1;
			if ((c & 0b10000000) == 0b00000000)       advance = 1;
			else if ((c & 0b11100000) == 0b11000000)  advance = 2;
			else if ((c & 0b11110000) == 0b11100000)  advance = 3;
			else if ((c & 0b11111000) == 0b11110000)  advance = 4;
			i += advance;
			cols++;
		}

	}
	return cols;
}

int String::GetCharacterCount() const
{
	int charCount = 0;
	for (int i = 0; i < (int)m_Data.size();) {
		unsigned char c = (unsigned char)m_Data[i];

		int advance = 1;
		if ((c & 0b10000000) == 0b00000000)       advance = 1;
		else if ((c & 0b11100000) == 0b11000000)  advance = 2;
		else if ((c & 0b11110000) == 0b11100000)  advance = 3;
		else if ((c & 0b11111000) == 0b11110000)  advance = 4;
		i += advance;
		charCount++;
	}
	return charCount;
}
