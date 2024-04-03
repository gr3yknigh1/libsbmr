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
GlobalVar HBITMAP    bmHandle   = nullptr;
GlobalVar void *     bmBuffer   = nullptr;
GlobalVar HDC        bmDC       = nullptr;

#define Win32_TextOutA_CStr(HDC, X, Y, MSG) TextOutA((HDC), (X), (Y), (MSG), CStr::GetLength((MSG)))

InternalFunc void
Win32_UpdateWindow(HDC deviceContext,
                   s32 x,
                   s32 y,
                   s32 width,
                   s32 height) noexcept
{
    /*
     * int StretchDIBits(
     *   [in] HDC              hdc,
     *   [in] int              xDest,
     *   [in] int              yDest,
     *   [in] int              DestWidth,
     *   [in] int              DestHeight,
     *   [in] int              xSrc,
     *   [in] int              ySrc,
     *   [in] int              SrcWidth,
     *   [in] int              SrcHeight,
     *   [in] const VOID       *lpBits,
     *   [in] const BITMAPINFO *lpbmi,
     *   [in] UINT             iUsage,       palletize (DIB_PAL_COLORS) or rgb (DIB_RGB_COLORS)
     *   [in] DWORD            rop           Raster-operation code: (lookup at msdn)
     * );
     */
    StretchDIBits(
        deviceContext,
        x, y, width, height, // Copying from buffer with exactly the same
        x, y, width, height, // offset and size, which is our dest.
        bmBuffer,
        &bmInfo,
        DIB_RGB_COLORS,
        SRCCOPY
    );
}

/*
 * DIB: Device Independent Bitmap
 */
InternalFunc void
Win32_ResizeDIBSection(s32 width,
                       s32 height) noexcept
{
    /* HBITMAP CreateDIBSection(
     *   [in]  HDC              hdc,
     *   [in]  const BITMAPINFO *pbmi,
     *   [in]  UINT             usage,
     *   [out] VOID             **ppvBits,
     *   [in]  HANDLE           hSection,
     *   [in]  DWORD            offset
     * );
     */

    if (bmHandle != nullptr) {
        DeleteObject(bmHandle);
    }

    if (bmDC == nullptr) {
        // TODO(ilya.a): Should we recreate context?
        bmDC = CreateCompatibleDC(nullptr);
    }

    bmInfo.bmiHeader.biSize          = sizeof(bmInfo.bmiHeader);
    bmInfo.bmiHeader.biWidth         = width;
    bmInfo.bmiHeader.biHeight        = height;
    bmInfo.bmiHeader.biPlanes        = 1;
    bmInfo.bmiHeader.biBitCount      = 32;      // NOTE: Align to WORD
    bmInfo.bmiHeader.biCompression   = BI_RGB;
    bmInfo.bmiHeader.biSizeImage     = 0;
    bmInfo.bmiHeader.biXPelsPerMeter = 0;
    bmInfo.bmiHeader.biYPelsPerMeter = 0;
    bmInfo.bmiHeader.biClrUsed       = 0;
    bmInfo.bmiHeader.biClrImportant  = 0;

    // TODO(ilya.a): Handle error
    // HDC deviceContext = GetCompatibleDC(nullptr); // Asking device for input.

    bmHandle = CreateDIBSection(
        bmDC,
        &bmInfo,
        DIB_RGB_COLORS,
        &bmBuffer,
        nullptr, 0);

    // ReleaseDC(0, deviceContext);
}


constexpr InternalFunc void
GetRectSize(In  const RECT *r,
            Out s32        *w,
            Out s32        *h) noexcept
{
    *w = r->right  - r->left;
    *h = r->bottom - r->top;
}


LRESULT CALLBACK
Win32_MainWindowProc(HWND window,
                     UINT message,
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

            s32 width = 0, height = 0;
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
                s32 x = paint.rcPaint.left;
                s32 y = paint.rcPaint.top;
                s32 width = 0, height = 0;
                GetRectSize(&(paint.rcPaint), &width, &height);

                Win32_UpdateWindow(deviceContext, x, y, width, height);

                PatBlt(deviceContext, x, y, width, height, WHITENESS);
                Win32_TextOutA_CStr(deviceContext, 0, 0, "Hello world!");
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

    MSG message = {};

    while (!shouldStop && GetMessageA(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return 0;
}
