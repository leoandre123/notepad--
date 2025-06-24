#pragma once
#include "ITextDocument.h"
class LineTextDocument :
    public ITextDocument
{
public:
	LineTextDocument(const std::string& originalText) {
		Init(originalText);
	}
	void Init(const std::string& originalText);
	int GetCursorPos() const { return m_CursorPos; }
	int GetColumn() const { return min(m_CursorX, m_Lines[m_CursorY].size()); }
	int GetRow() const { return m_CursorY; }
	int GetLength() const { return m_Length; }
	int GetLineCount() const { return m_LineCount; }

	void Write(const std::string& text);
	void Insert(int pos, const std::string& text);

	void DeleteAtCursor(bool backspace);
	void Delete(int pos, int length);

	void MoveCursor(CursorDirection direction, int steps);

	std::string GetText();
	std::vector<std::string> GetLines() const { return m_Lines; }
	std::vector<std::string> GetLines(int lineIndex, int count) const;
	std::string GetLine(int lineIndex) const;
	void PrintDebugString();

private:
	std::vector<std::string> SplitIntoLines(const std::string& text) const;

	int CalculateCursorPosition(int cursorX, int cursorY);
	std::pair<int, int> CalculateCursorCoordinates(int cursorPos);
	int CountCodePoints(const std::string& text, int byteLimit = -1);

private:
	std::vector<std::string> m_Lines;

	int m_CursorPos;
	int m_CursorX;
	int m_CursorY;

	int m_Length;
	int m_LineCount;
};

