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
 * Contains necessary functions to unzip a file.
 */


#ifndef CORELIB_ARCHIVER_HPP
#define CORELIB_ARCHIVER_HPP


#include <string>

namespace CoreLib {
class Archiver;
}

class CoreLib::Archiver
{
public:
    static bool UnGzip(const std::string &archive, const std::string &extractedFile);
    static bool UnGzip(const std::string &archive, const std::string &extractedFile,
                       std::string &out_error);

    static bool UnTar(const std::string &archive, const std::string &extractDirectory);
    static bool UnTar(const std::string &archive, const std::string &extractDirectory,
                       std::string &out_error);

    static bool UnZip(const std::string &archive, const std::string &extractionPath);
    static bool UnZip(const std::string &archive, const std::string &extractionPath,
                      std::string &out_error);
};


#endif /* CORELIB_ARCHIVER_HPP */
