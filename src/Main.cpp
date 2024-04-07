/*
 * ============================================
 * BREAKOUT
 * ============================================
 * FILE     src/Main.cpp
 * AUTHOR   Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE  Unlicensed
 * ============================================
 *
 * This is small educational project where I
 * doodling around with Windows API and computer
 * graphics.
 * */

#include <cstdio>
#include <cstdlib>
#include <memory>

#include <Windows.h>

#include "StringView.hpp"
#include "Types.hpp"
#include "String.hpp"
#include "Macros.hpp"

GlobalVar bool       shouldStop = false;
GlobalVar BITMAPINFO bmInfo     = {0};
GlobalVar void *     bmBuffer   = nullptr;

GlobalVar U8         bmBPP      = 4;
GlobalVar U32        bmOffsetX  = 0;
GlobalVar U32        bmOffsetY  = 0;
GlobalVar U32        bmWidth    = 0;
GlobalVar U32        bmHeight   = 0;


struct Rect {
    U16 X;
    U16 Y;
    U16 Width;
    U16 Height;

    constexpr Rect(U16 x = 0, U16 y = 0, U16 width = 0, U16 height = 0) noexcept 
        : X(x), Y(y), Width(width), Height(height)
    { }

    constexpr bool IsInside(U16 x, U16 y) const noexcept
    {
        return x >= X && x <= X + Width && y >= Y && y <= Y + Height;
    }
};


struct Color4 {
    U8 B;
    U8 G;
    U8 R;
    U8 A;

    constexpr Color4(U8 r = 0, U8 g = 0, U8 b = 0, U8 a = 0) noexcept
        : R(r), G(g), B(b), A(a)
    { }
};


static_assert(sizeof(Color4) == sizeof(U32));

GlobalVar const Color4 COLOR_WHITE = Color4(MAX_U8, MAX_U8, MAX_U8, MAX_U8);
GlobalVar const Color4 COLOR_RED   = Color4(MAX_U8, 0, 0, 0);
GlobalVar const Color4 COLOR_GREEN = Color4(0, MAX_U8, 0, 0);
GlobalVar const Color4 COLOR_BLUE  = Color4(0, 0, MAX_U8, 0);
GlobalVar const Color4 COLOR_BLACK = Color4(0, 0, 0, 0);


/*
 * RAII implementation of device context wrapper.
 *
 * Aquires device context and frees it on destruction.
 */
struct ScopedDC {
    HDC Handle;

    ScopedDC(HWND window) noexcept 
        : Handle(GetDC(window)), m_Window(window)
    { }

    ~ScopedDC() noexcept
    {
        ReleaseDC(this->m_Window, this->Handle);
    }

private:
    HWND m_Window;
};


#define Win32_TextOutA_CStr(HDC, X, Y, MSG) TextOutA((HDC), (X), (Y), (MSG), CStr_GetLength((MSG)))

void 
BM_FillWith(Color4 color) noexcept 
{
    for (U32 i = 0; i < bmWidth * bmHeight; ++i) 
    {
        ((Color4 *)bmBuffer)[i] = color;
    }
}

InternalFunc void
BM_RenderGradient(U32 xOffset, U32 yOffset) noexcept
{ 
    Size pitch = bmWidth * bmBPP;
    U8 * row = (U8 *) bmBuffer;

    for (U32 y = 0; y < bmHeight; ++y) 
    {
        Color4 * pixel = (Color4 *)row;

        for (U32 x = 0; x < bmWidth; ++x) 
        {
            *pixel = Color4(x + xOffset, y + yOffset, 0);
            ++pixel;
        }
        row += pitch;
    }
}


InternalFunc void
BM_RenderRect(Rect r, Color4 c) noexcept {
    Size pitch = bmWidth * bmBPP;
    U8 * row = (U8 *) bmBuffer;

    for (U32 y = 0; y < bmHeight; ++y) 
    {
        Color4 * pixel = (Color4 *)row;

        for (U32 x = 0; x < bmWidth; ++x) 
        {
            if (r.IsInside(x, y)) {
                *pixel = c;
            }

            ++pixel;
        }

        row += pitch;
    }
}


InternalFunc void
Win32_UpdateWindow(HDC deviceContext,
                   S32 windowOffsetX,
                   S32 windowOffsetY,
                   S32 windowWidth,
                   S32 windowHeight) noexcept
{
    StretchDIBits(
        deviceContext,
        bmOffsetX,     bmOffsetY,     bmWidth,     bmHeight,
        windowOffsetX, windowOffsetY, windowWidth, windowHeight,
        bmBuffer, &bmInfo,
        DIB_RGB_COLORS, SRCCOPY
    );
}

/*
 * DIB: Device Independent Bitmap
 */
InternalFunc void
Win32_ResizeDIBSection(S32 width,
                       S32 height) noexcept
{
    if (bmBuffer != nullptr && VirtualFree(bmBuffer, 0, MEM_RELEASE) == 0) {
        //                                              ^^^^^^^^^^^
        // NOTE(ilya.a): Might be more reasonable to use MEM_DECOMMIT instead for 
        // MEM_RELEASE. Because in that case it's will be keep buffer around, until
        // we use it again.
        // P.S. Also will be good to try protect buffer after deallocating or other
        // stuff.
        //
        // TODO(ilya.a): 
        //     - [ ] Checkout how it works.
        //     - [ ] Handle allocation error.
        OutputDebugString("Failed to free backbuffer memory!\n");
    }
    bmWidth = width;
    bmHeight = height;

    bmInfo.bmiHeader.biSize          = sizeof(bmInfo.bmiHeader);
    bmInfo.bmiHeader.biWidth         = bmWidth;
    bmInfo.bmiHeader.biHeight        = bmHeight;
    bmInfo.bmiHeader.biPlanes        = 1;
    bmInfo.bmiHeader.biBitCount      = 32;      // NOTE: Align to WORD
    bmInfo.bmiHeader.biCompression   = BI_RGB;
    bmInfo.bmiHeader.biSizeImage     = 0;
    bmInfo.bmiHeader.biXPelsPerMeter = 0;
    bmInfo.bmiHeader.biYPelsPerMeter = 0;
    bmInfo.bmiHeader.biClrUsed       = 0;
    bmInfo.bmiHeader.biClrImportant  = 0;

    Size bmSize = bmWidth * bmHeight * bmBPP;
    bmBuffer = VirtualAlloc(nullptr, bmSize, MEM_COMMIT, PAGE_READWRITE);

    if (bmBuffer == nullptr) {
        // TODO:(ilya.a): Check for errors.
        OutputDebugString("Failed to allocate memory for backbuffer!\n");
    }
}


constexpr InternalFunc void
GetRectSize(In  const RECT *r,
            Out S32        *w,
            Out S32        *h) noexcept
{
    *w = r->right  - r->left;
    *h = r->bottom - r->top;
}

LRESULT CALLBACK
Win32_MainWindowProc(HWND   window,
                     UINT   message,
                     WPARAM wParam,
                     LPARAM lParam)
{
    LRESULT result = 0;

    switch (message) {
        case WM_ACTIVATEAPP: {
            OutputDebugString("WM_ACTIVATEAPP\n");
        } break;
        case WM_SIZE: {
            OutputDebugString("WM_SIZE\n");

            RECT r = {0};
            GetClientRect(window, &r);

            S32 width = 0, height = 0;
            GetRectSize(&r, &width, &height);

            Win32_ResizeDIBSection(width, height);
        } break;
        case WM_PAINT: {
            OutputDebugString("WM_PAINT\n");
            PAINTSTRUCT paint = {0};
            HDC deviceContext = BeginPaint(window, &paint);

            if (deviceContext == nullptr) {
                // TODO(ilya.a): Handle error
            } else {
                S32 x = paint.rcPaint.left;
                S32 y = paint.rcPaint.top;
                S32 width = 0, height = 0;
                GetRectSize(&(paint.rcPaint), &width, &height);
                Win32_UpdateWindow(deviceContext, x, y, width, height);
            }

            EndPaint(window, &paint);
        } break;
        case WM_CLOSE: {
            // TODO(ilya.a): Ask for closing?
            OutputDebugString("WM_CLOSE\n");
            shouldStop = true;
        } break;
        case WM_DESTROY: {
            // TODO(ilya.a): Casey says that we maybe should recreate
            // window later?
            OutputDebugString("WM_DESTROY\n");
            // PostQuitMessage(0);
        } break;

        default: {
            // Leave other events to default Window's handler.
            result = DefWindowProc(window, message, wParam, lParam);
        } break;
    }

    return result;
}


int WINAPI
WinMain(HINSTANCE instance,
        HINSTANCE prevInstance,
        LPSTR commandLine,
        int showMode)
{

    PersistVar LPCSTR CLASS_NAME = "Breakout";
    PersistVar LPCSTR WINDOW_TITLE = "Breakout";

    WNDCLASS windowClass = {0};
    windowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    windowClass.lpfnWndProc = Win32_MainWindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = CLASS_NAME;

    if (RegisterClassA(&windowClass) == 0) {
        OutputDebugString("Failed to register window class!\n");
        return 0;
    }

    HWND window = CreateWindowExA(
        0,
        windowClass.lpszClassName,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,  // int x
        CW_USEDEFAULT,  // int y
        CW_USEDEFAULT,  // int width
        CW_USEDEFAULT,  // int height
        nullptr,        // windowParent
        nullptr,        // menu
        instance,
        nullptr);

    if (window == nullptr) {
        OutputDebugString("Failed to initialize window!\n");
        return 0;
    }

    ShowWindow(window, showMode);

    U32 xOffset = 0;
    U32 yOffset = 0;

    while (!shouldStop) {

        MSG message = {};
        while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                // NOTE(ilya.a): Make sure that we will quit the mainloop.
                shouldStop = true;
            }

            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        BM_FillWith(COLOR_WHITE);
        BM_RenderGradient(xOffset, yOffset);
        BM_RenderRect(Rect(100 + xOffset, 100, 200, 200), COLOR_BLACK);

        {
            auto dc = ScopedDC(window);

            RECT windowRect;
            GetClientRect(window, &windowRect);
            S32 x = windowRect.left;
            S32 y = windowRect.top;
            S32 width = 0, height = 0;
            GetRectSize(&windowRect, &width, &height);

            Win32_UpdateWindow(dc.Handle, x, y, width, height);
        }

        xOffset++;
        yOffset++;
    }

    return 0;
}
