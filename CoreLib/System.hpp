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
 * Cross-platform OS level utility function.
 */


#ifndef CORELIB_SYSTEM_HPP
#define CORELIB_SYSTEM_HPP


#include <string>
#include <vector>

#if defined ( __unix__ )
#include <sys/types.h>
#elif defined ( _WIN32 )
#include <windows.h>
#include <winbase.h>    // Synchapi.h on Windows 8 and Windows Server 2012
#endif  // defined ( __unix__ )

namespace CoreLib {
class System;
}

class CoreLib::System
{
private:
    struct Impl;

public:
    static bool Exec(const std::string &cmd);
    static bool Exec(const std::string &cmd, std::string &out_output);

#if defined ( __unix__ )
    static bool GetLock(const std::string &name, int &out_handle);
    static void ReleaseLock(int &handle);
#elif defined ( _WIN32 )
    static bool GetLock(const std::string &name, HANDLE &out_handle);
    static void ReleaseLock(HANDLE &handle);
#endif  // defined ( __unix__ )

    static std::string GetProcessNameFromPath(const std::string &fullPath);
    static std::size_t GetPidsOfProcess(const std::string &processName, std::vector<int> &out_pids);

    static long RandSeed();
};


#endif /* CORELIB_SYSTEM_HPP */
