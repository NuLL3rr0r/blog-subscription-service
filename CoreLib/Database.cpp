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


#include <unordered_map>
#include <cassert>
#include <cstdarg>
#if defined ( HAS_SQLITE3 )
#include <sqlite3.h>
#endif  // defined ( HAS_SQLITE3 )
#include <cppdb/backend.h>
#include <cppdb/driver_manager.h>
#include "make_unique.hpp"
#include "Database.hpp"
#include "Log.hpp"

#define     UNKNOWN_ERROR       "Unknow database error!"

using namespace std;
using namespace cppdb;
using namespace CoreLib;

struct Database::Impl
{
    typedef std::unordered_map<std::string, std::string> TableNamesHashTable;
    typedef std::unordered_map<std::string, std::string> TableFieldsHashTable;

#if defined ( CORELIB_STATIC )
#if defined ( HAS_CPPDB_SQLITE3_DRIVER )
    static bool IsSqlite3DriverLoaded;
#endif  // defined ( HAS_CPPDB_SQLITE3_DRIVER )
#if defined ( HAS_CPPDB_PGSQL_DRIVER )
    static bool IsPgSqlDriverLoaded;
#endif  // defined ( HAS_CPPDB_PGSQL_DRIVER )
#if defined ( HAS_CPPDB_MYSQL_DRIVER )
    static bool IsMySqlDriverLoaded;
#endif  // defined ( HAS_CPPDB_MYSQL_DRIVER )
#endif  // defined ( CORELIB_STATIC )

    cppdb::session Sql;

    TableNamesHashTable TableNames;
    TableFieldsHashTable TableFields;
};

#if defined ( CORELIB_STATIC )
#if defined ( HAS_CPPDB_SQLITE3_DRIVER )
bool Database::Impl::IsSqlite3DriverLoaded = false;
#endif  // defined ( HAS_CPPDB_SQLITE3_DRIVER )
#if defined ( HAS_CPPDB_PGSQL_DRIVER )
bool Database::Impl::IsPgSqlDriverLoaded = false;
#endif  // defined ( HAS_CPPDB_PGSQL_DRIVER )
#if defined ( HAS_CPPDB_MYSQL_DRIVER )
bool Database::Impl::IsMySqlDriverLoaded = false;
#endif  // defined ( HAS_CPPDB_MYSQL_DRIVER )
#endif  // defined ( CORELIB_STATIC )

#if defined ( CORELIB_STATIC )
extern "C" {
#if defined ( HAS_CPPDB_SQLITE3_DRIVER )
cppdb::backend::connection *cppdb_sqlite3_get_connection(cppdb::connection_info const &);
#endif  // defined ( HAS_CPPDB_SQLITE3_DRIVER )
#if defined ( HAS_CPPDB_PGSQL_DRIVER )
cppdb::backend::connection *cppdb_postgresql_get_connection(cppdb::connection_info const &);
#endif  // defined ( HAS_CPPDB_PGSQL_DRIVER )
#if defined ( HAS_CPPDB_MYSQL_DRIVER )
cppdb::backend::connection *cppdb_mysql_get_connection(cppdb::connection_info const &);
#endif  // defined ( HAS_CPPDB_MYSQL_DRIVER )
}
#endif  // defined ( CORELIB_STATIC )

#if defined ( CORELIB_STATIC )
#if defined ( HAS_CPPDB_SQLITE3_DRIVER )
bool Database::IsSqlite3DriverLoaded()
{
    return Impl::IsSqlite3DriverLoaded;
}
#endif  // defined ( HAS_CPPDB_SQLITE3_DRIVER )

#if defined ( HAS_CPPDB_PGSQL_DRIVER )
bool Database::IsPgSqlDriverLoaded()
{
    return Impl::IsPgSqlDriverLoaded;
}
#endif  // defined ( HAS_CPPDB_PGSQL_DRIVER )

#if defined ( HAS_CPPDB_MYSQL_DRIVER )
bool Database::IsMySqlDriverLoaded()
{
    return Impl::IsMySqlDriverLoaded;
}
#endif  // defined ( HAS_CPPDB_MYSQL_DRIVER )

#if defined ( HAS_CPPDB_SQLITE3_DRIVER )
void Database::LoadSqlite3Driver()
{
    if (!Impl::IsSQLite3DriverLoaded) {
        Impl::IsSQLite3DriverLoaded = true;
        cppdb::driver_manager::instance()
                .install_driver("sqlite3",
                                new cppdb::backend::static_driver(cppdb_sqlite3_get_connection));
    }
}
#endif  // defined ( HAS_CPPDB_SQLITE3_DRIVER )

#if defined ( HAS_CPPDB_PGSQL_DRIVER )
void Database::LoadPgSqlDriver()
{
    if (!Impl::IsSQLite3DriverLoaded) {
        Impl::IsSQLite3DriverLoaded = true;
        cppdb::driver_manager::instance()
                .install_driver("postgresql",
                                new cppdb::backend::static_driver(cppdb_postgresql_get_connection));
    }
}
#endif  // defined ( HAS_CPPDB_PGSQL_DRIVER )

#if defined ( HAS_CPPDB_MYSQL_DRIVER )
void Database::LoadMySqlDriver()
{
    if (!Impl::IsSQLite3DriverLoaded) {
        Impl::IsSQLite3DriverLoaded = true;
        cppdb::driver_manager::instance()
                .install_driver("mysql",
                                new cppdb::backend::static_driver(cppdb_mysql_get_connection));
    }
}
#endif  // defined ( HAS_CPPDB_MYSQL_DRIVER )
#endif  // defined ( CORELIB_STATIC )

#if defined ( HAS_SQLITE3 )
bool Database::Sqlite3Vacuum(const std::string &databaseFile)
{
    sqlite3 *db;

    int rc = sqlite3_open(databaseFile.c_str(), &db);
    if (!rc) {
        sqlite3_exec(db, "VACUUM;", 0, 0, 0);
        return true;
    }

    return false;
}
#endif  // defined ( HAS_SQLITE3 )


Database::Database(const std::string &connectionString) :
    m_pimpl(std::make_unique<Database::Impl>())
{
    if (!m_pimpl->Sql.is_open()) {
        bool isDatabaseOpenedSuccessfully = true;
        try {
            m_pimpl->Sql.open(connectionString);
        } catch (const std::exception &ex) {
            LOG_FATAL("Database connection failed!", ex.what());
            isDatabaseOpenedSuccessfully = false;
        } catch (...) {
            LOG_FATAL("Database connection failed!");
            isDatabaseOpenedSuccessfully = false;
        }
        assert(isDatabaseOpenedSuccessfully);
    }
}

Database::~Database()
{
    if (m_pimpl->Sql.is_open())
        m_pimpl->Sql.close();
}

cppdb::session &Database::Sql()
{
    return m_pimpl->Sql;
}

bool Database::CreateTable(const std::string &id)
{
    try {
        m_pimpl->Sql << "CREATE TABLE IF NOT EXISTS \""
                        + m_pimpl->TableNames[id]
                        + "\" ("
                        + m_pimpl->TableFields[id]
                        + ");"
                     << exec;

        return true;
    } catch (const std::exception &ex) {
        LOG_ERROR(ex.what());
    } catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    return false;
}

bool Database::DropTable(const std::string &id)
{
    try {
        m_pimpl->Sql << "DROP TABLE IF EXISTS \"" + m_pimpl->TableNames[id] + "\";"
                     << exec;

        return true;
    } catch (const std::exception &ex) {
        LOG_ERROR(ex.what());
    } catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    return false;
}

bool Database::RenameTable(const std::string &id, const std::string &newName)
{
    try {
        auto it = m_pimpl->TableNames.find(id);
        if (it != m_pimpl->TableNames.end()) {
            m_pimpl->Sql << "ALTER TABLE [" + m_pimpl->TableNames[id] + "] RENAME TO [" +  newName + "];"
                         << exec;
            it->second = newName;
            return true;
        }
    } catch (const std::exception &ex) {
        LOG_ERROR(ex.what());
    } catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    return false;
}

bool Database::Insert(const std::string &id,
                      const std::string &fields,
                      const std::initializer_list<std::string> &args)
{
    try {
        string ph;
        for (size_t i = 0; i < args.size(); ++i) {
            if (i != 0)
                ph += ", ";
            ph += "?";
        }

        statement stat = m_pimpl->Sql << "INSERT INTO \""
                                         + m_pimpl->TableNames[id]
                                         + "\" ("
                                         + fields
                                         + ") VALUES ("
                                         + ph
                                         + ");";

        for(const auto &arg : args) {
            stat.bind(arg);
        }

        stat.exec();

        return true;
    } catch (const std::exception &ex) {
        LOG_ERROR(ex.what());
    } catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    return false;
}

bool Database::Update(const std::string &id,
                      const std::string &where,
                      const std::string &value,
                      const std::string &set,
                      const std::initializer_list<std::string> &args)
{
    try {
        statement stat = m_pimpl->Sql << "UPDATE ONLY \""
                                         + m_pimpl->TableNames[id]
                                         + "\" SET "
                                         + set
                                         + " WHERE "
                                         + where
                                         + "=?;";

        for(const auto &arg : args) {
            stat.bind(arg);
        }

        stat.bind(value);

        stat.exec();

        return true;
    } catch (const std::exception &ex) {
        LOG_ERROR(ex.what());
    } catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    return false;
}

bool Database::Delete(const std::string &id,
                      const std::string &where,
                      const std::string &value)
{
    try {
        m_pimpl->Sql << "DELETE FROM ONLY \""
                        + m_pimpl->TableNames[id]
                        + "\" WHERE "
                        + where
                        + "=?;"
                     << value
                     << exec;

        return true;
    } catch (const std::exception &ex) {
        LOG_ERROR(ex.what());
    } catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    return false;
}

void Database::RegisterTable(const std::string &id,
                             const std::string &name,
                             const std::string &fields)
{
    m_pimpl->TableNames[id] = name;
    m_pimpl->TableFields[id] = fields;
}

std::string Database::GetTableName(const std::string &id)
{
    if (m_pimpl->TableNames.find(id) != m_pimpl->TableNames.end()) {
        return m_pimpl->TableNames[id];
    }

    return "{?}";
}

std::string Database::GetTableFields(const std::string &id)
{
    if (m_pimpl->TableFields.find(id) != m_pimpl->TableFields.end()) {
        return m_pimpl->TableFields[id];
    }

    return "{?}";
}

bool Database::SetTableName(const std::string &id, const std::string &name)
{
    auto it = m_pimpl->TableNames.find(id);
    if (it != m_pimpl->TableNames.end()) {
        it->second = name;
        return true;
    }

    return false;
}

bool Database::SetTableFields(const std::string &id, const std::string &fields)
{
    auto it = m_pimpl->TableFields.find(id);
    if (it != m_pimpl->TableFields.end()) {
        it->second = fields;
        return true;
    }

    return false;
}

bool Database::Initialize()
{
    try {
        cppdb::transaction guard(m_pimpl->Sql);

        for (Impl::TableNamesHashTable::const_iterator it = m_pimpl->TableNames.begin();
             it != m_pimpl->TableNames.end();
             ++it) {
            CreateTable(it->first);
        }

        guard.commit();

        return true;
    } catch (const std::exception &ex) {
        LOG_ERROR(ex.what());
    } catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    return false;
}

