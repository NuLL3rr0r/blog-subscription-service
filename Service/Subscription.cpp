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


#include <boost/algorithm/string.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <Wt/WApplication>
#include <Wt/WCheckBox>
#include <Wt/WContainerWidget>
#include <Wt/WImage>
#include <Wt/WIntValidator>
#include <Wt/WLengthValidator>
#include <Wt/WLineEdit>
#include <Wt/WMessageBox>
#include <Wt/WPushButton>
#include <Wt/WRegExpValidator>
#include <Wt/WString>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WTextArea>
#include <CoreLib/CDate.hpp>
#include <CoreLib/Database.hpp>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Log.hpp>
#include <CoreLib/Mail.hpp>
#include <CoreLib/make_unique.hpp>
#include "Captcha.hpp"
#include "CgiEnv.hpp"
#include "CgiRoot.hpp"
#include "Div.hpp"
#include "Pool.hpp"
#include "Subscription.hpp"

using namespace std;
using namespace boost;
using namespace Wt;
using namespace Service;

struct Subscription::Impl : public Wt::WObject
{
public:
    Wt::WLineEdit *EmailLineEdit;
    Wt::WCheckBox *EnContentsCheckBox;
    Wt::WCheckBox *FaContentsCheckBox;
    Wt::WLineEdit *CaptchaLineEdit;
    Service::Captcha *Captcha;
    Wt::WIntValidator *CaptchaValidator;
    Wt::WImage *CaptchaImage;

    std::unique_ptr<Wt::WMessageBox> MessageBox;

public:
    Impl();
    ~Impl();

public:
    void OnContentsCheckBoxStateChanged(Wt::WCheckBox *checkbox);
    void OnSubscribeFormSubmitted();

public:
    Wt::WWidget *GetSubscribeForm();
    Wt::WWidget *GetConfirmationPage();
    Wt::WWidget *GetUnsubscribeForm();
    Wt::WWidget *GetCancellationPage();
};

Subscription::Subscription() :
    Page(),
    m_pimpl(make_unique<Subscription::Impl>())
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    switch (cgiEnv->SubscriptionData.Subscribe) {
    case CgiEnv::Subscription::Action::Subscribe:
        cgiRoot->setTitle(tr("home-subscription-subscribe-page-title"));
        break;
    case CgiEnv::Subscription::Action::Confirm:
        cgiRoot->setTitle(tr("home-subscription-confirmation-page-title"));
        break;
    case CgiEnv::Subscription::Action::Unsubscribe:
        cgiRoot->setTitle(tr("home-subscription-unsubscribe-page-title"));
        break;
    case CgiEnv::Subscription::Action::Cancel:
        cgiRoot->setTitle(tr("home-subscription-cancellation-page-title"));
        break;
    case CgiEnv::Subscription::Action::None:
        cgiRoot->setTitle(tr("home-subscription-subscribe-page-title"));
        break;
    }

    this->clear();
    this->setId("SubscriptionPage");
    this->setStyleClass("subscription-page full-width full-height");
    this->addWidget(this->Layout());
}

Subscription::~Subscription() = default;

WWidget *Subscription::Layout()
{
    Div *container = new Div("Subscription", "subscription-layout full-width full-height");

    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    switch (cgiEnv->SubscriptionData.Subscribe) {
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
        container->addWidget(m_pimpl->GetSubscribeForm());
        break;
    }

    return container;
}

Subscription::Impl::Impl()
{

}

Subscription::Impl::~Impl() = default;

void Subscription::Impl::OnContentsCheckBoxStateChanged(Wt::WCheckBox *checkbox)
{
    if (!EnContentsCheckBox->isChecked() && !FaContentsCheckBox->isChecked()) {
        checkbox->setChecked(true);
    }
}

void Subscription::Impl::OnSubscribeFormSubmitted()
{

}

Wt::WWidget *Subscription::Impl::GetSubscribeForm()
{
    WTemplate *tmpl = new WTemplate();
    tmpl->setId("Subscribe");
    tmpl->setStyleClass("container-table");

    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    string htmlData;
    string file;
    if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
        file = "../templates/home-subscription-subscribe-fa.wtml";
    } else {
        file = "../templates/home-subscription-subscribe.wtml";
    }

    if (CoreLib::FileSystem::Read(file, htmlData)) {
        /// Fill the template
        tmpl->setTemplateText(WString(htmlData), TextFormat::XHTMLUnsafeText);

        EmailLineEdit = new WLineEdit();
        EmailLineEdit->setPlaceholderText(tr("home-subscription-subscribe-email-placeholder"));
        WRegExpValidator *emailValidator = new WRegExpValidator(Pool::Storage()->RegexEmail());
        emailValidator->setFlags(MatchCaseInsensitive);
        emailValidator->setMandatory(true);
        EmailLineEdit->setValidator(emailValidator);

        static const regex REGEX(Pool::Storage()->RegexEmail());
        smatch result;
        if (regex_search(cgiEnv->SubscriptionData.Inbox, result, REGEX)) {
            EmailLineEdit->setText(WString::fromUTF8(cgiEnv->SubscriptionData.Inbox));
        }

        WSignalMapper<WCheckBox *> *contentsSignalMapper = new WSignalMapper<WCheckBox *>(this);
        contentsSignalMapper->mapped().connect(this, &Subscription::Impl::OnContentsCheckBoxStateChanged);

        EnContentsCheckBox = new WCheckBox();
        FaContentsCheckBox = new WCheckBox();

        if (cgiEnv->SubscriptionData.Languages.size() > 0) {
            if (std::find(cgiEnv->SubscriptionData.Languages.begin(), cgiEnv->SubscriptionData.Languages.end(),
                          CgiEnv::Subscription::Language::En) != cgiEnv->SubscriptionData.Languages.end()) {
                EnContentsCheckBox->setChecked(true);
            }

            if (std::find(cgiEnv->SubscriptionData.Languages.begin(), cgiEnv->SubscriptionData.Languages.end(),
                          CgiEnv::Subscription::Language::Fa) != cgiEnv->SubscriptionData.Languages.end()) {
                FaContentsCheckBox->setChecked(true);
            }
        } else {
            if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
                EnContentsCheckBox->setChecked(true);
                FaContentsCheckBox->setChecked(true);
            } else {
                EnContentsCheckBox->setChecked(true);
                FaContentsCheckBox->setChecked(false);
            }
        }

        // ignore the checked event, register only unchecked event
        contentsSignalMapper->mapConnect(EnContentsCheckBox->unChecked(), EnContentsCheckBox);
        contentsSignalMapper->mapConnect(FaContentsCheckBox->unChecked(), FaContentsCheckBox);

        EnContentsCheckBox->setStyleClass("checkbox");
        FaContentsCheckBox->setStyleClass("checkbox");

        Captcha = new Service::Captcha();
        CaptchaImage = Captcha->Generate();
        CaptchaImage->setAlternateText(tr("home-captcha-hint"));
        CaptchaImage->setAttributeValue("title", tr("home-captcha-hint"));

        int captchaResult = (int)Captcha->GetResult();

        CaptchaLineEdit = new WLineEdit();
        CaptchaLineEdit->setPlaceholderText(tr("home-captcha-hint"));
        CaptchaValidator = new WIntValidator(captchaResult, captchaResult);
        CaptchaValidator->setMandatory(true);
        CaptchaLineEdit->setValidator(CaptchaValidator);

        WPushButton *subscribePushButton = new WPushButton(tr("home-subscription-subscribe-button"));
        subscribePushButton->setStyleClass("btn btn-primary");

        tmpl->bindString("email-input-id", EmailLineEdit->id());
        tmpl->bindString("captcha-input-id", CaptchaLineEdit->id());

        tmpl->bindWidget("title", new WText(tr("home-subscription-subscribe-page-title")));
        tmpl->bindWidget("email-label-text", new WText(tr("home-subscription-subscribe-email")));
        tmpl->bindWidget("contents-label-text", new WText(tr("home-subscription-subscribe-contents")));
        tmpl->bindWidget("en-contents-checkbox-text", new WText(tr("home-subscription-subscribe-contents-en")));
        tmpl->bindWidget("fa-contents-checkbox-text", new WText(tr("home-subscription-subscribe-contents-fa")));
        tmpl->bindWidget("captcha-label-text", new WText(tr("home-captcha")));

        tmpl->bindWidget("email-input", EmailLineEdit);
        tmpl->bindWidget("en-contents-checkbox", EnContentsCheckBox);
        tmpl->bindWidget("fa-contents-checkbox", FaContentsCheckBox);
        tmpl->bindWidget("captcha-input", CaptchaLineEdit);
        tmpl->bindWidget("captcha-image", CaptchaImage);
        tmpl->bindWidget("subscribe-button", subscribePushButton);

        EmailLineEdit->enterPressed().connect(this, &Subscription::Impl::OnSubscribeFormSubmitted);
        EnContentsCheckBox->enterPressed().connect(this, &Subscription::Impl::OnSubscribeFormSubmitted);
        FaContentsCheckBox->enterPressed().connect(this, &Subscription::Impl::OnSubscribeFormSubmitted);
        CaptchaLineEdit->enterPressed().connect(this, &Subscription::Impl::OnSubscribeFormSubmitted);
        subscribePushButton->clicked().connect(this, &Subscription::Impl::OnSubscribeFormSubmitted);

        EmailLineEdit->setFocus();
    }

    return tmpl;
}

Wt::WWidget *Subscription::Impl::GetConfirmationPage()
{
    WTemplate *tmpl = new WTemplate();
    tmpl->setId("Confirmation");
    tmpl->setStyleClass("full-width full-height");

    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    string htmlData;
    string file;
    if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
        file = "../templates/home-subscription-confirmation-fa.wtml";
    } else {
        file = "../templates/home-subscription-confirmation.wtml";
    }

    if (CoreLib::FileSystem::Read(file, htmlData)) {
        /// Fill the template
        tmpl->setTemplateText(WString(htmlData), TextFormat::XHTMLUnsafeText);
    }

    return tmpl;
}

Wt::WWidget *Subscription::Impl::GetUnsubscribeForm()
{
    WTemplate *tmpl = new WTemplate();
    tmpl->setId("Unsubscribe");
    tmpl->setStyleClass("full-width full-height");

    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    string htmlData;
    string file;
    if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
        file = "../templates/home-subscription-unsubscribe-fa.wtml";
    } else {
        file = "../templates/home-subscription-unsubscribe.wtml";
    }

    if (CoreLib::FileSystem::Read(file, htmlData)) {
        /// Fill the template
        tmpl->setTemplateText(WString(htmlData), TextFormat::XHTMLUnsafeText);
    }

    return tmpl;
}

Wt::WWidget *Subscription::Impl::GetCancellationPage()
{
    WTemplate *tmpl = new WTemplate();
    tmpl->setId("Cancellation");
    tmpl->setStyleClass("full-width full-height");

    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    string htmlData;
    string file;
    if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
        file = "../templates/home-subscription-cancellation-fa.wtml";
    } else {
        file = "../templates/home-subscription-cancellation.wtml";
    }

    if (CoreLib::FileSystem::Read(file, htmlData)) {
        /// Fill the template
        tmpl->setTemplateText(WString(htmlData), TextFormat::XHTMLUnsafeText);
    }

    return tmpl;
}

