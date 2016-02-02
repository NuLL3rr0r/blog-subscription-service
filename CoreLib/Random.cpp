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


#include <unordered_map>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <CoreLib/make_unique.hpp>
#include "Random.hpp"
#include "Utility.hpp"

using namespace std;
using namespace boost;
using namespace CoreLib;

struct Random::Impl
{
public:
    typedef std::unordered_map<const CoreLib::Random::Character, const std::string,
    CoreLib::Utility::Hasher<const CoreLib::Random::Character>> CharacterMapper_t;

public:
    CharacterMapper_t LookupTable;

public:
    Impl();
};

std::unique_ptr<Random::Impl> Random::s_pimpl = std::make_unique<Random::Impl>();

boost::random::mt19937 &Random::GetEngine()
{
    static boost::random_device rd;
    static boost::random::mt19937 rng(rd);
    return rng;
}

void Random::Characters(const Character &type, const size_t length, std::string &out_chars)
{
    boost::random::uniform_int_distribution<> index_dist(0, (int)s_pimpl->LookupTable[type].size() - 1);

    out_chars.clear();
    for (size_t i = 0; i < length; ++i) {
        out_chars += s_pimpl->LookupTable[type][(size_t)index_dist(GetEngine())];
    }
}

std::string Random::Characters(const Character &type, const size_t length)
{
    std::string chars;
    Characters(type, length, chars);
    return chars;
}

Random::Impl::Impl() :
    LookupTable {
{Character::Alphabetic, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"},
{Character::Alphanumeric, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"},
{Character::Blank, "\t "},
{Character::Control, "0123456789"},
{Character::Digits, "0123456789"},
{Character::Graphical, "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"},
{Character::Hexadecimal, "0123456789ABCDEFabcdef"},
{Character::Lower, "abcdefghijklmnopqrstuvwxyz"},
{Character::Printable, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"},
{Character::Punctuation, "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"},
{Character::Space, "\t\n\v\f\r "},
{Character::Upper, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"}
        }
{

}

