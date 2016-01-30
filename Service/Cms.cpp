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
 * The CMS start page.
 */


#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WSignalMapper>
#include <Wt/WStackedWidget>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WWidget>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Log.hpp>
#include <CoreLib/Random.hpp>
#include <CoreLib/System.hpp>
#include "CgiEnv.hpp"
#include "Cms.hpp"
#include "Cms.hpp"
#include "CmsChangeEmail.hpp"
#include "CmsChangePassword.hpp"
#include "CmsContacts.hpp"
#include "CmsDashboard.hpp"
#include "CmsNewsletter.hpp"
#include "CmsSubscribers.hpp"
#include "Div.hpp"
#include "Pool.hpp"
#include "SysMon.hpp"

using namespace std;
using namespace Wt;
using namespace CoreLib;
using namespace Service;

struct Cms::Impl : public Wt::WObject
{
public:
    Cms *m_parent;
    Wt::WStackedWidget *Contents;
    WText *LastSelectedMenuItem;

    SysMon *SystemMonitor;

public:
    explicit Impl(Cms *parent);

public:
    void OnMenuItemPressed(WText *sender);
};

Cms::Cms(CgiRoot *cgi) :
    Page(cgi),
    m_pimpl(std::make_unique<Cms::Impl>(this))
{
    m_cgiRoot->setTitle(tr("cms-page-title"));

    m_htmlRoot->clear();
    this->clear();
    this->setId("CmsPage");
    this->addWidget(Layout());
    m_htmlRoot->addWidget(this);
}

WWidget *Cms::Layout()
{
    Div *container = new Div("Cms", "container");
    Div *noScript = new Div(container);
    noScript->addWidget(new WText(tr("no-script")));

    std::string htmlData;
    std::string file;
    if (m_cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
        file = "../templates/cms-fa.wtml";
    } else {
        file = "../templates/cms.wtml";
    }

    if (CoreLib::FileSystem::Read(file, htmlData)) {
        WTemplate *tmpl = new WTemplate(container);
        tmpl->setTemplateText(WString(htmlData), TextFormat::XHTMLUnsafeText);

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
        changeEmail->setId("menu-item-change-email");
        changePassword->setId("menu-item-change-password");
        sysmon->setId("menu-item-system-monitor");
        switchLanguage->setId("menu-item-switch-language");
        exit->setId("menu-item-exit");

        tmpl->bindWidget("dashboard", dashboard);
        tmpl->bindWidget("newsletter", newsletter);
        tmpl->bindWidget("subscribers", subscribers);
        tmpl->bindWidget("contacts", contacts);
        tmpl->bindWidget("admin", new WText(tr("cms-dashboard-admin")));
        tmpl->bindWidget("change-email", changeEmail);
        tmpl->bindWidget("change-password", changePassword);
        tmpl->bindWidget("system-monitor", sysmon);
        tmpl->bindWidget("switch-language", switchLanguage);
        tmpl->bindWidget("exit", exit);

        m_pimpl->Contents = new Wt::WStackedWidget();
        m_pimpl->SystemMonitor = new SysMon(m_cgiRoot);
        m_pimpl->Contents->addWidget(m_pimpl->SystemMonitor);
        tmpl->bindWidget("stcked-widget", m_pimpl->Contents);

        WSignalMapper<WText *, WStackedWidget *> *menuItemSignalMapper = new WSignalMapper<WText *, WStackedWidget *>(m_pimpl.get());
        menuItemSignalMapper->mapped().connect(m_pimpl.get(), &Cms::Impl::OnMenuItemPressed);
        menuItemSignalMapper->mapConnect(dashboard->clicked(), dashboard);
        menuItemSignalMapper->mapConnect(newsletter->clicked(), newsletter);
        menuItemSignalMapper->mapConnect(subscribers->clicked(), subscribers);
        menuItemSignalMapper->mapConnect(contacts->clicked(), contacts);
        menuItemSignalMapper->mapConnect(changeEmail->clicked(), changeEmail);
        menuItemSignalMapper->mapConnect(changePassword->clicked(), changePassword);
        menuItemSignalMapper->mapConnect(sysmon->clicked(), sysmon);
        menuItemSignalMapper->mapConnect(switchLanguage->clicked(), switchLanguage);
        menuItemSignalMapper->mapConnect(exit->clicked(), exit);
    }

    return container;
}

Cms::Impl::Impl(Cms *parent)
    : m_parent(parent),
      LastSelectedMenuItem(nullptr)
{

}

void Cms::Impl::OnMenuItemPressed(WText *sender)
{
    if (LastSelectedMenuItem != nullptr) {
        LastSelectedMenuItem->removeStyleClass("selected");
    }
    LastSelectedMenuItem = sender;
    LastSelectedMenuItem->setStyleClass("selected");

     if (sender->id() == "menu-item-system-monitor") {
         SystemMonitor->Resume();
         Contents->setCurrentIndex(6);
         return;
     } else {
         SystemMonitor->Pause();
     }

    if (sender->id() == "menu-item-dashboard") {
        Contents->setCurrentIndex(0);
    } else if (sender->id() == "menu-item-newsletter") {
        Contents->setCurrentIndex(1);
    } else if (sender->id() == "menu-item-subscribers") {
        Contents->setCurrentIndex(2);
    } else if (sender->id() == "menu-item-contacts") {
        Contents->setCurrentIndex(3);
    } else if (sender->id() == "menu-item-change-email") {
        Contents->setCurrentIndex(4);
    } else if (sender->id() == "menu-item-change-password") {
        Contents->setCurrentIndex(5);

    } else if (sender->id() == "menu-item-switch-language") {
        switch (m_parent->m_cgiEnv->GetCurrentLanguage()) {
        case CgiEnv::Language::None:
        case CgiEnv::Language::Invalid:
            break;
        case CgiEnv::Language::En:
            m_parent->Redirect("/?root&lang=fa");
            break;
        case CgiEnv::Language::Fa:
            m_parent->Redirect("/?root&lang=en");
            break;
        }

        return;

    } else if (sender->id() == "menu-item-exit") {
        std::srand((unsigned int)System::RandSeed());

        m_parent->m_cgiRoot->removeCookie("cms-session-user");
        m_parent->m_cgiRoot->removeCookie("cms-session-token");
        m_parent->Exit("/?root&logout");
        return;

    } else {
        LOG_WARNING("ERROR: INVALID STACKED-WIDGET PAGE ID!");
    }
}

