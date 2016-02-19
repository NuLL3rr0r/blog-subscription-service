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
 * The contact form shown to the end-user.
 */


#include <boost/algorithm/string.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/format.hpp>
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WComboBox>
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
#include "ContactForm.hpp"
#include "Div.hpp"
#include "Pool.hpp"

#define         UNKNOWN_ERROR       "Unknown error!"

using namespace std;
using namespace boost;
using namespace cppdb;
using namespace Wt;
using namespace CoreLib;
using namespace CDate;
using namespace Service;

struct ContactForm::Impl : public Wt::WObject
{
public:
    Wt::WComboBox *RecipientComboBox;
    Wt::WLineEdit *FromLineEdit;
    Wt::WLineEdit *EmailLineEdit;
    Wt::WLineEdit *UrlLineEdit;
    Wt::WLineEdit *SubjectLineEdit;
    Wt::WTextArea *BodyTextArea;
    Wt::WLineEdit *CaptchaLineEdit;
    Service::Captcha *Captcha;
    Wt::WIntValidator *CaptchaValidator;
    Wt::WImage *CaptchaImage;

    std::unique_ptr<Wt::WMessageBox> MessageBox;

    bool UseRootEmailAsRecipient;
    int DefaultRecipientId;

private:
    ContactForm *m_parent;

public:
    explicit Impl(ContactForm *parent);
    ~Impl();

public:
    void OnContactFormSubmitted();
    void OnClearButtonPressed();
    void OnDialogClosed(Wt::StandardButton button);

private:
    void GenerateCaptcha();
    void SendUserMessageEmail(const std::string &to, CDate::Now &n);
    void ClearForm();
};

ContactForm::ContactForm()
    : Page(),
    m_pimpl(make_unique<ContactForm::Impl>(this))
{
    WApplication *app = WApplication::instance();
    app->setTitle(tr("home-contact-form-page-title"));

    this->clear();
    this->setId("ContactFormPage");
    this->setStyleClass("contact-form-page full-width full-height");
    this->addWidget(this->Layout());
}

ContactForm::~ContactForm() = default;

WWidget *ContactForm::Layout()
{
    Div *container = new Div("ContactForm", "contact-form-layout full-width full-height");

    try {
        CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
        CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

        string htmlData;
        string file;
        if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
            file = "../templates/home-contact-form-fa.wtml";
        } else {
            file = "../templates/home-contact-form.wtml";
        }

        if (CoreLib::FileSystem::Read(file, htmlData)) {
            /// Fill the template
            WTemplate *tmpl = new WTemplate(container);
            tmpl->setStyleClass("container-table");
            tmpl->setTemplateText(WString(htmlData), TextFormat::XHTMLUnsafeText);

            m_pimpl->RecipientComboBox = new WComboBox();

            {
                string recipientsQuery;
                if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
                    recipientsQuery = (format("SELECT recipient_fa, is_default"
                                              " FROM \"%1%\" ORDER BY recipient_fa ASC;")
                                       % Pool::Database()->GetTableName("CONTACTS")).str();
                } else {
                    recipientsQuery = (format("SELECT recipient, is_default"
                                              " FROM \"%1%\" ORDER BY recipient ASC;")
                                       % Pool::Database()->GetTableName("CONTACTS")).str();
                }

                result r = Pool::Database()->Sql() << recipientsQuery;

                int count = 0;
                string recipient;
                string is_default;

                while (r.next()) {
                    r >> recipient >> is_default;
                    m_pimpl->RecipientComboBox->addItem(WString::fromUTF8(recipient));
                    if (Database::IsTrue(is_default)) {
                        m_pimpl->RecipientComboBox->setCurrentIndex(count);
                        m_pimpl->DefaultRecipientId = count;
                    }
                    ++count;
                }

                if (count == 0) {
                    m_pimpl->UseRootEmailAsRecipient = true;
                    m_pimpl->RecipientComboBox->addItem(tr("home-contact-form-admin"));
                }
            }

            m_pimpl->FromLineEdit = new WLineEdit();
            m_pimpl->FromLineEdit->setPlaceholderText(tr("home-contact-form-from-placeholder"));
            WLengthValidator *fromValidator = new WLengthValidator(Pool::Storage()->MinEmailSenderNameLength(),
                                                                       Pool::Storage()->MaxEmailSenderNameLength());
            fromValidator->setMandatory(true);
            m_pimpl->FromLineEdit->setValidator(fromValidator);

            m_pimpl->EmailLineEdit = new WLineEdit();
            m_pimpl->EmailLineEdit->setPlaceholderText(tr("home-contact-form-email-placeholder"));
            WRegExpValidator *emailValidator = new WRegExpValidator(Pool::Storage()->RegexEmail());
            emailValidator->setFlags(MatchCaseInsensitive);
            emailValidator->setMandatory(true);
            m_pimpl->EmailLineEdit->setValidator(emailValidator);

            m_pimpl->UrlLineEdit = new WLineEdit();
            m_pimpl->UrlLineEdit->setPlaceholderText(tr("home-contact-form-url-placeholder"));
            WRegExpValidator *urlValidator = new WRegExpValidator(Pool::Storage()->RegexHttpUrl());
            urlValidator->setFlags(MatchCaseInsensitive);
            urlValidator->setMandatory(false);
            m_pimpl->UrlLineEdit->setValidator(urlValidator);

            m_pimpl->SubjectLineEdit = new WLineEdit();
            m_pimpl->SubjectLineEdit->setPlaceholderText(tr("home-contact-form-subject-placeholder"));
            WLengthValidator *subjectValidator = new WLengthValidator(Pool::Storage()->MinEmailSubjectLength(),
                                                                       Pool::Storage()->MaxEmailSubjectLength());
            subjectValidator->setMandatory(true);
            m_pimpl->SubjectLineEdit->setValidator(subjectValidator);

            m_pimpl->BodyTextArea = new WTextArea();
            m_pimpl->BodyTextArea->setPlaceholderText(tr("home-contact-form-body-placeholder"));
            WLengthValidator *bodyValidator = new WLengthValidator(Pool::Storage()->MinEmailBodyLength(),
                                                                       Pool::Storage()->MaxEmailBodyLength());
            bodyValidator->setMandatory(true);
            m_pimpl->BodyTextArea->setValidator(bodyValidator);

            m_pimpl->Captcha = new Service::Captcha();
            m_pimpl->CaptchaImage = m_pimpl->Captcha->Generate();
            m_pimpl->CaptchaImage->setAlternateText(tr("home-captcha-hint"));
            m_pimpl->CaptchaImage->setAttributeValue("title", tr("home-captcha-hint"));

            int captchaResult = (int)m_pimpl->Captcha->GetResult();

            m_pimpl->CaptchaLineEdit = new WLineEdit();
            m_pimpl->CaptchaLineEdit->setPlaceholderText(tr("home-captcha-hint"));
            m_pimpl->CaptchaValidator = new WIntValidator(captchaResult, captchaResult);
            m_pimpl->CaptchaValidator->setMandatory(true);
            m_pimpl->CaptchaLineEdit->setValidator(m_pimpl->CaptchaValidator);

            WPushButton *sendPushButton = new WPushButton(tr("home-contact-form-send"));
            sendPushButton->setStyleClass("btn btn-primary");

            WPushButton *clearPushButton = new WPushButton(tr("home-contact-form-clear"));
            clearPushButton->setStyleClass("btn btn-warning");

            tmpl->bindString("recipient-select-id", m_pimpl->RecipientComboBox->id());
            tmpl->bindString("from-input-id", m_pimpl->FromLineEdit->id());
            tmpl->bindString("email-input-id", m_pimpl->EmailLineEdit->id());
            tmpl->bindString("url-input-id", m_pimpl->UrlLineEdit->id());
            tmpl->bindString("subject-input-id", m_pimpl->SubjectLineEdit->id());
            tmpl->bindString("body-textarea-id", m_pimpl->BodyTextArea->id());
            tmpl->bindString("captcha-input-id", m_pimpl->CaptchaLineEdit->id());

            tmpl->bindWidget("contact-form-title", new WText(tr("home-contact-form-page-title")));
            tmpl->bindWidget("recipient-label-text", new WText(tr("home-contact-form-recipient")));
            tmpl->bindWidget("from-label-text", new WText(tr("home-contact-form-from")));
            tmpl->bindWidget("email-label-text", new WText(tr("home-contact-form-email")));
            tmpl->bindWidget("url-label-text", new WText(tr("home-contact-form-url")));
            tmpl->bindWidget("subject-label-text", new WText(tr("home-contact-form-subject")));
            tmpl->bindWidget("body-label-text", new WText(tr("home-contact-form-body")));
            tmpl->bindWidget("captcha-label-text", new WText(tr("home-captcha")));

            tmpl->bindWidget("recipient-select", m_pimpl->RecipientComboBox);
            tmpl->bindWidget("from-input", m_pimpl->FromLineEdit);
            tmpl->bindWidget("email-input", m_pimpl->EmailLineEdit);
            tmpl->bindWidget("url-input", m_pimpl->UrlLineEdit);
            tmpl->bindWidget("subject-input", m_pimpl->SubjectLineEdit);
            tmpl->bindWidget("body-textarea", m_pimpl->BodyTextArea);
            tmpl->bindWidget("captcha-input", m_pimpl->CaptchaLineEdit);
            tmpl->bindWidget("captcha-image", m_pimpl->CaptchaImage);
            tmpl->bindWidget("send-button", sendPushButton);
            tmpl->bindWidget("clear-button", clearPushButton);

            m_pimpl->RecipientComboBox->enterPressed().connect(m_pimpl.get(), &ContactForm::Impl::OnContactFormSubmitted);
            m_pimpl->FromLineEdit->enterPressed().connect(m_pimpl.get(), &ContactForm::Impl::OnContactFormSubmitted);
            m_pimpl->EmailLineEdit->enterPressed().connect(m_pimpl.get(), &ContactForm::Impl::OnContactFormSubmitted);
            m_pimpl->UrlLineEdit->enterPressed().connect(m_pimpl.get(), &ContactForm::Impl::OnContactFormSubmitted);
            m_pimpl->SubjectLineEdit->enterPressed().connect(m_pimpl.get(), &ContactForm::Impl::OnContactFormSubmitted);
            // No event for m_pimpl->BodyTextArea
            m_pimpl->CaptchaLineEdit->enterPressed().connect(m_pimpl.get(), &ContactForm::Impl::OnContactFormSubmitted);
            sendPushButton->clicked().connect(m_pimpl.get(), &ContactForm::Impl::OnContactFormSubmitted);
            clearPushButton->clicked().connect(m_pimpl.get(), &ContactForm::Impl::OnClearButtonPressed);

            m_pimpl->RecipientComboBox->setFocus();
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

ContactForm::Impl::Impl(ContactForm *parent)
    : UseRootEmailAsRecipient(false),
      DefaultRecipientId(0),
      m_parent(parent)
{

}

ContactForm::Impl::~Impl() = default;

void ContactForm::Impl::OnContactFormSubmitted()
{
    if (!m_parent->Validate(FromLineEdit)
            || !m_parent->Validate(EmailLineEdit)
            || !m_parent->Validate(UrlLineEdit)
            || !m_parent->Validate(SubjectLineEdit)
            || !m_parent->Validate(BodyTextArea)
            || !m_parent->Validate(CaptchaLineEdit)) {
        GenerateCaptcha();
        return;
    }

    try {
        CDate::Now n;
        CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
        CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

        string email;
        if (!UseRootEmailAsRecipient) {
            string recipientColumn;
            if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
                recipientColumn = "recipient_fa";
            } else {
                recipientColumn = "recipient";
            }

            string recipient = RecipientComboBox->currentText().trim().toUTF8();

            result r = Pool::Database()->Sql()
                    << (format("SELECT address FROM \"%1%\""
                               " WHERE %2%=?;")
                        % Pool::Database()->GetTableName("CONTACTS") % recipientColumn).str()
                    << recipient << row;

            if (!r.empty()) {
                r >> email;
            }
        } else {
            result r = Pool::Database()->Sql()
                    << (format("SELECT email"
                               " FROM \"%1%\" WHERE username=?;")
                        % Pool::Database()->GetTableName("ROOT")).str()
                    << Service::Pool::Storage()->RootUsername() << row;

            if (!r.empty()) {
                r >> email;
            }
        }

        if (email != "") {
            this->SendUserMessageEmail(email, n);

            MessageBox = std::make_unique<WMessageBox>(tr("home-contact-form-send-success-dialog-title"),
                                                       tr("home-contact-form-send-success-dialog-message"),
                                                       Information, NoButton);
            MessageBox->addButton(tr("home-contact-form-dialog-button-ok"), Ok);
            MessageBox->buttonClicked().connect(this, &ContactForm::Impl::OnDialogClosed);
            MessageBox->show();

            this->GenerateCaptcha();
            this->ClearForm();
        } else {
            MessageBox = std::make_unique<WMessageBox>(tr("home-contact-form-no-recipient-error-dialog-title"),
                                                       tr("home-contact-form-no-recipient-error-dialog-message"),
                                                       Critical, NoButton);
            MessageBox->addButton(tr("home-contact-form-dialog-button-ok"), Ok);
            MessageBox->buttonClicked().connect(this, &ContactForm::Impl::OnDialogClosed);
            MessageBox->show();

            this->GenerateCaptcha();
        }

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

    MessageBox = std::make_unique<WMessageBox>(tr("home-contact-form-send-error-dialog-title"),
                                               tr("home-contact-form-send-error-dialog-message"),
                                               Critical, NoButton);
    MessageBox->addButton(tr("home-contact-form-dialog-button-ok"), Ok);
    MessageBox->buttonClicked().connect(this, &ContactForm::Impl::OnDialogClosed);
    MessageBox->show();

    this->GenerateCaptcha();
}

void ContactForm::Impl::OnClearButtonPressed()
{
    this->GenerateCaptcha();
    this->ClearForm();
}

void ContactForm::Impl::OnDialogClosed(Wt::StandardButton button)
{
    (void)button;
    MessageBox.reset();
}

void ContactForm::Impl::GenerateCaptcha()
{
    CaptchaImage->setImageRef(Captcha->Generate()->imageRef());
    int captchaResult = (int)Captcha->GetResult();
    CaptchaValidator->setRange(captchaResult, captchaResult);
}

void ContactForm::Impl::SendUserMessageEmail(const std::string &to, CDate::Now &n)
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    string htmlData;
    string file;
    if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
        file = "../templates/email-user-message-fa.wtml";
    } else {
        file = "../templates/email-user-message.wtml";
    }

    string name(FromLineEdit->text().trim().toUTF8());
    string from(EmailLineEdit->text().trim().toUTF8());
    string url(UrlLineEdit->text().trim().toUTF8());
    string subject(SubjectLineEdit->text().trim().toUTF8());
    string body(BodyTextArea->text().trim().toUTF8());

    if (CoreLib::FileSystem::Read(file, htmlData)) {
        replace_all(htmlData, "${from}", name);
        replace_all(htmlData, "${email}", from);
        replace_all(htmlData, "${url}", url);
        replace_all(htmlData, "${subject}", subject);
        replace_all(htmlData, "${body}", body);
        replace_all(htmlData, "${client-ip}",
                           cgiEnv->GetClientInfo(CgiEnv::ClientInfo::IP));
        replace_all(htmlData, "${client-location}",
                           cgiEnv->GetClientInfo(CgiEnv::ClientInfo::Location));
        replace_all(htmlData, "${client-user-agent}",
                           cgiEnv->GetClientInfo(CgiEnv::ClientInfo::Browser));
        replace_all(htmlData, "${client-referer}",
                           cgiEnv->GetClientInfo(CgiEnv::ClientInfo::Referer));
        replace_all(htmlData, "${time}",
                           DateConv::ToJalali(n)
                           + " ~ "
                           + algorithm::trim_copy(DateConv::RawLocalDateTime(n)));

        CoreLib::Mail *mail = new CoreLib::Mail(from, to,
                    (format(tr("home-contact-form-email-subject").toUTF8())
                     % cgiEnv->GetServerInfo(CgiEnv::ServerInfo::Host) % name).str(),
                    htmlData);
        mail->SetDeleteLater(true);
        mail->SendAsync();
    }
}

void ContactForm::Impl::ClearForm()
{
    if (DefaultRecipientId < RecipientComboBox->count()) {
        RecipientComboBox->setCurrentIndex(DefaultRecipientId);
    }

    FromLineEdit->setText("");
    EmailLineEdit->setText("");
    UrlLineEdit->setText("");
    SubjectLineEdit->setText("");
    BodyTextArea->setText("");
    CaptchaLineEdit->setText("");
}

