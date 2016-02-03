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


#include <cassert>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>
#include "make_unique.hpp"
#include "Log.hpp"

using namespace std;
using namespace boost;
using namespace CoreLib;

struct Log::Impl
{
    struct ETypeHasher
    {
        std::size_t operator()(const EType &t) const
        {
            return std::hash<unsigned char>()(static_cast<unsigned char>(t));
        }
    };

    struct StorageStruct
    {
        bool MultiStream;
        bool Initialized;

        std::unordered_map<Log::EType, std::string, ETypeHasher> LogTypeHash;

        std::ostream *LogOutputStream;

        std::ofstream LogOutputFileStream;
        std::string LogOutputDirectoryPath;
        std::string LogOutputFilePrefix;
        std::string LogOutputFilePath;
    };

    typedef std::unique_ptr<StorageStruct> Storage_ptr;

    boost::mutex StorageMutex;
    Storage_ptr StorageInstance;

    boost::mutex LogMutex;

    Impl();
    virtual ~Impl();

    StorageStruct *Storage();
};

std::unique_ptr<Log::Impl> Log::s_pimpl = make_unique<Log::Impl>();

void Log::Initialize(std::ostream &out_outputStream)
{
    if (s_pimpl->Storage()->Initialized)
        return;

    s_pimpl->Storage()->LogOutputStream = &out_outputStream;

    if (!s_pimpl->Storage()->MultiStream)
        s_pimpl->Storage()->Initialized = true;
}

void Log::Initialize(const std::string &outputDirectoryPath,
                     const std::string &outputFilePrefix)
{
    if (s_pimpl->Storage()->Initialized)
        return;

    s_pimpl->Storage()->LogOutputDirectoryPath = outputDirectoryPath;
    s_pimpl->Storage()->LogOutputFilePrefix = outputFilePrefix;

    if (filesystem::exists(s_pimpl->Storage()->LogOutputDirectoryPath)) {
        if (!filesystem::is_directory(s_pimpl->Storage()->LogOutputDirectoryPath)) {
            filesystem::remove(s_pimpl->Storage()->LogOutputDirectoryPath);
        }
    }

    if (!filesystem::exists(s_pimpl->Storage()->LogOutputDirectoryPath)) {
        filesystem::create_directories(s_pimpl->Storage()->LogOutputDirectoryPath);
    }

    s_pimpl->Storage()->LogOutputFilePath =
            (filesystem::path(s_pimpl->Storage()->LogOutputDirectoryPath)
             / filesystem::path((format("%1%_%2%.txt")
                                 % s_pimpl->Storage()->LogOutputFilePrefix
                                 % algorithm::replace_all_copy(
                                     algorithm::replace_all_copy(
                                         posix_time::to_simple_string(
                                             posix_time::second_clock::local_time()),
                                         ":", "-"),
                                     " ", "_")).str())).string();

    if (!s_pimpl->Storage()->MultiStream)
        s_pimpl->Storage()->Initialized = true;
}

void Log::Initialize(std::ostream &out_outputStream,
                     const std::string &outputDirectoryPath,
                     const std::string &outputFilePrefix)
{
    if (s_pimpl->Storage()->Initialized)
        return;

    s_pimpl->Storage()->MultiStream = true;

    Initialize(out_outputStream);
    Initialize(outputDirectoryPath, outputFilePrefix);

    s_pimpl->Storage()->Initialized = true;
}

Log::Log(EType type, const std::string &file, const std::string &func, int line, ...)
    : m_hasEntries(false)
{
    assert(s_pimpl->Storage()->Initialized);

    m_buffer << "[ " << posix_time::second_clock::local_time()
             << " " << s_pimpl->Storage()->LogTypeHash[type]
                << " " << line << " " << func << " " << file << " ]"
                << "\n";
}

Log::~Log()
{
    m_buffer << "\n\n";
    m_buffer.flush();

    boost::lock_guard<boost::mutex> lock(s_pimpl->LogMutex);
    (void)lock;

    if (s_pimpl->Storage()->LogOutputStream) {
        (*s_pimpl->Storage()->LogOutputStream) << m_buffer.str();
        s_pimpl->Storage()->LogOutputStream->flush();
    }

    if (s_pimpl->Storage()->LogOutputFilePath != "") {
        if(!s_pimpl->Storage()->LogOutputFileStream.is_open()) {
            s_pimpl->Storage()->LogOutputFileStream.open(
                        s_pimpl->Storage()->LogOutputFilePath,
                        std::ios_base::out | std::ios_base::app);
            s_pimpl->Storage()->LogOutputFileStream.imbue(
                        std::locale(
                            s_pimpl->Storage()->LogOutputFileStream.getloc(),
                            new posix_time::time_facet()));
        }

        s_pimpl->Storage()->LogOutputFileStream << m_buffer.str();
        s_pimpl->Storage()->LogOutputFileStream.flush();
        s_pimpl->Storage()->LogOutputFileStream.close();
    }
}

Log::Impl::Impl()
{

}

Log::Impl::~Impl()
{
    boost::lock_guard<boost::mutex> lock(StorageMutex);
    (void)lock;

    StorageInstance.reset();
}

Log::Impl::StorageStruct *Log::Impl::Storage()
{
    boost::lock_guard<boost::mutex> lock(StorageMutex);
    (void)lock;

    if (StorageInstance == nullptr) {
        StorageInstance = make_unique<StorageStruct>();

        StorageInstance->MultiStream = false;
        StorageInstance->Initialized = false;

        StorageInstance->LogTypeHash.clear();
        StorageInstance->LogTypeHash[Log::EType::Trace] = "TRACE";
        StorageInstance->LogTypeHash[Log::EType::Debug] = "DEBUG";
        StorageInstance->LogTypeHash[Log::EType::Info] = "INFO";
        StorageInstance->LogTypeHash[Log::EType::Warning] = "WARNING";
        StorageInstance->LogTypeHash[Log::EType::Error] = "ERROR";
        StorageInstance->LogTypeHash[Log::EType::Fatal] = "FATAL";

        StorageInstance->LogOutputStream = NULL;

        if (StorageInstance->LogOutputFileStream.is_open())
            StorageInstance->LogOutputFileStream.close();

        StorageInstance->LogOutputDirectoryPath.clear();;
        StorageInstance->LogOutputFilePrefix.clear();;
        StorageInstance->LogOutputFilePath.clear();;
    }

    return StorageInstance.get();
}

