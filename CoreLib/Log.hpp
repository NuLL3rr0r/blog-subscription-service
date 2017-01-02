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
 * A thread-safe log class with support for log files and standard output that
 * provides different log levels.
 */


#ifndef CORELIB_LOG_HPP
#define CORELIB_LOG_HPP


#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

namespace CoreLib {
class Log;
}

class CoreLib::Log
{
public:
    enum class EType : unsigned char {
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Fatal
    };

private:
    struct Impl;
    static std::unique_ptr<Impl> s_pimpl;

private:
    std::ostringstream m_buffer;
    bool m_hasEntries;

public:
    static void Initialize(std::ostream &out_outputStream);
    static void Initialize(const std::string &outputDirectoryPath,
                           const std::string &outputFilePrefix);
    static void Initialize(std::ostream &out_outputStream,
                           const std::string &outputDirectoryPath,
                           const std::string &outputFilePrefix);

public:
    Log(const EType type, const std::string &file, const std::string &func, const int line, ...);
    virtual ~Log();

public:
    template<typename _T>
    Log &operator,(const _T &arg)
    {
        if (m_hasEntries)
            m_buffer << "\n";
        m_buffer << "  - " << arg;
        m_hasEntries = true;
        return *this;
    }
};


#define LOG_TRACE(...)  \
    (CoreLib::Log(CoreLib::Log::EType::Trace, __FILE__, __FUNCTION__, __LINE__)), __VA_ARGS__;

#define LOG_DEBUG(...)  \
    (CoreLib::Log(CoreLib::Log::EType::Debug, __FILE__, __FUNCTION__, __LINE__)), __VA_ARGS__;

#define LOG_INFO(...)  \
    (CoreLib::Log(CoreLib::Log::EType::Info, __FILE__, __FUNCTION__, __LINE__)), __VA_ARGS__;

#define LOG_WARNING(...)  \
    (CoreLib::Log(CoreLib::Log::EType::Warning, __FILE__, __FUNCTION__, __LINE__)), __VA_ARGS__;

#define LOG_ERROR(...)  \
    (CoreLib::Log(CoreLib::Log::EType::Error, __FILE__, __FUNCTION__, __LINE__)), __VA_ARGS__;

#define LOG_FATAL(...)  \
    (CoreLib::Log(CoreLib::Log::EType::Fatal, __FILE__, __FUNCTION__, __LINE__)), __VA_ARGS__;


#endif /* CORELIB_LOG_HPP */
