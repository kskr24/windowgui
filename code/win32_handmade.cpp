#include <Windows.h>

LRESULT CALLBACK MainWindowCallback(
	HWND Window,
	UINT Message,
	WPARAM WParam,
	LPARAM LParam
) {
	LRESULT Result = 0;
	switch (Message) {
		case WM_SIZE: { OutputDebugStringA("WM_SIZE\n"); } break;
		case WM_DESTROY: { OutputDebugStringA("WM_DESTROY\n"); } break;
		case WM_CLOSE: { OutputDebugStringA("WM_CLOSE\n"); } break;
		case WM_ACTIVATEAPP: { OutputDebugStringA("WM_ACTIVATEAPP\n"); } break;
		case WM_PAINT: {
			PAINTSTRUCT Paint;
			HDC DeviceContext =  BeginPaint(
				Window, &Paint
			);
			int x = Paint.rcPaint.left;
			int y = Paint.rcPaint.top;
			int Width = Paint.rcPaint.right - Paint.rcPaint.left;
			int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
			PatBlt(DeviceContext, x, y, Width, Height, WHITENESS);
			EndPaint(
				Window, &Paint
			);
		} break;
		default: {
			// OutputDebugStringA("Default\n");
			Result = DefWindowProc(Window, Message, WParam, LParam);
		} break;
	}

	return Result;
}
int CALLBACK
WinMain(
	HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow) {

	WNDCLASSA WindowClass = {};
	WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WindowClass.lpfnWndProc = MainWindowCallback;
	WindowClass.hInstance = Instance;
	// WindowClass.hIcon;
	WindowClass.lpszClassName = "HandMadeHeroWindowClass";

	if (RegisterClass(&WindowClass)) {
		HWND WindowHandle = CreateWindowEx(
			0,
			WindowClass.lpszClassName,
			"HandMade Hero",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			Instance,
			0);

		if (WindowHandle) {
			MSG Message;
			for(;;) {
				BOOL MessageResut = GetMessage(&Message, 0, 0, 0);
				if (MessageResut > 0) {
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}
				else {
					break;
				}
			}
		}
	}
	else {
		// TODO(kskr): Logging
	}
	return(0);
}
