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
 * Provides common filing and filesystem operations.
 */


#ifndef CORELIB_FILESYSTEM_HPP
#define CORELIB_FILESYSTEM_HPP


#include <string>

namespace CoreLib {
class FileSystem;
}

class CoreLib::FileSystem
{
public:
    static bool DirExists(const std::string &dir);
    static bool FileExists(const std::string &file);
    static std::size_t FileSize(const std::string &file);

    static bool CreateDir(const std::string &dir, bool parents=true);
    static bool Erase(const std::string &path, bool recursive=true);
    static bool Move(const std::string &from, const std::string &to);
    static bool CopyFile(const std::string &from, const std::string &to, bool overwrite = true);

    static bool Read(const std::string &file, std::string &out_data);
    static bool Write(const std::string &file, const std::string &data);
};


#endif /* CORELIB_FILESYSTEM_HPP */

