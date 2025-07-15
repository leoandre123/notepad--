#include <windows.h>
#include "NotepadApp.h"

HINSTANCE g_hInstance;
HWND g_HandleMainWindow;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	
	g_hInstance = hInstance;

	NotepadApp app;
	app.Run();
	return 0;
}
