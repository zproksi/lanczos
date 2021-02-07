#include "stdafx.h"
#include "ConnsoleCharWriter.h"


ConnsoleCharWriter::ConnsoleCharWriter(const HANDLE& consoleHandle, const WORD color, const char c)
    : _con(consoleHandle)
    , _col(color)
    , _c(c)
{
}


ConnsoleCharWriter::~ConnsoleCharWriter()
{
}

bool ConnsoleCharWriter::GetChar(const uint32_t x, const uint32_t y, char& c) const
{
    const COORD at{ static_cast<SHORT>(x), static_cast<SHORT>(y) };
    CHAR_INFO cinfo{ (decltype(CHAR_INFO::Char.UnicodeChar))0, _col };
    SMALL_RECT whereToRead{ at.X, at.Y, at.X + 1, at.Y + 1 };
    if (0 != ::ReadConsoleOutput(_con,
        &cinfo,
        COORD{ 1, 1 },
        COORD{ 0, 0 },
        &whereToRead))
    {
        c = cinfo.Char.AsciiChar;
        return true;
    }
    return false;
}

bool ConnsoleCharWriter::WriteChar(const uint32_t x, const uint32_t y, const uint32_t color, const char c) const
{
    const COORD at{ static_cast<SHORT>(x), static_cast<SHORT>(y) };
    return WriteChar(at, 0 == color ? _col : static_cast<WORD>(color), 0 == c ? _c : c);
}

bool ConnsoleCharWriter::WriteChar(const COORD& at, const WORD clr, char c) const
{
    CHAR_INFO cinfo{ (decltype(CHAR_INFO::Char.UnicodeChar))c, clr };
    SMALL_RECT whereToWrite{ at.X, at.Y, at.X + 1, at.Y + 1 };
    return 0 != ::WriteConsoleOutput(_con,
        &cinfo,
        COORD{ 1, 1 },
        COORD{ 0, 0 },
        &whereToWrite);
}

