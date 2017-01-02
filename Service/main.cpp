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
 * Main entry for the web application.
 */


#include <csignal>
#include <cstdlib>
#if defined ( _WIN32 )
#include <windows.h>
#else
#include <unistd.h>
#endif  // defined ( _WIN32 )
#include <boost/algorithm/string.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <Wt/WServer>
#if MAGICKPP_BACKEND == MAGICKPP_GM
#include <GraphicsMagick/Magick++.h>
#elif MAGICKPP_BACKEND == MAGICKPP_IM
#include <ImageMagick-6/Magick++.h>
#endif // MAGICKPP_BACKEND == MAGICKPP_GM
#include <statgrab.h>
#include <CoreLib/CoreLib.hpp>
#include <CoreLib/Database.hpp>
#include <CoreLib/Exception.hpp>
#include <CoreLib/Log.hpp>
#include <CoreLib/make_unique.hpp>
#include <CoreLib/System.hpp>
#include "CgiRoot.hpp"
#include "Exception.hpp"
#include "Pool.hpp"
#include "VersionInfo.hpp"

void Terminate [[noreturn]] (int signo);
void InitializeDatabase();

#if defined ( __unix__ )
int main(int argc, char **argv, char **envp)
#else
int main(int argc, char **argv)
#endif  // defined ( __unix__ )
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
        Service::Pool::Storage()->AppPath = appPath;


        /// Force changing the current path to executable path
        boost::filesystem::current_path(appPath);


        /// Initializing CoreLib
        CoreLib::CoreLibInitialize(argc, argv);


        /// Initializing log system
        CoreLib::Log::Initialize(std::cout,
                                 (boost::filesystem::path(appPath)
                                  / boost::filesystem::path("..")
                                  / boost::filesystem::path("log")).string(),
                                 "JobApplicationSystemServer");


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


        /// Logging application info
        LOG_INFO("Version Information", "", "BUILD_COMPILER             " VERSION_INFO_BUILD_COMPILER, "BUILD_DATE                 " VERSION_INFO_BUILD_DATE, "BUILD_HOST                 " VERSION_INFO_BUILD_HOST, "BUILD_PROCESSOR            " VERSION_INFO_BUILD_PROCESSOR, "BUILD_SYSTEM               " VERSION_INFO_BUILD_SYSTEM, "PRODUCT_COMPANY_NAME       " VERSION_INFO_PRODUCT_COMPANY_NAME, "PRODUCT_COPYRIGHT          " VERSION_INFO_PRODUCT_COPYRIGHT, "PRODUCT_INTERNAL_NAME      " VERSION_INFO_PRODUCT_INTERNAL_NAME, "PRODUCT_NAME               " VERSION_INFO_PRODUCT_NAME, "PRODUCT_VERSION            " VERSION_INFO_PRODUCT_VERSION, "PRODUCT_DESCRIPTION        " VERSION_INFO_PRODUCT_DESCRIPTION);


        if(!CoreLib::System::GetLock(lockId, lock)) {
            LOG_WARNING("Process is already running!");
        } else {
            LOG_INFO("Got the process lock!");
        }


        /*! Initializing Magick++ or You'll crash HARD!! */
        Magick::InitializeMagick(*argv);


        /// Initialize libstatgrab
#if defined ( __unix__ )
#if defined ( __FreeBSD__ )
        sg_init(1);
#else
        sg_init(0);
#endif  // defined ( __FreeBSD__ )
#elif defined ( _WIN32 )
        /// In order to make it work, we have to call it twice
        sg_snapshot();
        sg_snapshot();
#endif  // defined ( __unix__ )
        /// Drop setuid/setgid privileges.
        if (sg_drop_privileges() != SG_ERROR_NONE) {
            LOG_ERROR("Error: Failed to drop privileges!");
        }

        /// Initialize the whole database
        InitializeDatabase();

        /// Starting the server, otherwise going down
        LOG_INFO("Starting the server...");
        Wt::WServer server(argv[0]);
        server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);
        server.addEntryPoint(Wt::Application, Service::CgiRoot::CreateApplication, "", "favicon.ico");
        if (server.start()) {
            int sig = Wt::WServer::waitForShutdown();
            server.stop();

#if defined ( __unix__ )
            /// Experimental, UNIX only
            if (sig == SIGHUP)
                Wt::WServer::restart(argc, argv, envp);
#endif  // defined ( __unix__ )
        }


        /// Shutdown libstatgrab before return
        sg_shutdown();
    }

    catch (Service::Exception &ex) {
        LOG_ERROR(ex.what());
    }

    catch (CoreLib::Exception &ex) {
        LOG_ERROR(ex.what());
    }

    catch (Wt::WServer::Exception &ex) {
        LOG_ERROR(ex.what());
    }

    catch (boost::exception &ex) {
        LOG_ERROR(boost::diagnostic_information(ex));
    }

    catch (std::exception &ex) {
        LOG_ERROR(ex.what());
    }

    catch (...) {
        LOG_ERROR("Unknown error!");
    }

    return EXIT_SUCCESS;
}

void Terminate(int signo)
{
    LOG_WARNING((boost::format("Terminating by signal %1%...") % signo).str());
    exit(1);
}

void InitializeDatabase()
{
    try {
        Service::Pool::Database()->RegisterEnum("SUBSCRIPTION", "subscription",
                                                { "none", "en_fa", "en", "fa" });

        Service::Pool::Database()->RegisterTable("ROOT", "root",
                                                 " username TEXT NOT NULL PRIMARY KEY, "
                                                 " email TEXT NOT NULL UNIQUE, "
                                                 " last_login_ip TEXT, "
                                                 " last_login_location TEXT, "
                                                 " last_login_rawtime TEXT, "
                                                 " last_login_gdate TEXT, "
                                                 " last_login_jdate TEXT, "
                                                 " last_login_time TEXT, "
                                                 " last_login_user_agent TEXT, "
                                                 " last_login_referer TEXT, "
                                                 " pwd TEXT NOT NULL, "
                                                 " recovery_pwd TEXT ");

        Service::Pool::Database()->RegisterTable("ROOT_SESSIONS", "root_sessions",
                                                 " token TEXT NOT NULL PRIMARY KEY, "
                                                 " expiry TEXT NOT NULL DEFAULT '0', "
                                                 " ip TEXT, "
                                                 " location TEXT, "
                                                 " rawtime TEXT, "
                                                 " gdate TEXT, "
                                                 " jdate TEXT, "
                                                 " time TEXT, "
                                                 " user_agent TEXT, "
                                                 " referer TEXT ");

        Service::Pool::Database()->RegisterTable("SETTINGS", "settings",
                                                 " pseudo_id TEXT NOT NULL PRIMARY KEY, "
                                                 " homepage_url_en TEXT NOT NULL, "
                                                 " homepage_url_fa TEXT NOT NULL, "
                                                 " homepage_title_en TEXT NOT NULL, "
                                                 " homepage_title_fa TEXT NOT NULL ");

        Service::Pool::Database()->RegisterTable("CONTACTS", "contacts",
                                                 " recipient TEXT NOT NULL PRIMARY KEY, "
                                                 " recipient_fa TEXT NOT NULL UNIQUE, "
                                                 " address TEXT NOT NULL, "
                                                 " is_default BOOLEAN NOT NULL DEFAULT FALSE ");

        Service::Pool::Database()->RegisterTable("SUBSCRIBERS", "subscribers",
                                                 " inbox TEXT NOT NULL PRIMARY KEY, "
                                                 " uuid UUID NOT NULL UNIQUE, "
                                                 " subscription SUBSCRIPTION NOT NULL DEFAULT 'none', "
                                                 " pending_confirm SUBSCRIPTION NOT NULL DEFAULT 'none', "
                                                 " pending_cancel SUBSCRIPTION NOT NULL DEFAULT 'none', "
                                                 " join_date TEXT NOT NULL, "
                                                 " update_date TEXT NOT NULL ");

        Service::Pool::Database()->RegisterTable("VERSION", "version",
                                                 " version SMALLINT NOT NULL PRIMARY KEY ");

        Service::Pool::Database()->Initialize();

        cppdb::transaction guard(Service::Pool::Database()->Sql());

        cppdb::result r = Service::Pool::Database()->Sql()
                << (boost::format("SELECT username"
                                  " FROM %1% WHERE username=?;")
                    % Service::Pool::Database()->GetTableName("ROOT")).str()
                << Service::Pool::Storage()->RootUsername() << cppdb::row;

        if (r.empty()) {
            Service::Pool::Database()->Insert("ROOT", "username, email, pwd",
                                              {
                                                  Service::Pool::Storage()->RootUsername(),
                                                  Service::Pool::Storage()->RootInitialEmail(),
                                                  Service::Pool::Storage()->RootInitialPassword()
                                              });
        }

        r = Service::Pool::Database()->Sql()
                << (boost::format("SELECT pseudo_id"
                                  " FROM %1% WHERE pseudo_id = '0';")
                    % Service::Pool::Database()->GetTableName("SETTINGS")).str()
                << cppdb::row;

        if (r.empty()) {
            Service::Pool::Database()->Insert("SETTINGS",
                                              "pseudo_id, homepage_url_en, homepage_url_fa,"
                                              " homepage_title_en, homepage_title_fa",
                                              {
                                                  "0",
                                                  std::string(INITAL_EN_HOME_PAGE_URL),
                                                  std::string(INITAL_FA_HOME_PAGE_URL),
                                                  std::string(INITAL_EN_HOME_PAGE_TITLE),
                                                  std::string(INITAL_FA_HOME_PAGE_TITLE)
                                              });
        }

        r = Service::Pool::Database()->Sql()
                << (boost::format("SELECT version"
                                  " FROM %1% WHERE 1 = 1;")
                    % Service::Pool::Database()->GetTableName("VERSION")).str()
                << cppdb::row;

        if (r.empty()) {
            Service::Pool::Database()->Insert("VERSION", "version", { "1" });
        }

        guard.commit();
    }

    catch (std::exception &ex) {
        LOG_ERROR(ex.what());
    }

    catch (...) {
        LOG_ERROR("Unknown error!");
    }
}
