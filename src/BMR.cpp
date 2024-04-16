/*
 * ============================================
 * BREAKOUT
 * ============================================
 * FILE     src/BMR.cpp
 * AUTHOR   Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE  Unlicensed
 * ============================================
 * */

#include <Windows.h>

#include "BMR.hpp"

#include "Types.hpp"
#include "Lin.hpp"
#include "Geom.hpp"
#include "Coloring.hpp"
#include "Macros.hpp"
#include "Win32/Misc.hpp"
#include "Win32/ScopedDC.hpp"


#define BMR_RENDER_COMMAND_CAPACITY 1024


/*
 * Bitmap Renderer.
 */
GlobalVar struct {
    Color4 ClearColor;

    struct {
        U8 *Begin;
        U8 *End;
    } CommandQueue;
    U64 CommandCount;

    U8 BPP;
    U64 XOffset;
    U64 YOffset;
    struct {
        void *Buffer;
        U64 Width;
        U64 Height;
    } Pixels;
    BITMAPINFO Info;
    HWND Window;
} Inst;


namespace BMR {

    InternalFunc void
    _UpdateWindow(HDC dc,
                  S32 windowXOffset,
                  S32 windowYOffset,
                  S32 windowWidth,
                  S32 windowHeight) noexcept
    {
        StretchDIBits(
            dc,
            Inst.XOffset,  Inst.YOffset,  Inst.Pixels.Width, Inst.Pixels.Height,
            windowXOffset, windowYOffset, windowWidth,       windowHeight,
            Inst.Pixels.Buffer, &Inst.Info,
            DIB_RGB_COLORS, SRCCOPY
        );
    }



    void 
    Init() noexcept {
        Inst.ClearColor = COLOR_BLACK;
        Inst.CommandQueue.Begin = (U8 *)VirtualAlloc(
            nullptr, BMR_RENDER_COMMAND_CAPACITY, MEM_COMMIT, PAGE_READWRITE);
        Inst.CommandQueue.End = Inst.CommandQueue.Begin;
        Inst.CommandCount = 0;

        Inst.BPP = BMR_BPP;
        Inst.XOffset = 0;
        Inst.YOffset = 0;

        Inst.Pixels.Buffer = nullptr;
        Inst.Pixels.Width = 0;
        Inst.Pixels.Height = 0;
    }

    void 
    DeInit() noexcept
    { 
        if (Inst.CommandQueue.Begin != nullptr && VirtualFree(Inst.CommandQueue.Begin, 0, MEM_RELEASE) == 0) {
            // TODO(ilya.a): Handle memory free error.
        }
        else {
            Inst.CommandQueue.Begin = nullptr;
            Inst.CommandQueue.End   = nullptr;
        }

        if (Inst.Pixels.Buffer != nullptr && VirtualFree(Inst.Pixels.Buffer, 0, MEM_RELEASE) == 0) {
            // TODO(ilya.a): Handle memory free error.
        } else {
            Inst.Pixels.Buffer = nullptr;
        }
    }

    void 
    BeginDrawing(HWND window) noexcept 
    {
        Inst.Window = window;
    }

    void 
    EndDrawing() noexcept
    {
        Size pitch = Inst.Pixels.Width * Inst.BPP;
        U8 * row = (U8 *) Inst.Pixels.Buffer;

        for (U64 y = 0; y < Inst.Pixels.Height; ++y) {
            Color4 *pixel = (Color4 *)row;

            for (U64 x = 0; x < Inst.Pixels.Width; ++x) {
                Size offset = 0;

                for (U64 commandIdx = 0; commandIdx < Inst.CommandCount; ++commandIdx) {
                    RenderCommandType type = 
                        *((RenderCommandType *)(Inst.CommandQueue.Begin + offset));

                    offset += sizeof(RenderCommandType);

                    switch (type) {
                        case (RenderCommandType::CLEAR): {
                            Color4 color = *(Color4*)(Inst.CommandQueue.Begin + offset);
                            offset += sizeof(Color4);
                            *pixel = color;
                        } break;
                        case (RenderCommandType::RECT): {
                            Rect rect = *(Rect*)(Inst.CommandQueue.Begin + offset);
                            offset += sizeof(rect);

                            Color4 color = *(Color4*)(Inst.CommandQueue.Begin + offset);
                            offset += sizeof(Color4);

                            if (rect.IsInside(x, y)) {
                                *pixel = color;
                            }
                        } break;
                        case (RenderCommandType::GRADIENT): {
                            Vec2u v = *(Vec2u*)(Inst.CommandQueue.Begin + offset);
                            offset += sizeof(Vec2u);

                            *pixel = Color4(x + v.X, y + v.Y, 0);
                        } break;
                        case (RenderCommandType::NOP):
                        default: {
                            *pixel = Inst.ClearColor;
                        } break;
                    };
                }
                ++pixel;
            }

            row += pitch;
        }

        {
            // TODO(ilya.a): Check how it's differs with event thing.
            auto dc = ScopedDC(Inst.Window);

            RECT windowRect;
            GetClientRect(Inst.Window, &windowRect);
            S32 x = windowRect.left;
            S32 y = windowRect.top;
            S32 width = 0, height = 0;
            GetRectSize(&windowRect, &width, &height);

            _UpdateWindow(dc.Handle, x, y, width, height);
        }

        Inst.CommandQueue.End = Inst.CommandQueue.Begin;
        Inst.CommandCount = 0;
    }


    void 
    Update(HWND window) noexcept 
    {
        PAINTSTRUCT ps = {0};
        HDC dc = BeginPaint(window, &ps);

        if (dc == nullptr) {
            // TODO(ilya.a): Handle error
        } else {
            S32 x = ps.rcPaint.left;
            S32 y = ps.rcPaint.top;
            S32 width = 0, height = 0;
            GetRectSize(&(ps.rcPaint), &width, &height);
            _UpdateWindow(dc, x, y, width, height);
        }

        EndPaint(window, &ps);
    }


    void 
    Resize(S32 w, S32 h) noexcept
    {
        if (Inst.Pixels.Buffer != nullptr && VirtualFree(Inst.Pixels.Buffer, 0, MEM_RELEASE) == 0) {
            //                                                                  ^^^^^^^^^^^
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
        Inst.Pixels.Width = w;
        Inst.Pixels.Height = h;

        Inst.Info.bmiHeader.biSize          = sizeof(Inst.Info.bmiHeader);
        Inst.Info.bmiHeader.biWidth         = w;
        Inst.Info.bmiHeader.biHeight        = h;
        Inst.Info.bmiHeader.biPlanes        = 1;
        Inst.Info.bmiHeader.biBitCount      = 32;      // NOTE: Align to WORD
        Inst.Info.bmiHeader.biCompression   = BI_RGB;
        Inst.Info.bmiHeader.biSizeImage     = 0;
        Inst.Info.bmiHeader.biXPelsPerMeter = 0;
        Inst.Info.bmiHeader.biYPelsPerMeter = 0;
        Inst.Info.bmiHeader.biClrUsed       = 0;
        Inst.Info.bmiHeader.biClrImportant  = 0;

        Size bufferSize = w * h * Inst.BPP;
        Inst.Pixels.Buffer = 
            VirtualAlloc(nullptr, bufferSize, MEM_COMMIT, PAGE_READWRITE);

        if (Inst.Pixels.Buffer == nullptr) {
            // TODO:(ilya.a): Check for errors.
            OutputDebugString("Failed to allocate memory for backbuffer!\n");
        }
    }


    // TODO(ilya.a): Find better way to provide payload.
    template<typename T> InternalFunc void 
    _PushRenderCommand(RenderCommandType type, const T &payload) noexcept
    {
        *(RenderCommand<T> *)Inst.CommandQueue.End = RenderCommand<T>(type, payload);
        Inst.CommandQueue.End += sizeof(RenderCommand<T>);

        Inst.CommandCount++;
    }

    void 
    SetClearColor(const Color4 &c) noexcept 
    {
        Inst.ClearColor = c;    
    }

    struct _DrawRect_Payload {
        Rect Rect;
        Color4 Color;
    };

    void 
    Clear() noexcept 
    {
        _PushRenderCommand(
            RenderCommandType::CLEAR, 
            Inst.ClearColor
        );
    }

    void 
    DrawRect(const Rect &r, const Color4 &c) noexcept 
    {
        _PushRenderCommand(
            RenderCommandType::RECT, 
            _DrawRect_Payload{r, c}
        );
    }

    void 
    DrawRect(U32 x, U32 y, 
             U32 w, U32 h, 
             const Color4 &c) noexcept 
    {
        _PushRenderCommand(
            RenderCommandType::RECT, 
            _DrawRect_Payload{Rect(x, y, w, h), c}
        );
    }

    void 
    DrawGrad(U32 xOffset, U32 yOffset) noexcept 
    {
        _PushRenderCommand(
            RenderCommandType::GRADIENT, 
            Vec2u(xOffset, yOffset)
        );
    }

    void 
    DrawGrad(Vec2u offset) noexcept 
    {
        _PushRenderCommand(
            RenderCommandType::GRADIENT, 
            offset
        );
    }


};  // namespace BMR
