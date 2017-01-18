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
 * Contains necessary functions to uncompress a file.
 */


#include <algorithm>
#include <fstream>
#include <iterator>
#include <vector>
#include <fcntl.h>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <zip.h>
#include "Archiver.hpp"
#include "Compression.hpp"
#include "FileSystem.hpp"
#include "System.hpp"

using namespace std;
using namespace boost;
using namespace CoreLib;

bool Archiver::UnGzip(const std::string &archive, const std::string &extractedFile)
{
    string error;
    return UnGzip(archive, extractedFile, error);
}

bool Archiver::UnGzip(const std::string &archive, const std::string &extractedFile,
                      std::string &out_error)
{
    out_error.clear();

    ifstream ifs(archive, ios::binary);
    if (!ifs.is_open()) {
        out_error.assign((format("Archiver::UnGzip: Could not extract '%1%' as %2%!")
                          % archive % extractedFile).str());
        return false;
    }
    Compression::Buffer compressedContents((std::istreambuf_iterator<char>(ifs)),
                                           std::istreambuf_iterator<char>());
    ifs.close();

    Compression::Buffer uncompressedContents;
    Compression::Decompress(compressedContents, uncompressedContents,
                            Compression::Algorithm::Gzip);

    ofstream ofs(extractedFile, ios::binary);
    if (!ofs.is_open()) {
        out_error.assign((format("Archiver::UnGzip: Could not extract '%1%' as %2%!")
                          % archive % extractedFile).str());
        return false;
    }
    copy(uncompressedContents.begin(), uncompressedContents.end(),
         ostreambuf_iterator<char>(ofs));
    ofs.flush();
    ofs.close();

    return true;
}

bool Archiver::UnZip(const std::string &archive, const std::string &extractionPath)
{
    string error;
    return UnZip(archive, extractionPath, error);
}

bool Archiver::UnZip(const std::string &archive, const std::string &extractionPath,
                     std::string &out_error)
{
    out_error.clear();

    struct zip *za;
    char buf[100];
    int err;

    if ((za = zip_open(archive.c_str(), 0, &err)) == NULL) {
        zip_error_to_str(buf, sizeof(buf), err, errno);
        out_error.assign((format("Archiver::UnZip: Can't open zip archive `%1%': %2%!")
                          % archive % buf).str());
        return false;
    }

    struct zip_file *zf;
    struct zip_stat sb;
    zip_int64_t len;
    int fd;
    zip_uint64_t sum;

    for (zip_int64_t i = 0; i < zip_get_num_entries(za, 0); ++i) {
        if (zip_stat_index(za, static_cast<zip_uint64_t>(i), 0, &sb) != 0) {
            out_error.assign((format("Archiver::UnZip: Corrupted zip archive `%1%'!")
                              % archive).str());
            return false;
        }

        len = static_cast<zip_int64_t>(strlen(sb.name));
        if (sb.name[len - 1] == '/') {
            string dir((filesystem::path(extractionPath)
                        / sb.name).string());
            if (!FileSystem::CreateDir(dir, true)) {
                out_error.assign((format("Archiver::UnZip: Failed to create directory `%1%'!")
                                  % dir).str());
                return false;
            }
            continue;
        }

        boost::filesystem::path p(sb.name);
        if (p.parent_path().string() != "") {
            string dir((filesystem::path(extractionPath)
                        / p.parent_path()).string());
            if (!FileSystem::CreateDir(dir, true)) {
                out_error.assign((format("Archiver::UnZip: Failed to create directory `%1%'!")
                                  % dir).str());
                return false;
            }
        }

        zf = zip_fopen_index(za, static_cast<zip_uint64_t>(i), 0);
        if (!zf) {
            out_error.assign((format("Archiver::UnZip: Corrupted zip archive `%1%'!")
                              % archive).str());
            return false;
        }

#if ! defined ( _WIN32 )
        fd = open((filesystem::path(extractionPath)
                   / filesystem::path(sb.name)).string().c_str(),
                  O_RDWR | O_TRUNC | O_CREAT, 0644);
#else
        fd = open((filesystem::path(extractionPath)
                   / filesystem::path(sb.name)).string().c_str(),
                  O_RDWR | O_TRUNC | O_CREAT | O_BINARY, 0644);
#endif  // ! defined ( _WIN32 )

        if (fd < 0) {
            out_error.assign((format("Archiver::UnZip: Cannot open file `%1%' for writing!")
                              % sb.name).str());
            return false;
        }

        sum = 0;
        while (sum != sb.size) {
            len = zip_fread(zf, buf, 100);
            if (len < 0) {
                out_error.assign((format("Archiver::UnZip: Corrupted zip archive `%1%'!")
                                  % archive).str());
                return false;
            }
            write(fd, buf, static_cast<size_t>(len));
            sum += static_cast<zip_uint64_t>(len);
        }
        close(fd);
        zip_fclose(zf);
    }

    if (zip_close(za) == -1) {
        out_error.assign((format("Archiver::UnZip: Can't close zip archive `%1%'!")
                          % archive).str());
        return false;
    }

    return true;
}
