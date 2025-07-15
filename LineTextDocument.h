#pragma once
#include "ITextDocument.h"
#include "String.h"
#include <stack>
enum EditType {
	None,
	Insert,
	Delete,
};
struct EditAction {
	EditType type;
	int pos;
	String text;
};

class LineTextDocument :
	public ITextDocument
{
public:
	LineTextDocument(const String& originalText) {
		Init(originalText);
	}
	void Init(const String& originalText);
	int GetCursorPos() const { return m_CursorBytePosition; }
	int GetColumn() const;
	int GetRow() const { return m_CursorRow; }
	int GetLength() const { return m_Length; }
	int GetLineCount() const { return m_LineCount; }

	int GetSelectionColumn() const { return m_SelectionColumn; }
	int GetSelectionRow() const { return m_SelectionRow; }
	bool HasSelection() const { return m_HasSelection; }

	void Write(const String& text);
	void Insert(int pos, const String& text);

	void DeleteAtCursor(bool backspace);
	void Delete(int pos, int length);

	void MoveCursor(CursorDirection direction, int steps);
	void SetCursorCoordinates(int col, int row);

	void SetSelectionStart(int col, int row);

	String GetText() const;
	std::vector<String> GetLines() const { return m_Lines; }
	std::vector<String> GetLines(int lineIndex, int count) const;
	String GetLine(int lineIndex) const;
	void PrintDebugString();

	void Undo() override;
	void Redo() override;
private:
	std::vector<String> SplitIntoLines(const String& text) const;

	int CalculateCursorPosition(int cursorX, int cursorY);
	std::pair<int, int> CalculateCursorCoordinates(int cursorPos);

	/*
	int CountCodePoints(const String& text, int byteLimit = -1) const;
	int CountColumns(const String& text, int byteLimit = -1) const;
	int CountBytes(const String& text, int columns = -1) const;
	*/

	void UpdateCursorFromBytePosition();
	void UpdateCursorFromCoordinates();
private:
	std::vector<String> m_Lines;

	int m_CursorBytePosition;
	int m_CursorColumn;
	int m_CursorRow;
	int m_CursorBytePositionInRow;

	int m_SelectionColumn;
	int m_SelectionRow;
	bool m_HasSelection;

	int m_Length;
	int m_LineCount;
	
	std::stack<EditAction> m_UndoStack;
	std::stack<EditAction> m_RedoStack;
	EditAction m_CurrentAction;
};

