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
 * A class that provides access to other objects using proxy design pattern.
 */


#ifndef SERVICE_POOL_HPP
#define SERVICE_POOL_HPP


#include <memory>

namespace CoreLib {
class Crypto;
class Database;
}

namespace Service {
class Pool;
}

class Service::Pool
{
public:
    struct StorageStruct
    {
        const int &LanguageCookieLifespan() const;

        const std::string &RootUsername() const;
        const std::string &RootInitialPassword() const;
        const std::string &RootInitialEmail() const;
        const int &RootSessionLifespan() const;

        const std::string &RegexEmail() const;
        const int &MinUsernameLength() const;
        const int &MaxUsernameLength() const;
        const int &MinPasswordLength() const;
        const int &MaxPasswordLength() const;

        const int &MinEmailRecipientNameLength() const;
        const int &MaxEmailRecipientNameLength() const;
        const int &MinEmailSenderNameLength() const;
        const int &MaxEmailSenderNameLength() const;
        const int &MinEmailSubjectLength() const;
        const int &MaxEmailSubjectLength() const;
        const int &MaxEmailSubjectLength_RFC() const;
        const int &MinEmailBodyLength() const;
        const int &MaxEmailBodyLength() const;

        const std::string &RegexHttpUrl() const;

        const std::string &RegexUuid() const;
        const std::string &RegexLanguageArray() const;

        std::string AppPath;
    };

private:
    struct Impl;

public:
    static StorageStruct *Storage();
    static CoreLib::Crypto *Crypto();
    static CoreLib::Database *Database();
};


#endif /* SERVICE_POOL_HPP */

