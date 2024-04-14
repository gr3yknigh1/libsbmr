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
#include "Lin.hpp"
#include "Geom.hpp"
#include "Coloring.hpp"



constexpr InternalFunc U64 
GetOffset(U64 width, U64 y, U64 x) noexcept
{ 
    return width * y + x; 
}

/*
 * Extracts width and height of Win32's `RECT` type.
 */
constexpr InternalFunc void
GetRectSize(In  const RECT *r,
            Out S32        *w,
            Out S32        *h) noexcept
{
    *w = r->right  - r->left;
    *h = r->bottom - r->top;
}


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



enum class RenderCommandType {
    NOP  = 0,
    FILL = 1,

    RECT = 10,
};


template<typename T>
struct RenderCommand {
    RenderCommandType Type;
    T Payload;

    constexpr 
    RenderCommand(RenderCommandType type, 
                  T                 payload) noexcept
        : Type(type), Payload(payload)
    { }
};


#define BMR_RENDER_COMMAND_CAPACITY 1024

/*
 * Bitmap Renderer.
 */
GlobalVar struct {
    Color4 ClearColor;

    U8 *Queue;
    U64 QueueCount;

    U8 BPP;                 // Bytes Per Pixel
    U64 XOffset;
    U64 YOffset;
    struct {
        void *Buffer;
        U64 Width;
        U64 Height;
    } Pixels;
    BITMAPINFO Info;
    HWND Window;
} BMR;


InternalFunc void
BMR_Init(U8 bpp = 4) noexcept
{
    BMR.ClearColor = COLOR_BLACK;

    BMR.Queue = (U8 *)VirtualAlloc(
        nullptr, BMR_RENDER_COMMAND_CAPACITY, MEM_COMMIT, PAGE_READWRITE);
    BMR.QueueCount = 0;

    BMR.BPP = bpp;
    BMR.XOffset = 0;
    BMR.YOffset = 0;

    BMR.Pixels.Buffer = nullptr;
    BMR.Pixels.Width = 0;
    BMR.Pixels.Height = 0;

}


InternalFunc void
BMR_Destroy() noexcept
{ }


InternalFunc void
BMR_BeginDrawing(HWND window) noexcept
{
    BMR.Window = window;
}


InternalFunc void
Win32_UpdateWindow(HDC deviceContext,
                   S32 windowXOffset,
                   S32 windowYOffset,
                   S32 windowWidth,
                   S32 windowHeight) noexcept
{
    StretchDIBits(
        deviceContext,
        BMR.XOffset,   BMR.YOffset,   BMR.Pixels.Width,   BMR.Pixels.Height,
        windowXOffset, windowYOffset, windowWidth, windowHeight,
        BMR.Pixels.Buffer, &BMR.Info,
        DIB_RGB_COLORS, SRCCOPY
    );
}

void
BMR_Fill(const Color4 &c) 
{
    *((RenderCommand<Color4>*)BMR.Queue) = RenderCommand<Color4>(
        RenderCommandType::FILL, c);
    BMR.QueueCount++;
}



struct _BMR_DrawRectPayload {
    Rect Rect;
    Color4 Color;
};


InternalFunc void
BMR_DrawRect(const Rect &r, const Color4 &c) noexcept
{
    *((RenderCommand<_BMR_DrawRectPayload>*)BMR.Queue) = 
        RenderCommand<_BMR_DrawRectPayload>(
            RenderCommandType::RECT, {r, c});
    BMR.QueueCount++;
}


InternalFunc void
BMR_EndDrawing() noexcept
{

    Size pitch = BMR.Pixels.Width * BMR.BPP;
    U8 * row = (U8 *) BMR.Pixels.Buffer;

    for (U64 y = 0; y < BMR.Pixels.Height; ++y) {
        Color4 *pixel = (Color4 *)row;

        for (U64 x = 0; x < BMR.Pixels.Width; ++x) {
            Size offset = 0;

            for (U64 commandIdx = 0; commandIdx < BMR.QueueCount; ++commandIdx) {

                RenderCommandType type = 
                    *((RenderCommandType *)(BMR.Queue + offset));

                offset += sizeof(RenderCommandType);

                switch (type) {
                    case (RenderCommandType::FILL): {
                        Color4 color = *(Color4*)(BMR.Queue + offset);
                        offset += sizeof(Color4);
                        *pixel = color;
                    } break;
                    case (RenderCommandType::RECT): {
                        Rect rect = *(Rect*)(BMR.Queue + offset);
                        offset += sizeof(rect);

                        Color4 color = *(Color4*)(BMR.Queue + offset);
                        offset += sizeof(Color4);

                        if (rect.IsInside(x, y)) {
                            *pixel = color;
                        }

                    } break;
                    case (RenderCommandType::NOP):
                    default: {
                        *pixel = BMR.ClearColor;
                    } break;
                };
            }
            ++pixel;
        }

        row += pitch;
    }

    {
        auto dc = ScopedDC(BMR.Window);

        RECT windowRect;
        GetClientRect(BMR.Window, &windowRect);
        S32 x = windowRect.left;
        S32 y = windowRect.top;
        S32 width = 0, height = 0;
        GetRectSize(&windowRect, &width, &height);

        Win32_UpdateWindow(dc.Handle, x, y, width, height);
    }

    BMR.QueueCount = 0;
}


GlobalVar bool shouldStop = false;

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



// InternalFunc void
// BM_RenderGradient(U32 xOffset, U32 yOffset) noexcept
// { 
//     Size pitch = bmWidth * bmBPP;
//     U8 * row = (U8 *) bmBuffer;
// 
//     for (U32 y = 0; y < bmHeight; ++y) 
//     {
//         Color4 * pixel = (Color4 *)row;
// 
//         for (U32 x = 0; x < bmWidth; ++x) 
//         {
//             *pixel = Color4(x + xOffset, y + yOffset, 0);
//             ++pixel;
//         }
//         row += pitch;
//     }
// }


/*
 * DIB: Device Independent Bitmap
 */
InternalFunc void
Win32_ResizeDIBSection(S32 width,
                       S32 height) noexcept
{
    if (BMR.Pixels.Buffer != nullptr && VirtualFree(BMR.Pixels.Buffer, 0, MEM_RELEASE) == 0) {
        //                                                                ^^^^^^^^^^^
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
    BMR.Pixels.Width = width;
    BMR.Pixels.Height = height;

    BMR.Info.bmiHeader.biSize          = sizeof(BMR.Info.bmiHeader);
    BMR.Info.bmiHeader.biWidth         = width;
    BMR.Info.bmiHeader.biHeight        = height;
    BMR.Info.bmiHeader.biPlanes        = 1;
    BMR.Info.bmiHeader.biBitCount      = 32;      // NOTE: Align to WORD
    BMR.Info.bmiHeader.biCompression   = BI_RGB;
    BMR.Info.bmiHeader.biSizeImage     = 0;
    BMR.Info.bmiHeader.biXPelsPerMeter = 0;
    BMR.Info.bmiHeader.biYPelsPerMeter = 0;
    BMR.Info.bmiHeader.biClrUsed       = 0;
    BMR.Info.bmiHeader.biClrImportant  = 0;

    Size bufferSize = width * height * BMR.BPP;
    BMR.Pixels.Buffer = 
        VirtualAlloc(nullptr, bufferSize, MEM_COMMIT, PAGE_READWRITE);

    if (BMR.Pixels.Buffer == nullptr) {
        // TODO:(ilya.a): Check for errors.
        OutputDebugString("Failed to allocate memory for backbuffer!\n");
    }
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

    BMR_Init();

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

        BMR_BeginDrawing(window);

        BMR_Fill(COLOR_WHITE);
        // BM_RenderGradient(xOffset, yOffset);
        BMR_DrawRect(player.Rect, player.Color);
        // BM_RenderRect(box.Rect, box.Color);

        BMR_EndDrawing();

        xOffset++;
        yOffset++;
    }

    BMR_Destroy();

    return 0;
}
