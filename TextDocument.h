#pragma once
#include <string>
#include <vector>
#include <wtypes.h>
#include "ITextDocument.h"

enum class BufferType { Original, Add };
//enum CursorDirection{NORTH, EAST, SOUTH, WEST };

struct Piece
{
	BufferType buffer;
	int start;
	int length;
	int lineStartCount;
	int* lineStarts;
};

//struct LineData
//{
//	int pieceIndex; //The index of the piece that contains the first character of the line
//	int offset;		//The offset into that piece where the line starts
//	int lineLength;
//
//	int totalOffset;
//};

class TextDocument
{
public:
	TextDocument(const std::string& originalText);

	int GetCursorPos() const { return m_CursorPos; }
	int GetColumn() const { return m_CursorX; }
	int GetRow() const { return m_CursorY; }
	int GetLength() const { return m_Length; }
	int GetLineCount() const{ return m_LineCount; }

	void Write(const std::string& text);
	void Insert(int pos, const std::string& text);

	void DeleteAtCursor();
	void Delete(int pos, int length);


	void MoveCursor(CursorDirection direction, int steps);

	std::string GetText();
	std::vector<std::string> GetLines() const;
	std::vector<std::string> GetLines(int lineIndex, int count) const;

	std::string GetLine(int lineIndex) const;

	void PrintDebugString();

private:
	std::pair<int, int> GetPieceIndexByPos(int pos);
	int GetLineStartPos(int lineIndex);
	std::vector<std::string> SplitIntoLines(const std::string& text) const;

	std::string GetCleanedPieceString(int index);

	int CalculateCursorPosition(int cursorX, int cursorY);
	std::pair<int, int> CalculateCursorCoordinates(int cursorPos);


	void CalculateLineStartsForPiece(Piece& piece);

private:
	std::string m_OriginalText;
	std::string m_AddBuffer;
	std::vector<Piece> m_Pieces;

	int m_CursorPos;
	int m_CursorX;
	int m_CursorY;

	int m_Length;
	int m_LineCount;
};

