/**
 * @file
 * @author  Mamadou Babaei <info@babaei.net>
 * @version 0.1.0
 *
 * @section LICENSE
 *
 * (The MIT License)
 *
 * Copyright (c) 2016 - 2020 Mamadou Babaei
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
#include <boost/lexical_cast.hpp>
#include <pqxx/pqxx>
#include <Wt/WServer>
#include <Wt/WString>
#if MAGICKPP_BACKEND == MAGICKPP_GM
#include <GraphicsMagick/Magick++.h>
#elif MAGICKPP_BACKEND == MAGICKPP_IM
#include <ImageMagick-6/Magick++.h>
#endif // MAGICKPP_BACKEND == MAGICKPP_GM
#include <statgrab.h>
#include <CoreLib/CoreLib.hpp>
#include <CoreLib/CDate.hpp>
#include <CoreLib/Crypto.hpp>
#include <CoreLib/Database.hpp>
#include <CoreLib/Exception.hpp>
#include <CoreLib/Log.hpp>
#include <CoreLib/make_unique.hpp>
#include <CoreLib/Random.hpp>
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
        /// Gracefully handle SIGTERM
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
        Service::Pool::Storage().AppPath = appPath;


        /// Force changing the current path to executable path
        boost::filesystem::current_path(appPath);


        /// Initialize CoreLib
        CoreLib::CoreLibInitialize(argc, argv);


        /// Initializing log system
#if GDPR_COMPLIANCE
        CoreLib::Log::Initialize(std::cout);
#else // GDPR_COMPLIANCE
        CoreLib::Log::Initialize(std::cout,
                                 (boost::filesystem::path(appPath)
                                  / boost::filesystem::path("..")
                                  / boost::filesystem::path("log")).string(),
                                 "JobApplicationSystemServer");
#endif // GDPR_COMPLIANCE


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


        /// Log application info
        LOG_INFO("Version Information", "", "BUILD_COMPILER             " VERSION_INFO_BUILD_COMPILER, "BUILD_DATE                 " VERSION_INFO_BUILD_DATE, "BUILD_HOST                 " VERSION_INFO_BUILD_HOST, "BUILD_PROCESSOR            " VERSION_INFO_BUILD_PROCESSOR, "BUILD_SYSTEM               " VERSION_INFO_BUILD_SYSTEM, "PRODUCT_COMPANY_NAME       " VERSION_INFO_PRODUCT_COMPANY_NAME, "PRODUCT_COPYRIGHT          " VERSION_INFO_PRODUCT_COPYRIGHT, "PRODUCT_INTERNAL_NAME      " VERSION_INFO_PRODUCT_INTERNAL_NAME, "PRODUCT_NAME               " VERSION_INFO_PRODUCT_NAME, "PRODUCT_VERSION            " VERSION_INFO_PRODUCT_VERSION, "PRODUCT_DESCRIPTION        " VERSION_INFO_PRODUCT_DESCRIPTION);


        if(!CoreLib::System::GetLock(lockId, lock)) {
            LOG_WARNING("Process is already running!");
        } else {
            LOG_INFO("Got the process lock!");
        }


        /// Initialize CoreLib::Crypto
        CoreLib::Crypto::Initialize();


        /*! Initialize Magick++ or You'll crash HARD!! */
        LOG_INFO("Initializing Magick++...");
        Magick::InitializeMagick(*argv);
        LOG_INFO("Magick++ initialized successfully!");


        /// Initialize libstatgrab
        LOG_INFO("Initializing libstatgrab...");
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
            LOG_FATAL("Error: libstatgrab has failed to drop privileges!");
            return EXIT_FAILURE;
        }
        LOG_INFO("libstatgrab initialized successfully!");


        /// Prevent libpqxx from crashing the program when a connection to the database breaks
        signal(SIGPIPE, SIG_IGN);


        /// Initialize the whole database
        InitializeDatabase();


        /// Start the server, otherwise go down
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
        LOG_INFO("Shutting down libstatgrab...");
        sg_shutdown();
        LOG_INFO("libstatgrab shutdown successfully!");
    }

    catch (Service::Exception<std::wstring> &ex) {
        LOG_ERROR(Wt::WString(ex.What()).toUTF8());
    }

    catch (Service::Exception<std::string> &ex) {
        LOG_ERROR(ex.What());
    }

    catch (CoreLib::Exception<std::wstring> &ex) {
        LOG_ERROR(Wt::WString(ex.What()).toUTF8());
    }

    catch (CoreLib::Exception<std::string> &ex) {
        LOG_ERROR(ex.What());
    }

    catch (const Wt::WServer::Exception &ex) {
        LOG_ERROR(ex.what());
    }

    catch (const boost::exception &ex) {
        LOG_ERROR(boost::diagnostic_information(ex));
    }

    catch (const std::exception &ex) {
        LOG_ERROR(ex.what());
    }

    catch (...) {
        LOG_ERROR("Unknown error!");
    }

    return EXIT_SUCCESS;
}

void Terminate(int signo)
{
    LOG_WARNING((boost::format("Received signal %1%; terminating...") % signo).str());
    exit(EXIT_FAILURE);
}

void InitializeDatabase()
{
    try {
        LOG_INFO("main: Initializing database...");

        LOG_INFO("main: Registering database enums...");

        Service::Pool::Database().RegisterEnum("SUBSCRIPTION", "subscription",
        { "none", "en_fa", "en", "fa" });

        LOG_INFO("main: Registered all database enums!");

        LOG_INFO("main: Registering database tables...");

        Service::Pool::Database().RegisterTable("VERSION", "version",
                                                " version SMALLINT NOT NULL PRIMARY KEY ");

        Service::Pool::Database().RegisterTable("ROOT", "root",
                                                " user_id UUID NOT NULL PRIMARY KEY, "
                                                " username TEXT NOT NULL UNIQUE, "
                                                " email TEXT NOT NULL UNIQUE, "
                                                " creation_time TIMESTAMPTZ NOT NULL DEFAULT TIMESTAMPTZ $token$'EPOCH'''$token$, "
                                                " modification_time TIMESTAMPTZ NOT NULL DEFAULT TIMESTAMPTZ $token$'EPOCH'''$token$ ");

        Service::Pool::Database().RegisterTable("ROOT_CREDENTIALS", "root_credentials",
                                                " user_id UUID NOT NULL PRIMARY KEY, "
                                                " pwd TEXT NOT NULL, "
                                                " modification_time TIMESTAMPTZ NOT NULL DEFAULT TIMESTAMPTZ $token$'EPOCH'''$token$ ");

        Service::Pool::Database().RegisterTable("ROOT_CREDENTIALS_RECOVERY", "root_credentials_recovery",
                                                " token UUID NOT NULL PRIMARY KEY, "
                                                " user_id UUID NOT NULL, "
                                                " expiry TIMESTAMPTZ NOT NULL DEFAULT TIMESTAMPTZ $token$'EPOCH'''$token$, "
                                                " new_pwd TEXT NOT NULL, "
                                                " request_time TIMESTAMPTZ NOT NULL, "
                                                " request_ip_address INET, "
                                                " request_location_country_code TEXT, "
                                                " request_location_country_code3 TEXT, "
                                                " request_location_country_name TEXT, "
                                                " request_location_region TEXT, "
                                                " request_location_city TEXT, "
                                                " request_location_postal_code TEXT, "
                                                " request_location_latitude TEXT, "
                                                " request_location_longitude TEXT, "
                                                " request_location_metro_code TEXT, "
                                                " request_location_dma_code TEXT, "
                                                " request_location_area_code TEXT, "
                                                " request_location_charset TEXT, "
                                                " request_location_continent_code TEXT, "
                                                " request_location_netmask TEXT, "
                                                " request_location_asn TEXT, "
                                                " request_location_aso TEXT, "
                                                " request_location_raw_data JSONB, "
                                                " request_user_agent TEXT, "
                                                " request_referer TEXT, "
                                                " utilization_time TIMESTAMPTZ NOT NULL DEFAULT TIMESTAMPTZ $token$'EPOCH'''$token$, "
                                                " utilization_ip_address INET, "
                                                " utilization_location_country_code TEXT, "
                                                " utilization_location_country_code3 TEXT, "
                                                " utilization_location_country_name TEXT, "
                                                " utilization_location_region TEXT, "
                                                " utilization_location_city TEXT, "
                                                " utilization_location_postal_code TEXT, "
                                                " utilization_location_latitude TEXT, "
                                                " utilization_location_longitude TEXT, "
                                                " utilization_location_metro_code TEXT, "
                                                " utilization_location_dma_code TEXT, "
                                                " utilization_location_area_code TEXT, "
                                                " utilization_location_charset TEXT, "
                                                " utilization_location_continent_code TEXT, "
                                                " utilization_location_netmask TEXT, "
                                                " utilization_location_aso TEXT, "
                                                " utilization_location_asn TEXT, "
                                                " utilization_location_raw_data JSONB, "
                                                " utilization_user_agent TEXT, "
                                                " utilization_referer TEXT ");

        Service::Pool::Database().RegisterTable("ROOT_SESSIONS", "root_sessions",
                                                " token UUID NOT NULL PRIMARY KEY, "
                                                " user_id UUID NOT NULL, "
                                                " expiry TIMESTAMPTZ NOT NULL DEFAULT TIMESTAMPTZ $token$'EPOCH'''$token$, "
                                                " login_time TIMESTAMPTZ NOT NULL DEFAULT TIMESTAMPTZ $token$'EPOCH'''$token$, "
                                                " ip_address INET, "
                                                " location_country_code TEXT, "
                                                " location_country_code3 TEXT, "
                                                " location_country_name TEXT, "
                                                " location_region TEXT, "
                                                " location_city TEXT, "
                                                " location_postal_code TEXT, "
                                                " location_latitude REAL, "
                                                " location_longitude REAL, "
                                                " location_metro_code INTEGER, "
                                                " location_dma_code INTEGER, "
                                                " location_area_code INTEGER, "
                                                " location_charset INTEGER, "
                                                " location_continent_code TEXT, "
                                                " location_netmask INTEGER, "
                                                " location_asn INTEGER, "
                                                " location_aso TEXT, "
                                                " location_raw_data JSONB, "
                                                " user_agent TEXT, "
                                                " referer TEXT ");

        Service::Pool::Database().RegisterTable("SETTINGS", "settings",
                                                " pseudo_id TEXT NOT NULL PRIMARY KEY, "
                                                " homepage_url_en TEXT NOT NULL, "
                                                " homepage_url_fa TEXT NOT NULL, "
                                                " homepage_title_en TEXT NOT NULL, "
                                                " homepage_title_fa TEXT NOT NULL ");

        Service::Pool::Database().RegisterTable("CONTACTS", "contacts",
                                                " recipient TEXT NOT NULL PRIMARY KEY, "
                                                " recipient_fa TEXT NOT NULL UNIQUE, "
                                                " address TEXT NOT NULL, "
                                                " is_default BOOLEAN NOT NULL DEFAULT FALSE ");

        Service::Pool::Database().RegisterTable("SUBSCRIBERS", "subscribers",
                                                " inbox TEXT NOT NULL PRIMARY KEY, "
                                                " uuid UUID NOT NULL UNIQUE, "
                                                " subscription SUBSCRIPTION NOT NULL DEFAULT 'none', "
                                                " pending_confirm SUBSCRIPTION NOT NULL DEFAULT 'none', "
                                                " pending_cancel SUBSCRIPTION NOT NULL DEFAULT 'none', "
                                                " join_date TEXT NOT NULL, "
                                                " update_date TEXT NOT NULL ");

        LOG_INFO("main: Registered all database tables!");


        LOG_INFO("main: Calling Database::Initialize()...");
        Service::Pool::Database().Initialize();


        LOG_INFO("main: Setting up the database...");

        auto conn = Service::Pool::Database().Connection();
        conn->activate();
        pqxx::work txn(*conn.get());

        /// Check the database version
        pqxx::result r = txn.exec((boost::format("SELECT version FROM \"%1%\" WHERE 1 = 1;")
                                   % txn.esc(Service::Pool::Database().GetTableName("VERSION"))).str());

        /// If the database is un-versioned
        if (r.empty()) {
            /// Insert the version number 1
            Service::Pool::Database().Insert("VERSION", "version", { "1" });
        }

        /// Check whether the default root user already exists
        r = txn.exec((boost::format("SELECT username FROM \"%1%\" WHERE username=%2%;")
                      % txn.esc(Service::Pool::Database().GetTableName("ROOT"))
                      % txn.quote(Service::Pool::Storage().RootUsername())).str());

        /// If the default root user does not exists
        if (r.empty()) {
            std::string uuid;

            /// Continue untile we can generate a unique UUID
            while (true) {
                /// Generate a new UUID
                CoreLib::Random::Uuid(uuid);

                /// Query all existing UUIDs in order to avoid a possible duplicate UUID
                r = txn.exec((boost::format("SELECT user_id FROM \"%1%\" WHERE user_id=%2%;")
                              % txn.esc(Service::Pool::Database().GetTableName("ROOT"))
                              % txn.quote(uuid)).str());

                /// Break the loop if the new UUID is not a duplicate
                if (r.empty()) {
                    break;
                }
            }

            /// Get the current date/time
            CoreLib::CDate::Now n(CoreLib::CDate::Timezone::UTC);

            /// Insert a new default root user into the database with default password
            txn.exec((boost::format("INSERT INTO \"%1%\""
                                    " ( user_id, username, email, creation_time )"
                                    " VALUES ( %2%, %3%, %4%, TO_TIMESTAMP(%5%)::TIMESTAMPTZ );")
                      % txn.esc(Service::Pool::Database().GetTableName("ROOT"))
                      % txn.quote(uuid)
                      % txn.quote(Service::Pool::Storage().RootUsername())
                      % txn.quote(Service::Pool::Storage().RootInitialEmail())
                      % txn.esc(boost::lexical_cast<std::string>(n.RawTime()))).str());
            Service::Pool::Database().Insert("ROOT_CREDENTIALS",
                                             "user_id, pwd",
            {
                                                 uuid,
                                                 Service::Pool::Storage().RootInitialPassword()
                                             });
        }

        txn.commit();

        LOG_INFO("main: Database setup is complete!");

        LOG_INFO("main: Database initialization is complete!");
    }

    catch (const pqxx::sql_error &ex) {
        LOG_ERROR(ex.what(), ex.query());
    }

    catch (const boost::exception &ex) {
        LOG_ERROR(boost::diagnostic_information(ex));
    }

    catch (const std::exception &ex) {
        LOG_ERROR(ex.what());
    }

    catch (...) {
        LOG_ERROR("Unknown error!");
    }
}
