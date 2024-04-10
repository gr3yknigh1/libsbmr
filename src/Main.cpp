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

#include <Windows.h>

#include "StringView.hpp"
#include "Types.hpp"
#include "String.hpp"
#include "Macros.hpp"
#include "Win32/Keys.hpp"


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


// TODO: Scope `T` type to numeric only.
// template <typename T>
constexpr S32
Abs(S32 x) noexcept
{
    return x * ((x > 0) - (x < 0));
}


// TODO: Scope `T` type to numeric only.
// template <typename T>
struct Vec3i {
    S32 X;
    S32 Y;

    constexpr Vec3i(S32 x = 0, S32 y = 0) noexcept 
        : X(x), Y(y)
    { }
};


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

    constexpr bool IsOverlapping(const Rect &r) const noexcept
    {
        return IsInside(r.X + 0      , r.Y + 0) 
            || IsInside(r.X + r.Width, r.Y + r.Height)
            || IsInside(r.X + r.Width, r.Y + 0)
            || IsInside(r.X + 0      , r.Y + r.Height);
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

    constexpr Color4 operator+(const Color4 &other) const noexcept
    {
        return Color4(R+other.R, 
                      G+other.G, 
                      B+other.B, 
                      A+other.A);
    }
};


static_assert(sizeof(Color4) == sizeof(U32));

GlobalVar constexpr Color4 COLOR_WHITE = Color4(MAX_U8, MAX_U8, MAX_U8, MAX_U8);
GlobalVar constexpr Color4 COLOR_RED   = Color4(MAX_U8, 0, 0, 0);
GlobalVar constexpr Color4 COLOR_GREEN = Color4(0, MAX_U8, 0, 0);
GlobalVar constexpr Color4 COLOR_BLUE  = Color4(0, 0, MAX_U8, 0);
GlobalVar constexpr Color4 COLOR_BLACK = Color4(0, 0, 0, 0);

GlobalVar constexpr Color4 COLOR_YELLOW = COLOR_GREEN + COLOR_RED;


GlobalVar bool       shouldStop = false;
GlobalVar BITMAPINFO bmInfo     = {0};
GlobalVar void *     bmBuffer   = nullptr;
GlobalVar U8         bmBPP      = 4;
GlobalVar U32        bmOffsetX  = 0;
GlobalVar U32        bmOffsetY  = 0;
GlobalVar U32        bmWidth    = 0;
GlobalVar U32        bmHeight   = 0;


GlobalVar struct {
    Rect Rect; 
    Color4 Color;

    struct {
        bool LeftPressed;
        bool RightPressed;
    } Input;
} player;

GlobalVar struct {
    Rect Rect;
    Color4 Color;
    Vec3i Input;
} box;


#define PLAYER_INIT_X 100
#define PLAYER_INIT_Y 60
#define PLAYER_WIDTH 160
#define PLAYER_HEIGHT 80
#define PLAYER_COLOR (COLOR_RED + COLOR_BLUE)
#define PLAYER_SPEED 10


#define Win32_TextOutA_CStr(HDC, X, Y, MSG) TextOutA((HDC), (X), (Y), (MSG), CStr_GetLength((MSG)))

InternalFunc void 
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
        case WM_KEYDOWN: {
            switch (wParam) {
                case VK_LEFT: {
                    player.Input.LeftPressed = true;
                } break;
                case VK_RIGHT: {
                    player.Input.RightPressed = true;
                } break;
                case KEY_A: {
                    box.Input.X = -1;
                } break;
                case KEY_D: {
                    box.Input.X = +1;
                } break;
                case KEY_S: {
                    box.Input.Y = -1;
                } break;
                case KEY_W: {
                    box.Input.Y = +1;
                } break;
                default: {
                } break;
            }
        } break;
        case WM_KEYUP: {
            switch (wParam) {
                case VK_LEFT: {
                    player.Input.LeftPressed = false;
                } break;
                case VK_RIGHT: {
                    player.Input.RightPressed = false;
                } break;
                case KEY_A:
                case KEY_D: {
                    box.Input.X = 0;
                } break;
                case KEY_W:
                case KEY_S: {
                    box.Input.Y = 0;
                } break;
                default: {
                } break;
            }
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

    player.Rect.X = PLAYER_INIT_X;
    player.Rect.Y = PLAYER_INIT_Y;
    player.Rect.Width = PLAYER_WIDTH;
    player.Rect.Height = PLAYER_HEIGHT;
    player.Color = PLAYER_COLOR;

    box.Rect = Rect(0, 0, 100, 100);
    box.Color = COLOR_RED;

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


        if (player.Input.LeftPressed) {
            player.Rect.X -= PLAYER_SPEED;
        }

        if (player.Input.RightPressed) {
            player.Rect.X += PLAYER_SPEED;
        }

        box.Rect.X += PLAYER_SPEED * box.Input.X;
        box.Rect.Y += PLAYER_SPEED * box.Input.Y;

        if (player.Rect.IsOverlapping(box.Rect)) {
            player.Color = COLOR_YELLOW;
        } else {
            player.Color = PLAYER_COLOR;
        }

        BM_FillWith(COLOR_WHITE);
        BM_RenderGradient(xOffset, yOffset);
        BM_RenderRect(player.Rect, player.Color);
        BM_RenderRect(box.Rect, box.Color);

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
