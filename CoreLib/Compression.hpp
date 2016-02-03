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
 * Provides zlib, gzip and bzip2 comprission / decompression algorithms.
 */


#ifndef CORELIB_COMPRESSION_HPP
#define CORELIB_COMPRESSION_HPP


#include <string>
#include <vector>

namespace CoreLib {
class Compression;
}

class CoreLib::Compression
{
public:
    typedef std::vector<char> Buffer;

public:
    enum class Algorithm : unsigned char {
        Zlib,
        Gzip,
        Bzip2
    };

public:
    static void Compress(const char *data, size_t size,
                         Buffer &out_compressedBuffer,
                         const Algorithm &algorithm);
    static void Compress(const std::string &dataString,
                         Buffer &out_compressedBuffer,
                         const Algorithm &algorithm);
    static void Compress(const Buffer &dataBuffer,
                         Buffer &out_compressedBuffer,
                         const Algorithm &algorithm);
    static void Decompress(const Buffer &dataBuffer,
                           std::string &out_uncompressedString,
                           const Algorithm &algorithm);
    static void Decompress(const Buffer &dataBuffer,
                           Buffer &out_uncompressedBuffer,
                           const Algorithm &algorithm);
};


#endif /* CORELIB_COMPRESSION_HPP */



