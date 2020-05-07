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
 * The CMS start page.
 */


#include <ctime>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/format.hpp>
#include <pqxx/pqxx>
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WSignalMapper>
#include <Wt/WStackedWidget>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WTimer>
#include <Wt/WWidget>
#include <CoreLib/CDate.hpp>
#include <CoreLib/Database.hpp>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/make_unique.hpp>
#include <CoreLib/Log.hpp>
#include <CoreLib/Random.hpp>
#include <CoreLib/System.hpp>
#include "CgiEnv.hpp"
#include "CgiRoot.hpp"
#include "Cms.hpp"
#include "CmsChangeEmail.hpp"
#include "CmsChangePassword.hpp"
#include "CmsContacts.hpp"
#include "CmsDashboard.hpp"
#include "CmsNewsletter.hpp"
#include "CmsSettings.hpp"
#include "CmsSubscribers.hpp"
#include "Div.hpp"
#include "Pool.hpp"
#include "SysMon.hpp"

using namespace std;
using namespace boost;
using namespace pqxx;
using namespace Wt;
using namespace CoreLib;
using namespace Service;

struct Cms::Impl : public Wt::WObject
{
public:
    WStackedWidget *Contents;
    WText *LastSelectedMenuItem;

    SysMon *SystemMonitor;

public:
    Impl();
    ~Impl();

public:
    void OnMenuItemPressed(WText *sender);

public:
    void ValidateSession();

private:
    void ForceExit();
};

Cms::Cms()
    : Page(),
    m_pimpl(make_unique<Cms::Impl>())
{
    WApplication *app = WApplication::instance();
    app->setTitle(tr("cms-page-title"));

    this->clear();
    this->setId("CmsPage");
    this->setStyleClass("cms-page container-fluid");
    this->addWidget(this->Layout());

    app->root()->clear();
    app->root()->addWidget(this);

    WTimer *timer = new WTimer(this);
    timer->setInterval(60000);       // every one minute
    timer->timeout().connect(m_pimpl.get(), &Cms::Impl::ValidateSession);
    timer->start();
}

Cms::~Cms() = default;

WWidget *Cms::Layout()
{
    Div *container = new Div("Cms", "cms-layout container-fluid");
    Div *noScript = new Div(container);
    noScript->addWidget(new WText(tr("no-script")));

    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    string htmlData;
    string file;
    if (cgiEnv->GetInformation().Client.Language.Code
            == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
        file = "../templates/cms-fa.wtml";
    } else {
        file = "../templates/cms.wtml";
    }

    if (CoreLib::FileSystem::Read(file, htmlData)) {
        WTemplate *tmpl = new WTemplate(container);
        tmpl->setTemplateText(WString::fromUTF8(htmlData), TextFormat::XHTMLUnsafeText);

        tmpl->bindWidget("brand-title", new WText(tr("cms-page-title")));

        WText *dashboard = new WText(
                    WString("<div><a href=\"javascript:;\"><i class=\"fa fa-dashboard fa-lg\"></i> {1}</a></div>")
                    .arg(tr("cms-dashboard")), TextFormat::XHTMLUnsafeText);
        WText *newsletter = new WText(
                    WString("<div><a href=\"javascript:;\"><i class=\"fa fa-newspaper-o fa-lg\"></i> {1}</a></div>")
                    .arg(tr("cms-dashboard-newsletter")), TextFormat::XHTMLUnsafeText);
        WText *subscribers = new WText(
                    WString("<div><a href=\"javascript:;\"><i class=\"fa fa-users fa-lg\"></i> {1}</a></div>")
                    .arg(tr("cms-dashboard-subscribers")), TextFormat::XHTMLUnsafeText);
        WText *contacts = new WText(
                    WString("<div><a href=\"javascript:;\"><i class=\"fa fa-envelope-o fa-lg\"></i> {1}</a></div>")
                    .arg(tr("cms-dashboard-contacts")), TextFormat::XHTMLUnsafeText);
        WText *settings = new WText(
                    WString("<div><a href=\"javascript:;\"><i class=\"fa fa-gears fa-lg\"></i> {1}</a></div>")
                    .arg(tr("cms-dashboard-settings")), TextFormat::XHTMLUnsafeText);
        WText *changeEmail = new WText(
                    WString("<div><a href=\"javascript:;\"><i class=\"fa fa-envelope-o fa-lg\"></i> {1}</a></div>")
                    .arg(tr("cms-dashboard-admin-change-email")), TextFormat::XHTMLUnsafeText);
        WText *changePassword = new WText(
                    WString("<div><a href=\"javascript:;\"><i class=\"fa fa-key fa-lg\"></i> {1}</a></div>")
                    .arg(tr("cms-dashboard-admin-change-password")), TextFormat::XHTMLUnsafeText);
        WText *sysmon = new WText(
                    WString("<div><a href=\"javascript:;\"><i class=\"fa fa-line-chart fa-lg\"></i> {1}</a></div>")
                    .arg(tr("cms-dashboard-system-monitor")), TextFormat::XHTMLUnsafeText);
        WText *switchLanguage = new WText(
                    WString("<div><a href=\"javascript:;\"><i class=\"fa fa-refresh fa-lg\"></i> {1}</a></div>")
                    .arg(tr("cms-dashboard-switch-language")), TextFormat::XHTMLUnsafeText);
        WText *exit = new WText(
                    WString("<div><a href=\"javascript:;\"><i class=\"fa fa-sign-out fa-lg\"></i> {1}</a></div>")
                    .arg(tr("cms-dashboard-exit")), TextFormat::XHTMLUnsafeText);

        dashboard->setId("menu-item-dashboard");
        newsletter->setId("menu-item-newsletter");
        subscribers->setId("menu-item-subscribers");
        contacts->setId("menu-item-contacts");
        settings->setId("menu-item-settings");
        changeEmail->setId("menu-item-change-email");
        changePassword->setId("menu-item-change-password");
        sysmon->setId("menu-item-system-monitor");
        switchLanguage->setId("menu-item-switch-language");
        exit->setId("menu-item-exit");

        tmpl->bindWidget("dashboard", dashboard);
        tmpl->bindWidget("newsletter", newsletter);
        tmpl->bindWidget("subscribers", subscribers);
        tmpl->bindWidget("contacts", contacts);
        tmpl->bindWidget("settings", settings);
        tmpl->bindWidget("admin", new WText(tr("cms-dashboard-admin")));
        tmpl->bindWidget("change-email", changeEmail);
        tmpl->bindWidget("change-password", changePassword);
        tmpl->bindWidget("system-monitor", sysmon);
        tmpl->bindWidget("switch-language", switchLanguage);
        tmpl->bindWidget("exit", exit);

        m_pimpl->Contents = new Wt::WStackedWidget();
        m_pimpl->Contents->addWidget(new CmsDashboard());
        m_pimpl->Contents->addWidget(new CmsNewsletter());
        m_pimpl->Contents->addWidget(new CmsSubscribers());
        m_pimpl->Contents->addWidget(new CmsContacts());
        m_pimpl->Contents->addWidget(new CmsSettings());
        m_pimpl->Contents->addWidget(new CmsChangeEmail());
        m_pimpl->Contents->addWidget(new CmsChangePassword());
        m_pimpl->SystemMonitor = new SysMon();
        m_pimpl->Contents->addWidget(m_pimpl->SystemMonitor);
        tmpl->bindWidget("stcked-widget", m_pimpl->Contents);

        WSignalMapper<WText *, WStackedWidget *> *menuItemSignalMapper = new WSignalMapper<WText *, WStackedWidget *>(m_pimpl.get());
        menuItemSignalMapper->mapped().connect(m_pimpl.get(), &Cms::Impl::OnMenuItemPressed);
        menuItemSignalMapper->mapConnect(dashboard->clicked(), dashboard);
        menuItemSignalMapper->mapConnect(newsletter->clicked(), newsletter);
        menuItemSignalMapper->mapConnect(subscribers->clicked(), subscribers);
        menuItemSignalMapper->mapConnect(contacts->clicked(), contacts);
        menuItemSignalMapper->mapConnect(settings->clicked(), settings);
        menuItemSignalMapper->mapConnect(changeEmail->clicked(), changeEmail);
        menuItemSignalMapper->mapConnect(changePassword->clicked(), changePassword);
        menuItemSignalMapper->mapConnect(sysmon->clicked(), sysmon);
        menuItemSignalMapper->mapConnect(switchLanguage->clicked(), switchLanguage);
        menuItemSignalMapper->mapConnect(exit->clicked(), exit);
    }

    return container;
}

Cms::Impl::Impl()
    : LastSelectedMenuItem(nullptr)
{

}

Cms::Impl::~Impl() = default;

void Cms::Impl::OnMenuItemPressed(WText *sender)
{
    if (LastSelectedMenuItem != nullptr) {
        LastSelectedMenuItem->removeStyleClass("selected");
    }
    LastSelectedMenuItem = sender;
    LastSelectedMenuItem->setStyleClass("selected");

    if (sender->id() == "menu-item-system-monitor") {
        SystemMonitor->Resume();
        Contents->setCurrentIndex(7);
        return;
    } else {
        SystemMonitor->Pause();
    }

    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    if (sender->id() == "menu-item-dashboard") {
        Contents->setCurrentIndex(0);
    } else if (sender->id() == "menu-item-newsletter") {
        Contents->setCurrentIndex(1);
    } else if (sender->id() == "menu-item-subscribers") {
        Contents->setCurrentIndex(2);
    } else if (sender->id() == "menu-item-contacts") {
        Contents->setCurrentIndex(3);
    } else if (sender->id() == "menu-item-settings") {
        Contents->setCurrentIndex(4);
    } else if (sender->id() == "menu-item-change-email") {
        Contents->setCurrentIndex(5);
    } else if (sender->id() == "menu-item-change-password") {
        Contents->setCurrentIndex(6);
    } else if (sender->id() == "menu-item-switch-language") {
        switch (cgiEnv->GetInformation().Client.Language.Code) {
        case CgiEnv::InformationRecord::ClientRecord::LanguageCode::None:
        case CgiEnv::InformationRecord::ClientRecord::LanguageCode::Invalid:
            break;
        case CgiEnv::InformationRecord::ClientRecord::LanguageCode::En:
            cgiRoot->redirect("/?root&lang=fa");
            break;
        case CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa:
            cgiRoot->redirect("/?root&lang=en");
            break;
        }
        return;
    } else if (sender->id() == "menu-item-exit") {
        ForceExit();
        return;
    } else {
        LOG_WARNING("ERROR: INVALID STACKED-WIDGET PAGE ID!");
    }
}

void Cms::Impl::ValidateSession()
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    try {
        auto conn = Pool::Database().Connection();
        conn->activate();
        pqxx::work txn(*conn.get());

        string query((boost::format("SELECT EXTRACT ( EPOCH FROM expiry::TIMESTAMPTZ ) as expiry FROM \"%1%\""
                                    " WHERE token = %2%;")
                      % txn.esc(Service::Pool::Database().GetTableName("ROOT_SESSIONS"))
                      % txn.quote(cgiEnv->GetInformation().Client.Session.Token)).str());
        LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

        result r = txn.exec(query);

        string expiry("0");
        if (!r.empty()) {
            const pqxx::row row(r[0]);
            expiry = row["expiry"].c_str();
        }

        time_t rawTime = lexical_cast<time_t>(expiry);
        CDate::Now n(CDate::Timezone::UTC);

        /// 0 means force exit the current session
        /// if you don't know why,
        /// see Service::RootLogin::PreserveSessionData method
        if (rawTime == 0) {
            ForceExit();
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
}

void Cms::Impl::ForceExit()
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    srand(static_cast<unsigned int>(System::RandSeed()));
    try {
        cgiRoot->removeCookie("cms-session-token");
        LOG_INFO("Root logout succeed!", cgiEnv->GetInformation().ToJson());
    } catch(...) {
        LOG_ERROR("Root logout failed!", cgiEnv->GetInformation().ToJson());
    }
    cgiRoot->Exit("/?root&logout");
}
