/**
 * @file
 * @author  Mamadou Babaei <info@babaei.net>
 * @version 0.1.0
 *
 * @section LICENSE
 *
 * (The MIT License)
 *
 * Copyright (c) 2016 - 2019 Mamadou Babaei
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
#include <boost/foreach.hpp>
#include <CoreLib/Archiver.hpp>
#include <CoreLib/CoreLib.hpp>
#include <CoreLib/Defines.hpp>
#include <CoreLib/Exception.hpp>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Http.hpp>
#include <CoreLib/Log.hpp>
#include <CoreLib/System.hpp>

#define     UNKNOWN_ERROR           "Unknown error!"

#define     CITY_DATABASE_NAME              "GeoLite2-City.mmdb"
#define     CITY_DATABASE_PATH_USR          "/usr/share/GeoIP/" CITY_DATABASE_NAME
#define     CITY_DATABASE_PATH_USR_LOCAL    "/usr/local/share/GeoIP/" CITY_DATABASE_NAME

#define     COUNTRY_DATABASE_NAME           "GeoLite2-Country.mmdb"
#define     COUNTRY_DATABASE_PATH_USR       "/usr/share/GeoIP/" COUNTRY_DATABASE_NAME
#define     COUNTRY_DATABASE_PATH_USR_LOCAL "/usr/local/share/GeoIP/" COUNTRY_DATABASE_NAME

#define     ASN_DATABASE_NAME               "GeoLite2-ASN.mmdb"
#define     ASN_DATABASE_PATH_USR           "/usr/share/GeoIP/" ASN_DATABASE_NAME
#define     ASN_DATABASE_PATH_USR_LOCAL     "/usr/local/share/GeoIP/" ASN_DATABASE_NAME

[[ noreturn ]] void Terminate(int signo);

FORCEINLINE static const char* GetGeoLite2CityDatabase()
{
#if defined ( __FreeBSD__ )
    static const char* database = CITY_DATABASE_PATH_USR_LOCAL;
#elif defined ( __gnu_linux__ ) || defined ( __linux__ )
    static const char* database = CITY_DATABASE_PATH_USR;
#else /* defined ( __FreeBSD__ ) */
    static const char* database =
            CoreLib::FileSystem::FileExists(CITY_DATABASE_PATH_USR_LOCAL)
            ? CoreLib::FileSystem::FileExists(CITY_DATABASE_PATH_USR_LOCAL)
            : CoreLib::FileSystem::FileExists(CITY_DATABASE_PATH_USR);
#endif /* defined ( __FreeBSD__ ) */

    return database;
}

FORCEINLINE static const char* GetGeoLite2CountryDatabase()
{
#if defined ( __FreeBSD__ )
    static const char* database = COUNTRY_DATABASE_PATH_USR_LOCAL;
#elif defined ( __gnu_linux__ ) || defined ( __linux__ )
    static const char* database = COUNTRY_DATABASE_PATH_USR;
#else /* defined ( __FreeBSD__ ) */
    static const char* database =
            CoreLib::FileSystem::FileExists(COUNTRY_DATABASE_PATH_USR_LOCAL)
            ? CoreLib::FileSystem::FileExists(COUNTRY_DATABASE_PATH_USR_LOCAL)
            : CoreLib::FileSystem::FileExists(COUNTRY_DATABASE_PATH_USR);
#endif /* defined ( __FreeBSD__ ) */

    return database;
}

FORCEINLINE static const char* GetGeoLite2ASNDatabase()
{
#if defined ( __FreeBSD__ )
    static const char* database = ASN_DATABASE_PATH_USR_LOCAL;
#elif defined ( __gnu_linux__ ) || defined ( __linux__ )
    static const char* database = ASN_DATABASE_PATH_USR;
#else /* defined ( __FreeBSD__ ) */
    static const char* database =
            CoreLib::FileSystem::FileExists(ASN_DATABASE_PATH_USR_LOCAL)
            ? CoreLib::FileSystem::FileExists(ASN_DATABASE_PATH_USR_LOCAL)
            : CoreLib::FileSystem::FileExists(ASN_DATABASE_PATH_USR);
#endif /* defined ( __FreeBSD__ ) */

    return database;
}

void UpdateDatabase(const std::string &url,
                    const std::string &tag,
                    const std::string &dbFile);

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


#if NO_LOG_FILES
        CoreLib::Log::Initialize(std::cout);
#else // NO_LOG_FILES
        CoreLib::Log::Initialize(std::cout,
                                 (boost::filesystem::path(appPath)
                                  / boost::filesystem::path("..")
                                  / boost::filesystem::path("log")).string(),
                                 "GeoIPUpdater");
#endif // NO_LOG_FILES


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
            std::cerr << "Could not get lock!" << std::endl;
            std::cerr << "Probably process is already running!" << std::endl;
            return EXIT_FAILURE;
        } else {
            LOG_INFO("Got the process lock!");
        }


        const std::string cityTag("GeoLite2-City");
        const std::string countryTag("GeoLite2-Country");
        const std::string asnTag("GeoLite2-ASN");

        UpdateDatabase(GEOLITE2_CITY_MMDB_URL,
                       cityTag, GetGeoLite2CityDatabase());
        UpdateDatabase(GEOLITE2_COUNTRY_MMDB_URL,
                       countryTag, GetGeoLite2CountryDatabase());
        UpdateDatabase(GEOLITE2_ASN_MMDB_URL,
                       asnTag, GetGeoLite2ASNDatabase());
    }

    catch (CoreLib::Exception<std::string> &ex) {
        LOG_ERROR(ex.What());
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
    std::clog << "Terminating...." << std::endl;
    exit(signo);
}

void UpdateDatabase(const std::string &url,
                    const std::string &tag,
                    const std::string &targetMmdbFile)
{
    const std::string tempDir(CoreLib::FileSystem::CreateTempDir());

    const std::string gzipFile((boost::filesystem::path(tempDir)
                                / boost::filesystem::path(tag)).string() + ".tar.gz");

    {
        LOG_INFO(url, gzipFile, "Downloading...");

        if (CoreLib::Http::Download(url, gzipFile)) {
            LOG_INFO(url, gzipFile, "Download operation succeeded!");
        } else {
            LOG_ERROR(url, gzipFile, "Download operation failed!");
            return;
        }
    }

    const std::string tarFile((boost::filesystem::path(tempDir)
                               / boost::filesystem::path(tag)).string() + ".tar");

    {
        LOG_INFO(gzipFile, tarFile, "Uncompressing...");

        std::string err;
        if (CoreLib::Archiver::UnGzip(gzipFile, tarFile, err)) {
            LOG_INFO(gzipFile, tarFile, "Uncompress operation succeeded!");
        } else {
            LOG_INFO(gzipFile, tarFile, err, "Uncompress operation failed!");
            return;
        }
    }

    {
        LOG_INFO(tarFile, tempDir, "Extacting tar archive...");

        std::string err;
        if (CoreLib::Archiver::UnTar(tarFile, tempDir, err)) {
            LOG_INFO(tarFile, tempDir, "Tar archive exraction succeeded!");
        } else {
            LOG_INFO(tarFile, tempDir, err, "Tar archive exraction failed!");
            return;
        }
    }

    {
        LOG_INFO(tempDir, targetMmdbFile, "Copying mmdb file...");

        boost::filesystem::directory_iterator it{
            boost::filesystem::path(tempDir)};

        BOOST_FOREACH(const boost::filesystem::path  &p, std::make_pair(
                          it,  boost::filesystem::directory_iterator{}))
        {
            if(boost::filesystem::is_directory(p))
            {
                std::string targetPath(boost::filesystem::path(
                                           targetMmdbFile).parent_path().string());

                if (!CoreLib::FileSystem::DirExists(targetPath)) {
                    if (!CoreLib::FileSystem::CreateDir(targetPath)) {
                        LOG_INFO(targetPath, targetMmdbFile,
                                 "Failed to create target path!");

                        CoreLib::FileSystem::Erase(tempDir, true);

                        return;
                    }
                }

                std::string sourceMmdbFile(
                            (p / boost::filesystem::path(tag + ".mmdb")).string());

                if (CoreLib::FileSystem::FileExists(sourceMmdbFile)) {
                    if (CoreLib::FileSystem::CopyFile(
                                sourceMmdbFile, targetMmdbFile, true)) {
                        LOG_INFO(sourceMmdbFile, targetMmdbFile,
                                 "Copying mmdb file succeeded!");
                    } else {
                        LOG_INFO(sourceMmdbFile, targetMmdbFile,
                                 "Copying mmdb file failed!");
                    }
                }
            }
        }
    }

    LOG_INFO(targetMmdbFile, "Successfully updated!");

    CoreLib::FileSystem::Erase(tempDir, true);
}
