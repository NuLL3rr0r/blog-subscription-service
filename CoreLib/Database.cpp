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
 * Database accessibility wrapper on top of libpqxx and libpq with support
 * for PostgreSQL.
 */


#include <sstream>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>
#include <libpq-fe.h>
#include <pqxx/pqxx>
#include "make_unique.hpp"
#include "Database.hpp"
#include "Log.hpp"
#include "SharedObjectPool.hpp"

#define     MAX_DATABASE_CONNECTIONS    16
#define     QUERY_SUCCEED               "CoreLib::Database ==>  Query succeed!"
#define     UNKNOWN_ERROR               "Unknow database error!"

using namespace std;
using namespace boost;
using namespace CoreLib;

struct Database::Impl
{
    typedef std::unordered_map<std::string, std::string> EnumNamesHashTable;
    typedef std::unordered_map<std::string, std::vector<std::string>> EnumeratorsHashTable;

    typedef std::unordered_map<std::string, std::string> TableNamesHashTable;
    typedef std::unordered_map<std::string, std::string> TableFieldsHashTable;

    SharedObjectPool<pqxx::connection> Connections;
    boost::mutex ConnectionsMutex;

    EnumNamesHashTable EnumNames;
    EnumeratorsHashTable Enumerators;

    TableNamesHashTable TableNames;
    TableFieldsHashTable TableFields;
};

std::string Database::Escape(const char *begin, const char *end)
{
    std::string result;
    result.reserve(static_cast<size_t>(end - begin));

    for(; begin != end; ++begin) {
        char c = *begin;

        if(c == '\'') {
            result += "''";
        } else {
            result += c;
        }
    }

    return result;
}

std::string Database::Escape(const char *str)
{
    return Escape(str, str + strlen(str));
}

std::string Database::Escape(const std::string &str)
{
    return Escape(str.c_str(), str.c_str() + str.size());
}

bool Database::IsTrue(const std::string &value)
{
    if (value == "TRUE"
            || value == "t"
            || value == "true"
            || value == "y"
            || value == "yes"
            || value == "on"
            || value == "1") {
        return true;
    }

    return false;
}

Database::Database(const std::string &connectionString) :
    m_pimpl(make_unique<Database::Impl>())
{
    boost::lock_guard<boost::mutex> lock(m_pimpl->ConnectionsMutex);
    (void)lock;

    LOG_INFO("Setting up database connections...");

    for (int i = 0; i < MAX_DATABASE_CONNECTIONS; ++i) {
        try {
            std::unique_ptr<pqxx::connection> c(
                        std::make_unique<pqxx::connection>(connectionString));

            LOG_INFO((format("Database connection #%1% succeed!") % i).str(), (boost::format("Backend PID: %1%") % c->backendpid()).str(), (boost::format("Socket: %1%") % c->sock()).str(), (boost::format("Host Name: %1%") % c->hostname()).str(), (boost::format("Port Number: %1%") % c->port()).str(), (boost::format("Database Name: %1%") % c->dbname()).str(), (boost::format("User Name: %1%") % c->username()).str());

            m_pimpl->Connections.Add(c);
        } catch (const pqxx::sql_error &ex) {
            LOG_FATAL((format("Database connection #%1% failed!") % i).str(), ex.what());
        } catch (const std::exception &ex) {
            LOG_FATAL((format("Database connection #%1% failed!") % i).str(), ex.what());
        } catch (...) {
            LOG_FATAL((format("Database connection #%1% failed!") % i).str(), UNKNOWN_ERROR);
        }
    }

    LOG_INFO("Database connections setup successfully!");
}

Database::~Database()
{
    boost::lock_guard<boost::mutex> lock(m_pimpl->ConnectionsMutex);
    (void)lock;

    size_t i = 0;
    while (!m_pimpl->Connections.Empty()) {
        try {
            auto c(m_pimpl->Connections.Pool().top().release());

            LOG_INFO((format("Database connection #%1% disconnected successfully!") % i).str(), (boost::format("Backend PID: %1%") % c->backendpid()).str(), (boost::format("Socket: %1%") % c->sock()).str(), (boost::format("Host Name: %1%") % c->hostname()).str(), (boost::format("Port Number: %1%") % c->port()).str(), (boost::format("Database Name: %1%") % c->dbname()).str(), (boost::format("User Name: %1%") % c->username()).str());

            m_pimpl->Connections.Pool().pop();
            ++i;
        } catch (const pqxx::sql_error &ex) {
            LOG_ERROR((format("Failed to disconnect from connection #%1%!") % i).str(), ex.what());
        } catch (const std::exception &ex) {
            LOG_ERROR((format("Failed to disconnect from connection #%1%!") % i).str(), ex.what());
        } catch (...) {
            LOG_ERROR((format("Failed to disconnect from connection #%1%!") % i).str(), UNKNOWN_ERROR);
        }
    }
}

SharedObjectPool<pqxx::connection>::ptrType Database::Connection()
{
    for (;;) {
        size_t connectionNumber = MAX_DATABASE_CONNECTIONS - m_pimpl->Connections.Size();

        try {
            if (!m_pimpl->Connections.Empty()) {
                auto c(m_pimpl->Connections.Acquire());

                LOG_INFO((format("Acquired connection #%1% successfully!") % connectionNumber).str(), (boost::format("Backend PID: %1%") % c->backendpid()).str(), (boost::format("Socket: %1%") % c->sock()).str(), (boost::format("Host Name: %1%") % c->hostname()).str(), (boost::format("Port Number: %1%") % c->port()).str(), (boost::format("Database Name: %1%") % c->dbname()).str(), (boost::format("User Name: %1%") % c->username()).str());

                return c;
            } else {
                LOG_WARNING("No free connection is available! Retrying...");
            }
        } catch (const pqxx::sql_error &ex) {
            LOG_ERROR((format("Connection #%1% acquisition failed!") % connectionNumber).str(), ex.what());
        } catch (const std::exception &ex) {
            LOG_ERROR((format("Connection #%1% acquisition failed!") % connectionNumber).str(), ex.what());
        } catch (const char &ex) {
            /// To make Clang stop spitting out warnings about the unreached code below
            (void)ex;
            break;
        } catch (...) {
            LOG_ERROR((format("Connection #%1% acquisition failed!") % connectionNumber).str(), UNKNOWN_ERROR);
        }
    }

    /// The execution should never reach here
    /// Return an empty connection
    SharedObjectPool<pqxx::connection>::ptrType c;
    return c;
}

bool Database::CreateEnum(const std::string &id)
{
    try {
        auto c = this->Connection();
        pqxx::work txn(*c.get());

        pqxx::result r = txn.exec((format("SELECT EXISTS ( SELECT typname FROM pg_type WHERE typname = %1% );")
                                   % txn.quote(m_pimpl->EnumNames[id])).str());

        LOG_INFO(QUERY_SUCCEED, r.query());

        if (!r.empty()) {
            std::string exists(r[0][0].as<string>());

            if (exists == "f") {
                std::string ph;
                for (size_t i = 0; i < m_pimpl->Enumerators[id].size(); ++i) {
                    if (i != 0)
                        ph += ", ";
                    ph += txn.quote(m_pimpl->Enumerators[id][i]);
                }

                r = txn.exec((format("CREATE TYPE \"%1%\" AS ENUM ( %2% );")
                          % txn.esc(m_pimpl->EnumNames[id])
                          % ph).str());

                LOG_INFO(QUERY_SUCCEED, r.query());

                txn.commit();
            }

            return true;
        }
    } catch (const pqxx::sql_error &ex) {
        LOG_ERROR(ex.what(), ex.query());
    } catch (const std::exception &ex) {
        LOG_ERROR(ex.what());
    } catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    return false;
}

bool Database::CreateTable(const std::string &id)
{
    try {
        auto c = this->Connection();
        pqxx::work txn(*c.get());

        pqxx::result r = txn.exec((format("CREATE TABLE IF NOT EXISTS \"%1%\" ( %2% );")
                  % txn.esc(m_pimpl->TableNames[id])
                  % m_pimpl->TableFields[id]).str());

        LOG_INFO(QUERY_SUCCEED, r.query());

        txn.commit();

        return true;
    } catch (const pqxx::sql_error &ex) {
        LOG_ERROR(ex.what(), ex.query());
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
        auto c = this->Connection();
        pqxx::work txn(*c.get());

        pqxx::result r = txn.exec((format("DROP TABLE IF EXISTS \"%1%\";")
                  % txn.esc(m_pimpl->TableNames[id])).str());

        LOG_INFO(QUERY_SUCCEED, r.query());

        txn.commit();

        return true;
    } catch (const pqxx::sql_error &ex) {
        LOG_ERROR(ex.what(), ex.query());
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
            auto c = this->Connection();
            pqxx::work txn(*c.get());

            pqxx::result r = txn.exec((format("ALTER TABLE \"%1%\" RENAME TO \"%2%\";")
                      % txn.esc(m_pimpl->TableNames[id])
                      % txn.esc(newName)).str());

            LOG_INFO(QUERY_SUCCEED, r.query());

            txn.commit();

            it->second = newName;

            return true;
        }
    } catch (const pqxx::sql_error &ex) {
        LOG_ERROR(ex.what(), ex.query());
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
        auto c = this->Connection();
        pqxx::work txn(*c.get());

        stringstream ss;
        ss << (format("INSERT INTO \"%1%\" ( %2% ) VALUES ( ")
               % txn.esc(m_pimpl->TableNames[id])
               % txn.esc(fields)).str();

        size_t i = 0;
        for(const auto &arg : args) {
            if (i != 0) {
                ss << ", ";
            }
            ss << txn.quote(arg);
            ++i;
        }

        ss << ");";

        pqxx::result r = txn.exec(ss.str());

        LOG_INFO(QUERY_SUCCEED, r.query());

        txn.commit();

        return true;
    } catch (const pqxx::sql_error &ex) {
        LOG_ERROR(ex.what(), ex.query());
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
        auto c = this->Connection();
        pqxx::work txn(*c.get());

        string processedSet;
        for(const auto &arg : args) {
            processedSet = boost::replace_nth_copy(set, "?", 0, txn.quote(arg));
        }

        pqxx::result r = txn.exec((format("UPDATE ONLY \"%1%\" SET %2% WHERE \"%3%\" = %4%;")
                  % txn.esc(m_pimpl->TableNames[id])
                  % processedSet
                  % txn.esc(where)
                  % txn.quote(value)).str());

        LOG_INFO(QUERY_SUCCEED, r.query());

        txn.commit();

        return true;
    } catch (const pqxx::sql_error &ex) {
        LOG_ERROR(ex.what(), ex.query());
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
        auto c = this->Connection();
        pqxx::work txn(*c.get());

        pqxx::result r = txn.exec((format("DELETE FROM ONLY \"%1%\" WHERE \"%2%\"=%3%;")
                  % txn.esc(m_pimpl->TableNames[id])
                  % txn.esc(where)
                  % txn.quote(value)).str());

        LOG_INFO(QUERY_SUCCEED, r.query());

        txn.commit();

        return true;
    } catch (const pqxx::sql_error &ex) {
        LOG_ERROR(ex.what(), ex.query());
    } catch (const std::exception &ex) {
        LOG_ERROR(ex.what());
    } catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    return false;
}

void Database::RegisterEnum(const std::string &id,
                            const std::string &name,
                            const std::initializer_list<std::string> &enumerators)
{
    m_pimpl->EnumNames[id] = name;
    m_pimpl->Enumerators[id] = enumerators;
}

void Database::RegisterTable(const std::string &id,
                             const std::string &name,
                             const std::string &fields)
{
    m_pimpl->TableNames[id] = name;
    m_pimpl->TableFields[id] = fields;
}

std::string Database::GetTableName(const std::string &id) const
{
    if (m_pimpl->TableNames.find(id) != m_pimpl->TableNames.end()) {
        return m_pimpl->TableNames[id];
    }

    return "{?}";
}

std::string Database::GetTableFields(const std::string &id) const
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
    LOG_INFO("Initializing CoreLib::Database...");

    try {
        for (const auto &e : m_pimpl->EnumNames) {
            CreateEnum(e.first);
        }

        for (const auto &t : m_pimpl->TableNames) {
            CreateTable(t.first);
        }

        LOG_INFO("CoreLib::Crypto initialized successfully!");

        return true;
    } catch (const pqxx::sql_error &ex) {
        LOG_ERROR(ex.what(), ex.query());
    } catch (const std::exception &ex) {
        LOG_ERROR(ex.what());
    } catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    return false;
}
