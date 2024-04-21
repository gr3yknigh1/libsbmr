/*
 * ============================================
 * LIBSBMR
 * ============================================
 * FILE     src/String.hpp
 * AUTHOR   Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE  Unlicensed
 * ============================================
 * */
#ifndef BKR_STRING_HPP_INCLUDED
#define BKR_STRING_HPP_INCLUDED

#include "Types.hpp"


constexpr Size 
CStr_GetLength(CStr s) noexcept
{
	Size size = 0;
	while (s[size] != '\0')
	{
		size++;
	}
	return size;
}


#endif  // BKR_STRING_HPP_INCLUDED