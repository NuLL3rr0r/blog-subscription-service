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
 * Cross-platform OS level utility function.
 */


#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#if defined ( __unix__ )
#if defined ( __linux )
#include <dirent.h>
#endif  // defined ( __linux )
#if defined ( __linux )
#include <linux/sysctl.h>
#endif  // defined ( __linux )
#if defined ( __FreeBSD__ )
#include <kvm.h>
#include <sys/dirent.h>
#endif  // defined ( __FreeBSD__ )
#include <sys/fcntl.h>
#include <sys/param.h>
#if !defined ( __linux )
#include <sys/sysctl.h>
#endif  // !defined ( __linux )
#include <sys/time.h>
#include <sys/unistd.h>
#include <sys/user.h>
#endif  // defined ( __unix__ )
#if defined ( _WIN32 )
#include <tlhelp32.h>
#include <windows.h>
#endif  // defined ( _WIN32 )
#include "Log.hpp"
#include "make_unique.hpp"
#include "System.hpp"

using namespace std;
using namespace CoreLib;

struct System::Impl
{
#if defined ( __unix__ )
    static constexpr mode_t LOCK_FILE_PERMISSION = 0666;
#endif  // defined ( __unix__ )
};

bool System::Exec(const string &cmd)
{
    int r = system(cmd.c_str());
    if (r == EXIT_SUCCESS)
        return true;
    else
        return false;
}

bool System::Exec(const string &cmd, string &out_output)
{
    out_output.clear();

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
        return false;

    char buffer[256];
    while (!feof(pipe)) {
        if (fgets(buffer, 256, pipe) != NULL) {
            out_output += buffer;
        }
    }

    pclose(pipe);
    return true;
}

#if defined ( __unix__ )

bool System::GetLock(const std::string &name, int &out_handle)
{
    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 1;

    if ((out_handle = open(name.c_str(), O_WRONLY | O_CREAT, Impl::LOCK_FILE_PERMISSION)) == -1)
        return false;

    if (fcntl(out_handle, F_SETLK, &fl) == -1)
        return false;

    return true;
}

void System::ReleaseLock(int &handle)
{
    close(handle);
}

#elif defined ( _WIN32 )

bool System::GetLock(const std::string &name, HANDLE &out_handle)
{
    try {
        out_handle = OpenMutexA(MUTEX_ALL_ACCESS, TRUE, name.c_str());
        if (!out_handle) {
            out_handle = CreateMutexA(NULL, TRUE, name.c_str());
            return true;
        }
    } catch (...) {
    }

    return false;
}

void System::ReleaseLock(HANDLE &handle)
{
    ReleaseMutex(handle);
    CloseHandle(handle);
}

#endif  // defined ( __unix__ )

std::string System::GetProcessNameFromPath(const std::string &fullPath)
{
    return fullPath.substr(fullPath.rfind("/") + 1);
}

std::size_t System::GetPidsOfProcess(const std::string &process, std::vector<int> &out_pids)
{
    out_pids.clear();

#if defined ( __FreeBSD__ )
    static kvm_t *kd = nullptr;

    if ((kd = kvm_open("/dev/null", "/dev/null", "/dev/null", O_RDONLY, "kvm_open")) == nullptr) {
        LOG_ERROR(kvm_geterr(kd));
        return 0;
    }

    int count;
#if __FreeBSD__ >= 5
    struct kinfo_proc *p = kvm_getprocs(kd, KERN_PROC_PROC, 0, &count);
#else
    struct kinfo_proc *p = kvm_getprocs(kd, KERN_PROC_ALL, 0, &count);
#endif  // __FreeBSD__ >= 5

    for (int i = 0; i < count; ++i) {
#if __FreeBSD__ >= 5
        if (process.compare(0, COMMLEN, p[i].ki_comm) == 0) {
            out_pids.push_back(static_cast<int>(p[i].ki_pid));
#else
        if (process.compare(0, MAXCOMLEN, p[i].kp_proc.p_comm) == 0) {
            out_pids.push_back(static_cast<int>(p[i].kp_proc.p_pid));
#endif  // __FreeBSD__ >= 5
        }
    }

    kvm_close(kd);
#elif defined ( __linux )
    DIR *dp = opendir("/proc");

    if (dp != nullptr) {
        struct dirent *dir = readdir(dp);

        while (dir != nullptr) {
            int pid = atoi(dir->d_name);

            if (pid > 0) {
                std::stringstream ss;
                ss << "/proc/";
                ss << dir->d_name;
                ss << "/cmdline";
                std::ifstream cmdFile(ss.str().c_str());
                std::string cmdLine;
                getline(cmdFile, cmdLine);

                if (!cmdLine.empty()) {
                    size_t pos = cmdLine.find('\0');

                    if (pos != std::string::npos) {
                        cmdLine = cmdLine.substr(0, pos);
                    }

                    pos = cmdLine.rfind('/');

                    if (pos != std::string::npos) {
                        cmdLine = cmdLine.substr(pos + 1);
                    }

                    if (process.compare(cmdLine) == 0) {
                        out_pids.push_back(pid);
                    }
                }
            }

            dir = readdir(dp);
        }
    }
#elif defined ( _WIN32 )
    PROCESSENTRY32 pe32;
    HANDLE hSnapshot = nullptr;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (process.compare(pe32.szExeFile) == 0) {
                out_pids.push_back(pe32.th32ProcessID);
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    if(hSnapshot != INVALID_HANDLE_VALUE) {
        CloseHandle(hSnapshot);
    }
#endif  // defined( __FreeBSD__ )

    return out_pids.size();
}

long System::RandSeed()
{
    timespec ts;

#if defined ( __FreeBSD__ )
    clock_gettime(CLOCK_MONOTONIC, &ts);
#elif defined ( __linux )
    clock_gettime(CLOCK_REALTIME, &ts);
#endif  // defined ( __FreeBSD__ )

    return ts.tv_nsec;
}
