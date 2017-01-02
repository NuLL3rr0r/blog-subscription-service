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
#include <Wt/WApplication>
#include <Wt/WMessageBox>
#include <Wt/WPushButton>
#include <Wt/WString>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WWidget>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Database.hpp>
#include <CoreLib/Log.hpp>
#include "CgiEnv.hpp"
#include "CgiRoot.hpp"
#include "CmsDashboard.hpp"
#include "Div.hpp"
#include "Pool.hpp"

using namespace std;
using namespace boost;
using namespace cppdb;
using namespace Wt;
using namespace CoreLib;
using namespace Service;

struct CmsDashboard::Impl : public Wt::WObject
{
public:
    std::unique_ptr<Wt::WMessageBox> ForceTerminateAllSessionsMessageBox;
    std::unique_ptr<Wt::WMessageBox> SessionTerminationScheduledMessageBox;

public:
    Impl();
    ~Impl();

public:
    void OnForceTerminateAllSessionsPushButtonPressed();
    void OnForceTerminateAllSessionsDialogClosed(Wt::StandardButton button);
    void OnSessionTerminationScheduledDialogClosed(Wt::StandardButton button);
};

CmsDashboard::CmsDashboard()
    : Page(),
    m_pimpl(make_unique<CmsDashboard::Impl>())
{
    this->clear();
    this->setId("CmsDashboardPage");
    this->addWidget(this->Layout());
}

CmsDashboard::~CmsDashboard() = default;

WWidget *CmsDashboard::Layout()
{
    Div *container = new Div("CmsDashboard", "container-fluid");

    try {
        CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
        CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

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
            tmpl->setTemplateText(WString::fromUTF8(htmlData), TextFormat::XHTMLUnsafeText);


            WPushButton *forceTerminateAllSessionsPushButton = new WPushButton(tr("cms-dashboard-force-terminate-all-sessions"));
            forceTerminateAllSessionsPushButton->setStyleClass("btn btn-default");

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

            tmpl->bindWidget("force-terminate-all-sessions", forceTerminateAllSessionsPushButton);

            forceTerminateAllSessionsPushButton->clicked().connect(m_pimpl.get(), &CmsDashboard::Impl::OnForceTerminateAllSessionsPushButtonPressed);
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

void CmsDashboard::Impl::OnForceTerminateAllSessionsPushButtonPressed()
{
    ForceTerminateAllSessionsMessageBox =
            std::make_unique<WMessageBox>(tr("cms-dashboard-force-terminate-all-sessions-confirm-title"),
                                              tr("cms-dashboard-force-terminate-all-sessions-confirm-question"), Warning, NoButton);

    ForceTerminateAllSessionsMessageBox->addButton(tr("cms-dashboard-force-terminate-all-sessions-confirm-ok"), Ok);
    ForceTerminateAllSessionsMessageBox->addButton(tr("cms-dashboard-force-terminate-all-sessions-confirm-cancel"), Cancel);

    ForceTerminateAllSessionsMessageBox->buttonClicked().connect(this, &CmsDashboard::Impl::OnForceTerminateAllSessionsDialogClosed);

    ForceTerminateAllSessionsMessageBox->show();
}

void CmsDashboard::Impl::OnForceTerminateAllSessionsDialogClosed(Wt::StandardButton button)
{
    try {
        if (button == Ok) {

            transaction guard(Service::Pool::Database()->Sql());

            Pool::Database()->Sql()
                        << (format("UPDATE \"%1%\""
                                   " SET expiry='0';")
                           % Pool::Database()->GetTableName("ROOT_SESSIONS")).str()
                        << exec;

            guard.commit();

            ForceTerminateAllSessionsMessageBox.reset();

            SessionTerminationScheduledMessageBox =
                    std::make_unique<WMessageBox>(tr("cms-dashboard-force-terminate-all-sessions-has-been-scheduled-title"),
                                                  tr("cms-dashboard-force-terminate-all-sessions-has-been-scheduled-message"),
                                                  Warning, NoButton);
            SessionTerminationScheduledMessageBox->addButton(tr("cms-dashboard-force-terminate-all-sessions-has-been-scheduled-ok"), Ok);

            SessionTerminationScheduledMessageBox->buttonClicked().connect(
                        this, &CmsDashboard::Impl::OnSessionTerminationScheduledDialogClosed);

            SessionTerminationScheduledMessageBox->show();

            return;
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

    ForceTerminateAllSessionsMessageBox.reset();
}

void CmsDashboard::Impl::OnSessionTerminationScheduledDialogClosed(Wt::StandardButton button)
{
    (void)button;

    SessionTerminationScheduledMessageBox.reset();
}
