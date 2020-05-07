/**
 * @file
 * @author  Mamadou Babaei <info@babaei.net>
 * @version 0.1.0
 *
 * @section LICENSE
 *
 * (The MIT License)
 *
 * Copyright (c) 2016 - 2020 Mamadou Babaei
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * Utility class to measure execution time.
 */


#ifndef CORELIB_STOPWATCH_HPP
#define CORELIB_STOPWATCH_HPP


#include <chrono>

namespace CoreLib {
template<typename Clock_T = std::chrono::high_resolution_clock,
         typename Time_T = std::chrono::microseconds,
         typename Duration_T = double>
class Stopwatch;
}

template<typename Clock_T,
         typename Time_T,
         typename Duration_T>
class CoreLib::Stopwatch
{
private:
    typedef std::chrono::time_point<Clock_T>    TimePoint;

private:
    TimePoint m_start;
    TimePoint m_end;

public:
    Stopwatch()
    {
        Start();
    }

    virtual ~Stopwatch() = default;

public:
    void Start()
    {
        m_start = Clock_T::now();
        m_end = m_start;
    }

    Duration_T Stop()
    {
        m_end = Clock_T::now();
        return Elapsed();
    }

    Duration_T Elapsed()
    {
        auto delta = std::chrono::duration_cast<Time_T>(m_end - m_start);
        return delta.count();
    }
};


#endif /* CORELIB_STOPWATCH_HPP */
