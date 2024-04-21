/*
 * ============================================
 * LIBSBMR
 * ============================================
 * FILE     src/Coloring.hpp
 * AUTHOR   Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE  Copyright (c) 2024 Ilya Akkuzin
 * ============================================
 * */

#ifndef SBMR_COLORING_HPP_INCLUDED
#define SBMR_COLORING_HPP_INCLUDED

#include "Types.hpp"
#include "Macros.hpp"

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


#endif // SBMR_COLORING_HPP_INCLUDED

