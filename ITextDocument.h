#pragma once
#include <string>
#include <vector>
#include <wtypes.h>

enum CursorDirection { NORTH, EAST, SOUTH, WEST };

class ITextDocument
{
public:
	

	virtual void Init(const std::string& originalText) = 0;

	virtual int GetCursorPos() const = 0;
	virtual int GetColumn() const = 0;
	virtual int GetRow() const = 0;
	virtual int GetLength() const = 0;
	virtual int GetLineCount() const = 0;

	virtual void Write(const std::string& text) = 0;
	virtual void Insert(int pos, const std::string& text) = 0;

	virtual void DeleteAtCursor(bool backspace) = 0;
	virtual void Delete(int pos, int length) = 0;

	virtual void MoveCursor(CursorDirection direction, int steps) = 0;

	virtual std::string GetText() = 0;
	virtual std::vector<std::string> GetLines() const = 0;
	virtual std::vector<std::string> GetLines(int lineIndex, int count) const = 0;
	virtual std::string GetLine(int lineIndex) const = 0;
	virtual void PrintDebugString() = 0;

};

