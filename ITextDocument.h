#pragma once
#include "String.h"
#include <vector>
#include <wtypes.h>

enum CursorDirection { NORTH, EAST, SOUTH, WEST };

class ITextDocument
{
public:
	

	virtual void Init(const String& originalText) = 0;

	virtual int GetCursorPos() const = 0;
	virtual int GetColumn() const = 0;
	virtual int GetRow() const = 0;
	virtual int GetLength() const = 0;
	virtual int GetLineCount() const = 0;

	virtual int GetSelectionColumn() const = 0;
	virtual int GetSelectionRow() const = 0;
	virtual bool HasSelection() const = 0;

	virtual void Write(const String& text) = 0;
	virtual void Insert(int pos, const String& text) = 0;

	virtual void DeleteAtCursor(bool backspace) = 0;
	virtual void Delete(int pos, int length) = 0;

	virtual void MoveCursor(CursorDirection direction, int steps) = 0;
	virtual void SetCursorCoordinates(int col, int row) = 0;

	virtual void SetSelectionStart(int col, int row) = 0;

	virtual String GetText() const = 0;
	virtual std::vector<String> GetLines() const = 0;
	virtual std::vector<String> GetLines(int lineIndex, int count) const = 0;
	virtual String GetLine(int lineIndex) const = 0;
	virtual void PrintDebugString() = 0;

	virtual void Undo() = 0;
	virtual void Redo() = 0;
};

