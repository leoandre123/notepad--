#include "NotepadApp.h"
#include <consoleapi.h>
#include <cstdio>
#include "LineTextDocument.h"

void NotepadApp::Run()
{
	AllocConsole();

	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	freopen_s(&fp, "CONOUT$", "w", stdin);

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	//ITextDocument* doc = new LineTextDocument("Hello World");
	
	m_Window.Create();
	m_Window.Show();
	m_Window.StartMessageLoop();
}
