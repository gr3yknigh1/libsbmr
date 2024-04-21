/*
 * ============================================
 * LIBSBMR
 * ============================================
 * FILE     src/Geom.hpp
 * AUTHOR   Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE  Unlicensed
 * ============================================
 * */


#ifndef SBMR_GEOM_HPP_INCLUDED
#define SBMR_GEOM_HPP_INCLUDED

#include "Types.hpp"

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


    // TODO(ilya.a): Fix bug when `r` is bigger than `this`.
    constexpr bool IsOverlapping(const Rect &r) const noexcept
    {
        return IsInside(r.X + 0      , r.Y + 0) 
            || IsInside(r.X + r.Width, r.Y + r.Height)
            || IsInside(r.X + r.Width, r.Y + 0)
            || IsInside(r.X + 0      , r.Y + r.Height);
    }
};


constexpr U64 
GetOffset(U64 width, U64 y, U64 x) noexcept
{ 
    return width * y + x; 
}

#endif  // SBMR_GEOM_HPP_INCLUDED


