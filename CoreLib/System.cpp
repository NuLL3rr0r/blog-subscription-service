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


#include <cstdio>
#include <cstdlib>
#if defined ( __unix__ )
#include <err.h>
#include <fcntl.h>
#include <kvm.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/user.h>
#include <unistd.h>
#endif  // defined ( __unix__ )
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

int System::GetPidsOfProcess(const std::string &processName, std::vector<int> &pids)
{
    /// Based on code from pidof.c

    static kvm_t *kd = NULL;
    struct kinfo_proc *p;
    int i;
    int nProcesses;
    int processesFound = 0;

    if ((kd = kvm_open("/dev/null", "/dev/null", "/dev/null", O_RDONLY, "kvm_open")) == NULL) {
        (void)errx(1, "%s", kvm_geterr(kd));
    } else {
        p = kvm_getprocs(kd, KERN_PROC_PROC, 0, &nProcesses);

        /// WARNING: HACK
        string name;
        if (processName.size() <= 19) {
            name = processName;
        } else {
            name = processName.substr(0, 19);
        }
        /// WARNING: HACK /// END

        for (i = 0; i < nProcesses; ++i) {
            if (strncmp(name.c_str(), p[i].ki_comm, COMMLEN + 1) == 0) {
                pids.push_back((int)p[i].ki_pid);
                processesFound++;
            }
        }

        kvm_close(kd);
    }

    return processesFound;
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

