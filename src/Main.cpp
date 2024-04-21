/*
 * ============================================
 * LIBSBMR
 * ============================================
 * FILE     src/Main.cpp
 * AUTHOR   Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE  Copyright (c) 2024 Ilya Akkuzin
 * ============================================
 *
 * This is small educational project where I
 * doodling around with Windows API and computer
 * graphics.
 * */

#include <Windows.h>

#include "Types.hpp"
#include "String.hpp"
#include "Macros.hpp"
#include "Lin.hpp"
#include "Geom.hpp"
#include "Coloring.hpp"
#include "BMR.hpp"
#include "Win32/Keys.hpp"
#include "Win32/Misc.hpp"


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
    Vec2i Input;
} box;


#define PLAYER_INIT_X 100
#define PLAYER_INIT_Y 60
#define PLAYER_WIDTH 160
#define PLAYER_HEIGHT 80
#define PLAYER_COLOR (COLOR_RED + COLOR_BLUE)
#define PLAYER_SPEED 10

#define BLOCKS_XOFFSET 300
#define BLOCKS_YOFFSET 500
#define BLOCKS_PER_ROW 4
#define BLOCKS_ROWS_COUNT 2
#define BLOCKS_XPADDING 5
#define BLOCKS_YPADDING 5
#define BLOCK_WIDTH 100
#define BLOCK_HEIGHT 80
#define BLOCK_COLOR COLOR_RED


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
            S32 width = LOWORD(lParam);
            S32 height = HIWORD(lParam);
            BMR::Resize(width, height);
        } break;
        case WM_PAINT: {
            OutputDebugString("WM_PAINT\n");
            BMR::Update(window);
        } break;
        case WM_KEYDOWN: {
            switch (wParam) {
                case VK_LEFT: {
                    player.Input.LeftPressed = true;
                } break;
                case VK_RIGHT: {
                    player.Input.RightPressed = true;
                } break;
#ifdef COLLISSION_TESTING
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
#endif
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
#ifdef COLLISSION_TESTING
                case KEY_A:
                case KEY_D: {
                    box.Input.X = 0;
                } break;
                case KEY_W:
                case KEY_S: {
                    box.Input.Y = 0;
                } break;
#endif
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
WinMain(_In_ HINSTANCE instance,
        _In_opt_ HINSTANCE prevInstance,
        _In_ LPSTR commandLine,
        _In_ int showMode)
{

    BMR::Init();

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

    BMR::SetClearColor(COLOR_WHITE);

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

#ifdef COLLISSION_TESTING
        box.Rect.X += PLAYER_SPEED * box.Input.X;
        box.Rect.Y += PLAYER_SPEED * box.Input.Y;

        if (player.Rect.IsOverlapping(box.Rect)) {
            player.Color = COLOR_YELLOW;
        } else {
            player.Color = PLAYER_COLOR;
        }
#endif

        BMR::BeginDrawing(window);

        BMR::Clear();
        BMR::DrawGrad(xOffset, yOffset);
        BMR::DrawRect(player.Rect, player.Color);

        BMR::DrawLine(100, 200, 500, 600);

#ifdef BLOCKS_RENDERING
        // NOTE(ilya.a): This is really dog-slow :c
        for (U32 blockYGrid = 0; blockYGrid < BLOCKS_ROWS_COUNT; ++blockYGrid) {
            for (U32 blockXGrid = 0; blockXGrid < BLOCKS_PER_ROW; ++blockXGrid) {
                U32 blockXCoord = blockXGrid * BLOCK_WIDTH + BLOCKS_XOFFSET + BLOCKS_XPADDING * blockXGrid;
                U32 blockYCoord = blockYGrid * BLOCK_HEIGHT + BLOCKS_YOFFSET + BLOCKS_YPADDING * blockYGrid;
                BMR::DrawRect(
                    blockXCoord, blockYCoord, BLOCK_WIDTH, BLOCK_HEIGHT, BLOCK_COLOR);
            }
        }
#endif

#ifdef COLLISSION_TESTING
        BMR::DrawRect(box.Rect, box.Color);
#endif

        BMR::EndDrawing();

        xOffset++;
        yOffset++;
    }

    BMR::DeInit();

    return 0;
}
