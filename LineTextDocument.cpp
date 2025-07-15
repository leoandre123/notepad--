#include "LineTextDocument.h"
#include <iostream>
#include <algorithm>

void LineTextDocument::Init(const String& originalText) {
	std::vector<String> lines = SplitIntoLines(originalText);
	m_Length = 0;
	m_LineCount = 0;
	m_Lines.clear();
	m_CursorBytePosition = 0;
	m_CursorColumn = 0;
	m_CursorBytePositionInRow = 0;
	m_CursorRow = 0;
	for (int i = 0; i < lines.size(); i++) {
		m_Lines.push_back(lines[i]);
		m_LineCount++;
		m_Length += lines[i].GetByteCount();
	}
}

int LineTextDocument::GetColumn() const
{
	int rowColumns = m_Lines[m_CursorRow].GetColumnCount();
	return min(m_CursorColumn, rowColumns);
}



void LineTextDocument::Write(const String& text)
{
	Insert(m_CursorBytePosition, text);
	m_CursorBytePosition += text.GetByteCount();
	UpdateCursorFromBytePosition();
	//std::pair<int, int> coords = CalculateCursorCoordinates(m_CursorBytePosition);
	//m_CursorColumn = coords.first;
	//m_CursorRow = coords.second;
}

void LineTextDocument::Insert(int pos, const String& text)
{
	m_Length += text.GetByteCount();

	std::vector<String> inputLines = SplitIntoLines(text);
	m_LineCount += inputLines.size() - 1;

	if (m_CurrentAction.type == EditType::Insert
		&& m_CurrentAction.pos + m_CurrentAction.text.GetByteCount() == pos) {
		m_CurrentAction.text += text;
	}
	else {
		m_CurrentAction.type = EditType::Insert;
		m_CurrentAction.pos = pos;
		m_CurrentAction.text = text.ToString();
	}


	int length = 0;
	for (int i = 0; i < m_Lines.size(); i++)
	{
		if (length + m_Lines[i].GetByteCount() + 1 > pos)
		{
			if (inputLines.size() == 1) {
				m_Lines[i].Insert(pos - length, text);
			}
			else {
				std::vector<String> newLines;
				newLines.push_back(m_Lines[i].Substring(0, pos - length) + inputLines[0]);
				for (int j = 1; j < inputLines.size() - 1; j++) {
					newLines.push_back(inputLines[j]);
				}
				newLines.push_back(m_Lines[i].Substring(pos - length) + inputLines.back());

				m_Lines.erase(m_Lines.begin() + i);
				m_Lines.insert(m_Lines.begin() + i, newLines.begin(), newLines.end());
			}

			return;
		}
		length += m_Lines[i].GetByteCount() + 1;
	}
}

void LineTextDocument::DeleteAtCursor(bool backspace)
{
	if (backspace) {
		if(m_CursorBytePosition > 0)
		{
			Delete(m_CursorBytePosition - 1, 1);
			m_CursorBytePosition--;
			UpdateCursorFromBytePosition();
			//std::pair<int, int> coords = CalculateCursorCoordinates(m_CursorBytePosition);
			//m_CursorColumn = coords.first;
			//m_CursorRow = coords.second;
		}
	}
	else {
		Delete(m_CursorBytePosition, 1);
	}
}

void LineTextDocument::Delete(int pos, int length)
{
	if (length <= 0 || pos < 0 || pos >= m_Length)
		return;



	if (pos + length > m_Length)
		length = m_Length - pos;

	int cursor = 0;
	int startLine = 0;
	int startOffset = 0;

	for (int i = 0; i < m_Lines.size(); ++i) {
		int lineLength = m_Lines[i].GetByteCount() + 1;

		if (cursor + lineLength > pos) {
			startLine = i;
			startOffset = pos - cursor;
			break;
		}
		cursor += lineLength;
	}

	int endPos = pos + length;
	cursor = 0;
	int endLine = 0;
	int endOffset = 0;

	for (int i = 0; i < m_Lines.size(); ++i) {
		int lineLength = m_Lines[i].GetByteCount() + 1;

		if (cursor + lineLength > endPos) {
			endLine = i;
			endOffset = endPos - cursor;
			break;
		}
		cursor += lineLength;
	}

	if (startLine == endLine) {
		m_Lines[startLine].Delete(startOffset, length);
	}
	else {
		String prefix = m_Lines[startLine].Substring(0, startOffset);
		String suffix = m_Lines[endLine].Substring(endOffset);
		m_Lines[startLine] = prefix + suffix;
		m_Lines.erase(m_Lines.begin() + startLine + 1, m_Lines.begin() + endLine + 1);
		m_LineCount -= (endLine - startLine);
	}

	m_Length -= length;
}

void LineTextDocument::MoveCursor(CursorDirection direction, int steps)
{
	if (direction == CursorDirection::NORTH) {
		if (steps > m_CursorRow)
			return;
		m_CursorRow -= steps;
		UpdateCursorFromCoordinates();
		//m_CursorBytePosition = CalculateCursorPosition(m_CursorX, m_CursorY);
	}
	else if (direction == CursorDirection::SOUTH) {
		if (steps >= m_LineCount - m_CursorRow)
			return;
		m_CursorRow += steps;
		UpdateCursorFromCoordinates();
		//m_CursorPos = CalculateCursorPosition(m_CursorX, m_CursorY);
	}
	else if (direction == CursorDirection::WEST) {
		if (steps > m_CursorBytePosition)
			return;
		for (int i = 0; i < steps; i++) {
			do {
				m_CursorBytePosition--;
				UpdateCursorFromBytePosition();
			} while ((m_Lines[m_CursorRow].ByteAt(m_CursorBytePositionInRow) & 0b11000000) == 0b10000000);
		}
	}
	else if (direction == CursorDirection::EAST) {
		if (steps > m_Length - m_CursorBytePosition)
			return;
		for (int i = 0; i < steps; i++) {
			do {
				m_CursorBytePosition++;
				UpdateCursorFromBytePosition();
			} while ((m_Lines[m_CursorRow].ByteAt(m_CursorBytePositionInRow) & 0b11000000) == 0b10000000);
		}
	}
}

void LineTextDocument::SetCursorCoordinates(int col, int row)
{
	m_CursorRow = min(row, m_LineCount - 1);
	m_CursorColumn = min(col, m_Lines[m_CursorRow].GetColumnCount());
	UpdateCursorFromCoordinates();
}

void LineTextDocument::SetSelectionStart(int col, int row)
{
	m_SelectionColumn = col;
	m_SelectionRow = row;
}

String LineTextDocument::GetText() const
{
	return String::Join("\n", m_Lines.data(), m_LineCount);
}

std::vector<String> LineTextDocument::GetLines(int lineIndex, int count) const
{
	int c = min(count, m_Lines.size() - lineIndex);
	return std::vector<String>(m_Lines.begin() + lineIndex, m_Lines.begin() + lineIndex + c);
}

String LineTextDocument::GetLine(int lineIndex) const
{
	return m_Lines[lineIndex];
}

void LineTextDocument::PrintDebugString()
{
	std::cout << "------------------------------------\n";
	std::cout << "Lines:\n";
	for (int i = 0; i < m_Lines.size(); i++)
	{
		std::string line = GetLine(i).ToString();
		std::replace(line.begin(), line.end(), '\n', '^');
		std::cout << i << ": " << line << "\n";
	}
	std::cout << "------------------------------------\n";
}

void LineTextDocument::Undo()
{
	EditAction actionToUndo;

	if (m_CurrentAction.type != EditType::None) {
		actionToUndo = m_CurrentAction;
	}
	else {
		if (m_UndoStack.empty()) {
			return;
		}
		actionToUndo = m_UndoStack.top();
		m_UndoStack.pop();
	}


	switch (actionToUndo.type) {
	case EditType::Insert:{
		Delete(actionToUndo.pos, actionToUndo.text.GetByteCount());
		break;
		}
	case EditType::Delete: {
		Insert(actionToUndo.pos, actionToUndo.text);
		break;
	}
	}

	m_RedoStack.push(actionToUndo);
}

void LineTextDocument::Redo()
{
}

std::vector<String> LineTextDocument::SplitIntoLines(const String& text) const
{
	std::vector<String> lines;
	String current;
	for (int i = 0; i < text.GetByteCount(); ++i) {

		if (text.ByteAt(i) == '\n') {
			lines.push_back(current);
			current.Clear();
		}
		else {
			current += text.ByteAt(i);
		}
	}
	lines.push_back(current);
	return lines;
}


int LineTextDocument::CalculateCursorPosition(int cursorX, int cursorY)
{
	int pos = 0;
	for (int i = 0; i < cursorY; i++) {
		pos += m_Lines[i].GetByteCount() + 1;
	}
	pos += m_Lines[cursorY].GetByteCount(min(cursorX, m_Lines[cursorY].GetByteCount()));
	return pos;
}

std::pair<int, int> LineTextDocument::CalculateCursorCoordinates(int cursorPos)
{
	int pos = 0;
	for (int i = 0; i < m_Lines.size(); i++) {
		int length = m_Lines[i].GetByteCount() + 1;
		if (pos + length > cursorPos) {
			int cols = m_Lines[i].GetColumnCount(cursorPos - pos);//CountColumns(m_Lines[i], cursorPos - pos);
			return { cols, i};
		}
		pos += length;
	}
	return{ 0,0 };
}

/*
int LineTextDocument::CountCodePoints(const String& str, int byteLimit)const
{
	int count = 0;
	for (int i = 0; i < str.size();) {
		if (byteLimit != -1 && i >= byteLimit)
			break;

		unsigned char c = (unsigned char)str[i];

		int advance = 1;
		if ((c & 0b10000000) == 0b00000000)       advance = 1;
		else if ((c & 0b11100000) == 0b11000000)  advance = 2;
		else if ((c & 0b11110000) == 0b11100000)  advance = 3;
		else if ((c & 0b11111000) == 0b11110000)  advance = 4;

		i += advance;
		++count;
	}
	return count;
}

int LineTextDocument::CountColumns(const String& text, int byteLimit)const
{
	int cols = 0;
	for (int i = 0; i < (int)text.size();) {
		if (i == byteLimit)
			break;

		unsigned char c = (unsigned char)text[i];

		if (c == '\t') {
			int spacesToNextTab = 4 - (cols % 4);
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

int LineTextDocument::CountBytes(const String& text, int columns)const
{
	if (columns == -1)
		return (int)text.size();

	int cols = 0;
	for (int i = 0; i < (int)text.size();) {
		if (cols >= columns)
			return i;

		unsigned char c = (unsigned char)text[i];

		if (c == '\t') {
			int spacesToNextTab = 4 - (cols % 4);
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
	return (int)text.size();
}
*/
void LineTextDocument::UpdateCursorFromBytePosition()
{
	std::pair<int, int> coords = CalculateCursorCoordinates(m_CursorBytePosition);
	m_CursorColumn = coords.first;
	m_CursorRow = coords.second;
	m_CursorBytePositionInRow = m_Lines[m_CursorRow].GetByteCount(m_CursorColumn);
}

void LineTextDocument::UpdateCursorFromCoordinates()
{
	m_CursorBytePosition = CalculateCursorPosition(m_CursorColumn, m_CursorRow);
}
