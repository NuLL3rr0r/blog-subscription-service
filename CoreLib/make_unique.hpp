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
 * C++14 make_unique implementation for C++11 compilers.
 */


#ifndef CORELIB_MAKE_UNIQUE_HPP
#define CORELIB_MAKE_UNIQUE_HPP


#if CXX_COMPILE_MODE == CXX_COMPILE_MODE_98 \
    || CXX_COMPILE_MODE == CXX_COMPILE_MODE_03 \
    || CXX_COMPILE_MODE == CXX_COMPILE_MODE_0x \
    || CXX_COMPILE_MODE == CXX_COMPILE_MODE_11

#include <memory>
#include <utility>

namespace std {
template<typename _T, typename ..._ARGS>
std::unique_ptr<_T> make_unique(_ARGS &&...args)
{
    return std::unique_ptr<_T>(new _T(std::forward<_ARGS>(args)...));
}
}

#endif /* CXX_COMPILE_MODE == CXX_COMPILE_MODE_98 \
    || CXX_COMPILE_MODE == CXX_COMPILE_MODE_03 \
    || CXX_COMPILE_MODE == CXX_COMPILE_MODE_0x \
    || CXX_COMPILE_MODE == CXX_COMPILE_MODE_11 */


#endif /* CORELIB_MAKE_UNIQUE_HPP */
