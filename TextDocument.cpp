#include "TextDocument.h"
#include <iostream>
#include <algorithm>

TextDocument::TextDocument(const std::string& originalText) :
	m_OriginalText(originalText),
	m_CursorPos(0),
	m_CursorX(0),
	m_CursorY(0),
	m_LineCount(1)
{
	if (!originalText.empty())
	{

		std::vector<std::string> lines = SplitIntoLines(originalText);
		int lineStartCount = lines.size();
		int* lineStarts = new int[lineStartCount];
		lineStarts[0] = 0;
		for (int i = 1; i < lineStartCount; i++) {
			lineStarts[i - 1] = lines[i].size();
		}

		m_LineCount = lines.size();
		m_Pieces.push_back({ BufferType::Original, 0, (int)originalText.size(),lineStartCount, lineStarts });
		m_Length = originalText.size();
	}
	else
	{
		
	}
}

void TextDocument::Write(const std::string& text)
{
	Insert(m_CursorPos, text);
	m_CursorPos += text.size();
	std::vector<std::string> lines = SplitIntoLines(text);
	m_CursorY += lines.size() - 1;
	m_CursorX = lines.size() == 1 ? m_CursorX += text.size() : lines.back().size();
}

void TextDocument::Delete(int pos, int deleteLength)
{
	if (!deleteLength)return;


	std::vector<Piece> result;
	result.reserve(m_Pieces.capacity());


	m_Length -= deleteLength;
	int toDelete = deleteLength;

	int length = 0;
	for (int i = 0; i < m_Pieces.size(); i++)
	{
		int pieceLength = m_Pieces[i].length;

		if (length + pieceLength < pos) {
			result.push_back(m_Pieces[i]);
		}
		else if (toDelete > 0)
		{
			int offset = max(pos - length, 0);
			if (offset > 0)
			{
				result.push_back({ m_Pieces[i].buffer, m_Pieces[i].start, offset });
			}
			if (toDelete < pieceLength - offset) {
				result.push_back({ m_Pieces[i].buffer, m_Pieces[i].start + toDelete, m_Pieces[i].length - toDelete });
				toDelete = 0;
			}
			else
			{
				toDelete -= pieceLength - offset;
			}
		}
		else
		{
			result.push_back(m_Pieces[i]);
		}


		length += pieceLength;
	}

	m_Pieces = std::move(result);
}

void TextDocument::Insert(int pos, const std::string& text)
{
	if (text.empty())
		return;

	int addStart = m_AddBuffer.size();
	m_AddBuffer += text;
	int addLength = text.size();
	m_Length += addLength;

	std::pair<int, int> res = GetPieceIndexByPos(pos);
	int insertionPieceIndex = res.first;
	int insertionPieceOffset = res.second;
	int insertionRemainingOffset = 0;

	std::vector<std::string> lines = SplitIntoLines(text);
	m_LineCount += lines.size() - 1;


	//OM VI KAN FORTSÄTTA DÄR VA LADE TILL SIST
	if (insertionPieceOffset == 0
		&& insertionPieceIndex > 0
		&& m_Pieces[insertionPieceIndex - 1].buffer == BufferType::Add
		&& m_Pieces[insertionPieceIndex - 1].start + m_Pieces[insertionPieceIndex - 1].length == addStart)
	{
		int lineBreakCount = lines.size() - 1;

		if (lineBreakCount) {
			int oldLength = m_Pieces[insertionPieceIndex - 1].lineStartCount;

			m_Pieces[insertionPieceIndex - 1].lineStartCount += lineBreakCount;

			int* lineBreaks = new int[m_Pieces[insertionPieceIndex - 1].lineStartCount];
			memcpy(lineBreaks, m_Pieces[insertionPieceIndex - 1].lineStarts, oldLength * sizeof(int));
			for (int i = 0; i < lineBreakCount; i++) {
				lineBreaks[oldLength + i] = lines[i].size();
			}


			if (m_Pieces[insertionPieceIndex - 1].lineStarts != nullptr) {
				delete[] m_Pieces[insertionPieceIndex - 1].lineStarts;
			}

			m_Pieces[insertionPieceIndex - 1].lineStarts = lineBreaks;

		}
		m_Pieces[insertionPieceIndex - 1].length += addLength;
	}
	//OM VI ÄR SIST I DOKUMENTET/ALTERNATIV TOMT DOKUMENT
	else if (insertionPieceIndex >= m_Pieces.size())
	{
		int lineBreakCount = lines.size() - 1;
		int* lineBreaks = nullptr;
		if (lineBreakCount) {
			lineBreaks = new int[lineBreakCount];
			for (int i = 0; i < lineBreakCount; i++) {
				lineBreaks[i] = lines[i].size();
			}
		}
		m_Pieces.push_back({ BufferType::Add, addStart, addLength,lineBreakCount, lineBreaks });
	}
	//ANNARS
	else
	{
		std::vector<Piece> newPieces;
		for (int i = 0; i < m_Pieces.size(); i++) {
			if (i == insertionPieceIndex)
			{
				if (insertionPieceOffset > 0) {
					newPieces.push_back({ m_Pieces[i].buffer, m_Pieces[i].start, insertionPieceOffset ,0,nullptr });
					CalculateLineStartsForPiece(newPieces.back());
				}
				newPieces.push_back({ BufferType::Add, addStart, addLength,0,nullptr });
				CalculateLineStartsForPiece(newPieces.back());
				if (insertionPieceOffset < m_Pieces[i].length)
				{
					insertionRemainingOffset = m_Pieces[i].length - insertionPieceOffset;
					newPieces.push_back({ m_Pieces[i].buffer, m_Pieces[i].start + insertionPieceOffset, insertionRemainingOffset ,0,nullptr });
					CalculateLineStartsForPiece(newPieces.back());
				}
			}
			else
			{
				newPieces.push_back(m_Pieces[i]);
			}
		}


		m_Pieces = std::move(newPieces);
	}
}

void TextDocument::MoveCursor(CursorDirection direction, int steps)
{
	if (direction == CursorDirection::NORTH) {
		if (steps > m_CursorY)
			return;
		m_CursorY -= steps;
		m_CursorPos = CalculateCursorPosition(m_CursorX, m_CursorY);
	}
	else if (direction == CursorDirection::SOUTH) {
		if (steps > m_LineCount - m_CursorY)
			return;
		m_CursorY += steps;
		m_CursorPos = CalculateCursorPosition(m_CursorX, m_CursorY);
	}
	else if (direction == CursorDirection::WEST) {
		if (steps > m_CursorPos)
			return;
		m_CursorPos -= steps;
		std::pair<int, int> coords = CalculateCursorCoordinates(m_CursorPos);
		m_CursorX = coords.first;
		m_CursorY = coords.second;
	}
	else if (direction == CursorDirection::EAST) {
		if (steps > m_Length - m_CursorPos)
			return;
		m_CursorPos += steps;
		std::pair<int, int> coords = CalculateCursorCoordinates(m_CursorPos);
		m_CursorX = coords.first;
		m_CursorY = coords.second;
	}
}

std::string TextDocument::GetText()
{
	std::string result;
	for (int i = 0; i < m_Pieces.size(); i++)
	{
		result += (m_Pieces[i].buffer == BufferType::Original ? m_OriginalText : m_AddBuffer).substr(m_Pieces[i].start, m_Pieces[i].length);
	}

	return result;
}

std::vector<std::string> TextDocument::GetLines() const {
	std::vector<std::string> lines;
	std::string current;
	for (const auto& p : m_Pieces) {
		const std::string& src = (p.buffer == BufferType::Original) ? m_OriginalText : m_AddBuffer;
		for (size_t i = 0; i < p.length; ++i) {
			char c = src[p.start + i];
			if (c == '\n') {
				lines.push_back(current);
				current.clear();
			}
			else if (c != '\r') {
				current += c;
			}
		}
	}
	lines.push_back(current); // push final line
	return lines;
}

std::vector<std::string> TextDocument::GetLines(int lineIndex, int count) const
{
	std::vector<std::string> lines;
	for (int i = 0; i < count; i++)
	{
		if (lineIndex + i >= m_LineCount)
			break;
		lines.push_back(GetLine(lineIndex + i));
	}
	return lines;
}

std::string TextDocument::GetLine(int lineIndex) const
{
	int pieceIndex = 0;
	int offset = 0;// = m_LineData[lineIndex].offset;
	//int lengthRemaining = 0;// = m_LineData[lineIndex].lineLength;



	int lineCount = 0;
	for (int i = 0; i < m_Pieces.size(); i++) {
		int count = m_Pieces[i].lineStartCount;
		if (lineCount + count >= lineIndex) {
			pieceIndex = i;
			offset = lineIndex == 0 ? 0 : m_Pieces[i].lineStarts[lineIndex - lineCount - 1];
			break;
		}
		lineCount += count;
	}


	std::string line;
	while (pieceIndex < m_Pieces.size())
	{
		for (int i = offset; i < m_Pieces[pieceIndex].length; i++) {
			char ch = (m_Pieces[pieceIndex].buffer == BufferType::Original ?
				m_OriginalText : m_AddBuffer)[m_Pieces[pieceIndex].start + i];
			line.push_back(ch);

			if (ch == '\n') {
				return line;
			}

		}
		offset = 0;
		pieceIndex++;
	}

	return line;
}

void TextDocument::PrintDebugString()
{
	std::string orig(m_OriginalText);
	std::string add(m_AddBuffer);
	std::replace(orig.begin(), orig.end(), '\n', '^');
	std::replace(add.begin(), add.end(), '\n', '^');

	std::cout << "Original\n" << orig << "\nAddBuffer\n" << add << "\n";
	std::cout << "Piece Table:\n";
	for (int i = 0; i < m_Pieces.size(); i++)
	{
		std::cout << i << ": " << (m_Pieces[i].buffer == BufferType::Original ? "Ori" : "Add") << " - " << m_Pieces[i].start << " - " << m_Pieces[i].length << "     :     " << GetCleanedPieceString(i) << "\n";
	}
	std::cout << "------------------------------------\n";
	std::cout << "Lines:\n";
	for (int i = 0; i < m_LineCount; i++)
	{
		std::string line = GetLine(i);
		std::replace(line.begin(), line.end(), '\n', '^');
		std::cout << i << ": " << line << "\n";
	}
	std::cout << "------------------------------------\n";
	//std::cout << "Lines:\n";
	//for (int i = 0; i < m_LineData.size(); i++)
	//{
	//	std::string line = GetLine(i);
	//	std::replace(line.begin(), line.end(), '\n', '^');
	//	std::cout << i << ": " << m_LineData[i].pieceIndex << " - " << m_LineData[i].offset << " - " << m_LineData[i].lineLength << "     :     " << line << "\n";
	//}
	//std::cout << "------------------------------------\n";
}

std::pair<int, int> TextDocument::GetPieceIndexByPos(int pos)
{
	int length = 0;
	int index = 0;
	int pieceLength = 0;
	for (; index < m_Pieces.size(); index++)
	{
		pieceLength = m_Pieces[index].length;

		if (length + pieceLength > pos)
		{
			return { index, pos - length };
		}
		length += pieceLength;
	}

	return { index, pos - length };
}


int TextDocument::GetLineStartPos(int lineIndex)
{
	return -1;
}

std::vector<std::string> TextDocument::SplitIntoLines(const std::string& text) const
{
	std::vector<std::string> lines;
	std::string current;
	for (int i = 0; i < text.size(); ++i) {
		current += text[i];
		if (text[i] == '\n') {
			lines.push_back(current);
			current.clear();
		}
	}
	lines.push_back(current);
	return lines;
}

std::string TextDocument::GetCleanedPieceString(int index)
{
	std::string part = (m_Pieces[index].buffer == BufferType::Original ? m_OriginalText : m_AddBuffer).substr(m_Pieces[index].start, m_Pieces[index].length);
	std::replace(part.begin(), part.end(), '\n', '^');
	return part;
}

int TextDocument::CalculateCursorPosition(int cursorX, int cursorY)
{
	int pos = 0;
	int lines = 0;
	for (int i = 0; i < m_Pieces.size(); i++) {

		if (lines + m_Pieces[i].lineStartCount >= cursorY) {
			pos += m_Pieces[i].lineStarts[cursorY - lines];
			pos += cursorX;
			break;
		}
		pos += m_Pieces[i].length;
		lines += m_Pieces[i].lineStartCount;
		//pos += m_LineData[i].lineLength;
	}

	return pos;
	//TODO: pos = m_LineData[cursorY].totalOffset;
}

std::pair<int, int> TextDocument::CalculateCursorCoordinates(int cursorPos)
{
	int pos = 0;
	int y = 0;

	for (int i = 0; i < m_Pieces.size(); i++) {
		if (pos + m_Pieces[i].length > cursorPos) {
			for (int j = 0; j < m_Pieces[i].lineStartCount; j++) {
				if (pos + m_Pieces[i].lineStarts[j] >= cursorPos) {
					return { cursorPos - pos - m_Pieces[i].lineStarts[j],y + j };
				}
			}
			break;
		}


		pos += m_Pieces[i].length;
		y += m_Pieces[i].lineStartCount;
	}

	return std::make_pair(cursorPos - pos, y);
}

void TextDocument::CalculateLineStartsForPiece(Piece& piece)
{
	std::vector<int> starts;

	for (int i = 0; i < piece.length; i++) {
		char ch = (piece.buffer == BufferType::Original ?
			m_OriginalText : m_AddBuffer)[piece.start + i];

		if (ch == '\n') {
			starts.push_back(i);
		}
	}

	if (piece.lineStartCount) {
		delete[] piece.lineStarts;
	}


	piece.lineStarts = new int[starts.size()];
	piece.lineStartCount = starts.size();
	memcpy(piece.lineStarts, starts.data(), starts.size() * sizeof(int));
}

