/**
 * @file
 * @author  Mohammad S. Babaei <info@babaei.net>
 * @version 0.1.0
 *
 * @section LICENSE
 *
 * (The MIT License)
 *
 * Copyright (c) 2016 Mohammad S. Babaei
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
 * Provides useful random functions.
 */


#ifndef CORELIB_RANDOM_HPP
#define CORELIB_RANDOM_HPP


#include <string>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include "System.hpp"

namespace CoreLib {
class Random;
}

class CoreLib::Random
{
public:
    enum class Character : unsigned char {
        Alphabetic,
        Alphanumeric,
        Blank,
        Control,
        Digits,
        Graphical,
        Hexadecimal,
        Lower,
        Punctuation,
        Printable,
        Space,
        Upper
    };

private:
    struct Impl;

public:
    static void Characters(const Character &type, const size_t length, std::string &out_chars);
    static std::string Characters(const Character &type, const size_t length);

    template <typename _T>
    static _T Number(const _T lowerBound, const _T upperBound)
    {
        boost::lock_guard<boost::mutex> guard(GetLock());
        boost::random::uniform_int_distribution<> dist(lowerBound, upperBound);
        return dist(GetEngine());
    }

    static void Uuid(std::string &out_uuid);
    static std::string Uuid();

private:
    static boost::random::mt19937 &GetEngine();
    static boost::mutex &GetLock();
};


#endif /* CORELIB_RANDOM_HPP */
