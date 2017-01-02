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
 * Database object with support for PostgreSQL, MariaDB / MySQL and SQLite3.
 */


#ifndef CORELIB_DATABASE_HPP
#define CORELIB_DATABASE_HPP


#include <initializer_list>
#include <memory>
#include <string>
#include <cppdb/frontend.h>

namespace CoreLib {
class Database;
}

class CoreLib::Database
{
private:
    struct Impl;
    std::unique_ptr<Impl> m_pimpl;

public:
#if defined ( CORELIB_STATIC )
#if defined ( HAS_CPPDB_SQLITE3_DRIVER )
    static bool IsSqlite3DriverLoaded();
#endif  // defined ( HAS_CPPDB_SQLITE3_DRIVER )
#if defined ( HAS_CPPDB_PGSQL_DRIVER )
    static bool IsPgSqlDriverLoaded();
#endif  // defined ( HAS_CPPDB_PGSQL_DRIVER )
#if defined ( HAS_CPPDB_MYSQL_DRIVER )
    static bool IsMySqlDriverLoaded();
#endif  // defined ( HAS_CPPDB_MYSQL_DRIVER )

#if defined ( HAS_CPPDB_SQLITE3_DRIVER )
    static void LoadSqlite3Driver();
#endif  // defined ( HAS_CPPDB_SQLITE3_DRIVER )
#if defined ( HAS_CPPDB_PGSQL_DRIVER )
    static void LoadPgSqlDriver();
#endif  // defined ( HAS_CPPDB_PGSQL_DRIVER )
#if defined ( HAS_CPPDB_MYSQL_DRIVER )
    static void LoadMySqlDriver();
#endif  // defined ( HAS_CPPDB_MYSQL_DRIVER )
#endif  // defined ( CORELIB_STATIC )

#if defined ( HAS_SQLITE3 )
    static bool Sqlite3Vacuum(const std::string &databaseFile);
#endif  // defined ( HAS_SQLITE3 )

    static bool IsTrue(const std::string &value);

public:
    explicit Database(const std::string &connectionString);
    virtual ~Database();

    cppdb::session &Sql();

    bool CreateEnum(const std::string &id);

    bool CreateTable(const std::string &id);
    bool DropTable(const std::string &id);
    bool RenameTable(const std::string &id, const std::string &newName);

    bool Insert(const std::string &id,
                const std::string &fields,
                const std::initializer_list<std::string> &args);
    bool Update(const std::string &id,
                const std::string &where,
                const std::string &value,
                const std::string &set,
                const std::initializer_list<std::string> &args);
    bool Delete(const std::string &id,
                const std::string &where,
                const std::string &value);

    void RegisterEnum(const std::string &id,
                      const std::string &name,
                      const std::initializer_list<std::string> &enumerators);

    void RegisterTable(const std::string &id,
                       const std::string &name,
                       const std::string &fields);
    std::string GetTableName(const std::string &id) const;
    std::string GetTableFields(const std::string &id) const;
    bool SetTableName(const std::string &id, const std::string &newName);
    bool SetTableFields(const std::string &id, const std::string &fields);

    bool Initialize();
};


#endif /* CORELIB_DATABASE_HPP */
