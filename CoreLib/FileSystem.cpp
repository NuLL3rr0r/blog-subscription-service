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

bool FileSystem::DirExists(const std::string &dir)
{
    try {
        boost::filesystem::path p(dir);
        if (boost::filesystem::exists(p)) {
            if (boost::filesystem::is_directory(p)) {
                return true;
            }  else if (boost::filesystem::is_regular_file(p)) {
                return false;
            } else {
                return false;
            }
        } else {
            return false;
        }
    } catch (const boost::filesystem::filesystem_error &ex) {
        LOG_ERROR(dir, ex.what());
    } catch (...) {
        LOG_ERROR(dir, UNKNOWN_ERROR);
    }

    return false;
}

bool FileSystem::FileExists(const std::string &file)
{
    try {
        boost::filesystem::path p(file);
        if (boost::filesystem::exists(p)) {
            if (boost::filesystem::is_regular_file(p)) {
                return true;
            } else if (boost::filesystem::is_directory(p)) {
                return false;
            } else {
                return false;
            }
        } else {
            return false;
        }
    } catch (const boost::filesystem::filesystem_error &ex) {
        LOG_ERROR(file, ex.what());
    } catch (...) {
        LOG_ERROR(file, UNKNOWN_ERROR);
    }

    return false;
}

size_t FileSystem::FileSize(const std::string &file)
{
    try {
        return boost::filesystem::file_size(file);
    } catch(const boost::filesystem::filesystem_error &ex) {
        LOG_ERROR(file, ex.what());
    } catch (...) {
        LOG_ERROR(file, UNKNOWN_ERROR);
    }

    return 0;
}

bool FileSystem::CreateDir(const std::string &dir, const bool parents)
{
    try {
        boost::filesystem::path p(dir);
        if (parents) {
            boost::filesystem::create_directories(p);
        } else {
            boost::filesystem::create_directory(p);
        }
        return true;
    }
    catch(const boost::filesystem::filesystem_error &ex) {
        LOG_ERROR(ex.what());
    }
    catch(...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    return false;
}

bool FileSystem::Erase(const std::string &path, const bool recursive)
{
    try {
        if (recursive) {
            boost::filesystem::remove_all(path);
        } else {
            boost::filesystem::remove(path);
        }
        return true;
    }
    catch(const boost::filesystem::filesystem_error &ex) {
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
        boost::filesystem::rename(from, to);
        return true;
    } catch(const boost::filesystem::filesystem_error &ex) {
        LOG_ERROR(from, to, ex.what());
    } catch (...) {
        LOG_ERROR(from, to, UNKNOWN_ERROR);
    }

    return false;
}

bool FileSystem::CopyFile(const std::string &from, const std::string &to, const bool overwrite)
{
    try {
        boost::filesystem::copy_file(from, to,
                              overwrite ? boost::filesystem::copy_option::overwrite_if_exists
                                        : boost::filesystem::copy_option::fail_if_exists);

        return true;
    } catch(const boost::filesystem::filesystem_error &ex) {
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
        ifstream ifs(file);
        out_data.assign((istreambuf_iterator<char>(ifs)),
                        istreambuf_iterator<char>());
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
