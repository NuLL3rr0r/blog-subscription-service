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
 * Provides zlib, gzip and bzip2 comprission / decompression algorithms.
 */


#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include "Compression.hpp"
#include "Log.hpp"

#define     COMP_ERROR          "Unknow compression error!";
#define     DECOMP_ERROR        "Unknow decompression error!";

using namespace std;
using namespace boost;
using namespace CoreLib;

void Compression::Compress(const char *data, const size_t size,
                           Buffer &out_compressedBuffer,
                           const Algorithm &algorithm)
{
    const Compression::Buffer buffer(data, data + size);
    Compress(buffer, out_compressedBuffer, algorithm);
}

void Compression::Compress(const std::string &dataString,
                           Buffer &out_compressedBuffer,
                           const Algorithm &algorithm)
{
    const Buffer buffer(dataString.begin(), dataString.end());
    Compress(buffer, out_compressedBuffer, algorithm);
}

void Compression::Compress(const Buffer &dataBuffer,
                           Buffer &out_compressedBuffer,
                           const Algorithm &algorithm)
{
    try {
        out_compressedBuffer.clear();
        iostreams::filtering_streambuf<iostreams::output> output;

        switch(algorithm) {
        case Algorithm::Zlib:
            output.push(iostreams::zlib_compressor());
            break;
        case Algorithm::Gzip:
            output.push(iostreams::gzip_compressor());
            break;
        case Algorithm::Bzip2:
            output.push(iostreams::bzip2_compressor());
            break;
        }

        output.push(iostreams::back_inserter(out_compressedBuffer));
        iostreams::write(output, &dataBuffer[0], static_cast<std::streamsize>(dataBuffer.size()));
    } catch(...) {
        LOG_ERROR(COMP_ERROR)
    }
}

void Compression::Decompress(const Buffer &dataBuffer,
                             std::string &out_uncompressedString,
                             const Algorithm &algorithm)
{
    Buffer buffer;
    Decompress(dataBuffer, buffer, algorithm);
    out_uncompressedString = std::string(buffer.begin(), buffer.end());
}

void Compression::Decompress(const Buffer &dataBuffer,
                             Buffer &out_uncompressedBuffer,
                             const Algorithm &algorithm)
{
    try {
        out_uncompressedBuffer.clear();
        iostreams::filtering_streambuf<iostreams::output> output;

        switch(algorithm) {
        case Algorithm::Zlib:
            output.push(iostreams::zlib_decompressor());
            break;
        case Algorithm::Gzip:
            output.push(iostreams::gzip_decompressor());
            break;
        case Algorithm::Bzip2:
            output.push(iostreams::bzip2_decompressor());
            break;
        }

        output.push(iostreams::back_inserter(out_uncompressedBuffer));
        iostreams::write(output, &dataBuffer[0], static_cast<streamsize>(dataBuffer.size()));
    } catch(...) {
        LOG_ERROR(DECOMP_ERROR)
    }
}
