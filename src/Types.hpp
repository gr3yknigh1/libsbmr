/*
 * ============================================
 * LIBSBMR
 * ============================================
 * FILE     src/Types.hpp
 * AUTHOR   Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE  Unlicensed
 * ============================================
 * */
#ifndef SBMR_TYPES_HPP_INCLUDED
#define SBMR_TYPES_HPP_INCLUDED

typedef signed char     S8;
typedef signed short    S16;
typedef signed int      S32;
typedef signed long     S64;

typedef unsigned char   U8;
typedef unsigned short  U16;
typedef unsigned int    U32;
typedef unsigned long   U64;

#define MAX_U8  255
#define MAX_U16 65535
#define MAX_U32 4294967295
#define MAX_U64 18446744073709551615

typedef float           F32;
typedef double          F64;

typedef unsigned long   Size;
typedef const char *    CStr;

#endif // SBMR_TYPES_HPP_INCLUDED
