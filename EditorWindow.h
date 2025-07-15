#pragma once
#include <wtypes.h>
#include "String.h"
#include "ITextDocument.h"
#include "TabContext.h"


class EditorWindow
{
public:
	EditorWindow();
	bool Create(HWND parentHandle);
	void Show();

	void SetTabContext(TabContext* tabContext);
	void ScrollToCursor();

	HWND GetHandle() const { return m_WindowHandle; }
private:
	static LRESULT CALLBACK EditorProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	//MESSAGES
	void PaintWindow();
	void OnScroll(WPARAM wParam, LPARAM lParam);
	void OnResize(int newWidth, int newHeight);
	void OnMouseButtonDown(int button, int x, int y);
	void OnMouseButtonUp(int button, int x, int y);
	void OnMouseMove(int x, int y);

	void DrawString(HDC hdc, int x, int y, String str, int cursorPos,
		int selectionStart = -1, int selectionEnd = -1);
	

	POINT MouseToCursorCoordinates(int x, int y);

private:
	HWND m_WindowHandle;

	//ITextDocument* m_ActiveDocument;
	//int m_LineOffset;

	TabContext* m_ActiveTabContext;

	int m_VisibleLineCount;
	bool m_IsSelecting;
};

