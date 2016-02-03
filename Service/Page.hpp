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
 * An abstract base class for each web page in the application.
 */


#ifndef SERVICE_PAGE_HPP
#define SERVICE_PAGE_HPP


#include <memory>
#include <string>
#include <Wt/WContainerWidget>

namespace Wt {
class WText;
class WWidget;
}

#include "CgiRoot.hpp"

namespace Service {
class Page;
}

class Service::Page : public Wt::WContainerWidget
{
protected:
    CgiRoot *m_cgiRoot;
    Wt::WContainerWidget *m_htmlRoot;
    CgiRoot::CgiEnv_ptr m_cgiEnv;

public:
    explicit Page(CgiRoot *cgiRoot);
    virtual ~Page() = 0;

protected:
    void Redirect(const std::string &url);
    void Exit(const std::string &url);
    void Exit();

    bool Validate(Wt::WFormWidget *widget);
    void HtmlError(const std::string &err, Wt::WText *txt);
    void HtmlError(const std::wstring &err, Wt::WText *txt);
    void HtmlInfo(const std::string &msg, Wt::WText *txt);
    void HtmlInfo(const std::wstring &msg, Wt::WText *txt);

    virtual Wt::WWidget *Layout() = 0;
};


#endif /* SERVICE_PAGE_HPP */


