#include "LineTextDocument.h"
#include <iostream>
#include <algorithm>

void LineTextDocument::Init(const std::string& originalText) {
	m_Lines.push_back("");
	m_LineCount = 1;
}


void LineTextDocument::Write(const std::string& text)
{
	Insert(m_CursorPos, text);
	m_CursorPos += text.size();
	std::pair<int, int> coords = CalculateCursorCoordinates(m_CursorPos);
	m_CursorX = coords.first;
	m_CursorY = coords.second;
}

void LineTextDocument::Insert(int pos, const std::string& text)
{
	m_Length += text.size();

	std::vector<std::string> inputLines = SplitIntoLines(text);
	m_LineCount += inputLines.size() - 1;

	int length = 0;
	for (int i = 0; i < m_Lines.size(); i++)
	{
		if (length + m_Lines[i].size() + 1 > pos)
		{
			if (inputLines.size() == 1) {
				m_Lines[i].insert(pos - length, text);
			}
			else {
				std::vector<std::string> newLines;
				newLines.push_back(m_Lines[i].substr(0, pos - length) + inputLines[0]);
				for (int j = 1; j < inputLines.size() - 1; j++) {
					newLines.push_back(inputLines[j]);
				}
				newLines.push_back(m_Lines[i].substr(pos - length) + inputLines.back());

				m_Lines.erase(m_Lines.begin() + i);
				m_Lines.insert(m_Lines.begin() + i, newLines.begin(), newLines.end());
			}

			return;
		}
		length += m_Lines[i].size() + 1;
	}
}

void LineTextDocument::DeleteAtCursor(bool backspace)
{
	if (backspace) {
		Delete(m_CursorPos - 1, 1);
		m_CursorPos--;
		std::pair<int, int> coords = CalculateCursorCoordinates(m_CursorPos);
		m_CursorX = coords.first;
		m_CursorY = coords.second;
	}
	else {
		Delete(m_CursorPos, 1);
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
		int lineLength = m_Lines[i].size() + 1;

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
		int lineLength = m_Lines[i].size() + 1;

		if (cursor + lineLength > endPos) {
			endLine = i;
			endOffset = endPos - cursor;
			break;
		}
		cursor += lineLength;
	}

	if (startLine == endLine) {
		m_Lines[startLine].erase(startOffset, length);
	}
	else {
		std::string prefix = m_Lines[startLine].substr(0, startOffset);
		std::string suffix = m_Lines[endLine].substr(endOffset);
		m_Lines[startLine] = prefix + suffix;
		m_Lines.erase(m_Lines.begin() + startLine + 1, m_Lines.begin() + endLine + 1);
		m_LineCount -= (endLine - startLine);
	}

	m_Length -= length;
}

void LineTextDocument::MoveCursor(CursorDirection direction, int steps)
{
	if (direction == CursorDirection::NORTH) {
		if (steps > m_CursorY)
			return;
		m_CursorY -= steps;
		m_CursorPos = CalculateCursorPosition(m_CursorX, m_CursorY);
	}
	else if (direction == CursorDirection::SOUTH) {
		if (steps >= m_LineCount - m_CursorY)
			return;
		m_CursorY += steps;
		m_CursorPos = CalculateCursorPosition(m_CursorX, m_CursorY);
	}
	else if (direction == CursorDirection::WEST) {
		if (steps > m_CursorPos)
			return;
		for (int i = 0; i < steps; i++) {
			do {
				m_CursorPos--;
				std::pair<int, int> coords = CalculateCursorCoordinates(m_CursorPos);
				m_CursorX = coords.first;
				m_CursorY = coords.second;
			} while ((m_Lines[m_CursorY][m_CursorX] & 0b11000000) == 0b10000000);
		}
	}
	else if (direction == CursorDirection::EAST) {
		if (steps > m_Length - m_CursorPos)
			return;
		for (int i = 0; i < steps; i++) {
			do {
				m_CursorPos++;
				std::pair<int, int> coords = CalculateCursorCoordinates(m_CursorPos);
				m_CursorX = coords.first;
				m_CursorY = coords.second;
			} while ((m_Lines[m_CursorY][m_CursorX] & 0b11000000) == 0b10000000);
		}
	}
}

std::string LineTextDocument::GetText()
{
	std::string text;
	for (auto line : m_Lines) {
		text.append(line);
	}
	return text;
}

std::vector<std::string> LineTextDocument::GetLines(int lineIndex, int count) const
{
	int c = min(count, m_Lines.size() - lineIndex);
	return std::vector<std::string>(m_Lines.begin() + lineIndex, m_Lines.begin() + lineIndex + c);
}

std::string LineTextDocument::GetLine(int lineIndex) const
{
	return m_Lines[lineIndex];
}

void LineTextDocument::PrintDebugString()
{
	std::cout << "------------------------------------\n";
	std::cout << "Lines:\n";
	for (int i = 0; i < m_Lines.size(); i++)
	{
		std::string line = GetLine(i);
		std::replace(line.begin(), line.end(), '\n', '^');
		std::cout << i << ": " << line << "\n";
	}
	std::cout << "------------------------------------\n";
}

std::vector<std::string> LineTextDocument::SplitIntoLines(const std::string& text) const
{
	std::vector<std::string> lines;
	std::string current;
	for (int i = 0; i < text.size(); ++i) {

		if (text[i] == '\n') {
			lines.push_back(current);
			current.clear();
		}
		else {
			current += text[i];
		}
	}
	lines.push_back(current);
	return lines;
}


int LineTextDocument::CalculateCursorPosition(int cursorX, int cursorY)
{
	int pos = 0;
	for (int i = 0; i < cursorY; i++) {
		pos += m_Lines[i].size() + 1;
	}

	pos += min(cursorX, m_Lines[cursorY].size());

	return pos;
}

std::pair<int, int> LineTextDocument::CalculateCursorCoordinates(int cursorPos)
{
	int pos = 0;
	for (int i = 0; i < m_Lines.size(); i++) {
		int length = m_Lines[i].size() + 1;
		if (pos + length > cursorPos) {
			int codePointCount = CountCodePoints(m_Lines[i], cursorPos - pos);
			return { codePointCount,i };
		}
		pos += length;
	}
	return{ 0,0 };
}

int LineTextDocument::CountCodePoints(const std::string& str, int byteLimit)
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
