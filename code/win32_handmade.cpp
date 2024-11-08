#include <Windows.h>
#include <windef.h>
#include <wingdi.h>

#include <cstddef>
#include <cstdint>

#define local_persist static;
#define global_variable static;
#define internal static;

typedef uint8_t  uint8;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint64_t uint64;

typedef int8_t  int8;
typedef int32_t int32;
typedef int16_t int16;
typedef int64_t int64;

global_variable bool       Running = false;
global_variable BITMAPINFO BitMapInfo;
global_variable void      *BitMapMemory;
global_variable int        BitMapHeight;
global_variable int        BitMapWidth;
global_variable int        BytesPerPixel = 4;

internal void RenderWeirdGradient(int XOffset, int YOffset) {
  int    Width  = BitMapWidth;
  int    Height = BitMapHeight;
  uint8 *Row    = (UINT8 *)BitMapMemory;
  int    Pitch  = Width * BytesPerPixel;

  for (int Y = 0; Y < BitMapHeight; ++Y) {
    uint8 *Pixel = (uint8 *)Row;
    for (int X = 0; X < BitMapWidth; ++X) {
      /*
      Pixel in memory  RR  GG  BB  xx
      LITTLE ENDIAN Architecture
      OxxxBBGGRR
      */
      *Pixel = (uint8)(X + XOffset);
      ++Pixel;

      *Pixel = (uint8)(Y + YOffset);
      ++Pixel;

      *Pixel = 0;
      ++Pixel;

      *Pixel = 0;
      ++Pixel;
    }
    Row += Pitch;
  }
}

internal void Win32ResizeDIBSection(int Width, int Height) {
  // TODO(kskr) Memory Management

  if (BitMapMemory) {
    VirtualFree(BitMapMemory, NULL, MEM_RELEASE);
  }

  BitMapWidth  = Width;
  BitMapHeight = Height;

  BitMapInfo.bmiHeader.biSize        = sizeof(BitMapInfo.bmiHeader);
  BitMapInfo.bmiHeader.biWidth       = BitMapWidth;
  BitMapInfo.bmiHeader.biHeight      = -BitMapHeight;
  BitMapInfo.bmiHeader.biPlanes      = 1;
  BitMapInfo.bmiHeader.biBitCount    = 32;
  BitMapInfo.bmiHeader.biCompression = BI_RGB;

  // TODO(kskr) Read more about BitBlt
  int BytesPerPixel    = 4;
  int BitMapMemorySize = (BitMapWidth * BitMapHeight * BytesPerPixel);

  BitMapMemory = VirtualAlloc(0, BitMapMemorySize, MEM_COMMIT, PAGE_READWRITE);
  // TODO(kskr) Probably clean this to black
}

internal void Win32UpdateWindow(HDC   DeviceContext,
                                RECT *ClientRect,
                                int   X,
                                int   Y,
                                int   Width,
                                int   Height) {
  int WindowWidth  = ClientRect->right - ClientRect->left;
  int WindowHeight = ClientRect->bottom - ClientRect->top;
  StretchDIBits(DeviceContext,
                /*
                              X,
                              Y,
                              Width,
                              Height,
                              X,
                              Y,
                              Width,
                              Height,
                              */
                0,
                0,
                BitMapWidth,
                BitMapHeight,
                0,
                0,
                WindowWidth,
                WindowHeight,
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
      OutputDebugStringA("WM_PAINT\n");
      PAINTSTRUCT Paint;
      HDC         DeviceContext = BeginPaint(Window, &Paint);
      int         X             = Paint.rcPaint.left;
      int         Y             = Paint.rcPaint.top;
      int         Width         = Paint.rcPaint.right - Paint.rcPaint.left;
      int         Height        = Paint.rcPaint.bottom - Paint.rcPaint.top;

      RECT ClientRect;
      GetClientRect(Window, &ClientRect);
      Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
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
    HWND Window = CreateWindowEx(0,
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

    if (Window) {
      Running     = true;
      int XOffset = 0;
      int YOffset = 0;
      while (Running) {
        MSG Message;
        while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
          if (Message.message == WM_QUIT) {
            Running = false;
          }
          TranslateMessage(&Message);
          DispatchMessage(&Message);
        }
        RenderWeirdGradient(XOffset, YOffset);
        HDC  DeviceContext = GetDC(Window);
        RECT ClientRect;
        GetClientRect(Window, &ClientRect);

        int WindowWidth  = ClientRect.right - ClientRect.left;
        int WindowHeight = ClientRect.bottom - ClientRect.top;
        Win32UpdateWindow(
            DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
        ReleaseDC(Window, DeviceContext);
        ++XOffset;
      }

    } else {
      // TODO(kskr) :
    }
  } else {
    // TODO(kskr): Logging
  }
  return (0);
}
