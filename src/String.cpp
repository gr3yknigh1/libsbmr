/*
 * ============================================
 * BREAKOUT
 * ============================================
 * FILE     src/String.cpp
 * AUTHOR   Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE  Unlicensed
 * ============================================
 * */
#include "String.hpp"


// TODO: Check if we can mark it as `constexpr`.
Size
CStr_GetLength(CStr s) noexcept {
	Size size = 0;
	while (s[size] != '\0') {
		size++;
	}
	return size;
}
