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
#include <pqxx/pqxx>
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
using namespace pqxx;
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

    switch (cgiEnv->GetInformation().Subscription.Subscribe) {
    case CgiEnv::InformationRecord::SubscriptionRecord::Action::Subscribe:
        cgiRoot->setTitle(tr("home-subscription-subscribe-page-title"));
        break;
    case CgiEnv::InformationRecord::SubscriptionRecord::Action::Confirm:
        cgiRoot->setTitle(tr("home-subscription-confirmation-page-title"));
        break;
    case CgiEnv::InformationRecord::SubscriptionRecord::Action::Unsubscribe:
        cgiRoot->setTitle(tr("home-subscription-unsubscribe-page-title"));
        break;
    case CgiEnv::InformationRecord::SubscriptionRecord::Action::Cancel:
        cgiRoot->setTitle(tr("home-subscription-cancellation-page-title"));
        break;
    case CgiEnv::InformationRecord::SubscriptionRecord::Action::None:
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

    switch (cgiEnv->GetInformation().Subscription.Subscribe) {
    case CgiEnv::InformationRecord::SubscriptionRecord::Action::Subscribe:
        container->addWidget(m_pimpl->GetSubscribeForm());
        break;
    case CgiEnv::InformationRecord::SubscriptionRecord::Action::Confirm:
        container->addWidget(m_pimpl->GetConfirmationPage());
        break;
    case CgiEnv::InformationRecord::SubscriptionRecord::Action::Unsubscribe:
        container->addWidget(m_pimpl->GetUnsubscribeForm());
        break;
    case CgiEnv::InformationRecord::SubscriptionRecord::Action::Cancel:
        container->addWidget(m_pimpl->GetCancellationPage());
        break;
    case CgiEnv::InformationRecord::SubscriptionRecord::Action::None:
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
        } catch (...) {
        }
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

    try {
        if (!EnContentsCheckBox->isChecked() && !FaContentsCheckBox->isChecked()) {
            if (cgiEnv->GetInformation().Client.Language.Code
                    == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
                FaContentsCheckBox->setFocus();
                return;
            } else {
                EnContentsCheckBox->setFocus();
                return;
            }
        }

        CDate::Now n(CDate::Timezone::UTC);
        string date(lexical_cast<std::string>(n.RawTime()));
        string inbox(EmailLineEdit->text().trim().toUTF8());

        string pendingConfirm;
        if (EnContentsCheckBox->isChecked() && FaContentsCheckBox->isChecked()) {
            pendingConfirm = "en_fa";
        } else if (EnContentsCheckBox->isChecked()) {
            pendingConfirm = "en";
        } else if (FaContentsCheckBox->isChecked()) {
            pendingConfirm = "fa";
        } else {
            pendingConfirm = "none";
        }

        auto conn = Pool::Database().Connection();
        conn->activate();
        pqxx::work txn(*conn.get());

        string query((boost::format("SELECT uuid FROM \"%1%\""
                                    " WHERE inbox = %2%;")
                      % txn.esc(Service::Pool::Database().GetTableName("SUBSCRIBERS"))
                      % txn.quote(inbox)).str());
        LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

        result r = txn.exec(query);

        string uuid;

        if (r.empty()) {
            while (true) {
                CoreLib::Random::Uuid(uuid);

                query.assign((boost::format("SELECT inbox FROM \"%1%\""
                                            " WHERE uuid = %2%;")
                              % txn.esc(Service::Pool::Database().GetTableName("SUBSCRIBERS"))
                              % txn.quote(uuid)).str());
                LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

                r = txn.exec(query);

                if (r.empty()) {
                    break;
                }
            }

            Pool::Database().Insert("SUBSCRIBERS",
                                    "inbox, uuid, subscription, pending_confirm, pending_cancel, join_date, update_date",
            { inbox, uuid, "none", pendingConfirm, "none", date, date });
        } else {
            const result::tuple row(r[0]);
            uuid.assign(row["uuid"].c_str());

            Pool::Database().Update("SUBSCRIBERS",
                                    "inbox",
                                    inbox,
                                    "pending_confirm=?, pending_cancel=?",
            { pendingConfirm, "none" });
        }

        SendMessage(Message::Confirm, uuid, inbox);

        MessageBox = std::make_unique<WMessageBox>(tr("home-subscription-subscribe-success-dialog-title"),
                                                   tr("home-subscription-subscribe-success-dialog-message"),
                                                   Information, NoButton);
        MessageBox->addButton(tr("home-dialog-button-ok"), Ok);
        MessageBox->buttonClicked().connect(this, &Subscription::Impl::OnDialogClosed);
        MessageBox->show();

        this->GenerateCaptcha();
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

void Subscription::Impl::OnUnsubscribeFormSubmitted()
{
    if (!m_parent->Validate(EmailLineEdit)
            || !m_parent->Validate(CaptchaLineEdit)) {
        this->GenerateCaptcha();
        return;
    }

    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    try {
        if (!EnContentsCheckBox->isChecked() && !FaContentsCheckBox->isChecked()) {
            if (cgiEnv->GetInformation().Client.Language.Code
                    == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
                FaContentsCheckBox->setFocus();
                return;
            } else {
                EnContentsCheckBox->setFocus();
                return;
            }
        }

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

        auto conn = Pool::Database().Connection();
        conn->activate();
        pqxx::work txn(*conn.get());

        string query((boost::format("SELECT inbox FROM \"%1%\""
                                    " WHERE inbox = %2%;")
                      % txn.esc(Service::Pool::Database().GetTableName("SUBSCRIBERS"))
                      % txn.quote(inbox)).str());
        LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

        result r = txn.exec(query);

        if (r.empty()) {
            MessageBox = std::make_unique<WMessageBox>(tr("home-subscription-invalid-recipient-id-title"),
                                                       tr("home-subscription-invalid-recipient-id-message"),
                                                       Information, NoButton);
            MessageBox->addButton(tr("home-dialog-button-ok"), Ok);
            MessageBox->buttonClicked().connect(this, &Subscription::Impl::OnDialogClosed);
            MessageBox->show();

            this->GenerateCaptcha();

            return;
        }

        Pool::Database().Update("SUBSCRIBERS",
                                "inbox",
                                inbox,
                                "pending_cancel=?",
        { pending_cancel });

        SendMessage(Message::Cancel, cgiEnv->GetInformation().Subscription.Uuid, inbox);

        MessageBox = std::make_unique<WMessageBox>(tr("home-subscription-unsubscribe-success-dialog-title"),
                                                   tr("home-subscription-unsubscribe-success-dialog-message"),
                                                   Information, NoButton);
        MessageBox->addButton(tr("home-dialog-button-ok"), Ok);
        MessageBox->buttonClicked().connect(this, &Subscription::Impl::OnDialogClosed);
        MessageBox->show();

        this->GenerateCaptcha();
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

    if (cgiEnv->GetInformation().Subscription.Subscribe
            != CgiEnv::InformationRecord::SubscriptionRecord::Action::Subscribe
            && cgiEnv->GetInformation().Subscription.Subscribe
            != CgiEnv::InformationRecord::SubscriptionRecord::Action::None) {
        return new WText(L"Oops!");
    }

    WTemplate *tmpl = new WTemplate();
    tmpl->setId("Subscribe");
    tmpl->setStyleClass("container-table");

    try {
        string htmlData;
        string file;
        if (cgiEnv->GetInformation().Client.Language.Code
                == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
            file = "../templates/home-subscription-subscribe-fa.wtml";
        } else {
            file = "../templates/home-subscription-subscribe.wtml";
        }

        if (CoreLib::FileSystem::Read(file, htmlData)) {
            /// Fill the template
            tmpl->setTemplateText(WString::fromUTF8(htmlData), TextFormat::XHTMLUnsafeText);

            EmailLineEdit = new WLineEdit();
            EmailLineEdit->setPlaceholderText(tr("home-subscription-subscribe-email-placeholder"));
            WRegExpValidator *emailValidator = new WRegExpValidator(Pool::Storage().RegexEmail());
            emailValidator->setFlags(MatchCaseInsensitive);
            emailValidator->setMandatory(true);
            EmailLineEdit->setValidator(emailValidator);

            if (cgiEnv->GetInformation().Subscription.Subscribe
                    == CgiEnv::InformationRecord::SubscriptionRecord::Action::Subscribe) {
                static const regex REGEX_EMAIL(Pool::Storage().RegexEmail());
                smatch result;
                if (regex_search(cgiEnv->GetInformation().Subscription.Inbox, result, REGEX_EMAIL)) {
                    EmailLineEdit->setText(WString::fromUTF8(cgiEnv->GetInformation().Subscription.Inbox));
                }
            }

            WSignalMapper<WCheckBox *> *contentsSignalMapper = new WSignalMapper<WCheckBox *>(this);
            contentsSignalMapper->mapped().connect(this, &Subscription::Impl::OnContentsCheckBoxStateChanged);

            EnContentsCheckBox = new WCheckBox();
            FaContentsCheckBox = new WCheckBox();

            if ((cgiEnv->GetInformation().Subscription.Subscribe
                 == CgiEnv::InformationRecord::SubscriptionRecord::Action::Subscribe
                 || cgiEnv->GetInformation().Subscription.Subscribe
                 == CgiEnv::InformationRecord::SubscriptionRecord::Action::None)
                    && cgiEnv->GetInformation().Subscription.Languages.size() > 0) {
                if (std::find(cgiEnv->GetInformation().Subscription.Languages.begin(),
                              cgiEnv->GetInformation().Subscription.Languages.end(),
                              CgiEnv::InformationRecord::SubscriptionRecord::Language::En)
                        != cgiEnv->GetInformation().Subscription.Languages.end()) {
                    EnContentsCheckBox->setChecked(true);
                }

                if (std::find(cgiEnv->GetInformation().Subscription.Languages.begin(),
                              cgiEnv->GetInformation().Subscription.Languages.end(),
                              CgiEnv::InformationRecord::SubscriptionRecord::Language::Fa)
                        != cgiEnv->GetInformation().Subscription.Languages.end()) {
                    FaContentsCheckBox->setChecked(true);
                }
            } else {
                if (cgiEnv->GetInformation().Client.Language.Code
                        == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
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

    return tmpl;
}

Wt::WWidget *Subscription::Impl::GetConfirmationPage()
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    if (cgiEnv->GetInformation().Subscription.Subscribe
            != CgiEnv::InformationRecord::SubscriptionRecord::Action::Confirm) {
        return new WText(L"Oops!");
    }

    WTemplate *tmpl = new WTemplate();
    tmpl->setId("Confirmation");
    tmpl->setStyleClass("container-table");

    try {
        static const regex REGEX_UUID(Pool::Storage().RegexUuid());
        smatch result;
        if (cgiEnv->GetInformation().Subscription.Uuid == ""
                || !regex_search(cgiEnv->GetInformation().Subscription.Uuid, result, REGEX_UUID)) {
            cgiRoot->setTitle(tr("home-subscription-invalid-recipient-id-title"));
            this->GetMessageTemplate(tmpl,
                                     tr("home-subscription-invalid-recipient-id-title"),
                                     tr("home-subscription-invalid-recipient-id-message"));
            return tmpl;
        }

        auto conn = Pool::Database().Connection();
        conn->activate();
        pqxx::work txn(*conn.get());

        string query((boost::format("SELECT inbox, subscription, pending_confirm FROM \"%1%\""
                                    " WHERE uuid = %2%;")
                      % txn.esc(Service::Pool::Database().GetTableName("SUBSCRIBERS"))
                      % txn.quote(cgiEnv->GetInformation().Subscription.Uuid)).str());
        LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

        pqxx::result r = txn.exec(query);

        if (r.empty()) {
            cgiRoot->setTitle(tr("home-subscription-invalid-recipient-id-title"));
            this->GetMessageTemplate(tmpl,
                                     tr("home-subscription-invalid-recipient-id-title"),
                                     tr("home-subscription-invalid-recipient-id-message"));
            return tmpl;
        }

        CDate::Now n(CDate::Timezone::UTC);
        string date(lexical_cast<std::string>(n.RawTime()));

        const result::tuple row(r[0]);
        const string inbox(row["inbox"].c_str());
        const string subscription(row["subscription"].c_str());
        const string pendingConfirm(row["pending_confirm"].c_str());

        if (pendingConfirm == "none" && subscription == "none") {
            cgiEnv->SetSubscriptionAction(CgiEnv::InformationRecord::SubscriptionRecord::Action::Subscribe);
            cgiEnv->SetSubscriptionInbox(inbox);

            if (subscription == "en_fa") {
                cgiEnv->AddSubscriptionLanguage(CgiEnv::InformationRecord::SubscriptionRecord::Language::En);
                cgiEnv->AddSubscriptionLanguage(CgiEnv::InformationRecord::SubscriptionRecord::Language::Fa);
            } else if (subscription == "en") {
                cgiEnv->AddSubscriptionLanguage(CgiEnv::InformationRecord::SubscriptionRecord::Language::En);
            } else if (subscription == "fa") {
                cgiEnv->AddSubscriptionLanguage(CgiEnv::InformationRecord::SubscriptionRecord::Language::Fa);
            } else {
                cgiEnv->AddSubscriptionLanguage(CgiEnv::InformationRecord::SubscriptionRecord::Language::En);

                if (cgiEnv->GetInformation().Client.Language.Code
                        == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
                    cgiEnv->AddSubscriptionLanguage(CgiEnv::InformationRecord::SubscriptionRecord::Language::Fa);
                }
            }

            cgiRoot->setTitle(tr("home-subscription-subscribe-page-title"));

            return GetSubscribeForm();
        } else if (pendingConfirm == "none") {
            cgiRoot->setTitle(tr("home-subscription-confirmation-already-confirmed-title"));
            this->GetMessageTemplate(tmpl,
                                     tr("home-subscription-confirmation-already-confirmed-title"),
                                     tr("home-subscription-confirmation-already-confirmed-message"));
            return tmpl;
        }

        string htmlData;
        string file;
        if (cgiEnv->GetInformation().Client.Language.Code
                == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
            file = "../templates/home-subscription-confirmation-fa.wtml";
        } else {
            file = "../templates/home-subscription-confirmation.wtml";
        }

        if (CoreLib::FileSystem::Read(file, htmlData)) {
            /// Fill the template
            tmpl->setTemplateText(WString::fromUTF8(htmlData), TextFormat::XHTMLUnsafeText);

            string finalSubscription;

            if (subscription == "none") {
                finalSubscription = pendingConfirm;
            } else if (subscription == "en_fa") {
                finalSubscription = subscription;
            } else if (subscription == "en") {
                if (pendingConfirm == "fa") {
                    finalSubscription = "en_fa";
                } else {
                    finalSubscription = subscription;
                }
            } else if (subscription == "fa") {
                if (pendingConfirm == "en") {
                    finalSubscription = "en_fa";
                } else {
                    finalSubscription = subscription;
                }
            } else {
                finalSubscription = "en_fa";
            }

            Pool::Database().Update("SUBSCRIBERS",
                                    "inbox",
                                    inbox,
                                    "subscription=?, pending_confirm=?, pending_cancel=?, update_date=?",
            { finalSubscription, "none", "none", date });

            SendMessage(Message::Confirmed, cgiEnv->GetInformation().Subscription.Uuid, inbox);

            tmpl->bindString("title", tr("home-subscription-confirmation-congratulation-title"));
            tmpl->bindString("message", tr("home-subscription-confirmation-congratulation-message"));

            string homePageFields;
            if (cgiEnv->GetInformation().Client.Language.Code
                    == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
                homePageFields = "homepage_url_fa, homepage_title_fa";
            } else {
                homePageFields = "homepage_url_en, homepage_title_en";
            }

            query.assign((boost::format("SELECT %1% FROM \"%2%\""
                                        " WHERE pseudo_id = '0';")
                          % homePageFields
                          % txn.esc(Service::Pool::Database().GetTableName("SETTINGS"))).str());
            LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

            pqxx::result r = txn.exec(query);

            if (!r.empty()) {
                const result::tuple row(r[0]);
                const string homePageUrl(row[0].c_str());
                const string homePageTitle(row[1].c_str());

                tmpl->bindString("home-page-url", WString::fromUTF8(homePageUrl));
                tmpl->bindString("home-page-title", WString::fromUTF8(homePageTitle));
            }
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

    return tmpl;
}

Wt::WWidget *Subscription::Impl::GetUnsubscribeForm()
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    if (cgiEnv->GetInformation().Subscription.Subscribe
            != CgiEnv::InformationRecord::SubscriptionRecord::Action::Unsubscribe) {
        return new WText(L"Oops!");
    }

    WTemplate *tmpl = new WTemplate();
    tmpl->setId("Unsubscribe");
    tmpl->setStyleClass("container-table");

    try {
        static const regex REGEX_UUID(Pool::Storage().RegexUuid());
        smatch result;
        if (cgiEnv->GetInformation().Subscription.Uuid == ""
                || !regex_search(cgiEnv->GetInformation().Subscription.Uuid, result, REGEX_UUID)) {
            cgiRoot->setTitle(tr("home-subscription-invalid-recipient-id-title"));
            this->GetMessageTemplate(tmpl,
                                     tr("home-subscription-invalid-recipient-id-title"),
                                     tr("home-subscription-invalid-recipient-id-message"));
            return tmpl;
        }

        auto conn = Pool::Database().Connection();
        conn->activate();
        pqxx::work txn(*conn.get());

        string query((boost::format("SELECT inbox, subscription FROM \"%1%\""
                                    " WHERE uuid = %2%;")
                      % txn.esc(Service::Pool::Database().GetTableName("SUBSCRIBERS"))
                      % txn.quote(cgiEnv->GetInformation().Subscription.Uuid)).str());
        LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

        pqxx::result r = txn.exec(query);

        if (r.empty()) {
            cgiRoot->setTitle(tr("home-subscription-invalid-recipient-id-title"));
            this->GetMessageTemplate(tmpl,
                                     tr("home-subscription-invalid-recipient-id-title"),
                                     tr("home-subscription-invalid-recipient-id-message"));
            return tmpl;
        }

        const result::tuple row(r[0]);
        const string inbox(row["inbox"].c_str());
        const string subscription(row["subscription"].c_str());

        if (subscription == "none") {
            cgiRoot->setTitle(tr("home-subscription-unsubscribe-already-unsubscribed-title"));
            this->GetMessageTemplate(tmpl,
                                     tr("home-subscription-unsubscribe-already-unsubscribed-title"),
                                     tr("home-subscription-unsubscribe-already-unsubscribed-message"));
            return tmpl;
        }

        string htmlData;
        string file;
        if (cgiEnv->GetInformation().Client.Language.Code
                == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
            file = "../templates/home-subscription-unsubscribe-fa.wtml";
        } else {
            file = "../templates/home-subscription-unsubscribe.wtml";
        }

        if (CoreLib::FileSystem::Read(file, htmlData)) {
            /// Fill the template
            tmpl->setTemplateText(WString::fromUTF8(htmlData), TextFormat::XHTMLUnsafeText);

            EmailLineEdit = new WLineEdit();
            EmailLineEdit->setPlaceholderText(tr("home-subscription-unsubscribe-email-placeholder"));
            WRegExpValidator *emailValidator = new WRegExpValidator(Pool::Storage().RegexEmail());
            emailValidator->setFlags(MatchCaseInsensitive);
            emailValidator->setMandatory(true);
            EmailLineEdit->setValidator(emailValidator);
            EmailLineEdit->setReadOnly(true);

            static const regex REGEX_EMAIL(Pool::Storage().RegexEmail());
            if (regex_search(inbox, result, REGEX_EMAIL)) {
                EmailLineEdit->setText(WString::fromUTF8(inbox));
            }

            WSignalMapper<WCheckBox *> *contentsSignalMapper = new WSignalMapper<WCheckBox *>(this);
            contentsSignalMapper->mapped().connect(this, &Subscription::Impl::OnContentsCheckBoxStateChanged);

            EnContentsCheckBox = new WCheckBox();
            FaContentsCheckBox = new WCheckBox();

            if (cgiEnv->GetInformation().Subscription.Languages.size() > 0) {
                if (std::find(cgiEnv->GetInformation().Subscription.Languages.begin(),
                              cgiEnv->GetInformation().Subscription.Languages.end(),
                              CgiEnv::InformationRecord::SubscriptionRecord::Language::En)
                        != cgiEnv->GetInformation().Subscription.Languages.end()) {
                    EnContentsCheckBox->setChecked(true);
                }

                if (std::find(cgiEnv->GetInformation().Subscription.Languages.begin(),
                              cgiEnv->GetInformation().Subscription.Languages.end(),
                              CgiEnv::InformationRecord::SubscriptionRecord::Language::Fa)
                        != cgiEnv->GetInformation().Subscription.Languages.end()) {
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
                    if (cgiEnv->GetInformation().Client.Language.Code
                            == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
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

    return tmpl;
}

Wt::WWidget *Subscription::Impl::GetCancellationPage()
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    if (cgiEnv->GetInformation().Subscription.Subscribe
            != CgiEnv::InformationRecord::SubscriptionRecord::Action::Cancel) {
        return new WText(L"Oops!");
    }

    WTemplate *tmpl = new WTemplate();
    tmpl->setId("Cancellation");
    tmpl->setStyleClass("container-table");

    try {
        static const regex REGEX_UUID(Pool::Storage().RegexUuid());
        smatch result;
        if (cgiEnv->GetInformation().Subscription.Uuid == ""
                || !regex_search(cgiEnv->GetInformation().Subscription.Uuid, result, REGEX_UUID)) {
            cgiRoot->setTitle(tr("home-subscription-invalid-recipient-id-title"));
            this->GetMessageTemplate(tmpl,
                                     tr("home-subscription-invalid-recipient-id-title"),
                                     tr("home-subscription-invalid-recipient-id-message"));
            return tmpl;
        }

        auto conn = Pool::Database().Connection();
        conn->activate();
        pqxx::work txn(*conn.get());

        string query((boost::format("SELECT inbox, subscription, pending_cancel FROM \"%1%\""
                                    " WHERE uuid = %2%;")
                      % txn.esc(Service::Pool::Database().GetTableName("SUBSCRIBERS"))
                      % txn.quote(cgiEnv->GetInformation().Subscription.Uuid)).str());
        LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

        pqxx::result r = txn.exec(query);

        if (r.empty()) {
            cgiRoot->setTitle(tr("home-subscription-invalid-recipient-id-title"));
            this->GetMessageTemplate(tmpl,
                                     tr("home-subscription-invalid-recipient-id-title"),
                                     tr("home-subscription-invalid-recipient-id-message"));
            return tmpl;
        }

        CDate::Now n(CDate::Timezone::UTC);
        string date(lexical_cast<std::string>(n.RawTime()));

        if (cgiEnv->GetInformation().Subscription.Timestamp == 0
                || (n.RawTime() - cgiEnv->GetInformation().Subscription.Timestamp) >= Pool::Storage().TokenLifespan()) {
            cgiRoot->setTitle(tr("home-subscription-token-has-expired-title"));
            this->GetMessageTemplate(tmpl,
                                     tr("home-subscription-token-has-expired-title"),
                                     tr("home-subscription-token-has-expired-message"));
            return tmpl;
        }

        const result::tuple row(r[0]);
        const string inbox(row["inbox"].c_str());
        const string subscription(row["subscription"].c_str());
        const string pendingCancel(row["pending_cancel"].c_str());

        if (pendingCancel == "none" && subscription == "none") {
            cgiRoot->setTitle(tr("home-subscription-cancellation-cancelled-title"));
            this->GetMessageTemplate(tmpl,
                                     tr("home-subscription-cancellation-already-cancelled-title"),
                                     tr("home-subscription-cancellation-already-cancelled-message"));
            return tmpl;
        } else if (pendingCancel == "none") {
            cgiRoot->setTitle(tr("home-subscription-cancellation-invalid-request-title"));
            this->GetMessageTemplate(tmpl,
                                     tr("home-subscription-cancellation-invalid-request-title"),
                                     tr("home-subscription-cancellation-invalid-request-message"));
            return tmpl;
        }

        string htmlData;
        string file;
        if (cgiEnv->GetInformation().Client.Language.Code
                == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
            file = "../templates/home-subscription-cancellation-fa.wtml";
        } else {
            file = "../templates/home-subscription-cancellation.wtml";
        }

        if (CoreLib::FileSystem::Read(file, htmlData)) {
            /// Fill the template
            tmpl->setTemplateText(WString::fromUTF8(htmlData), TextFormat::XHTMLUnsafeText);

            string finalSubscription;

            if (pendingCancel == "en_fa") {
                finalSubscription = "none";
            } else if (pendingCancel == "en") {
                if (subscription == "en_fa" || subscription == "fa") {
                    finalSubscription = "fa";
                } else {
                    finalSubscription = "none";
                }
            } else if (pendingCancel == "fa") {
                if (subscription == "en_fa" || subscription == "en") {
                    finalSubscription = "en";
                } else {
                    finalSubscription = "none";
                }
            } else {
                finalSubscription = "none";
            }

            Pool::Database().Update("SUBSCRIBERS",
                                    "inbox",
                                    inbox,
                                    "subscription=?, pending_cancel=?, update_date=?",
            { finalSubscription, "none", date });

            SendMessage(Message::Cancelled, cgiEnv->GetInformation().Subscription.Uuid, inbox);

            tmpl->bindString("title", tr("home-subscription-cancellation-cancelled-title"));
            tmpl->bindString("message", tr("home-subscription-cancellation-cancelled-message"));

            string homePageFields;
            if (cgiEnv->GetInformation().Client.Language.Code
                    == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
                homePageFields = "homepage_url_fa, homepage_title_fa";
            } else {
                homePageFields = "homepage_url_en, homepage_title_en";
            }

            query.assign((boost::format("SELECT %1% FROM \"%2%\""
                                        " WHERE pseudo_id = '0';")
                          % homePageFields
                          % txn.esc(Service::Pool::Database().GetTableName("SETTINGS"))).str());
            LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

            r = txn.exec(query);

            if (!r.empty()) {
                const result::tuple row(r[0]);
                const string homePageUrl(row[0].c_str());
                const string homePageTitle(row[1].c_str());

                tmpl->bindString("home-page-url", WString::fromUTF8(homePageUrl));
                tmpl->bindString("home-page-title", WString::fromUTF8(homePageTitle));
            }
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

    return tmpl;
}

void Subscription::Impl::GetMessageTemplate(WTemplate *tmpl, const Wt::WString &title, const Wt::WString &message)
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    try {
        string htmlData;
        string file;
        if (cgiEnv->GetInformation().Client.Language.Code
                == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
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
            if (cgiEnv->GetInformation().Client.Language.Code
                    == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
                homePageFields = "homepage_url_fa, homepage_title_fa";
            } else {
                homePageFields = "homepage_url_en, homepage_title_en";
            }

            auto conn = Pool::Database().Connection();
            conn->activate();
            pqxx::work txn(*conn.get());

            string query((boost::format("SELECT %1% FROM \"%2%\""
                                        " WHERE pseudo_id = '0';")
                          % homePageFields
                          % txn.esc(Service::Pool::Database().GetTableName("SETTINGS"))).str());
            LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

            result r = txn.exec(query);

            if (!r.empty()) {
                const result::tuple row(r[0]);
                const string homePageUrl(row[0].c_str());
                const string homePageTitle(row[1].c_str());

                tmpl->bindString("home-page-url", WString::fromUTF8(homePageUrl));
                tmpl->bindString("home-page-title", WString::fromUTF8(homePageTitle));
            }
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

void Subscription::Impl::SendMessage(const Message &type, const string &uuid, const string &inbox)
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    try {
        CDate::Now n(CDate::Timezone::UTC);

        string htmlData;
        string file;
        if (cgiEnv->GetInformation().Client.Language.Code
                == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
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
                           % cgiEnv->GetInformation().Server.Hostname).str();
                break;
            case Message::Confirmed:
                subject = (format(tr("email-subject-subscription-confirmed").toUTF8())
                           % cgiEnv->GetInformation().Server.Hostname).str();
                break;
            case Message::Cancel:
                subject = (format(tr("email-subject-cancel-subscription").toUTF8())
                           % cgiEnv->GetInformation().Server.Hostname).str();
                break;
            case Message::Cancelled:
                subject = (format(tr("email-subject-subscription-cancelled").toUTF8())
                           % cgiEnv->GetInformation().Server.Hostname).str();
                break;
            }

            replace_all(htmlData, "${client-ip}",
                        cgiEnv->GetInformation().Client.IPAddress);
            replace_all(htmlData, "${client-user-agent}",
                        cgiEnv->GetInformation().Client.UserAgent);
            replace_all(htmlData, "${client-referer}",
                        cgiEnv->GetInformation().Client.Referer);

            if (cgiEnv->GetInformation().Client.Language.Code
                    == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
                replace_all(htmlData, "${time}",
                            (format("%1% ~ %2%")
                             % WString(DateConv::FormatToPersianNums(DateConv::ToJalali(n))).toUTF8()
                             % algorithm::trim_copy(DateConv::DateTimeString(n))).str());
            } else {
                replace_all(htmlData, "${time}",
                            algorithm::trim_copy(DateConv::DateTimeString(n)));
            }

            replace_all(htmlData, "${client-location-country-code}",
                        cgiEnv->GetInformation().Client.GeoLocation.CountryCode);
            replace_all(htmlData, "${client-location-country-code3}",
                        cgiEnv->GetInformation().Client.GeoLocation.CountryCode3);
            replace_all(htmlData, "${client-location-country-name}",
                        cgiEnv->GetInformation().Client.GeoLocation.CountryName);
            replace_all(htmlData, "${client-location-region}",
                        cgiEnv->GetInformation().Client.GeoLocation.Region);
            replace_all(htmlData, "${client-location-city}",
                        cgiEnv->GetInformation().Client.GeoLocation.City);
            replace_all(htmlData, "${client-location-postal-code}",
                        cgiEnv->GetInformation().Client.GeoLocation.PostalCode);
            replace_all(htmlData, "${client-location-latitude}",
                        lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.Latitude));
            replace_all(htmlData, "${client-location-longitude}",
                        lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.Longitude));
            replace_all(htmlData, "${client-location-metro-code}",
                        lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.MetroCode));
            replace_all(htmlData, "${client-location-dma-code}",
                        lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.DmaCode));
            replace_all(htmlData, "${client-location-area-code}",
                        lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.AreaCode));
            replace_all(htmlData, "${client-location-charset}",
                        lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.Charset));
            replace_all(htmlData, "${client-location-continent-code}",
                        cgiEnv->GetInformation().Client.GeoLocation.ContinentCode);
            replace_all(htmlData, "${client-location-netmask}",
                        lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.Netmask));

            string homePageFields;
            if (cgiEnv->GetInformation().Client.Language.Code
                    == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
                homePageFields = "homepage_url_fa, homepage_title_fa";
            } else {
                homePageFields = "homepage_url_en, homepage_title_en";
            }

            auto conn = Pool::Database().Connection();
            conn->activate();
            pqxx::work txn(*conn.get());

            string query((boost::format("SELECT %1% FROM \"%2%\""
                                        " WHERE pseudo_id = '0';")
                          % homePageFields
                          % txn.esc(Service::Pool::Database().GetTableName("SETTINGS"))).str());
            LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

            result r = txn.exec(query);

            string homePageUrl;
            string homePageTitle;
            if (!r.empty()) {
                const result::tuple row(r[0]);
                homePageUrl.assign(row[0].c_str());
                homePageTitle.assign(row[1].c_str());
            }

            replace_all(htmlData, "${home-page-url}", homePageUrl);
            replace_all(htmlData, "${home-page-title}", homePageTitle);

            string link(cgiEnv->GetInformation().Server.Url);

            if (!ends_with(link, "/"))
                link += "/";

            if (type == Message::Confirm) {
                link += (format("?subscribe=2&recipient=%1%")
                         % uuid).str();

                replace_all(htmlData, "${confirm-link}", link);
            } else if (type == Message::Cancel) {
                std::string token;
                Pool::Crypto().Encrypt(lexical_cast<string>(n.RawTime()), token);

                link += (format("?subscribe=-2&recipient=%1%&token=%2%")
                         % uuid
                         % token).str();

                replace_all(htmlData, "${cancel-link}", link);
            }

            CoreLib::Mail *mail = new CoreLib::Mail(
                        cgiEnv->GetInformation().Server.NoReplyAddress,
                        inbox, subject, htmlData);
            mail->SetDeleteLater(true);
            mail->SendAsync();
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
