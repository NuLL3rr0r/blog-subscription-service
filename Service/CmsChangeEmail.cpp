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
 * Change admin email.
 */


#include <boost/exception/diagnostic_information.hpp>
#include <boost/format.hpp>
#include <cppdb/frontend.h>
#include <Wt/WApplication>
#include <Wt/WLengthValidator>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WRegExpValidator>
#include <Wt/WString>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WWidget>
#include <CoreLib/Crypto.hpp>
#include <CoreLib/Database.hpp>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Log.hpp>
#include "CgiEnv.hpp"
#include "CgiRoot.hpp"
#include "CmsChangeEmail.hpp"
#include "Div.hpp"
#include "Pool.hpp"

using namespace std;
using namespace boost;
using namespace cppdb;
using namespace Wt;
using namespace Service;

struct CmsChangeEmail::Impl : public Wt::WObject
{
public:
    WLineEdit *EmailLineEdit;
    WLineEdit *PasswordLineEdit;
    WText *ChangeEmailMessageArea;

private:
    CmsChangeEmail *m_parent;

public:
    explicit Impl(CmsChangeEmail *parent);
    ~Impl();

public:
    void OnEmailChangeFormSubmitted();
};

CmsChangeEmail::CmsChangeEmail()
    : Page(),
    m_pimpl(make_unique<CmsChangeEmail::Impl>(this))
{
    this->clear();
    this->setId("CmsChangeEmailPage");
    this->addWidget(this->Layout());
}

CmsChangeEmail::~CmsChangeEmail() = default;

WWidget *CmsChangeEmail::Layout()
{
    Div *container = new Div("CmsChangeEmail", "container-fluid");

    try {
        CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
        CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

        string htmlData;
        string file;
        if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
            file = "../templates/cms-change-email-fa.wtml";
        } else {
            file = "../templates/cms-change-email.wtml";
        }

        if (CoreLib::FileSystem::Read(file, htmlData)) {
            /// Fill the template
            WTemplate *tmpl = new WTemplate(container);
            tmpl->setTemplateText(WString::fromUTF8(htmlData), TextFormat::XHTMLUnsafeText);

            m_pimpl->EmailLineEdit = new WLineEdit();
            m_pimpl->EmailLineEdit->setPlaceholderText(tr("cms-change-email-mailbox-placeholder"));
            WRegExpValidator *emailValidator = new WRegExpValidator(Pool::Storage()->RegexEmail());
            emailValidator->setMandatory(true);
            m_pimpl->EmailLineEdit->setValidator(emailValidator);
            m_pimpl->EmailLineEdit->setText(WString::fromUTF8(cgiEnv->SignedInUser.Email));

            m_pimpl->PasswordLineEdit = new WLineEdit();
            m_pimpl->PasswordLineEdit->setEchoMode(WLineEdit::Password);
            m_pimpl->PasswordLineEdit->setPlaceholderText(tr("cms-change-email-password-placeholder"));
            WLengthValidator *passwordValidator = new WLengthValidator(Pool::Storage()->MinPasswordLength(),
                                                                       Pool::Storage()->MaxPasswordLength());
            passwordValidator->setMandatory(true);
            m_pimpl->PasswordLineEdit->setValidator(passwordValidator);

            WPushButton *changeEmailPushButton = new WPushButton(tr("cms-change-email-change-mailbox"));
            changeEmailPushButton->setStyleClass("btn btn-default");

            m_pimpl->ChangeEmailMessageArea = new WText();

            tmpl->bindString("email-input-id", m_pimpl->EmailLineEdit->id());
            tmpl->bindString("password-input-id", m_pimpl->PasswordLineEdit->id());

            tmpl->bindWidget("change-email-title", new WText(tr("cms-change-email-page-title")));
            tmpl->bindWidget("email-label-text", new WText(tr("cms-change-email-mailbox")));
            tmpl->bindWidget("password-label-text", new WText(tr("cms-change-email-password")));

            tmpl->bindWidget("email-input", m_pimpl->EmailLineEdit);
            tmpl->bindWidget("password-input", m_pimpl->PasswordLineEdit);

            tmpl->bindWidget("change-email-button", changeEmailPushButton);
            tmpl->bindWidget("change-email-message-area", m_pimpl->ChangeEmailMessageArea);

            m_pimpl->EmailLineEdit->enterPressed().connect(m_pimpl.get(), &CmsChangeEmail::Impl::OnEmailChangeFormSubmitted);
            m_pimpl->PasswordLineEdit->enterPressed().connect(m_pimpl.get(), &CmsChangeEmail::Impl::OnEmailChangeFormSubmitted);
            changeEmailPushButton->clicked().connect(m_pimpl.get(), &CmsChangeEmail::Impl::OnEmailChangeFormSubmitted);

            m_pimpl->PasswordLineEdit->setFocus();
        }
    }

    catch (const boost::exception &ex) {
        LOG_ERROR(boost::diagnostic_information(ex));
    }

    catch (const std::exception &ex) {
        LOG_ERROR(ex.what());
    }

    catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    return container;
}

CmsChangeEmail::Impl::Impl(CmsChangeEmail *parent)
    : m_parent(parent)
{

}

CmsChangeEmail::Impl::~Impl() = default;

void CmsChangeEmail::Impl::OnEmailChangeFormSubmitted()
{
    if (!m_parent->Validate(EmailLineEdit)
            || !m_parent->Validate(PasswordLineEdit)) {
        return;
    }

    transaction guard(Service::Pool::Database()->Sql());

    try {
        CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
        CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

        bool success = false;

        result r = Pool::Database()->Sql()
                << (format("SELECT pwd FROM \"%1%\""
                                  " WHERE username=?;")
                    % Pool::Database()->GetTableName("ROOT")).str()
                << cgiEnv->SignedInUser.Username
                << row;

        if (!r.empty()) {
            string hashedPwd;
            r >> hashedPwd;

            Pool::Crypto()->Decrypt(hashedPwd, hashedPwd);

            if (Pool::Crypto()->Argon2iVerify(PasswordLineEdit->text().toUTF8(), hashedPwd)) {
                success = true;
            }
        }

        if (!success) {
            guard.rollback();
            m_parent->HtmlError(tr("cms-change-email-invalid-pwd-error"), ChangeEmailMessageArea);
            PasswordLineEdit->setFocus();
            return;
        }

        Pool::Database()->Update("ROOT",
                                 "username", cgiEnv->SignedInUser.Username,
                                 "email=?",
                                 { EmailLineEdit->text().toUTF8() });

        guard.commit();

        cgiEnv->SignedInUser.Email = EmailLineEdit->text().toUTF8();

        PasswordLineEdit->setText("");
        EmailLineEdit->setFocus();

        m_parent->HtmlInfo(tr("cms-change-email-success-message"), ChangeEmailMessageArea);

        return;
    }

    catch (const boost::exception &ex) {
        LOG_ERROR(boost::diagnostic_information(ex));
    }

    catch (const std::exception &ex) {
        LOG_ERROR(ex.what());
    }

    catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    guard.rollback();
}
