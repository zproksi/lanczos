#pragma once
class ConnsoleCharWriter
{
public:
    ConnsoleCharWriter(const HANDLE& consoleHandle, const WORD color, const char c);
    ~ConnsoleCharWriter();

///@brief Get Char from console position
    bool GetChar(const uint32_t x, const uint32_t y, char& c) const;

///@brief Put Char to console position with specified color
    bool WriteChar(const uint32_t x, const uint32_t y, const uint32_t color = 0, const char c = 0) const;

    ///@ just to check our pattern
    char MyChar()const throw() { return _c; }
protected:
    bool WriteChar(const COORD& at, const WORD clr, char c) const;

protected:
    const HANDLE& _con;
    const char _c;
    const WORD _col;
};

#define COLOR_FOR_US_001 FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define COLOR_FOR_US_002 BACKGROUND_INTENSITY | BACKGROUND_BLUE
#define COLOR_FOR_US_003 FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY
#define COLOR_FOR_US_004 BACKGROUND_INTENSITY | BACKGROUND_BLUE | BACKGROUND_GREEN
#define COLOR_FOR_US_005 BACKGROUND_INTENSITY | BACKGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY
