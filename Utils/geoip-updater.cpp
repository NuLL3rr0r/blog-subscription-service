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
 * A tiny utility program to fetch and update GeoIP database.
 * You might want to run this as a cron job.
 */


#include <iostream>
#include <string>
#include <csignal>
#include <cstdlib>
#include <boost/algorithm/string.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/filesystem.hpp>
#include <CoreLib/Archiver.hpp>
#include <CoreLib/CoreLib.hpp>
#include <CoreLib/Exception.hpp>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Http.hpp>
#include <CoreLib/Log.hpp>
#include <CoreLib/System.hpp>

#define     UNKNOWN_ERROR           "Unknown error!"

using namespace std;
using namespace boost;
using namespace CoreLib;

[[ noreturn ]] void Terminate(int signo);

void UpdateDatabase(std::string url, std::string file, std::string dbFile);

int main(int argc, char **argv)
{
    try {
        /// Gracefully handling SIGTERM
        void (*prev_fn)(int);
        prev_fn = signal(SIGTERM, Terminate);
        if (prev_fn == SIG_IGN)
            signal(SIGTERM, SIG_IGN);


        /// Extract the executable path and name
        boost::filesystem::path path(boost::filesystem::initial_path<boost::filesystem::path>());
        if (argc > 0 && argv[0] != NULL)
            path = boost::filesystem::system_complete(boost::filesystem::path(argv[0]));
        std::string appId(path.filename().string());
        std::string appPath(boost::algorithm::replace_last_copy(path.string(), appId, ""));

        /// Force changing the current path to executable path
        boost::filesystem::current_path(appPath);


        /// Initializing CoreLib
        CoreLib::CoreLibInitialize(argc, argv);


        /// Initializing log system
        CoreLib::Log::Initialize(std::cout,
                                 (boost::filesystem::path(appPath)
                                  / boost::filesystem::path("..")
                                  / boost::filesystem::path("log")).string(),
                                 "SpawnWtHttpd");


        /// Acquiring process lock
        std::string lockId;
#if defined ( __unix__ )
        int lock;
        lockId = (boost::filesystem::path(appPath)
                  / boost::filesystem::path("..")
                  / boost::filesystem::path("tmp")
                  / (appId + ".lock")).string();
#elif defined ( _WIN32 )
        HANDLE lock;
        lockId = appId;
#endif  // defined ( __unix__ )


        if(!CoreLib::System::GetLock(lockId, lock)) {
            cerr << "Could not get lock!" << endl;
            cerr << "Probably process is already running!" << endl;
            return EXIT_FAILURE;
        } else {
            LOG_INFO("Got the process lock!");
        }


        const std::string downloadPath((boost::filesystem::path(appPath)
                                        / boost::filesystem::path("..")
                                        / boost::filesystem::path("tmp")
                                        ).string());
        const std::string countryFile((boost::filesystem::path(downloadPath)
                                       / boost::filesystem::path("GeoIP.dat.gz")).string());
        const std::string cityFile((boost::filesystem::path(downloadPath)
                                    / boost::filesystem::path("GeoLiteCity.dat.gz")).string());

#if defined ( __FreeBSD__ )
        const std::string countryDbFile("/usr/local/share/GeoIP/GeoIP.dat");
        const std::string cityDbFile("/usr/local/share/GeoIP/GeoLiteCity.dat");
#elif defined ( __linux )
        const std::string countryDbFile("/usr/share/GeoIP/GeoIP.dat");
        const std::string cityDbFile("/usr/share/GeoIP/GeoLiteCity.dat");
#endif

        UpdateDatabase(GEO_LITE_COUNTRY_DB_URL, countryFile, countryDbFile);
        UpdateDatabase(GEO_LITE_CITY_DB_URL, cityFile, cityDbFile);
    }

    catch (CoreLib::Exception &ex) {
        LOG_ERROR(ex.what());
    }

    catch (boost::exception &ex) {
        LOG_ERROR(boost::diagnostic_information(ex));
    }

    catch (std::exception &ex) {
        LOG_ERROR(ex.what());
    }

    catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    return EXIT_SUCCESS;
}


void Terminate(int signo)
{
    clog << "Terminating...." << endl;
    exit(signo);
}

void UpdateDatabase(std::string url, std::string gzFile, std::string dbFile)
{
    bool rc;
    std::string error;

    LOG_INFO("Downloading...", url);
    rc = Http::Download(url, gzFile);

    if (!rc) {
        LOG_ERROR(url, "Download failed!");
        return;
    }

    LOG_INFO(gzFile, "Uncompressing...");

    string err;
    if (CoreLib::Archiver::UnGzip(gzFile, dbFile, err)) {
        LOG_INFO(dbFile, "Successfully Updated!");
    } else {
        LOG_INFO(dbFile, err, "Update operation failed!");
    }

    CoreLib::FileSystem::Erase(gzFile);
}

