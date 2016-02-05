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
 * The subscribe form shown to the end-user.
 */


#include <Wt/WContainerWidget>
#include <Wt/WString>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <CoreLib/make_unique.hpp>
#include "CgiEnv.hpp"
#include "Div.hpp"
#include "Subscription.hpp"

using namespace std;
using namespace boost;
using namespace Wt;
using namespace Service;

struct Subscription::Impl : public Wt::WObject
{
private:
    Subscription *m_parent;

public:
    explicit Impl(Subscription *parent);
    ~Impl();

public:
    Wt::WWidget *GetSubscribeForm();
    Wt::WWidget *GetConfirmationPage();
    Wt::WWidget *GetUnsubscribeForm();
    Wt::WWidget *GetCancellationPage();
};

Subscription::Subscription(CgiRoot *cgi) :
    Page(cgi),
    m_pimpl(make_unique<Subscription::Impl>(this))
{
    switch (m_cgiEnv->SubscriptionData.Subscribe) {
    case CgiEnv::Subscription::Action::Subscribe:
        m_cgiRoot->setTitle(tr("home-subscription-subscribe-page-title"));
        break;
    case CgiEnv::Subscription::Action::Confirm:
        m_cgiRoot->setTitle(tr("home-subscription-confirmation-page-title"));
        break;
    case CgiEnv::Subscription::Action::Unsubscribe:
        m_cgiRoot->setTitle(tr("home-subscription-unsubscribe-page-title"));
        break;
    case CgiEnv::Subscription::Action::Cancel:
        m_cgiRoot->setTitle(tr("home-subscription-cancellation-page-title"));
        break;
    case CgiEnv::Subscription::Action::None:
        m_cgiRoot->setTitle(tr("home-subscription-subscribe-page-title"));
        break;
    }

    this->clear();
    this->setId("SubscriptionPage");
    this->addWidget(Layout());
}

Subscription::~Subscription() = default;

WWidget *Subscription::Layout()
{
    Div *container = new Div("Subscription", "container");

    switch (m_cgiEnv->SubscriptionData.Subscribe) {
    case CgiEnv::Subscription::Action::Subscribe:
        container->addWidget(m_pimpl->GetSubscribeForm());
        break;
    case CgiEnv::Subscription::Action::Confirm:
        container->addWidget(m_pimpl->GetConfirmationPage());
        break;
    case CgiEnv::Subscription::Action::Unsubscribe:
        container->addWidget(m_pimpl->GetUnsubscribeForm());
        break;
    case CgiEnv::Subscription::Action::Cancel:
        container->addWidget(m_pimpl->GetCancellationPage());
        break;
    case CgiEnv::Subscription::Action::None:
        container->addWidget(m_pimpl->GetCancellationPage());
        break;
    }

    return container;
}

Subscription::Impl::Impl(Subscription *parent)
    : m_parent(parent)
{

}

Subscription::Impl::~Impl() = default;

Wt::WWidget *Subscription::Impl::GetSubscribeForm()
{
    WTemplate *tmpl = new WTemplate();
    tmpl->setId("Subscribe");
    tmpl->setStyleClass("full-width full-height");

    string htmlData;
    string file;
    if (m_parent->m_cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
        file = "../templates/home-subscription-subscribe-fa.wtml";
    } else {
        file = "../templates/home-subscription-subscribe.wtml";
    }

    tmpl->setTemplateText(WString(htmlData), TextFormat::XHTMLUnsafeText);


    return tmpl;
}

Wt::WWidget *Subscription::Impl::GetConfirmationPage()
{
    WTemplate *tmpl = new WTemplate();
    tmpl->setId("Confirmation");
    tmpl->setStyleClass("full-width full-height");

    string htmlData;
    string file;
    if (m_parent->m_cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
        file = "../templates/home-subscription-confirmation-fa.wtml";
    } else {
        file = "../templates/home-subscription-confirmation.wtml";
    }

    tmpl->setTemplateText(WString(htmlData), TextFormat::XHTMLUnsafeText);

    return tmpl;
}

Wt::WWidget *Subscription::Impl::GetUnsubscribeForm()
{
    WTemplate *tmpl = new WTemplate();
    tmpl->setId("Unsubscribe");
    tmpl->setStyleClass("full-width full-height");

    string htmlData;
    string file;
    if (m_parent->m_cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
        file = "../templates/home-subscription-unsubscribe-fa.wtml";
    } else {
        file = "../templates/home-subscription-unsubscribe.wtml";
    }

    tmpl->setTemplateText(WString(htmlData), TextFormat::XHTMLUnsafeText);

    return tmpl;
}

Wt::WWidget *Subscription::Impl::GetCancellationPage()
{
    WTemplate *tmpl = new WTemplate();
    tmpl->setId("Cancellation");
    tmpl->setStyleClass("full-width full-height");
    string htmlData;
    string file;
    if (m_parent->m_cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
        file = "../templates/home-subscription-cancellation-fa.wtml";
    } else {
        file = "../templates/home-subscription-cancellation.wtml";
    }

    tmpl->setTemplateText(WString(htmlData), TextFormat::XHTMLUnsafeText);

    return tmpl;
}

