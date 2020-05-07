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
 * A class that provides access to other objects using proxy design pattern.
 */


#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/thread/once.hpp>
#include <CoreLib/make_unique.hpp>
#include <CoreLib/Crypto.hpp>
#include <CoreLib/Database.hpp>
#include <CoreLib/Log.hpp>
#include "Pool.hpp"

using namespace std;
using namespace boost;
using namespace Service;

const int &Pool::StorageStruct::LanguageCookieLifespan() const
{
    /// 365 Days * 24 Hours * 60 Minutes * 60 Seconds = Number of Days in Seconds
    static constexpr int DURATION = 365 * 24 * 60 * 60;
    return DURATION;
}

const std::string &Pool::StorageStruct::RootUsername() const
{
    static string rootUsername;
    if (rootUsername == "") {
        rootUsername = CoreLib::Crypto::HexStringToString(ROOT_USERNAME);
    }
    return rootUsername;
}

const std::string &Pool::StorageStruct::RootInitialPassword() const
{
    static string rootInitialPassowrd;
    if (rootInitialPassowrd == "") {
        CoreLib::Crypto::Argon2(
                    CoreLib::Crypto::HexStringToString(ROOT_INITIAL_PASSWORD),
                    rootInitialPassowrd,
                    CoreLib::Crypto::Argon2OpsLimit::Min,
                    CoreLib::Crypto::Argon2MemLimit::Min);
        Crypto().Encrypt(rootInitialPassowrd, rootInitialPassowrd);
    }
    return rootInitialPassowrd;
}

const std::string &Pool::StorageStruct::RootInitialEmail() const
{
    static string rootInitialEmail;
    if (rootInitialEmail == "") {
        rootInitialEmail= CoreLib::Crypto::HexStringToString(ROOT_INITIAL_EMAIL);
    }
    return rootInitialEmail;
}

const int &Pool::StorageStruct::RootSessionLifespan() const
{
    // 7 Days * 24 Hours * 60 Minutes * 60 Seconds = Number of Days in Seconds
    static constexpr int DURATION = 7 * 24 * 60 * 60;
    return DURATION;
}

const std::string &Pool::StorageStruct::RegexEmail() const
{
    static const string regex("[a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*@(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\\.)+(?:[a-z]{2}|com|org|net|edu|gov|mil|biz|info|mobi|name|aero|asia|jobs|museum)\\b");
    return regex;
}

const int &Pool::StorageStruct::MinUsernameLength() const
{
    static constexpr int LENGTH = 4;
    return LENGTH;
}

const int &Pool::StorageStruct::MaxUsernameLength() const
{
    static constexpr int LENGTH = 16;
    return LENGTH;
}

const int &Pool::StorageStruct::MinPasswordLength() const
{
    static constexpr int LENGTH = 8;
    return LENGTH;
}

const int &Pool::StorageStruct::MaxPasswordLength() const
{
    static constexpr int LENGTH = 24;
    return LENGTH;
}

const int &Pool::StorageStruct::MinEmailRecipientNameLength() const
{
    static constexpr int LENGTH = 1;
    return LENGTH;
}

const int &Pool::StorageStruct::MaxEmailRecipientNameLength() const
{
    static constexpr int LENGTH = 64;
    return LENGTH;
}

const int &Pool::StorageStruct::MinEmailSenderNameLength() const
{
    static constexpr int LENGTH = 1;
    return LENGTH;
}

const int &Pool::StorageStruct::MaxEmailSenderNameLength() const
{
    static constexpr int LENGTH = 24;
    return LENGTH;
}

const int &Pool::StorageStruct::MinEmailSubjectLength() const
{
    static constexpr int LENGTH = 1;
    return LENGTH;
}

const int &Pool::StorageStruct::MaxEmailSubjectLength() const
{
    static constexpr int LENGTH = 48; // RFC says its 78. But, we need those extra chracters.
    return LENGTH;
}

const int &Pool::StorageStruct::MaxEmailSubjectLength_RFC() const
{
    static constexpr int LENGTH = 78;
    return LENGTH;
}

const int &Pool::StorageStruct::MinEmailBodyLength() const
{
    static constexpr int LENGTH = 1;
    return LENGTH;
}

const int &Pool::StorageStruct::MaxEmailBodyLength() const
{
    static constexpr int LENGTH = 2048;
    return LENGTH;
}

const std::string &Pool::StorageStruct::RegexHttpUrl() const
{
    static const string regex("^(https?://)"
                              "?(([0-9a-z_!~*'().&=+$%-]+: )?[0-9a-z_!~*'().&=+$%-]+@)?" //user@
                              "(([0-9]{1,3}\\.){3}[0-9]{1,3}" // IP
                              "|" // allows either IP or domain
                              "([0-9a-z_!~*'()-]+\\.)*" // tertiary domain(s)- www.
                              "([0-9a-z][0-9a-z-]{0,61})?[0-9a-z]\\." // second level domain
                              "[a-z]{2,6})" // first level domain- .com or .museum
                              "(:[0-9]{1,4})?" // port number- :80
                              "((/?)|" // a slash isn't required if there is no file name
                              "(/[0-9a-z_!~*'().;?:@&=+$,%#-]+)+/?)$");
    return regex;
}

const std::string &Pool::StorageStruct::RegexUuid() const
{
    static const string REGEX("^[0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$");
    return REGEX;
}

const std::string &Pool::StorageStruct::RegexLanguageArray() const
{
    static const string REGEX("^((^|,)(en|fa))+$");
    return REGEX;
}

const int &Pool::StorageStruct::TokenLifespan() const
{
    // 60 Minutes * 60 Seconds = 1 Hour
    static constexpr int DURATION = 60 * 60;
    return DURATION;
}


const int &Pool::StorageStruct::MinHomePageTitleLength() const
{
    static constexpr int LENGTH = 1;
    return LENGTH;
}

const int &Pool::StorageStruct::MaxHomePageTitleLength() const
{
    static constexpr int LENGTH = 64;
    return LENGTH;
}

const int &Pool::StorageStruct::ResetPwdLifespan() const
{
    // 1 Hour(s) * 60 Minutes * 60 Seconds = Number of Days in Seconds
    static constexpr int DURATION = 1 * 60 * 60;
    return DURATION;
}

Pool::StorageStruct &Pool::Storage()
{
    /// C++11 specifies it to be thread safe.
    /// Warning: VS2013 does not implement thread-safe construction of function-local statics.
    static StorageStruct instance;
    return instance;
}

CoreLib::Crypto &Pool::Crypto()
{
    static const string KEY = CoreLib::Crypto::HexStringToString(CRYPTO_KEY);
    static const string IV = CoreLib::Crypto::HexStringToString(CRYPTO_IV);

    static CoreLib::Crypto instance(reinterpret_cast<const CoreLib::Crypto::Byte *>(KEY.c_str()), KEY.size(),
                                    reinterpret_cast<const CoreLib::Crypto::Byte *>(IV.c_str()), IV.size());

    return instance;
}

CoreLib::Database &Pool::Database()
{
#if defined ( PGSQL_CONNECTION_STRING )
    static const string CONNECTION_STRING(PGSQL_CONNECTION_STRING);
#else
    static const string CONNECTION_STRING(
                (format("host=%1% port=%2% dbname=%3% user=%4% password=%5%")
                 % trim_copy(std::string(PGSQL_HOST))
                 % trim_copy(std::string(PGSQL_PORT))
                 % trim_copy(std::string(PGSQL_DATABASE))
                 % trim_copy(std::string(PGSQL_USER))
                 % trim_copy(std::string(PGSQL_PASSWORD))).str());
#endif  // defined ( PGSQL_CONNECTION_STRING )
    static CoreLib::Database instance(CONNECTION_STRING);

    return instance;
}
