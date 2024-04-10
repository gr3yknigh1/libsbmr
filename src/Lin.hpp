/*
 * ============================================
 * BREAKOUT
 * ============================================
 * FILE     src/Lin.hpp
 * AUTHOR   Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE  Unlicensed
 * ============================================
 * */


#ifndef BRK_LIN_HPP_INCLUDED
#define BRK_LIN_HPP_INCLUDED


#include "Types.hpp"


// TODO: Scope `T` type to numeric only.
// template <typename T>
struct Vec3i {
    S32 X;
    S32 Y;

    constexpr Vec3i(S32 x = 0, S32 y = 0) noexcept 
        : X(x), Y(y)
    { }
};

#endif // BRK_LIN_HPP_INCLUDED
