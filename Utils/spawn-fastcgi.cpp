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
 * A tiny utility program to spawn a collection of fastcgi applications using
 * spawnfcgi utility from lighttpd project if they won't exists in memory
 * already. In addition to that, it monitors the application and does re-spawns
 * it in case of a crash.
 * You might want to run this as a cron job.
 */


#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <vector>
#include <csignal>
#include <cstdlib>
#include <boost/algorithm/string.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <CoreLib/CoreLib.hpp>
#include <CoreLib/Exception.hpp>
#include <CoreLib/Log.hpp>
#include <CoreLib/System.hpp>

#define     UNKNOWN_ERROR           "Unknown error!"

using namespace std;
using namespace boost;
using namespace CoreLib;

const string DATABASE_FILE_NAME = "spawn-fastcgi.db";

static property_tree::ptree appsTree;

[[ noreturn ]] void Terminate(int signo);

bool ReadApps();
void ReSpawn();

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
                                 "SpawnFastCGI");


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


        if(!ReadApps()) {
            return EXIT_FAILURE;
        }

        if (appsTree.empty()
                || appsTree.count("apps") == 0
                || appsTree.get_child("apps").count("") == 0) {
            cerr << "There is no WtHttpd app to spawn!" << endl;
            return EXIT_FAILURE;
        }

        while (true) {
            ReSpawn();
            sleep(1);
        }
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

bool ReadApps()
{
    ifstream dbFile(
                (boost::filesystem::path("..")
                 / boost::filesystem::path("db")
                 / DATABASE_FILE_NAME).string()
                );

    if (!dbFile.is_open()) {
        cerr << "Unable to open the database file!" << endl;
        return false;
    }

    try {
        property_tree::read_json(dbFile, appsTree);
    }

    catch (std::exception const &ex) {
        LOG_ERROR(ex.what());
        return false;
    }

    catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }


    return true;
}

void ReSpawn()
{
    try {
        BOOST_FOREACH(property_tree::ptree::value_type &appNode, appsTree.get_child("apps")) {
            if (appNode.first.empty()) {
                // 'optional<property_tree::basic_ptree<string, string> &>' works too.
                // but 'auto' is a conservative option on C++11-enabled compiler.
                auto socket = appNode.second.get_child_optional("socket");
                auto port = appNode.second.get_child_optional("port");

                string cmd;

                if (socket || port) {
                    if (socket) {
                        cmd = (format("/usr/bin/netstat | /usr/bin/grep '%1%'")
                               % appNode.second.get<std::string>("socket")).str();
                    } else {
                        cmd = (format("/usr/bin/netstat -an | /usr/bin/grep '%1%'")
                               % appNode.second.get<std::string>("port")).str();
                    }
                } else {
                    throw "No socket or port specified!";
                }

                string output;
                bool running = false;
                if (System::Exec(cmd, output)) {
                    if (socket) {
                        if (algorithm::trim_copy(output) != "") {
                            running = true;
                        }
                    } else {
                        if (output.find("LISTEN") != string::npos) {
                            running = true;
                        }
                    }
                }

                if (!running) {
                    vector<int> pids;
                    if (System::GetPidsOfProcess(System::GetProcessNameFromPath(appNode.second.get<std::string>("app")), pids)) {
                        for (std::vector<int>::const_iterator it =
                             pids.begin(); it != pids.end(); ++it) {
                            cmd = (format("/bin/ps -p %1% | grep '%2%'")
                                   % *it
                                   % appNode.second.get<std::string>("app")).str();
                            System::Exec(cmd, output);
                            if (output.find(appNode.second.get<std::string>("app")) != string::npos) {
                                clog << endl
                                     << (format("  * KILLING ==>  %1%"
                                                "\n\t%2%")
                                         % *it
                                         % appNode.second.get<std::string>("app")).str()
                                     << endl;

                                cmd = (format("/bin/kill -SIGKILL %1%")
                                       % *it).str();
                                System::Exec(cmd);
                            }
                        }
                    }

                    if (socket) {
                        clog << endl
                             << (format("  * RESPAWNING FASTCGI APP ==>  %1%"
                                        "\n\tConfig File        :  %2%"
                                        "\n\tUnix domain socket :  %3%"
                                        "\n\tWorking directory  :  %4%")
                                 % appNode.second.get<std::string>("app")
                                 % appNode.second.get<std::string>("config")
                                 % appNode.second.get<std::string>("socket")
                                 % appNode.second.get<std::string>("workdir")
                                 ).str()
                             << endl << endl;

                        cmd = (format("cd %2% && WT_CONFIG_XML=%1% /usr/local/bin/spawn-fcgi -d %2% -s %3% -n -M 0770 -u www -g www -- %4% &")
                               % appNode.second.get<std::string>("config")
                               % appNode.second.get<std::string>("workdir")
                               % appNode.second.get<std::string>("socket")
                               % appNode.second.get<std::string>("app")
                               ).str();
                    } else {
                        clog << endl
                             << (format("  * RESPAWNING FASTCGI APP ==>  %1%"
                                        "\n\tConfig File        :  %2%"
                                        "\n\tTCP port           :  %3%"
                                        "\n\tWorking directory  :  %4%")
                                 % appNode.second.get<std::string>("app")
                                 % appNode.second.get<std::string>("config")
                                 % appNode.second.get<std::string>("port")
                                 % appNode.second.get<std::string>("workdir")
                                 ).str()
                             << endl << endl;

                        cmd = (format("cd %2% && WT_CONFIG_XML=%1% /usr/local/bin/spawn-fcgi -d %2% -a 127.0.0.1 -p %3% -n -u www -g www -- %4% &")
                               % appNode.second.get<std::string>("config")
                               % appNode.second.get<std::string>("workdir")
                               % appNode.second.get<std::string>("port")
                               % appNode.second.get<std::string>("app")
                               ).str();
                    }

                    System::Exec(cmd);
                }
            }
        }
    }

    catch (const char * const &ex) {
        LOG_ERROR(ex);
    }

    catch (std::exception const &ex) {
        LOG_ERROR(ex.what());
    }

    catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }
}

