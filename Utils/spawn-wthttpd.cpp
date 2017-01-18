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
 * A tiny utility program to spawn a collection of wthttpd applications if they
 * won't exists in memory already. In addition to that, it monitors the
 * application and does re-spawn it in case of a crash.
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
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <CoreLib/CoreLib.hpp>
#include <CoreLib/Exception.hpp>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Log.hpp>
#include <CoreLib/System.hpp>

#define     UNKNOWN_ERROR           "Unknown error!"

const std::string DATABASE_FILE_NAME = "spawn-wthttpd.db";

static boost::property_tree::ptree appsTree;

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
            std::cerr << "Could not get lock!" << std::endl;
            std::cerr << "Probably process is already running!" << std::endl;
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
            std::cerr << "There is no WtHttpd app to spawn!" << std::endl;
            return EXIT_FAILURE;
        }

        while (true) {
            ReSpawn();
            sleep(1);
        }
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

bool ReadApps()
{
    std::ifstream dbFile(
                (boost::filesystem::path("..")
                 / boost::filesystem::path("db")
                 / DATABASE_FILE_NAME).string()
                );

    if (!dbFile.is_open()) {
        std::cerr << "Unable to open the database file!" << std::endl;
        return false;
    }

    try {
        boost::property_tree::read_json(dbFile, appsTree);
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
        BOOST_FOREACH(boost::property_tree::ptree::value_type &appNode, appsTree.get_child("apps")) {
            if (appNode.first.empty()) {
                bool running = false;
                std::vector<int> pids;

                if (appNode.second.get_child_optional("pid-file")) {
                    if (CoreLib::FileSystem::FileExists(appNode.second.get<std::string>("pid-file"))) {
                        std::ifstream pidFile(appNode.second.get<std::string>("pid-file"));
                        if (pidFile.is_open()) {
                            std::string firstLine;
                            getline(pidFile, firstLine);
                            pidFile.close();
                            boost::algorithm::trim(firstLine);
                            try {
                                int pid = boost::lexical_cast<int>(firstLine);

                                // The shell way:
                                // To check if ${PID} exists
                                // kill -0 ${PID}
                                // Other possible ways to get process name or full-path
                                // ps -p ${PID} -o comm=
                                // or
                                // FreeBSD
                                // ps axwww -o pid,command | grep ${PID}
                                // GNU/Linux
                                // ps ax -o pid,cmd | grep ${PID}

                                if (CoreLib::System::GetPidsOfProcess(
                                            CoreLib::System::GetProcessNameFromPath(appNode.second.get<std::string>("app")),
                                            pids)) {
                                    if (pids.size() > 0 &&
                                            std::find(pids.begin(), pids.end(), pid) != pids.end()) {
                                        running = true;
                                    }
                                }
                            } catch (...) {

                            }
                        } else {

                        }
                    } else {

                    }
                } else {
                    throw "No socket or port specified!";
                }

                if (!running) {
                    for (std::vector<int>::const_iterator it =
                         pids.begin(); it != pids.end(); ++it) {
                        std::string output;
                        std::string cmd = (boost::format("/bin/ps -p %1% | grep '%2%'")
                                           % *it
                                           % appNode.second.get<std::string>("app")).str();
                        CoreLib::System::Exec(cmd, output);
                        if (output.find(appNode.second.get<std::string>("app")) != std::string::npos) {
                            std::clog << std::endl
                                      << (boost::format("  * KILLING ==>  %1%"
                                                        "\n\t%2%")
                                          % *it
                                          % appNode.second.get<std::string>("app")).str()
                                      << std::endl;

                            cmd = (boost::format("/bin/kill -SIGKILL %1%")
                                   % *it).str();
                            CoreLib::System::Exec(cmd);
                        }
                    }

                    std::clog << std::endl
                              << (boost::format("  * RESPAWNING WTHTTPD APP ==>  %1%"
                                                "\n\tWorking Directory       :  %2%"
                                                "\n\tThreads                 :  %3%"
                                                "\n\tServer Name             :  %4%"
                                                "\n\tDocument Root           :  %5%"
                                                "\n\tApplication Root        :  %6%"
                                                "\n\tError Root              :  %7%"
                                                "\n\tAccess Log              :  %8%"
                                                "\n\tCompression             :  %9%"
                                                "\n\tDeploy Path             :  %10%"
                                                "\n\tSession ID Prefix       :  %11%"
                                                "\n\tPid File                :  %12%"
                                                "\n\tConfig File             :  %13%"
                                                "\n\tMax Memory Request Size :  %14%"
                                                "\n\tGDB                     :  %15%"
                                                "\n\tHttp Address            :  %16%"
                                                "\n\tHttp Port               :  %17%"
                                                "\n\tHttps Address           :  %18%"
                                                "\n\tHttps Port              :  %19%"
                                                "\n\tSSL Certificate         :  %20%"
                                                "\n\tSSL Private Key         :  %21%"
                                                "\n\tSSL Temp Diffie Hellman :  %22%"
                                                "\n\tSSL Enable v3           :  %23%"
                                                "\n\tSSL Client Verification :  %24%"
                                                "\n\tSSL Verify Depth        :  %25%"
                                                "\n\tSSL CA Certificates     :  %26%"
                                                "\n\tSSL Cipherlist          :  %27%")
                                  % appNode.second.get<std::string>("app")
                                  % appNode.second.get<std::string>("workdir")
                                  % appNode.second.get<std::string>("threads")
                                  % appNode.second.get<std::string>("servername")
                                  % appNode.second.get<std::string>("docroot")
                                  % appNode.second.get<std::string>("approot")
                                  % appNode.second.get<std::string>("errroot")
                                  % appNode.second.get<std::string>("accesslog")
                                  % appNode.second.get<std::string>("compression")
                                  % appNode.second.get<std::string>("deploy-path")
                                  % appNode.second.get<std::string>("session-id-prefix")
                                  % appNode.second.get<std::string>("pid-file")
                                  % appNode.second.get<std::string>("config")
                                  % appNode.second.get<std::string>("max-memory-request-size")
                                  % appNode.second.get<std::string>("gdb")
                                  % appNode.second.get<std::string>("http-address")
                                  % appNode.second.get<std::string>("http-port")
                                  % appNode.second.get<std::string>("https-address")
                                  % appNode.second.get<std::string>("https-port")
                                  % appNode.second.get<std::string>("ssl-certificate")
                                  % appNode.second.get<std::string>("ssl-private-key")
                                  % appNode.second.get<std::string>("ssl-tmp-dh")
                                  % appNode.second.get<std::string>("ssl-enable-v3")
                                  % appNode.second.get<std::string>("ssl-client-verification")
                                  % appNode.second.get<std::string>("ssl-verify-depth")
                                  % appNode.second.get<std::string>("ssl-ca-certificates")
                                  % appNode.second.get<std::string>("ssl-cipherlist")
                                  ).str()
                              << std::endl << std::endl;

                    std::string cmd((boost::format("cd %2% && %1%")
                                     % appNode.second.get<std::string>("app")
                                     % appNode.second.get<std::string>("workdir")
                                     ).str());
                    if (!appNode.second.get<std::string>("threads").empty()
                            && appNode.second.get<std::string>("threads") != "-1") {
                        cmd += (boost::format(" --threads %1%")
                                % appNode.second.get<std::string>("threads")).str();
                    }
                    if (!appNode.second.get<std::string>("servername").empty()) {
                        cmd += (boost::format(" --servername %1%")
                                % appNode.second.get<std::string>("servername")).str();
                    }
                    if (!appNode.second.get<std::string>("docroot").empty()) {
                        cmd += (boost::format(" --docroot \"%1%\"")
                                % appNode.second.get<std::string>("docroot")).str();
                    }
                    if (!appNode.second.get<std::string>("approot").empty()) {
                        cmd += (boost::format(" --approot \"%1%\"")
                                % appNode.second.get<std::string>("approot")).str();
                    }
                    if (!appNode.second.get<std::string>("errroot").empty()) {
                        cmd += (boost::format(" --errroot \"%1%\"")
                                % appNode.second.get<std::string>("errroot")).str();
                    }
                    if (!appNode.second.get<std::string>("accesslog").empty()) {
                        cmd += (boost::format(" --accesslog \"%1%\"")
                                % appNode.second.get<std::string>("accesslog")).str();
                    }
                    if (!appNode.second.get<std::string>("compression").empty()
                            && appNode.second.get<std::string>("compression") != "yes") {
                        cmd += " --no-compression";
                    }
                    if (!appNode.second.get<std::string>("deploy-path").empty()) {
                        cmd += (boost::format(" --deploy-path %1%")
                                % appNode.second.get<std::string>("deploy-path")).str();
                    }
                    if (!appNode.second.get<std::string>("session-id-prefix").empty()) {
                        cmd += (boost::format(" --session-id-prefix %1%")
                                % appNode.second.get<std::string>("session-id-prefix")).str();
                    }
                    if (!appNode.second.get<std::string>("pid-file").empty()) {
                        cmd += (boost::format(" --pid-file \"%1%\"")
                                % appNode.second.get<std::string>("pid-file")).str();
                    }
                    if (!appNode.second.get<std::string>("config").empty()) {
                        cmd += (boost::format(" --config \"%1%\"")
                                % appNode.second.get<std::string>("config")).str();
                    }
                    if (!appNode.second.get<std::string>("max-memory-request-size").empty()) {
                        cmd += (boost::format(" --max-memory-request-size %1%")
                                % appNode.second.get<std::string>("max-memory-request-size")).str();
                    }
                    if (!appNode.second.get<std::string>("gdb").empty()
                            && appNode.second.get<std::string>("gdb") != "yes") {
                        cmd += " --gdb";
                    }
                    if (!appNode.second.get<std::string>("http-address").empty()) {
                        cmd += (boost::format(" --http-address %1%")
                                % appNode.second.get<std::string>("http-address")).str();
                    }
                    if (!appNode.second.get<std::string>("http-port").empty()) {
                        cmd += (boost::format(" --http-port %1%")
                                % appNode.second.get<std::string>("http-port")).str();
                    }
                    if (!appNode.second.get<std::string>("https-address").empty()) {
                        cmd += (boost::format(" --https-address %1%")
                                % appNode.second.get<std::string>("https-address")).str();
                    }
                    if (!appNode.second.get<std::string>("https-port").empty()) {
                        cmd += (boost::format(" --https-port %1%")
                                % appNode.second.get<std::string>("https-port")).str();
                    }
                    if (!appNode.second.get<std::string>("ssl-certificate").empty()) {
                        cmd += (boost::format(" --ssl-certificate \"%1%\"")
                                % appNode.second.get<std::string>("ssl-certificate")).str();
                    }
                    if (!appNode.second.get<std::string>("ssl-private-key").empty()) {
                        cmd += (boost::format(" --ssl-private-key \"%1%\"")
                                % appNode.second.get<std::string>("ssl-private-key")).str();
                    }
                    if (!appNode.second.get<std::string>("ssl-tmp-dh").empty()) {
                        cmd += (boost::format(" --ssl-tmp-dh \"%1%\"")
                                % appNode.second.get<std::string>("ssl-tmp-dh")).str();
                    }
                    if (!appNode.second.get<std::string>("ssl-enable-v3").empty()
                            && appNode.second.get<std::string>("ssl-enable-v3") != "yes") {
                        cmd += " --ssl-enable-v3";
                    }
                    if (!appNode.second.get<std::string>("ssl-client-verification").empty()) {
                        cmd += (boost::format(" --ssl-client-verification %1%")
                                % appNode.second.get<std::string>("ssl-client-verification")).str();
                    }
                    if (!appNode.second.get<std::string>("ssl-verify-depth").empty()) {
                        cmd += (boost::format(" --ssl-verify-depth %1%")
                                % appNode.second.get<std::string>("ssl-verify-depth")).str();
                    }
                    if (!appNode.second.get<std::string>("ssl-ca-certificates").empty()) {
                        cmd += (boost::format(" --ssl-ca-certificates \"%1%\"")
                                % appNode.second.get<std::string>("ssl-ca-certificates")).str();
                    }
                    if (!appNode.second.get<std::string>("ssl-cipherlist").empty()) {
                        cmd += (boost::format(" --ssl-cipherlist %1%")
                                % appNode.second.get<std::string>("ssl-cipherlist")).str();
                    }
                    cmd += " &";

                    CoreLib::System::Exec(cmd);
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

