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


#ifndef CORELIB_DATABASE_HPP
#define CORELIB_DATABASE_HPP


#include <memory>
#include <string>
#include <pqxx/connection>
#include "SharedObjectPool.hpp"

namespace CoreLib {
class Database;
}

class CoreLib::Database
{
private:
    struct Impl;
    std::unique_ptr<Impl> m_pimpl;

public:
    static std::string Escape(const char *begin, const char *end);
    static std::string Escape(const char *str);
    static std::string Escape(const std::string &str);

    static bool IsTrue(const std::string &value);

public:
    explicit Database(const std::string &connectionString);
    virtual ~Database();

    SharedObjectPool<pqxx::connection>::ptrType Connection();

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
