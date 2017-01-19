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
#include <pqxx/pqxx>
#include <Wt/WApplication>
#include <Wt/WMessageBox>
#include <Wt/WPushButton>
#include <Wt/WString>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WWidget>
#include <CoreLib/CDate.hpp>
#include <CoreLib/Database.hpp>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Log.hpp>
#include "CgiEnv.hpp"
#include "CgiRoot.hpp"
#include "CmsDashboard.hpp"
#include "Div.hpp"
#include "Pool.hpp"

using namespace std;
using namespace boost;
using namespace pqxx;
using namespace Wt;
using namespace CoreLib;
using namespace CoreLib::CDate;
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

    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    try {
        string htmlData;
        string file;
        if (cgiEnv->GetInformation().Client.Language.Code
                == CgiEnv::InformationRecord::ClientRecord::LanguageCode::En) {
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
            tmpl->bindWidget("last-login-user-agent-label", new WText(tr("cms-dashboard-last-login-info-user-agent")));
            tmpl->bindWidget("last-login-referer-label", new WText(tr("cms-dashboard-last-login-info-referer")));
            tmpl->bindWidget("last-login-time-label", new WText(tr("cms-dashboard-last-login-info-time")));
            tmpl->bindWidget("last-login-location-label", new WText(tr("cms-dashboard-last-login-info-location")));
            tmpl->bindWidget("last-login-location-country-code-label", new WText(tr("cms-dashboard-last-login-info-location-country-code")));
            tmpl->bindWidget("last-login-location-country-code3-label", new WText(tr("cms-dashboard-last-login-info-location-country-code3")));
            tmpl->bindWidget("last-login-location-country-name-label", new WText(tr("cms-dashboard-last-login-info-location-country-name")));
            tmpl->bindWidget("last-login-location-region-label", new WText(tr("cms-dashboard-last-login-info-location-region")));
            tmpl->bindWidget("last-login-location-city-label", new WText(tr("cms-dashboard-last-login-info-location-city")));
            tmpl->bindWidget("last-login-location-postal-code-label", new WText(tr("cms-dashboard-last-login-info-location-postal-code")));
            tmpl->bindWidget("last-login-location-latitude-label", new WText(tr("cms-dashboard-last-login-info-location-latitude")));
            tmpl->bindWidget("last-login-location-longitude-label", new WText(tr("cms-dashboard-last-login-info-location-longitude")));
            tmpl->bindWidget("last-login-location-metro-code-label", new WText(tr("cms-dashboard-last-login-info-location-metro-code")));
            tmpl->bindWidget("last-login-location-dma-code-label", new WText(tr("cms-dashboard-last-login-info-location-dma-code")));
            tmpl->bindWidget("last-login-location-area-code-label", new WText(tr("cms-dashboard-last-login-info-location-area-code")));
            tmpl->bindWidget("last-login-location-charset-label", new WText(tr("cms-dashboard-last-login-info-location-charset")));
            tmpl->bindWidget("last-login-location-continent-code-label", new WText(tr("cms-dashboard-last-login-info-location-continent-code")));
            tmpl->bindWidget("last-login-location-netmask-label", new WText(tr("cms-dashboard-last-login-info-location-netmask")));

            tmpl->bindWidget("last-login-ip", new WText(WString::fromUTF8(cgiEnv->GetInformation().Client.Session.LastLogin.IPAddress)));
            tmpl->bindWidget("last-login-user-agent", new WText(WString::fromUTF8(cgiEnv->GetInformation().Client.Session.LastLogin.UserAgent)));
            tmpl->bindWidget("last-login-referer", new WText(WString::fromUTF8(cgiEnv->GetInformation().Client.Session.LastLogin.Referer)));
            tmpl->bindWidget("last-login-time",new WText(
                                 WString::fromUTF8(DateConv::DateTimeString(cgiEnv->GetInformation().Client.Session.LastLogin.Time,
                                                                            CDate::Timezone::UTC))));
            tmpl->bindWidget("last-login-location-country-code", new WText(
                                 WString::fromUTF8(cgiEnv->GetInformation().Client.Session.LastLogin.GeoLocation.CountryCode)));
            tmpl->bindWidget("last-login-location-country-code3", new WText(
                                 WString::fromUTF8(cgiEnv->GetInformation().Client.Session.LastLogin.GeoLocation.CountryCode3)));
            tmpl->bindWidget("last-login-location-country-name", new WText(
                                 WString::fromUTF8(cgiEnv->GetInformation().Client.Session.LastLogin.GeoLocation.CountryName)));
            tmpl->bindWidget("last-login-location-region", new WText(
                                 WString::fromUTF8(cgiEnv->GetInformation().Client.Session.LastLogin.GeoLocation.Region)));
            tmpl->bindWidget("last-login-location-city", new WText(
                                 WString::fromUTF8(cgiEnv->GetInformation().Client.Session.LastLogin.GeoLocation.City)));
            tmpl->bindWidget("last-login-location-postal-code", new WText(
                                 WString::fromUTF8(cgiEnv->GetInformation().Client.Session.LastLogin.GeoLocation.PostalCode)));
            tmpl->bindWidget("last-login-location-latitude", new WText(
                                 WString::fromUTF8(lexical_cast<string>(cgiEnv->GetInformation().Client.Session.LastLogin.GeoLocation.Latitude))));
            tmpl->bindWidget("last-login-location-longitude", new WText(
                                 WString::fromUTF8(lexical_cast<string>(cgiEnv->GetInformation().Client.Session.LastLogin.GeoLocation.Longitude))));
            tmpl->bindWidget("last-login-location-metro-code", new WText(
                                 WString::fromUTF8(lexical_cast<string>(cgiEnv->GetInformation().Client.Session.LastLogin.GeoLocation.MetroCode))));
            tmpl->bindWidget("last-login-location-dma-code", new WText(
                                 WString::fromUTF8(lexical_cast<string>(cgiEnv->GetInformation().Client.Session.LastLogin.GeoLocation.DmaCode))));
            tmpl->bindWidget("last-login-location-area-code", new WText(
                                 WString::fromUTF8(lexical_cast<string>(cgiEnv->GetInformation().Client.Session.LastLogin.GeoLocation.AreaCode))));
            tmpl->bindWidget("last-login-location-charset", new WText(
                                 WString::fromUTF8(lexical_cast<string>(cgiEnv->GetInformation().Client.Session.LastLogin.GeoLocation.Charset))));
            tmpl->bindWidget("last-login-location-continent-code", new WText(
                                 WString::fromUTF8(cgiEnv->GetInformation().Client.Session.LastLogin.GeoLocation.ContinentCode)));
            tmpl->bindWidget("last-login-location-netmask", new WText(
                                 WString::fromUTF8(lexical_cast<string>(cgiEnv->GetInformation().Client.Session.LastLogin.GeoLocation.Netmask))));

            tmpl->bindWidget("force-terminate-all-sessions", forceTerminateAllSessionsPushButton);

            forceTerminateAllSessionsPushButton->clicked().connect(m_pimpl.get(), &CmsDashboard::Impl::OnForceTerminateAllSessionsPushButtonPressed);
       }
    }

    catch (const boost::exception &ex) {
        LOG_ERROR(boost::diagnostic_information(ex), cgiEnv->GetInformation().ToJson());
    }

    catch (const std::exception &ex) {
        LOG_ERROR(ex.what(), cgiEnv->GetInformation().ToJson());
    }

    catch (...) {
        LOG_ERROR(UNKNOWN_ERROR, cgiEnv->GetInformation().ToJson());
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
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    try {
        if (button == Ok) {

            auto conn = Pool::Database().Connection();
            conn->activate();
            pqxx::work txn(*conn.get());

            string query((boost::format("UPDATE ONLY \"%1%\""
                                        " SET expiry = '19700101'::TIMESTAMPTZ"
                                        " WHERE user_id = %2% AND expiry > '19700101'::TIMESTAMPTZ;")
                          % txn.esc(Service::Pool::Database().GetTableName("ROOT_SESSIONS"))
                          % txn.quote(cgiEnv->GetInformation().Client.Session.UserId)).str());
            LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

            txn.exec(query);

            txn.commit();

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

    catch (const pqxx::sql_error &ex) {
        LOG_ERROR(ex.what(), ex.query(), cgiEnv->GetInformation().ToJson());
    }

    catch (const boost::exception &ex) {
        LOG_ERROR(boost::diagnostic_information(ex), cgiEnv->GetInformation().ToJson());
    }

    catch (const std::exception &ex) {
        LOG_ERROR(ex.what(), cgiEnv->GetInformation().ToJson());
    }

    catch (...) {
        LOG_ERROR(UNKNOWN_ERROR, cgiEnv->GetInformation().ToJson());
    }

    ForceTerminateAllSessionsMessageBox.reset();
}

void CmsDashboard::Impl::OnSessionTerminationScheduledDialogClosed(Wt::StandardButton button)
{
    (void)button;

    SessionTerminationScheduledMessageBox.reset();
}
