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


#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <boost/thread/once.hpp>
#include <CoreLib/Crypto.hpp>
#include <CoreLib/Database.hpp>
#include <CoreLib/make_unique.hpp>
#include <CoreLib/Log.hpp>
#include "Pool.hpp"

using namespace std;
using namespace boost;
using namespace Service;

struct Pool::Impl {
public:
    static boost::once_flag StorageOnceFlag;
    static unique_ptr<StorageStruct> StorageInstance;

    static boost::once_flag CryptoOnceFlag;
    static unique_ptr<CoreLib::Crypto> CryptoInstance;

    static boost::once_flag DatabaseOnceFlag;
    static unique_ptr<CoreLib::Database> DatabaseInstance;
};

boost::once_flag Pool::Impl::StorageOnceFlag;
unique_ptr<Pool::StorageStruct> Pool::Impl::StorageInstance = nullptr;

boost::once_flag Pool::Impl::CryptoOnceFlag;
unique_ptr<CoreLib::Crypto> Pool::Impl::CryptoInstance = nullptr;

boost::once_flag Pool::Impl::DatabaseOnceFlag;
unique_ptr<CoreLib::Database> Pool::Impl::DatabaseInstance = nullptr;

const int &Pool::StorageStruct::LanguageCookieLifespan() const
{
    // 365 Days * 24 Hours * 60 Minutes * 60 Seconds = Number of Days in Seconds
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
        CoreLib::Crypto::Hash(
                    CoreLib::Crypto::HexStringToString(ROOT_INITIAL_PASSWORD),
                    rootInitialPassowrd);
        Crypto()->Encrypt(rootInitialPassowrd, rootInitialPassowrd);
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
    static const string regex("^[0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$");
    return regex;
}

const std::string &Pool::StorageStruct::RegexLanguageArray() const
{
    static const string regex("^((^|,)(en|fa))+$");
    return regex;
}

Pool::StorageStruct *Pool::Storage()
{
    boost::call_once(Impl::StorageOnceFlag, [] {
        Impl::StorageInstance = make_unique<StorageStruct>();
    });

    return Impl::StorageInstance.get();
}

CoreLib::Crypto *Pool::Crypto()
{
    boost::call_once(Impl::CryptoOnceFlag, [] {
        static const string KEY = CoreLib::Crypto::HexStringToString(CRYPTO_KEY);
        static const string IV = CoreLib::Crypto::HexStringToString(CRYPTO_IV);

        Impl::CryptoInstance = make_unique<CoreLib::Crypto>(reinterpret_cast<const CoreLib::Crypto::Byte *>(KEY.c_str()), KEY.size(),
                                                            reinterpret_cast<const CoreLib::Crypto::Byte *>(IV.c_str()), IV.size());
    });

    return Impl::CryptoInstance.get();
}

CoreLib::Database *Pool::Database()
{
    boost::call_once(Impl::DatabaseOnceFlag, [] {
#if DATABASE_BACKEND == PGSQL

#ifdef CORELIB_STATIC
#if defined ( HAS_CPPDB_PGSQL_DRIVER )
        if (!Database::IsPgSqlDriverLoaded()) {
            Database::LoadPgSqlDriver();
        }
#endif  // defined ( HAS_CPPDB_PGSQL_DRIVER )
#endif  // CORELIB_STATIC

        string parameters;
        string pgSqlHost(PGSQL_HOST);
        string pgSqlPort(PGSQL_PORT);
        string pgSqlDatabase(PGSQL_DATABASE);
        string pgSqlUser(PGSQL_USER);
        string pgSqlPassword(PGSQL_PASSWORD);

        if (trim_copy(pgSqlHost) != "")
            parameters += (format("host=%1%;") % pgSqlHost).str();

        if (trim_copy(pgSqlPort) != "")
            parameters += (format("port=%1%;") % pgSqlPort).str();

        if (trim_copy(pgSqlDatabase) != "")
            parameters += (format("dbname=%1%;") % pgSqlDatabase).str();

        if (trim_copy(pgSqlUser) != "")
            parameters += (format("user=%1%;") % pgSqlUser).str();

        if (trim_copy(pgSqlPassword) != "")
            parameters += (format("password=%1%;") % pgSqlPassword).str();

        static const string CONNECTION_STRING(
                    (format("postgresql:%1%")
                     % parameters).str());

#elif DATABASE_BACKEND == MYSQL

#ifdef CORELIB_STATIC
#if defined ( HAS_CPPDB_MYSQL_DRIVER )
        if (!Database::IsMySqlDriverLoaded()) {
            Database::LoadMySqlDriver();
        }
#endif  // defined ( HAS_CPPDB_MYSQL_DRIVER )
#endif  // CORELIB_STATIC

        string parameters;
        string mySqlHost(MYSQL_HOST);
        string mySqlPort(MYSQL_PORT);
        string mySqlUnixSocket(MYSQL_UNIX_SOCKET);
        string mySqlDatabase(MYSQL_DATABASE);
        string mySqlUser(MYSQL_USER);
        string mySqlPassword(MYSQL_PASSWORD);

        if (trim_copy(mySqlHost) != "")
            parameters += (format("host=%1%;") % mySqlHost).str();

        if (trim_copy(mySqlPort) != "")
            parameters += (format("port=%1%;") % mySqlPort).str();

        if (trim_copy(mySqlUnixSocket) != "")
            parameters += (format("unix_socket=%1%;") % mySqlUnixSocket).str();

        if (trim_copy(mySqlDatabase) != "")
            parameters += (format("database=%1%;") % mySqlDatabase).str();

        if (trim_copy(mySqlUser) != "")
            parameters += (format("user=%1%;") % mySqlUser).str();

        if (trim_copy(mySqlPassword) != "")
            parameters += (format("password=%1%;") % mySqlPassword).str();

        static const string CONNECTION_STRING(
                    (format("mysql:%1%")
                     % parameters).str());

#else   // SQLITE3

#if defined ( CORELIB_STATIC )
#if defined ( HAS_CPPDB_SQLITE3_DRIVER )
        if (!Database::IsSqlite3DriverLoaded()) {
            Database::LoadSQLite3Driver();
        }
#endif  // defined ( HAS_CPPDB_SQLITE3_DRIVER )
#endif  // defined ( CORELIB_STATIC )

        static const string DB_FILE((filesystem::path(Storage()->AppPath)
                                     / filesystem::path(SQLITE3_DATABASE_FILE_PATH)
                                     / filesystem::path(SQLITE3_DATABASE_FILE_NAME)).string());
        static const string CONNECTION_STRING(
                    (format("sqlite3:db=%1%")
                     % DB_FILE).str());

#if defined ( HAS_SQLITE3 )
        CoreLib::Database::Sqlite3Vacuum(DB_FILE);
#endif  // defined ( HAS_SQLITE3 )

#endif // DATABASE_BACKEND == PGSQL

        Impl::DatabaseInstance = make_unique<CoreLib::Database>(CONNECTION_STRING);
    });

    return Impl::DatabaseInstance.get();
}

