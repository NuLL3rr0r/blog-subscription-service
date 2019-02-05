/**
 * @file
 * @author  Mamadou Babaei <info@babaei.net>
 * @version 0.1.0
 *
 * @section LICENSE
 *
 * (The MIT License)
 *
 * Copyright (c) 2016 - 2019 Mamadou Babaei
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
 * A smart thread-safe object pool.
 */


#ifndef CORELIB_SHARED_OBJECT_POOL_HPP
#define CORELIB_SHARED_OBJECT_POOL_HPP


#include <memory>
#include <stack>
#include <string>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>
#include "Exception.hpp"

namespace CoreLib {
template <typename _T, typename _D = std::default_delete<_T>>
class SharedObjectPool;
}

template <typename _T, typename _D>
class CoreLib::SharedObjectPool
{
private:
    struct ReturnToPoolDeleter {
    private:
        std::weak_ptr<SharedObjectPool<_T, _D> *> m_pool;

    public:
        explicit ReturnToPoolDeleter(const std::weak_ptr<SharedObjectPool<_T, _D> *> pool = { })
            : m_pool(pool) {

        }

        void operator()(_T *ptr) {
            if (auto poolPtr = m_pool.lock()) {
                std::unique_ptr<_T, _D> uptr{ptr};
                (*poolPtr.get())->Add(uptr);
            } else {
                _D{}(ptr);
            }
        }
    };

public:
    using ptrType = std::unique_ptr<_T, ReturnToPoolDeleter>;

private:
    std::shared_ptr<SharedObjectPool<_T, _D> *> m_thisPtr;
    std::stack<std::unique_ptr<_T, _D>> m_pool;
    boost::mutex m_mutex;

public:
    SharedObjectPool()
        : m_thisPtr(std::make_shared<SharedObjectPool<_T, _D> *>(this)) {

    }

    virtual ~SharedObjectPool(){

    }

public:
    void Add(std::unique_ptr<_T, _D> &uptr) {
        boost::lock_guard<boost::mutex> lock(m_mutex);
        (void)lock;

        m_pool.push(std::move(uptr));
    }

    ptrType Acquire() {
        boost::lock_guard<boost::mutex> lock(m_mutex);
        (void)lock;

        if (m_pool.empty()) {
            throw CoreLib::Exception<std::string>("Cannot acquire object from an empty pool.");
        }

        ptrType tmp(m_pool.top().release(),
                    ReturnToPoolDeleter{
                        std::weak_ptr<SharedObjectPool<_T, _D> *>{m_thisPtr}});
        m_pool.pop();

        return std::move(tmp);
    }

    bool Empty() const
    {
        return m_pool.empty();
    }

    std::size_t Size() const
    {
        return m_pool.size();
    }

    std::stack<std::unique_ptr<_T, _D>> &Pool()
    {
        return m_pool;
    }
};


#endif /* CORELIB_SHARED_OBJECT_POOL_HPP */
