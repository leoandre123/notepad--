#pragma once
#include <string>
#include <wtypes.h>
/// <summary>
/// A string class that holds context for rendering
/// </summary>
class String
{
public:
	String(int tabWidth = 4):m_TabWidth(tabWidth), m_Data("") {}

	String(const char* literal, int tabWidth = 4) : m_TabWidth(tabWidth), m_Data(literal) {}
	String(const wchar_t* literal, int tabWidth = 4) : m_TabWidth(tabWidth), m_Data(WideToUTF8(literal)) {}

	String(const std::string string, int tabWidth = 4) : m_TabWidth(tabWidth), m_Data(string) {}


	//int Size() const { return (int)m_Data.size(); }
	int GetByteCount(int columnLimit = -1) const;
	int GetByteCount(int columnStart, int columnCount) const;
	int GetColumnCount(int byteLimit = -1) const;
	int GetCharacterCount() const;


	String CharacterAt(int characterIndex) const;
	String CharacterAtColumn(int columnIndex) const;
	String CharacterAtByte(int byteIndex) const;

	char ByteAt(int byteIndex) const { return m_Data[byteIndex]; }

	int CharacterIndexAtScreenPosition() const;


	int GetVisualWidth(HDC hdc, int columnLimit = -1) const;

	std::string ToString() const { return std::string(m_Data); }
	std::wstring ToWString() const { return UTF8ToWide(m_Data); }
	std::wstring ToWStringWithExpandedTabs() const;

	String Substring(int pos, int count) const;
	String Substring(int pos) const;

	String SubstringColumn(int column, int count) const;
	String SubstringColumn(int column) const;

	void Clear();
	void Insert(int pos, const String& text);
	void Delete(int pos, int length);


	void operator+=(const String& rhs);
	void operator+=(const char& rhs);
	String operator + (const String& rhs) const;

public:
	static String Join(const String seperator, const String* values, int count);



private:
	std::string WideToUTF8(const std::wstring str) const;
	std::wstring UTF8ToWide(const std::string str) const;

private:
	std::string m_Data;
	int m_TabWidth;
};

