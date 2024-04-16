/*
 * ============================================
 * BREAKOUT
 * ============================================
 * FILE     src/Win32/ScopedDC.hpp
 * AUTHOR   Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE  Unlicensed
 * ============================================
 * */

#ifndef BRK_WIN32_SCOPEDDC_HPP_INCLUDED
#define BRK_WIN32_SCOPEDDC_HPP_INCLUDED

#include <Windows.h>


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

#endif  // BRK_WIN32_SCOPEDDC_HPP_INCLUDED
