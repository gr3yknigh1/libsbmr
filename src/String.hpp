/*
 * ============================================
 * BREAKOUT
 * ============================================
 * FILE     src/String.hpp
 * AUTHOR   Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE  Unlicensed
 * ============================================
 * */
#ifndef BKR_STRING_HPP_INCLUDED
#define BKR_STRING_HPP_INCLUDED

#include "Types.hpp"
#include "Macros.hpp"


namespace CStr {

    constexpr usize GetLength(cstr s) noexcept
    {
    	usize size = 0;
    	while (s[size] != '\0') {
    		size++;
    	}
    	return size;
    }

}  // namespace CStr

#endif  // BKR_STRING_HPP_INCLUDED