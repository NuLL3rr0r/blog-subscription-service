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
#include <CoreLib/Crypto.hpp>
#include <CoreLib/Database.hpp>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Log.hpp>
#include <CoreLib/Mail.hpp>
#include <CoreLib/make_unique.hpp>
#include <CoreLib/Random.hpp>
#include "Captcha.hpp"
#include "CgiEnv.hpp"
#include "CgiRoot.hpp"
#include "Div.hpp"
#include "Pool.hpp"
#include "Subscription.hpp"

using namespace std;
using namespace boost;
using namespace cppdb;
using namespace Wt;
using namespace CoreLib;
using namespace CoreLib::CDate;
using namespace Service;

struct Subscription::Impl : public Wt::WObject
{
public:
    enum class Message : unsigned char {
        Confirm,
        Confirmed,
        Cancel,
        Cancelled
    };

public:
    Wt::WLineEdit *EmailLineEdit;
    Wt::WCheckBox *EnContentsCheckBox;
    Wt::WCheckBox *FaContentsCheckBox;
    Wt::WLineEdit *CaptchaLineEdit;
    Service::Captcha *Captcha;
    Wt::WIntValidator *CaptchaValidator;
    Wt::WImage *CaptchaImage;

    std::unique_ptr<Wt::WMessageBox> MessageBox;

private:
    Subscription *m_parent;

public:
    explicit Impl(Subscription *parent);
    ~Impl();

public:
    void OnDialogClosed(Wt::StandardButton button);
    void OnContentsCheckBoxStateChanged(Wt::WCheckBox *checkbox);
    void OnSubscribeFormSubmitted();
    void OnUnsubscribeFormSubmitted();

public:
    void GenerateCaptcha();

    Wt::WWidget *GetSubscribeForm();
    Wt::WWidget *GetConfirmationPage();
    Wt::WWidget *GetUnsubscribeForm();
    Wt::WWidget *GetCancellationPage();

    void GetMessageTemplate(WTemplate *tmpl, const Wt::WString &title, const Wt::WString &message);

    void SendMessage(const Message &type, const string &uuid, const string &inbox);
};

Subscription::Subscription() :
    Page(),
    m_pimpl(make_unique<Subscription::Impl>(this))
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

Subscription::Impl::Impl(Subscription *parent)
    : m_parent(parent)
{

}

Subscription::Impl::~Impl() = default;

void Subscription::Impl::OnDialogClosed(Wt::StandardButton button)
{
    (void)button;
    MessageBox.reset();
}

void Subscription::Impl::OnContentsCheckBoxStateChanged(Wt::WCheckBox *checkbox)
{
    if (!EnContentsCheckBox->isChecked() && !FaContentsCheckBox->isChecked()) {
        checkbox->setChecked(true);
    } else {
        try {
            string action(checkbox->attributeValue("action").toUTF8());
            string visibility(checkbox->attributeValue("visibility").toUTF8());

            if (action == "unsubscribe") {
                if (visibility != "en_fa") {
                    checkbox->setChecked(true);
                }
            }
        } catch (...) {  }
    }
}

void Subscription::Impl::OnSubscribeFormSubmitted()
{
    if (!m_parent->Validate(EmailLineEdit)
            || !m_parent->Validate(CaptchaLineEdit)) {
        this->GenerateCaptcha();
        return;
    }

    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    if (!EnContentsCheckBox->isChecked() && !FaContentsCheckBox->isChecked()) {
        if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
            FaContentsCheckBox->setFocus();
            return;
        } else {
            EnContentsCheckBox->setFocus();
            return;
        }
    }

    try {
        CDate::Now n;
        string date(lexical_cast<std::string>(n.RawTime));
        string inbox(EmailLineEdit->text().trim().toUTF8());

        string pending_confirm;
        if (EnContentsCheckBox->isChecked() && FaContentsCheckBox->isChecked()) {
            pending_confirm = "en_fa";
        } else if (EnContentsCheckBox->isChecked()) {
            pending_confirm = "en";
        } else if (FaContentsCheckBox->isChecked()) {
            pending_confirm = "fa";
        } else {
            pending_confirm = "none";
        }

        transaction guard(Service::Pool::Database()->Sql());

        result r = Pool::Database()->Sql()
                << (format("SELECT uuid FROM \"%1%\""
                           " WHERE inbox=?;")
                    % Pool::Database()->GetTableName("SUBSCRIBERS")).str()
                << inbox << row;

        string uuid;

        if (r.empty()) {
            while (true) {
                CoreLib::Random::Uuid(uuid);

                r = Pool::Database()->Sql()
                        << (format("SELECT inbox FROM \"%1%\""
                                   " WHERE uuid=?;")
                            % Pool::Database()->GetTableName("SUBSCRIBERS")).str()
                        << uuid << row;

                if (r.empty()) {
                    break;
                }
            }

            Pool::Database()->Insert("SUBSCRIBERS",
                                     "inbox, uuid, subscription, pending_confirm, pending_cancel, join_date, update_date",
                                     { inbox, uuid, "none", pending_confirm, "none", date, date });
        } else {
            r >> uuid;

            Pool::Database()->Update("SUBSCRIBERS",
                                     "inbox",
                                     inbox,
                                     "pending_confirm=?, pending_cancel=?",
                                     { pending_confirm, "none" });
        }

        guard.commit();

        SendMessage(Message::Confirm, uuid, inbox);

        MessageBox = std::make_unique<WMessageBox>(tr("home-subscription-subscribe-success-dialog-title"),
                                                   tr("home-subscription-subscribe-success-dialog-message"),
                                                   Information, NoButton);
        MessageBox->addButton(tr("home-dialog-button-ok"), Ok);
        MessageBox->buttonClicked().connect(this, &Subscription::Impl::OnDialogClosed);
        MessageBox->show();

        this->GenerateCaptcha();

        return;
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
}

void Subscription::Impl::OnUnsubscribeFormSubmitted()
{
    if (!m_parent->Validate(EmailLineEdit)
            || !m_parent->Validate(CaptchaLineEdit)) {
        this->GenerateCaptcha();
        return;
    }

    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    if (!EnContentsCheckBox->isChecked() && !FaContentsCheckBox->isChecked()) {
        if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
            FaContentsCheckBox->setFocus();
            return;
        } else {
            EnContentsCheckBox->setFocus();
            return;
        }
    }

    try {
        string inbox(EmailLineEdit->text().trim().toUTF8());

        string pending_cancel;
        // in reverse-order, compared to subscribe
        if (EnContentsCheckBox->isChecked() && FaContentsCheckBox->isChecked()) {
            pending_cancel = "en_fa";
        } else if (EnContentsCheckBox->isChecked()) {
            pending_cancel = "en";
        } else if (FaContentsCheckBox->isChecked()) {
            pending_cancel = "fa";
        } else {
            pending_cancel = "none";
        }

        transaction guard(Service::Pool::Database()->Sql());

        result r = Pool::Database()->Sql()
                << (format("SELECT inbox FROM \"%1%\""
                           " WHERE inbox=?;")
                    % Pool::Database()->GetTableName("SUBSCRIBERS")).str()
                << inbox << row;

        if (r.empty()) {
            guard.rollback();

            MessageBox = std::make_unique<WMessageBox>(tr("home-subscription-invalid-recipient-id-title"),
                                                       tr("home-subscription-invalid-recipient-id-message"),
                                                       Information, NoButton);
            MessageBox->addButton(tr("home-dialog-button-ok"), Ok);
            MessageBox->buttonClicked().connect(this, &Subscription::Impl::OnDialogClosed);
            MessageBox->show();

            this->GenerateCaptcha();

            return;
        }

        Pool::Database()->Update("SUBSCRIBERS",
                                 "inbox",
                                 inbox,
                                 "pending_cancel=?",
                                 { pending_cancel });

        guard.commit();

        SendMessage(Message::Cancel, cgiEnv->SubscriptionData.Uuid, inbox);

        MessageBox = std::make_unique<WMessageBox>(tr("home-subscription-unsubscribe-success-dialog-title"),
                                                   tr("home-subscription-unsubscribe-success-dialog-message"),
                                                   Information, NoButton);
        MessageBox->addButton(tr("home-dialog-button-ok"), Ok);
        MessageBox->buttonClicked().connect(this, &Subscription::Impl::OnDialogClosed);
        MessageBox->show();

        this->GenerateCaptcha();

        return;
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
}

void Subscription::Impl::GenerateCaptcha()
{
    CaptchaImage->setImageRef(Captcha->Generate()->imageRef());
    int captchaResult = static_cast<int>(Captcha->GetResult());
    CaptchaValidator->setRange(captchaResult, captchaResult);
}

Wt::WWidget *Subscription::Impl::GetSubscribeForm()
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    if (cgiEnv->SubscriptionData.Subscribe != CgiEnv::Subscription::Action::Subscribe
            && cgiEnv->SubscriptionData.Subscribe != CgiEnv::Subscription::Action::None) {
        return new WText(L"Oops!");
    }

    WTemplate *tmpl = new WTemplate();
    tmpl->setId("Subscribe");
    tmpl->setStyleClass("container-table");

    string htmlData;
    string file;
    if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
        file = "../templates/home-subscription-subscribe-fa.wtml";
    } else {
        file = "../templates/home-subscription-subscribe.wtml";
    }

    try {
        if (CoreLib::FileSystem::Read(file, htmlData)) {
            /// Fill the template
            tmpl->setTemplateText(WString::fromUTF8(htmlData), TextFormat::XHTMLUnsafeText);

            EmailLineEdit = new WLineEdit();
            EmailLineEdit->setPlaceholderText(tr("home-subscription-subscribe-email-placeholder"));
            WRegExpValidator *emailValidator = new WRegExpValidator(Pool::Storage()->RegexEmail());
            emailValidator->setFlags(MatchCaseInsensitive);
            emailValidator->setMandatory(true);
            EmailLineEdit->setValidator(emailValidator);

            if (cgiEnv->SubscriptionData.Subscribe == CgiEnv::Subscription::Action::Subscribe) {
                static const regex REGEX_EMAIL(Pool::Storage()->RegexEmail());
                smatch result;
                if (regex_search(cgiEnv->SubscriptionData.Inbox, result, REGEX_EMAIL)) {
                    EmailLineEdit->setText(WString::fromUTF8(cgiEnv->SubscriptionData.Inbox));
                }
            }

            WSignalMapper<WCheckBox *> *contentsSignalMapper = new WSignalMapper<WCheckBox *>(this);
            contentsSignalMapper->mapped().connect(this, &Subscription::Impl::OnContentsCheckBoxStateChanged);

            EnContentsCheckBox = new WCheckBox();
            FaContentsCheckBox = new WCheckBox();

            if ((cgiEnv->SubscriptionData.Subscribe == CgiEnv::Subscription::Action::Subscribe
                 || cgiEnv->SubscriptionData.Subscribe == CgiEnv::Subscription::Action::None)
                    && cgiEnv->SubscriptionData.Languages.size() > 0) {
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

            int captchaResult = static_cast<int>(Captcha->GetResult());

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

    return tmpl;
}

Wt::WWidget *Subscription::Impl::GetConfirmationPage()
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    if (cgiEnv->SubscriptionData.Subscribe != CgiEnv::Subscription::Action::Confirm) {
        return new WText(L"Oops!");
    }

    WTemplate *tmpl = new WTemplate();
    tmpl->setId("Confirmation");
    tmpl->setStyleClass("container-table");

    try {
        static const regex REGEX_UUID(Pool::Storage()->RegexUuid());
        smatch result;
        if (cgiEnv->SubscriptionData.Uuid == "" || !regex_search(cgiEnv->SubscriptionData.Uuid, result, REGEX_UUID)) {
            cgiRoot->setTitle(tr("home-subscription-invalid-recipient-id-title"));
            this->GetMessageTemplate(tmpl,
                                     tr("home-subscription-invalid-recipient-id-title"),
                                     tr("home-subscription-invalid-recipient-id-message"));
            return tmpl;
        }

        cppdb::result r = Pool::Database()->Sql()
                << (format("SELECT inbox, subscription, pending_confirm FROM \"%1%\""
                           " WHERE uuid=?;")
                    % Pool::Database()->GetTableName("SUBSCRIBERS")).str()
                << cgiEnv->SubscriptionData.Uuid << row;

        if (r.empty()) {
            cgiRoot->setTitle(tr("home-subscription-invalid-recipient-id-title"));
            this->GetMessageTemplate(tmpl,
                                     tr("home-subscription-invalid-recipient-id-title"),
                                     tr("home-subscription-invalid-recipient-id-message"));
            return tmpl;
        }

        CDate::Now n;
        string date(lexical_cast<std::string>(n.RawTime));

        string inbox;
        string subscription;
        string pending_confirm;
        r >> inbox >> subscription >> pending_confirm;

        if (pending_confirm == "none" && subscription == "none") {
            cgiEnv->SubscriptionData.Subscribe = CgiEnv::Subscription::Action::Subscribe;
            cgiEnv->SubscriptionData.Inbox = inbox;

            if (subscription == "en_fa") {
                cgiEnv->SubscriptionData.Languages.push_back(CgiEnv::Subscription::Language::En);
                cgiEnv->SubscriptionData.Languages.push_back(CgiEnv::Subscription::Language::Fa);
            } else if (subscription == "en") {
                cgiEnv->SubscriptionData.Languages.push_back(CgiEnv::Subscription::Language::En);
            } else if (subscription == "fa") {
                cgiEnv->SubscriptionData.Languages.push_back(CgiEnv::Subscription::Language::Fa);
            } else {
                cgiEnv->SubscriptionData.Languages.push_back(CgiEnv::Subscription::Language::En);

                if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
                    cgiEnv->SubscriptionData.Languages.push_back(CgiEnv::Subscription::Language::Fa);
                }
            }

            cgiRoot->setTitle(tr("home-subscription-subscribe-page-title"));

            return GetSubscribeForm();
        } else if (pending_confirm == "none") {
            cgiRoot->setTitle(tr("home-subscription-confirmation-already-confirmed-title"));
            this->GetMessageTemplate(tmpl,
                                     tr("home-subscription-confirmation-already-confirmed-title"),
                                     tr("home-subscription-confirmation-already-confirmed-message"));
            return tmpl;
        }

        string htmlData;
        string file;
        if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
            file = "../templates/home-subscription-confirmation-fa.wtml";
        } else {
            file = "../templates/home-subscription-confirmation.wtml";
        }

        if (CoreLib::FileSystem::Read(file, htmlData)) {
            /// Fill the template
            tmpl->setTemplateText(WString::fromUTF8(htmlData), TextFormat::XHTMLUnsafeText);

            string final_subscription;

            if (subscription == "none") {
                final_subscription = pending_confirm;
            } else if (subscription == "en_fa") {
                final_subscription = subscription;
            } else if (subscription == "en") {
                if (pending_confirm == "fa") {
                    final_subscription = "en_fa";
                } else {
                    final_subscription = subscription;
                }
            } else if (subscription == "fa") {
                if (pending_confirm == "en") {
                    final_subscription = "en_fa";
                } else {
                    final_subscription = subscription;
                }
            } else {
                final_subscription = "en_fa";
            }

            transaction guard(Service::Pool::Database()->Sql());

            Pool::Database()->Update("SUBSCRIBERS",
                                     "inbox",
                                     inbox,
                                     "subscription=?, pending_confirm=?, pending_cancel=?, update_date=?",
                                     { final_subscription, "none", "none", date });

            guard.commit();

            SendMessage(Message::Confirmed, cgiEnv->SubscriptionData.Uuid, inbox);

            tmpl->bindString("title", tr("home-subscription-confirmation-congratulation-title"));
            tmpl->bindString("message", tr("home-subscription-confirmation-congratulation-message"));

            string homePageFields;
            if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
                homePageFields = "homepage_url_fa, homepage_title_fa";
            } else {
                homePageFields = "homepage_url_en, homepage_title_en";
            }

            r = Pool::Database()->Sql()
                    << (format("SELECT %1%"
                               " FROM \"%2%\" WHERE pseudo_id = '0';")
                        % homePageFields
                        % Pool::Database()->GetTableName("SETTINGS")).str()
                    << row;

            if (!r.empty()) {
                string homePageUrl;
                string homePageTitle;

                r >> homePageUrl >> homePageTitle;

                tmpl->bindString("home-page-url", WString::fromUTF8(homePageUrl));
                tmpl->bindString("home-page-title", WString::fromUTF8(homePageTitle));
            }
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

    return tmpl;
}

Wt::WWidget *Subscription::Impl::GetUnsubscribeForm()
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    if (cgiEnv->SubscriptionData.Subscribe != CgiEnv::Subscription::Action::Unsubscribe) {
        return new WText(L"Oops!");
    }

    WTemplate *tmpl = new WTemplate();
    tmpl->setId("Unsubscribe");
    tmpl->setStyleClass("container-table");

    try {
        static const regex REGEX_UUID(Pool::Storage()->RegexUuid());
        smatch result;
        if (cgiEnv->SubscriptionData.Uuid == "" || !regex_search(cgiEnv->SubscriptionData.Uuid, result, REGEX_UUID)) {
            cgiRoot->setTitle(tr("home-subscription-invalid-recipient-id-title"));
            this->GetMessageTemplate(tmpl,
                                     tr("home-subscription-invalid-recipient-id-title"),
                                     tr("home-subscription-invalid-recipient-id-message"));
            return tmpl;
        }

        cppdb::result r = Pool::Database()->Sql()
                << (format("SELECT inbox, subscription FROM \"%1%\""
                           " WHERE uuid=?;")
                    % Pool::Database()->GetTableName("SUBSCRIBERS")).str()
                << cgiEnv->SubscriptionData.Uuid << row;

        if (r.empty()) {
            cgiRoot->setTitle(tr("home-subscription-invalid-recipient-id-title"));
            this->GetMessageTemplate(tmpl,
                                     tr("home-subscription-invalid-recipient-id-title"),
                                     tr("home-subscription-invalid-recipient-id-message"));
            return tmpl;
        }

        string inbox;
        string subscription;
        r >> inbox >> subscription;

        if (subscription == "none") {
            cgiRoot->setTitle(tr("home-subscription-unsubscribe-already-unsubscribed-title"));
            this->GetMessageTemplate(tmpl,
                                     tr("home-subscription-unsubscribe-already-unsubscribed-title"),
                                     tr("home-subscription-unsubscribe-already-unsubscribed-message"));
            return tmpl;
        }

        string htmlData;
        string file;
        if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
            file = "../templates/home-subscription-unsubscribe-fa.wtml";
        } else {
            file = "../templates/home-subscription-unsubscribe.wtml";
        }

        if (CoreLib::FileSystem::Read(file, htmlData)) {
            /// Fill the template
            tmpl->setTemplateText(WString::fromUTF8(htmlData), TextFormat::XHTMLUnsafeText);

            EmailLineEdit = new WLineEdit();
            EmailLineEdit->setPlaceholderText(tr("home-subscription-unsubscribe-email-placeholder"));
            WRegExpValidator *emailValidator = new WRegExpValidator(Pool::Storage()->RegexEmail());
            emailValidator->setFlags(MatchCaseInsensitive);
            emailValidator->setMandatory(true);
            EmailLineEdit->setValidator(emailValidator);
            EmailLineEdit->setReadOnly(true);

            static const regex REGEX_EMAIL(Pool::Storage()->RegexEmail());
            if (regex_search(inbox, result, REGEX_EMAIL)) {
                EmailLineEdit->setText(WString::fromUTF8(inbox));
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
                if (subscription == "en_fa") {
                    EnContentsCheckBox->setChecked(true);
                    FaContentsCheckBox->setChecked(true);
                } else if (subscription == "en") {
                    EnContentsCheckBox->setChecked(true);
                    FaContentsCheckBox->setChecked(false);
                } else if (subscription == "fa") {
                    EnContentsCheckBox->setChecked(false);
                    FaContentsCheckBox->setChecked(true);
                } else {
                    if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
                        EnContentsCheckBox->setChecked(false);
                        FaContentsCheckBox->setChecked(true);
                    } else {
                        EnContentsCheckBox->setChecked(true);
                        FaContentsCheckBox->setChecked(false);
                    }
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

            int captchaResult = static_cast<int>(Captcha->GetResult());

            CaptchaLineEdit = new WLineEdit();
            CaptchaLineEdit->setPlaceholderText(tr("home-captcha-hint"));
            CaptchaValidator = new WIntValidator(captchaResult, captchaResult);
            CaptchaValidator->setMandatory(true);
            CaptchaLineEdit->setValidator(CaptchaValidator);

            WPushButton *unsubscribePushButton = new WPushButton(tr("home-subscription-unsubscribe-button"));
            unsubscribePushButton->setStyleClass("btn btn-primary");

            tmpl->bindString("email-input-id", EmailLineEdit->id());
            tmpl->bindString("captcha-input-id", CaptchaLineEdit->id());

            tmpl->bindWidget("title", new WText(tr("home-subscription-unsubscribe-page-title")));
            tmpl->bindWidget("email-label-text", new WText(tr("home-subscription-unsubscribe-email")));
            tmpl->bindWidget("contents-label-text", new WText(tr("home-subscription-unsubscribe-contents")));
            tmpl->bindWidget("en-contents-checkbox-text", new WText(tr("home-subscription-unsubscribe-contents-en")));
            tmpl->bindWidget("fa-contents-checkbox-text", new WText(tr("home-subscription-unsubscribe-contents-fa")));
            tmpl->bindWidget("captcha-label-text", new WText(tr("home-captcha")));

            tmpl->bindWidget("email-input", EmailLineEdit);
            tmpl->bindWidget("en-contents-checkbox", EnContentsCheckBox);
            tmpl->bindWidget("fa-contents-checkbox", FaContentsCheckBox);
            tmpl->bindWidget("captcha-input", CaptchaLineEdit);
            tmpl->bindWidget("captcha-image", CaptchaImage);
            tmpl->bindWidget("unsubscribe-button", unsubscribePushButton);

            if (subscription != "none") {
                tmpl->bindString("contents-display", "block");

                if (subscription == "en_fa") {
                    tmpl->bindString("en-contents-display", "block");
                    tmpl->bindString("fa-contents-display", "block");
                    EnContentsCheckBox->setAttributeValue("action", WString::fromUTF8("unsubscribe"));
                    EnContentsCheckBox->setAttributeValue("visibility", WString::fromUTF8("en_fa"));
                    FaContentsCheckBox->setAttributeValue("action", WString::fromUTF8("unsubscribe"));
                    FaContentsCheckBox->setAttributeValue("visibility", WString::fromUTF8("en_fa"));
                } else if (subscription == "en") {
                    tmpl->bindString("en-contents-display", "block");
                    tmpl->bindString("fa-contents-display", "none");
                    EnContentsCheckBox->setChecked(true);
                    EnContentsCheckBox->setAttributeValue("action", WString::fromUTF8("unsubscribe"));
                    EnContentsCheckBox->setAttributeValue("visibility", WString::fromUTF8("en"));
                } else if (subscription == "fa") {
                    tmpl->bindString("en-contents-display", "none");
                    tmpl->bindString("fa-contents-display", "block");
                    FaContentsCheckBox->setChecked(true);
                    FaContentsCheckBox->setAttributeValue("action", WString::fromUTF8("unsubscribe"));
                    FaContentsCheckBox->setAttributeValue("visibility", WString::fromUTF8("fa"));
                }
            } else {
                tmpl->bindString("contents-display", "none");
                tmpl->bindString("en-contents-display", "none");
                tmpl->bindString("fa-contents-display", "none");
            }

            EmailLineEdit->enterPressed().connect(this, &Subscription::Impl::OnUnsubscribeFormSubmitted);
            EnContentsCheckBox->enterPressed().connect(this, &Subscription::Impl::OnUnsubscribeFormSubmitted);
            FaContentsCheckBox->enterPressed().connect(this, &Subscription::Impl::OnUnsubscribeFormSubmitted);
            CaptchaLineEdit->enterPressed().connect(this, &Subscription::Impl::OnUnsubscribeFormSubmitted);
            unsubscribePushButton->clicked().connect(this, &Subscription::Impl::OnUnsubscribeFormSubmitted);

            EmailLineEdit->setFocus();
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

    return tmpl;
}

Wt::WWidget *Subscription::Impl::GetCancellationPage()
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    if (cgiEnv->SubscriptionData.Subscribe != CgiEnv::Subscription::Action::Cancel) {
        return new WText(L"Oops!");
    }

    WTemplate *tmpl = new WTemplate();
    tmpl->setId("Cancellation");
    tmpl->setStyleClass("container-table");

    static const regex REGEX_UUID(Pool::Storage()->RegexUuid());
    smatch result;
    if (cgiEnv->SubscriptionData.Uuid == "" || !regex_search(cgiEnv->SubscriptionData.Uuid, result, REGEX_UUID)) {
        cgiRoot->setTitle(tr("home-subscription-invalid-recipient-id-title"));
        this->GetMessageTemplate(tmpl,
                                 tr("home-subscription-invalid-recipient-id-title"),
                                 tr("home-subscription-invalid-recipient-id-message"));
        return tmpl;
    }

    cppdb::result r = Pool::Database()->Sql()
            << (format("SELECT inbox, subscription, pending_cancel FROM \"%1%\""
                       " WHERE uuid=?;")
                % Pool::Database()->GetTableName("SUBSCRIBERS")).str()
            << cgiEnv->SubscriptionData.Uuid << row;

    if (r.empty()) {
        cgiRoot->setTitle(tr("home-subscription-invalid-recipient-id-title"));
        this->GetMessageTemplate(tmpl,
                                 tr("home-subscription-invalid-recipient-id-title"),
                                 tr("home-subscription-invalid-recipient-id-message"));
        return tmpl;
    }

    CDate::Now n;
    string date(lexical_cast<std::string>(n.RawTime));

    if (cgiEnv->SubscriptionData.Timestamp == 0
            || (n.RawTime - cgiEnv->SubscriptionData.Timestamp) >= Pool::Storage()->TokenLifespan()) {
        cgiRoot->setTitle(tr("home-subscription-token-has-expired-title"));
        this->GetMessageTemplate(tmpl,
                                 tr("home-subscription-token-has-expired-title"),
                                 tr("home-subscription-token-has-expired-message"));
        return tmpl;
    }

    string inbox;
    string subscription;
    string pending_cancel;
    r >> inbox >> subscription >> pending_cancel;

    if (pending_cancel == "none" && subscription == "none") {
        cgiRoot->setTitle(tr("home-subscription-cancellation-cancelled-title"));
        this->GetMessageTemplate(tmpl,
                                 tr("home-subscription-cancellation-already-cancelled-title"),
                                 tr("home-subscription-cancellation-already-cancelled-message"));
        return tmpl;
    } else if (pending_cancel == "none") {
        cgiRoot->setTitle(tr("home-subscription-cancellation-invalid-request-title"));
        this->GetMessageTemplate(tmpl,
                                 tr("home-subscription-cancellation-invalid-request-title"),
                                 tr("home-subscription-cancellation-invalid-request-message"));
        return tmpl;
    }

    try {
        string htmlData;
        string file;
        if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
            file = "../templates/home-subscription-cancellation-fa.wtml";
        } else {
            file = "../templates/home-subscription-cancellation.wtml";
        }

        if (CoreLib::FileSystem::Read(file, htmlData)) {
            /// Fill the template
            tmpl->setTemplateText(WString::fromUTF8(htmlData), TextFormat::XHTMLUnsafeText);

            string final_subscription;

            if (pending_cancel == "en_fa") {
                final_subscription = "none";
            } else if (pending_cancel == "en") {
                if (subscription == "en_fa" || subscription == "fa") {
                    final_subscription = "fa";
                } else {
                    final_subscription = "none";
                }
            } else if (pending_cancel == "fa") {
                if (subscription == "en_fa" || subscription == "en") {
                    final_subscription = "en";
                } else {
                    final_subscription = "none";
                }
            } else {
                final_subscription = "none";
            }

            transaction guard(Service::Pool::Database()->Sql());

            Pool::Database()->Update("SUBSCRIBERS",
                                     "inbox",
                                     inbox,
                                     "subscription=?, pending_cancel=?, update_date=?",
                                     { final_subscription, "none", date });

            guard.commit();

            SendMessage(Message::Cancelled, cgiEnv->SubscriptionData.Uuid, inbox);

            tmpl->bindString("title", tr("home-subscription-cancellation-cancelled-title"));
            tmpl->bindString("message", tr("home-subscription-cancellation-cancelled-message"));

            string homePageFields;
            if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
                homePageFields = "homepage_url_fa, homepage_title_fa";
            } else {
                homePageFields = "homepage_url_en, homepage_title_en";
            }

            r = Pool::Database()->Sql()
                    << (format("SELECT %1%"
                               " FROM \"%2%\" WHERE pseudo_id = '0';")
                        % homePageFields
                        % Pool::Database()->GetTableName("SETTINGS")).str()
                    << row;

            if (!r.empty()) {
                string homePageUrl;
                string homePageTitle;

                r >> homePageUrl >> homePageTitle;

                tmpl->bindString("home-page-url", WString::fromUTF8(homePageUrl));
                tmpl->bindString("home-page-title", WString::fromUTF8(homePageTitle));
            }
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

    return tmpl;
}

void Subscription::Impl::GetMessageTemplate(WTemplate *tmpl, const Wt::WString &title, const Wt::WString &message)
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    string htmlData;
    string file;
    if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
        file = "../templates/home-subscription-message-template-fa.wtml";
    } else {
        file = "../templates/home-subscription-message-template.wtml";
    }

    if (CoreLib::FileSystem::Read(file, htmlData)) {
        /// Fill the template
        tmpl->setTemplateText(WString::fromUTF8(htmlData), TextFormat::XHTMLUnsafeText);

        tmpl->bindString("title", title);
        tmpl->bindString("message", message);

        string homePageFields;
        if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
            homePageFields = "homepage_url_fa, homepage_title_fa";
        } else {
            homePageFields = "homepage_url_en, homepage_title_en";
        }

        result r = Pool::Database()->Sql()
                << (format("SELECT %1%"
                           " FROM \"%2%\" WHERE pseudo_id = '0';")
                    % homePageFields
                    % Pool::Database()->GetTableName("SETTINGS")).str()
                << row;

        if (!r.empty()) {
            string homePageUrl;
            string homePageTitle;

            r >> homePageUrl >> homePageTitle;

            tmpl->bindString("home-page-url", WString::fromUTF8(homePageUrl));
            tmpl->bindString("home-page-title", WString::fromUTF8(homePageTitle));
        }
    }
}

void Subscription::Impl::SendMessage(const Message &type, const string &uuid, const string &inbox)
{
    try {
        CDate::Now n;

        CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
        CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

        string htmlData;
        string file;
        if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
            switch (type) {
            case Message::Confirm:
                file = "../templates/email-confirm-subscription-fa.wtml";
                break;
            case Message::Confirmed:
                file = "../templates/email-subscription-confirmed-fa.wtml";
                break;
            case Message::Cancel:
                file = "../templates/email-cancel-subscription-fa.wtml";
                break;
            case Message::Cancelled:
                file = "../templates/email-subscription-cancelled-fa.wtml";
                break;
            }
        } else {
            switch (type) {
            case Message::Confirm:
                file = "../templates/email-confirm-subscription.wtml";
                break;
            case Message::Confirmed:
                file = "../templates/email-subscription-confirmed.wtml";
                break;
            case Message::Cancel:
                file = "../templates/email-cancel-subscription.wtml";
                break;
            case Message::Cancelled:
                file = "../templates/email-subscription-cancelled.wtml";
                break;
            }
        }

        if (CoreLib::FileSystem::Read(file, htmlData)) {
            string subject;

            switch (type) {
            case Message::Confirm:
                subject = (format(tr("email-subject-confirm-subscription").toUTF8())
                                  % cgiEnv->GetServerInfo(CgiEnv::ServerInfo::Host)).str();
                break;
            case Message::Confirmed:
                subject = (format(tr("email-subject-subscription-confirmed").toUTF8())
                                  % cgiEnv->GetServerInfo(CgiEnv::ServerInfo::Host)).str();
                break;
            case Message::Cancel:
                subject = (format(tr("email-subject-cancel-subscription").toUTF8())
                                  % cgiEnv->GetServerInfo(CgiEnv::ServerInfo::Host)).str();
                break;
            case Message::Cancelled:
                subject = (format(tr("email-subject-subscription-cancelled").toUTF8())
                                  % cgiEnv->GetServerInfo(CgiEnv::ServerInfo::Host)).str();
                break;
            }

            replace_all(htmlData, "${client-ip}",
                               cgiEnv->GetClientInfo(CgiEnv::ClientInfo::IP));
            replace_all(htmlData, "${client-location}",
                               cgiEnv->GetClientInfo(CgiEnv::ClientInfo::Location));
            replace_all(htmlData, "${client-user-agent}",
                               cgiEnv->GetClientInfo(CgiEnv::ClientInfo::Browser));
            replace_all(htmlData, "${client-referer}",
                               cgiEnv->GetClientInfo(CgiEnv::ClientInfo::Referer));
            if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
                replace_all(htmlData, "${time}",
                            (format("%1% ~ %2%")
                             % WString(DateConv::FormatToPersianNums(DateConv::ToJalali(n))).toUTF8()
                             % algorithm::trim_copy(DateConv::RawLocalDateTime(n))).str());
            } else {
                replace_all(htmlData, "${time}",
                            algorithm::trim_copy(DateConv::RawLocalDateTime(n)));
            }

            string homePageFields;
            if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
                homePageFields = "homepage_url_fa, homepage_title_fa";
            } else {
                homePageFields = "homepage_url_en, homepage_title_en";
            }

            result r = Pool::Database()->Sql()
                    << (format("SELECT %1%"
                               " FROM \"%2%\" WHERE pseudo_id = '0';")
                        % homePageFields
                        % Pool::Database()->GetTableName("SETTINGS")).str()
                    << row;

            string homePageUrl;
            string homePageTitle;
            if (!r.empty()) {
                r >> homePageUrl >> homePageTitle;
            }

            replace_all(htmlData, "${home-page-url}", homePageUrl);
            replace_all(htmlData, "${home-page-title}", homePageTitle);

            string link(cgiEnv->GetServerInfo(CgiEnv::ServerInfo::URL));

            if (!ends_with(link, "/"))
                link += "/";

            if (type == Message::Confirm) {
                link += (format("?subscribe=2&recipient=%1%")
                         % uuid).str();

                replace_all(htmlData, "${confirm-link}", link);
            } else if (type == Message::Cancel) {
                std::string token;
                Pool::Crypto()->Encrypt(lexical_cast<string>(n.RawTime), token);

                link += (format("?subscribe=-2&recipient=%1%&token=%2%")
                         % uuid
                         % token).str();

                replace_all(htmlData, "${cancel-link}", link);
            }

            CoreLib::Mail *mail = new CoreLib::Mail(
                        cgiEnv->GetServerInfo(CgiEnv::ServerInfo::NoReplyAddr),
                        inbox, subject, htmlData);
            mail->SetDeleteLater(true);
            mail->SendAsync();
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
}

