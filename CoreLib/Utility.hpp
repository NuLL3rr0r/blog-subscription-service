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
 * Common utility funtions.
 */


#ifndef CORELIB_UTILITY_HPP
#define CORELIB_UTILITY_HPP


#include <string>
#include <unordered_map>

namespace CoreLib {
class Utility;
}

class CoreLib::Utility
{
public:
    template <typename _T>
    struct Hasher
    {
        std::size_t operator()(const _T &t) const
        {
            return std::hash<unsigned char>()(static_cast<unsigned char>(t));
        }
    };

    template <typename _T>
    struct HashMapper
    {
        typedef std::unordered_map<_T, std::string, Hasher<_T>> HashToString;
        typedef std::unordered_map<std::string, _T> HashToEnumClass;
    };

    template <typename _E>
    static constexpr typename std::underlying_type<_E>::type ToUnderlyingType(const _E e) {
        return static_cast<typename std::underlying_type<_E>::type>(e);
    }

    static constexpr bool StringsEqual(const char *s1, const char *s2)
    {
#if CXX_COMPILE_MODE == CXX_COMPILE_MODE_0x \
        || CXX_COMPILE_MODE == CXX_COMPILE_MODE_11
        return *s1 == *s2 && (*s1 == '\0' || StringsEqual(s1 + 1, s2 + 1));
#elif CXX_COMPILE_MODE == CXX_COMPILE_MODE_1y \
        || CXX_COMPILE_MODE == CXX_COMPILE_MODE_14 \
        || CXX_COMPILE_MODE == CXX_COMPILE_MODE_1z
        while (*s1 || *s2) {
            if (*s1++ != *s2++) {
                return false;
            }
        }
        return true;
#endif	/* CXX_COMPILE_MODE == CXX_COMPILE_MODE_0x \
        || CXX_COMPILE_MODE == CXX_COMPILE_MODE_11 */ 
    }

    static std::string CalculateSize(const std::size_t size);
};


#endif /* CORELIB_UTILITY_HPP */
