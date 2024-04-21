/*
 * ============================================
 * LIBSBMR
 * ============================================
 * FILE     src/Lin.hpp
 * AUTHOR   Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE  Unlicensed
 * ============================================
 * */


#ifndef SBMR_LIN_HPP_INCLUDED
#define SBMR_LIN_HPP_INCLUDED


#include "Types.hpp"


// TODO: Scope `T` type to numeric only.
// template <typename T>
struct Vec2i {
    S32 X;
    S32 Y;

    constexpr Vec2i(S32 x = 0, S32 y = 0) noexcept 
        : X(x), Y(y)
    { }
};

struct Vec2u {
    U32 X;
    U32 Y;

    constexpr Vec2u(U32 x = 0, U32 y = 0) noexcept 
        : X(x), Y(y)
    { }
};

#endif // SBMR_LIN_HPP_INCLUDED
