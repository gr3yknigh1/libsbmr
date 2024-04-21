/*
 * ============================================
 * LIBSBMR
 * ============================================
 * FILE     src/StringView.hpp
 * AUTHOR   Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE  Copyright (c) 2024 Ilya Akkuzin
 * ============================================
 * */

#include "Macros.hpp"
#include "Types.hpp"
#include "String.hpp"


class StringView {
public:
    constexpr StringView() noexcept
        : m_Data(nullptr), m_Size(0)
    { }

    constexpr explicit StringView(CStr s) noexcept
        : m_Data(s), m_Size(CStr_GetLength(s))
    { }

    constexpr explicit StringView(const void *s, Size size) noexcept
        : m_Data(s), m_Size(size)
    { }

    constexpr StringView(const StringView &s) noexcept
        : m_Data(s.m_Data), m_Size(s.m_Size)
    { }

    constexpr StringView(StringView &&s) noexcept
        : m_Data(s.m_Data), m_Size(s.m_Size)
    { }

    constexpr StringView& operator=(const StringView &s) noexcept
    {
        return *this = StringView(s);
    }

    constexpr StringView& operator=(StringView &&s) noexcept
    {
        this->m_Data = s.m_Data;
        this->m_Size = s.m_Size;
        return *this;
    }

    constexpr const void * GetData() ConstMethod noexcept { return m_Data; }
    constexpr         Size GetSize() ConstMethod noexcept { return m_Size; }

private:
    const void * m_Data;
    Size         m_Size;
};
