#include <Windows.h>
#include <windef.h>
#include <wingdi.h>

#define local_persist static;
#define global_variable static;
#define internal static;

global_variable bool       Running = false;
global_variable BITMAPINFO BitMapInfo;
global_variable void      *BitMapMemory;
global_variable HBITMAP    BitMapHandle;
global_variable HDC        BitMapDeviceContext;

internal void Win32ResizeDIBSection(int Width, int Height) {
  // TODO(kskr) Memory Management
  if (BitMapHandle) {
    DeleteObject(BitMapHandle);
  }
  if (!BitMapDeviceContext) {
    BitMapDeviceContext = CreateCompatibleDC(0);
  }

  BitMapInfo.bmiHeader.biSize          = sizeof(BitMapInfo.bmiHeader);
  BitMapInfo.bmiHeader.biWidth         = Width;
  BitMapInfo.bmiHeader.biHeight        = Height;
  BitMapInfo.bmiHeader.biPlanes        = 1;
  BitMapInfo.bmiHeader.biBitCount      = 32;
  BitMapInfo.bmiHeader.biCompression   = BI_RGB;
  BitMapInfo.bmiHeader.biSizeImage     = 0;
  BitMapInfo.bmiHeader.biXPelsPerMeter = 0;
  BitMapInfo.bmiHeader.biYPelsPerMeter = 0;
  BitMapInfo.bmiHeader.biClrUsed       = 0;
  BitMapInfo.bmiHeader.biClrImportant  = 0;
  HBITMAP BitMapHandle                 = CreateDIBSection(BitMapDeviceContext,
                                          &BitMapInfo,
                                          DIB_RGB_COLORS,
                                          &BitMapMemory,
                                          NULL,
                                          NULL);
}

internal void Win32UpdateWindow(HDC DeviceContext,
                                int X,
                                int Y,
                                int Width,
                                int Height) {
  StretchDIBits(DeviceContext,
                X,
                Y,
                Width,
                Height,
                X,
                Y,
                Width,
                Height,
                BitMapMemory,
                &BitMapInfo,
                DIB_RGB_COLORS,
                SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND   Window,
                                         UINT   Message,
                                         WPARAM WParam,
                                         LPARAM LParam) {
  LRESULT Result = 0;
  switch (Message) {
    case WM_SIZE: {
      OutputDebugStringA("WM_SIZE\n");
      RECT ClientRect;
      GetClientRect(Window, &ClientRect);
      int Width  = ClientRect.right - ClientRect.left;
      int Height = ClientRect.bottom - ClientRect.top;
      Win32ResizeDIBSection(Width, Height);
    } break;
    case WM_DESTROY: {
      OutputDebugStringA("WM_DESTROY\n");
      Running = false;
    } break;
    case WM_CLOSE: {
      OutputDebugStringA("WM_CLOSE\n");
      Running = false;
    } break;
    case WM_ACTIVATEAPP: {
      OutputDebugStringA("WM_ACTIVATEAPP\n");
    } break;
    case WM_PAINT: {
      PAINTSTRUCT Paint;
      HDC         DeviceContext = BeginPaint(Window, &Paint);
      int         X             = Paint.rcPaint.left;
      int         Y             = Paint.rcPaint.top;
      int         Width         = Paint.rcPaint.right - Paint.rcPaint.left;
      int         Height        = Paint.rcPaint.bottom - Paint.rcPaint.top;
      Win32UpdateWindow(DeviceContext, X, Y, Width, Height);
      EndPaint(Window, &Paint);
    } break;
    default: {
      // OutputDebugStringA("Default\n");
      Result = DefWindowProc(Window, Message, WParam, LParam);
    } break;
  }
  return Result;
}
int CALLBACK WinMain(HINSTANCE Instance,
                     HINSTANCE PrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow) {
  WNDCLASSA WindowClass   = {};
  WindowClass.style       = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  WindowClass.lpfnWndProc = Win32MainWindowCallback;
  WindowClass.hInstance   = Instance;
  // WindowClass.hIcon;
  WindowClass.lpszClassName = "HandMadeHeroWindowClass";

  if (RegisterClass(&WindowClass)) {
    HWND WindowHandle = CreateWindowEx(0,
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
      Running = true;
      while (Running) {
        MSG  Message;
        BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
        if (MessageResult > 0) {
          TranslateMessage(&Message);
          DispatchMessage(&Message);
        } else {
          break;
        }
      }
    } else {
      // TODO(kskr) :
    }

  } else {
    // TODO(kskr): Logging
  }
  return (0);
}
