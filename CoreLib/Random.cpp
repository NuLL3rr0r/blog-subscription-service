/**
 * @file
 * @author  Mamadou Babaei <info@babaei.net>
 * @version 0.1.0
 *
 * @section LICENSE
 *
 * (The MIT License)
 *
 * Copyright (c) 2016 - 2021 Mamadou Babaei
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
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
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
    CoreLib::Utility::Hasher<const CoreLib::Random::Character>> CharactersHashTable;

public:
    static CharactersHashTable &GetLookupTable();
};

void Random::Characters(const Character &type, const size_t length, std::string &out_chars)
{
    boost::lock_guard<boost::mutex> guard(GetLock());

    random::uniform_int_distribution<> index_dist(0, static_cast<int>(Impl::GetLookupTable()[type].size()) - 1);

    out_chars.clear();
    for (size_t i = 0; i < length; ++i) {
        out_chars += Impl::GetLookupTable()[type][static_cast<size_t>(index_dist(GetEngine()))];
    }
}

std::string Random::Characters(const Character &type, const size_t length)
{
    string chars;
    Characters(type, length, chars);
    return chars;
}

void Random::Uuid(std::string &out_uuid)
{
    static uuids::basic_random_generator<random::mt19937> rng(&GetEngine());
    uuids::uuid u = rng();
    out_uuid.assign(uuids::to_string(u));
}

std::string Random::Uuid()
{
    string uuid;
    Uuid(uuid);
    return uuid;
}

boost::random::mt19937 &Random::GetEngine()
{
    static random::random_device rd;
    static random::mt19937 rng(rd);
    return rng;
}

boost::mutex &Random::GetLock()
{
    static boost::mutex lock;
    return lock;
}

Random::Impl::CharactersHashTable &Random::Impl::GetLookupTable()
{
    static CharactersHashTable lookupTable {
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
    };
    return lookupTable;
}
