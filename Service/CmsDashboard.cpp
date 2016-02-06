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
 * CMS welcome screen a.k.a. dashboard.
 */


#include <boost/exception/diagnostic_information.hpp>
#include <boost/format.hpp>
#include <Wt/WString>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WWidget>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Log.hpp>
#include "CgiEnv.hpp"
#include "CmsDashboard.hpp"
#include "Div.hpp"
#include "Pool.hpp"

#define         UNKNOWN_ERROR       "Unknown error!"

using namespace std;
using namespace Wt;
using namespace CoreLib;
using namespace Service;

struct CmsDashboard::Impl : public Wt::WObject
{
public:
    Impl();
    ~Impl();
};

CmsDashboard::CmsDashboard()
    : Page(),
    m_pimpl(make_unique<CmsDashboard::Impl>())
{
    this->clear();
    this->setId("CmsDashboardPage");
    this->addWidget(Layout());
}

CmsDashboard::~CmsDashboard() = default;

WWidget *CmsDashboard::Layout()
{
    Div *container = new Div("CmsDashboard", "container-fluid");

    try {
        CgiEnv *cgiEnv = CgiEnv::GetInstance();

        string htmlData;
        string file;
        if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
            file = "../templates/cms-dashboard-fa.wtml";
        } else {
            file = "../templates/cms-dashboard.wtml";
        }

        if (CoreLib::FileSystem::Read(file, htmlData)) {
            /// Fill the template
            WTemplate *tmpl = new WTemplate(container);
            tmpl->setTemplateText(WString(htmlData), TextFormat::XHTMLUnsafeText);

            tmpl->bindWidget("welcome-message", new WText(tr("cms-dashboard-welcome-message")));

            tmpl->bindWidget("last-login-title", new WText(tr("cms-dashboard-last-login-info-title")));
            tmpl->bindWidget("last-login-ip-label", new WText(tr("cms-dashboard-last-login-info-ip")));
            tmpl->bindWidget("last-login-location-label", new WText(tr("cms-dashboard-last-login-info-location")));
            tmpl->bindWidget("last-login-user-agent-label", new WText(tr("cms-dashboard-last-login-info-user-agent")));
            tmpl->bindWidget("last-login-referer-label", new WText(tr("cms-dashboard-last-login-info-referer")));
            tmpl->bindWidget("last-login-time-label", new WText(tr("cms-dashboard-last-login-info-time")));

            tmpl->bindWidget("last-login-ip", new WText(WString::fromUTF8(cgiEnv->SignedInUser.LastLogin.IP)));
            tmpl->bindWidget("last-login-location", new WText(WString::fromUTF8(cgiEnv->SignedInUser.LastLogin.Location)));
            tmpl->bindWidget("last-login-user-agent", new WText(WString::fromUTF8(cgiEnv->SignedInUser.LastLogin.UserAgent)));
            tmpl->bindWidget("last-login-referer", new WText(WString::fromUTF8(cgiEnv->SignedInUser.LastLogin.Referer)));
            tmpl->bindWidget("last-login-time-gdate", new WText(WString::fromUTF8(cgiEnv->SignedInUser.LastLogin.LoginGDate)));
            tmpl->bindWidget("last-login-time-jdate", new WText(WString::fromUTF8(cgiEnv->SignedInUser.LastLogin.LoginJDate)));
            tmpl->bindWidget("last-login-time", new WText(WString::fromUTF8(cgiEnv->SignedInUser.LastLogin.LoginTime)));
        }
    }

    catch (boost::exception &ex) {
        LOG_ERROR(boost::diagnostic_information(ex));
    }

    catch (std::exception &ex) {
        LOG_ERROR(ex.what());
    }

    catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    return container;
}

CmsDashboard::Impl::Impl()
{

}

CmsDashboard::Impl::~Impl() = default;

