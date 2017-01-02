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
 * Main entry for the web application. When a user visits the website, this is
 * the first thing that runs.
 */


#ifndef SERVICE_CGIROOT_HPP
#define SERVICE_CGIROOT_HPP


#include <memory>
#include <Wt/WApplication>

namespace Wt {
class WEnvironment;
}

namespace Service {
class CgiEnv;
class CgiRoot;
}

class Service::CgiRoot : public Wt::WApplication
{
private:
    struct Impl;
    std::unique_ptr<Impl> m_pimpl;

public:
    static Wt::WApplication *CreateApplication(const Wt::WEnvironment &env);

public:
    explicit CgiRoot(const Wt::WEnvironment &env);
    virtual ~CgiRoot();

public:
    CgiEnv *GetCgiEnvInstance();

    void Exit(const std::string &url);
};


#endif /* SERVICE_CGIROOT_HPP */
