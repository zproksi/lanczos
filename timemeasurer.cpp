#include "stdafx.h"
#include "timemeasurer.h"

timemeasurer::~timemeasurer()
{
    auto elapsed = std::chrono::high_resolution_clock::now() - _start;
    const long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    std::string s = std::to_string(microseconds);
    const size_t len = s.length();
    for (size_t i = 0; len > 3 && i < (len - 1) / 3; ++i)
    {
        s.insert(len - ((i + 1) * 3), 1, ',');
    }
    std::cout << "Working time " << s << " microseconds" << std::endl;
}
