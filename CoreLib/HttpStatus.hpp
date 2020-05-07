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
 * Translates all Http responses.
 */


#ifndef CORELIB_HTTP_STATUS
#define CORELIB_HTTP_STATUS


#include <string>

namespace CoreLib {
class HttpStatus;
}

class CoreLib::HttpStatus
{
public:
    enum class HttpStatusCode : unsigned char {
        HTTP_100,
        HTTP_101,
        HTTP_200,
        HTTP_201,
        HTTP_202,
        HTTP_203,
        HTTP_204,
        HTTP_205,
        HTTP_206,
        HTTP_300,
        HTTP_301,
        HTTP_302,
        HTTP_303,
        HTTP_304,
        HTTP_305,
        HTTP_307,
        HTTP_400,
        HTTP_401,
        HTTP_402,
        HTTP_403,
        HTTP_404,
        HTTP_405,
        HTTP_406,
        HTTP_407,
        HTTP_408,
        HTTP_409,
        HTTP_410,
        HTTP_411,
        HTTP_412,
        HTTP_413,
        HTTP_414,
        HTTP_415,
        HTTP_416,
        HTTP_417,
        HTTP_500,
        HTTP_501,
        HTTP_502,
        HTTP_503,
        HTTP_504,
        HTTP_505
    };

private:
    struct Impl;
    static std::unique_ptr<Impl> s_pimpl;

public:
    static std::wstring GetHttpResponse(const HttpStatusCode &code);
};


#endif /* CORELIB_HTTP_STATUS */
