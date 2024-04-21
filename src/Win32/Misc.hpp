/*
 * ============================================
 * LIBSBMR
 * ============================================
 * FILE     src/Win32/Misc.hpp
 * AUTHOR   Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE  Copyright (c) 2024 Ilya Akkuzin
 * ============================================
 * */

#ifndef SBMR_WIN32_MISC_HPP_INCLUDED
#define SBMR_WIN32_MISC_HPP_INCLUDED

#include <Windows.h>


#include "Macros.hpp"
#include "Types.hpp"
#include "String.hpp"


/*
 * Extracts width and height of Win32's `RECT` type.
 */
constexpr void
GetRectSize(In  const RECT *r,
            Out S32        *w,
            Out S32        *h) noexcept
{
    *w = r->right  - r->left;
    *h = r->bottom - r->top;
}


#define Win32_TextOutA_CStr(HDC, X, Y, MSG) TextOutA((HDC), (X), (Y), (MSG), CStr_GetLength((MSG)))


#endif  // SBMR_WIN32_MISC_HPP_INCLUDED
