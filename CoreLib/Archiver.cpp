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
 * Contains necessary functions to uncompress a file.
 */


#include <algorithm>
#include <fstream>
#include <iterator>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <archive.h>
#include <archive_entry.h>
#include <zip.h>
#include "Archiver.hpp"
#include "Compression.hpp"
#include "FileSystem.hpp"
#include "System.hpp"

using namespace std;
using namespace boost;
using namespace CoreLib;

static int UnTarCopyData(archive *ar, archive *aw)
{
    int r;
    const void *buffer;

    size_t size;

#if ARCHIVE_VERSION_NUMBER >= 3000000
    int64_t offset;
#else
    off_t offset;
#endif

    for (;;) {
        r = archive_read_data_block(ar, &buffer, &size, &offset);

        if (r == ARCHIVE_EOF) {
            return (ARCHIVE_OK);
        }

        if (r != ARCHIVE_OK) {
            return r;
        }

        r = static_cast<int>(
                    archive_write_data_block(aw, buffer, size, offset));

        if (r != ARCHIVE_OK) {
            /// archive_write_data_block() failed!
            //archive_error_string(aw));
            return r;
        }
    }
}

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

bool Archiver::UnTar(const std::string &archivePath, const std::string &extractDirectory)
{
    string error;
    return UnTar(archivePath, extractDirectory, error);
}

bool Archiver::UnTar(const std::string &archivePath, const std::string &extractDirectory,
                     std::string &out_error)
{
    out_error.clear();
    bool success = false;

    int flags = ARCHIVE_EXTRACT_TIME;
    //    flags |= ARCHIVE_EXTRACT_PERM;
    //    flags |= ARCHIVE_EXTRACT_ACL;
    //    flags |= ARCHIVE_EXTRACT_FFLAGS;

    archive *a = archive_read_new();
    archive *ext = archive_write_disk_new();

    archive_write_disk_set_options(ext, flags);
    archive_read_support_format_tar(a);

    int r;
    if ((r = archive_read_open_filename(a, archivePath.c_str(), 10240))) {
        out_error.assign(
                    (format("Archiver::UnTar: archive_read_open_filename() failed: %1%; %2")
                     % r % archive_error_string(a)).str());
    } else {
        archive_entry *entry;

        for (;;) {
            r = archive_read_next_header(a, &entry);

            if (r == ARCHIVE_EOF) {
                success = true;
                break;
            }

            if (r != ARCHIVE_OK) {
                out_error.assign(
                            (format("Archiver::UnTar: archive_read_next_header() failed: %1%; %2")
                             % 1 % archive_error_string(a)).str());
            } else {
                const char *file = archive_entry_pathname(entry);
                const std::string filePath =
                        (boost::filesystem::path(extractDirectory)
                        / boost::filesystem::path(file)).string();
                archive_entry_set_pathname(entry, filePath.c_str());

                r = archive_write_header(ext, entry);

                if (r != ARCHIVE_OK) {
                    out_error +=
                            (format("Archiver::UnTar: archive_write_header() failed: %1%; %2\n")
                             % archive_error_string(ext)).str();
                } else {
                    (void)UnTarCopyData(a, ext);

                    r = archive_write_finish_entry(ext);

                    if (r != ARCHIVE_OK) {
                        out_error +=
                                (format("Archiver::UnTar: archive_write_finish_entry() failed: %1%; %2\n")
                                 % 1 % archive_error_string(ext)).str();
                    }
                }
            }
        }
    }

    archive_read_close(a);
    archive_read_free(a);

    archive_write_close(ext);
    archive_write_free(ext);

    return success;
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
            string dir((boost::filesystem::path(extractionPath)
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
            string dir((boost::filesystem::path(extractionPath)
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
        fd = open((boost::filesystem::path(extractionPath)
                   / boost::filesystem::path(sb.name)).string().c_str(),
                  O_RDWR | O_TRUNC | O_CREAT, 0644);
#else
        fd = open((boost::filesystem::path(extractionPath)
                   / boost::filesystem::path(sb.name)).string().c_str(),
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
