/**
 * @file
 * @author  Mamadou Babaei <info@babaei.net>
 * @version 0.1.0
 *
 * @section LICENSE
 *
 * (The MIT License)
 *
 * Copyright (c) 2016 - 2021 Mamadou Babaei
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
 * A (a)synchronous thread-safe mail class with a built-in queue system.
 */


#ifndef CORELIB_MAILER_HPP
#define CORELIB_MAILER_HPP


#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace CoreLib {
class Mail;
}

class CoreLib::Mail
{
public:
    typedef std::function<void(bool, const std::string &)> SendCallback;

private:
    struct Impl;
    std::shared_ptr<Impl> m_pimpl;

public:
    Mail();
    Mail(const std::string &from, const std::string &to,
         const std::string &subject, const std::string &body,
         const std::vector<std::string> &attachments = {  });
    virtual ~Mail();

public:
    std::string GetFrom();
    std::string GetTo();
    std::string GetSubject();
    std::string GetBody();
    const std::vector<std::string> &GetAttachments();
    void SetFrom(const std::string &from);
    void SetTo(const std::string &to);
    void SetSubject(const std::string &subject);
    void SetBody(const std::string &body);
    void SetAttachments(const std::vector<std::string> &attachments);

    bool GetDeleteLater() const;
    void SetDeleteLater(const bool del) const;

public:
    bool Send() const;
    bool Send(std::string &out_error) const;

    void SendAsync(const SendCallback callback = nullptr);
};


#endif /* CORELIB_MAILER_HPP */
