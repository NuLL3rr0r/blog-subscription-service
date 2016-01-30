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
 * Provides common filing and filesystem operations.
 */


#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include "FileSystem.hpp"
#include "Log.hpp"

#define     UNKNOWN_ERROR       "Unknow filesystem error!"

using namespace std;
using namespace boost;
using namespace CoreLib;

namespace fs = boost::filesystem;

bool FileSystem::DirExists(const std::string &dir)
{
    try {
        fs::path p(dir);
        if (fs::exists(p)) {
            if (fs::is_directory(p)) {
                return true;
            }  else if (fs::is_regular_file(p)) {
                return false;
            } else {
                return false;
            }
        } else {
            return false;
        }
    } catch (const fs::filesystem_error &ex) {
        LOG_ERROR(dir, ex.what());
    } catch (...) {
        LOG_ERROR(dir, UNKNOWN_ERROR);
    }

    return false;
}

bool FileSystem::FileExists(const std::string &file)
{
    try {
        fs::path p(file);
        if (fs::exists(p)) {
            if (fs::is_regular_file(p)) {
                return true;
            } else if (fs::is_directory(p)) {
                return false;
            } else {
                return false;
            }
        } else {
            return false;
        }
    } catch (const fs::filesystem_error &ex) {
        LOG_ERROR(file, ex.what());
    } catch (...) {
        LOG_ERROR(file, UNKNOWN_ERROR);
    }

    return false;
}

size_t FileSystem::FileSize(const std::string &file)
{
    try {
        return fs::file_size(file);
    } catch(const fs::filesystem_error &ex) {
        LOG_ERROR(file, ex.what());
    } catch (...) {
        LOG_ERROR(file, UNKNOWN_ERROR);
    }

    return 0;
}

bool FileSystem::CreateDir(const std::string &dir, bool parents)
{
    try {
        fs::path p(dir);
        if (parents) {
            fs::create_directories(p);
        } else {
            fs::create_directory(p);
        }
        return true;
    }
    catch(const fs::filesystem_error &ex) {
        LOG_ERROR(ex.what());
    }
    catch(...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    return false;
}

bool FileSystem::Erase(const std::string &path, bool recursive)
{
    try {
        if (recursive) {
            fs::remove_all(path);
        } else {
            fs::remove(path);
        }
        return true;
    }
    catch(const fs::filesystem_error &ex) {
        LOG_ERROR(ex.what());
    }
    catch(...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    return false;
}

bool FileSystem::Move(const std::string &from, const std::string &to)
{
    try {
        fs::rename(from, to);
        return true;
    } catch(const fs::filesystem_error &ex) {
        LOG_ERROR(from, to, ex.what());
    } catch (...) {
        LOG_ERROR(from, to, UNKNOWN_ERROR);
    }

    return false;
}

bool FileSystem::CopyFile(const std::string &from, const std::string &to, bool overwrite)
{
    try {
        // Affects Boost 1.55.0 - 1.57.0
        // https://svn.boost.org/trac/boost/ticket/10038

        // uncomment if it won't affect you
        //fs::copy_file(from, to,
        //              overwrite ? fs::copy_option::overwrite_if_exists
        //                        : fs::copy_option::fail_if_exists );

        // start alternative - remove these lines if you are not affected by the bug

        if (FileExists(to) && !overwrite)
            return false;

        ifstream ifs(from, ios::in | ios::binary);
        if (!ifs.is_open())
            return false;

        ofstream ofs(to, ios::out | ios::binary);
        if (!ofs.is_open())
            return false;

        ofs << ifs.rdbuf();
        ofs.close();
        ifs.close();

        // end alternative

        return true;
    } catch(const fs::filesystem_error &ex) {
        LOG_ERROR(from, to, overwrite, ex.what());
    } catch (...) {
        LOG_ERROR(from, to, overwrite, UNKNOWN_ERROR);
    }

    return false;
}

bool FileSystem::Read(const std::string &file, std::string &out_data)
{
    try {
        out_data.clear();
        std::ifstream ifs(file);
        out_data.assign((std::istreambuf_iterator<char>(ifs)),
                         std::istreambuf_iterator<char>());
        ifs.close();
        return true;
    } catch (const std::ifstream::failure &ex) {
        LOG_ERROR(ex.what());
    } catch (const std::exception &ex) {
        LOG_ERROR(ex.what());
    } catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    return false;
}

bool FileSystem::Write(const std::string &file, const std::string &data)
{
    try {
        ofstream ofs(file.c_str(), ios::out | ios::trunc);
        if (ofs.is_open()) {
            ofs << data;
            ofs.close();
            return true;
        }
    } catch (const std::ifstream::failure &ex) {
        LOG_ERROR(ex.what());
    } catch (const std::exception &ex) {
        LOG_ERROR(ex.what());
    } catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    return false;
}


