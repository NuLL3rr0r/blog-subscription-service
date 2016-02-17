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
#include <CoreLib/Database.hpp>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Log.hpp>
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

public:
    Impl();
    ~Impl();
};

ContactForm::ContactForm()
    : Page(),
    m_pimpl(make_unique<ContactForm::Impl>())
{
    WApplication *app = WApplication::instance();
    app->setTitle(tr("home-contact-form-page-title"));

    this->clear();
    this->setId("ContactFormPage");
    this->setStyleClass("contact-form-page full-width full-height");
    this->addWidget(Layout());
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

            if (!r.empty()) {
                string recipient;
                string is_default;
                int i = 0;
                while (r.next()) {
                    r >> recipient >> is_default;
                    m_pimpl->RecipientComboBox->addItem(WString::fromUTF8(recipient));
                    if (Database::IsTrue(is_default)) {
                        m_pimpl->RecipientComboBox->setCurrentIndex(i);
                    }
                    ++i;
                }
            } else {
                r = Pool::Database()->Sql()
                        << (format("SELECT email"
                                   " FROM \"%1%\" WHERE username=?;")
                            % Pool::Database()->GetTableName("ROOT")).str()
                        << Service::Pool::Storage()->RootUsername() << row;

                if (!r.empty()) {
                    string email;
                    r >> email;

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
            urlValidator->setMandatory(true);
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
            sendPushButton->setStyleClass("btn btn-default");

            WPushButton *clearPushButton = new WPushButton(tr("home-contact-form-clear"));
            clearPushButton->setStyleClass("btn btn-default");

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

            container->addWidget(tmpl);
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

ContactForm::Impl::Impl()
{

}

ContactForm::Impl::~Impl() = default;

