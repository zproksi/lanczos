#pragma once
#include <chrono>

class timemeasurer
{
public:
    timemeasurer() = default;
    ~timemeasurer();

    timemeasurer(const timemeasurer&) = delete;
    timemeasurer(timemeasurer&&) = delete;
    timemeasurer& operator =(const timemeasurer&) = delete;
    timemeasurer&& operator =(timemeasurer&&) = delete;
protected:
    std::chrono::steady_clock::time_point _start = std::chrono::high_resolution_clock::now();
};

