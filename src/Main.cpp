/*
 * ============================================
 * BREAKOUT
 * ============================================
 * FILE     src/Main.cpp
 * AUTHOR   Akkuzin Ilya <gr3yknigh1@gmail.com>
 * LICENSE  Unlicensed
 * ============================================
 * */
#include <Windows.h>

#include "Types.hpp"
#include "String.hpp"


#define TextOutA_CStr(HDC, X, Y, MSG) \
    TextOutA((HDC), (X), (Y), (MSG), CStr_GetLength((MSG)))


static constexpr void
GetSizeOfRect(const RECT *r, S64 *width, S64 *height) noexcept
{
    *height = r->bottom - r->top;
    *width = r->right - r->left;
}


LRESULT CALLBACK
WindowProc(HWND window,
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
        } break;
        case WM_PAINT: {
            PAINTSTRUCT paint = {0};
            HDC deviceContext = BeginPaint(window, &paint);

            if (deviceContext == nullptr) {
                // TODO: Handle error
            }

            S64 x = paint.rcPaint.left;
            S64 y = paint.rcPaint.top;
            S64 width = 0, height = 0;
            GetSizeOfRect(&(paint.rcPaint), &width, &height);
            PatBlt(deviceContext, x, y, width, height, WHITENESS);
            colorSwitch = false;
            TextOutA_CStr(deviceContext, 0, 0, "Hello world!");
            EndPaint(window, &paint);
        } break;
        case WM_CLOSE:
        case WM_DESTROY: {
            OutputDebugString("WM_DESTROY\n");
            PostQuitMessage(0);
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

    static LPCSTR CLASS_NAME = "Breakout";
    static LPCSTR WINDOW_TITLE = "Breakout";

    WNDCLASS windowClass = {0};
    windowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    windowClass.lpfnWndProc = WindowProc;
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

    while (GetMessageA(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return 0;
}
