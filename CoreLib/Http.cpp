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
 * Necessary functions to download a file or web page.
 */


#include <fstream>
#include <curlpp/internal/SList.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Options.hpp>
#include "Http.hpp"

#define     UNKNOWN_ERROR           "Unknown error!"
#define     OPEN_FILE_ERROR         "Could not open file!"

using namespace std;
using namespace curlpp;
using namespace CoreLib;

bool Http::Download(const std::string &remoteAddr, const std::string &localPath)
{
    string err;
    return Download(remoteAddr.c_str(), localPath, err);
}

bool Http::Download(const std::string &remoteAddr, const std::string &localPath,
                        std::string &out_error)
{
    try {
        out_error.clear();

        // It is required to do the cleanup of used resources
        Cleanup cleanup;
        (void)cleanup;

        list<std::string> headers;
        headers.push_back("ACCEPT: text/html, */*; q=0.01");
        headers.push_back("ACCEPT_CHARSET: utf-8;q=0.7,*;q=0.7");
        headers.push_back("ACCEPT_ENCODING: gzip, deflate");
        headers.push_back("ACCEPT_LANGUAGE: en-US,en;q=0.5");
        headers.push_back("CONNECTION: keep-alive");
        headers.push_back("KEEP_ALIVE: 300");

        internal::SList slist(headers);

        string userAgent = "Mozilla/5.0 (Windows; FreeBSD amd64; rv:26.0) Gecko/20100101 Firefox/26.0";

        Easy request;
        request.setOpt<options::HttpHeader>(slist);
        request.setOpt<options::UserAgent>(userAgent.c_str());

        // If an empty string, "", is set, a header containing all supported encoding types is sent.
        // Supported encodings are "identity", "deflate", and "gzip".
        request.setOpt<options::Encoding>("");

        request.setOpt<options::Url>(remoteAddr);
        ofstream ofs(localPath, std::ios::binary);

        if (!ofs.is_open()) {
            out_error.assign(OPEN_FILE_ERROR);
            return false;
        }

        options::WriteStream ws(&ofs);
        request.setOpt(ws);
        request.perform();

        ofs.flush();
        ofs.close();

        return true;
    }

    catch (const curlpp::RuntimeError &ex) {
        out_error.assign(ex.what());
    }

    catch (const curlpp::LogicError &ex) {
        out_error.assign(ex.what());
    }

    catch (const std::runtime_error &ex) {
        out_error.assign(ex.what());
    }

    catch (...) {
        out_error.assign(UNKNOWN_ERROR);
    }

    return false;
}

