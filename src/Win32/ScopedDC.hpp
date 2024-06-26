/*
 * ============================================
 * LIBSBMR
 * ============================================
 * FILE     src/Win32/ScopedDC.hpp
 * AUTHOR   Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE  Copyright (c) 2024 Ilya Akkuzin
 * ============================================
 * */

#ifndef SBMR_WIN32_SCOPEDDC_HPP_INCLUDED
#define SBMR_WIN32_SCOPEDDC_HPP_INCLUDED

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

#endif  // SBMR_WIN32_SCOPEDDC_HPP_INCLUDED
