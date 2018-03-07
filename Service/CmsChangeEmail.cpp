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
#include <pqxx/pqxx>
#include <Wt/WApplication>
#include <Wt/WLengthValidator>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WRegExpValidator>
#include <Wt/WString>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WWidget>
#include <CoreLib/CDate.hpp>
#include <CoreLib/Crypto.hpp>
#include <CoreLib/Database.hpp>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Log.hpp>
#include <CoreLib/make_unique.hpp>
#include "CgiEnv.hpp"
#include "CgiRoot.hpp"
#include "CmsChangeEmail.hpp"
#include "Div.hpp"
#include "Pool.hpp"

using namespace std;
using namespace boost;
using namespace pqxx;
using namespace Wt;
using namespace CoreLib;
using namespace CoreLib::CDate;
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

    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    try {
        string htmlData;
        string file;
        if (cgiEnv->GetInformation().Client.Language.Code
                == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
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
            WRegExpValidator *emailValidator = new WRegExpValidator(Pool::Storage().RegexEmail());
            emailValidator->setMandatory(true);
            m_pimpl->EmailLineEdit->setValidator(emailValidator);
            m_pimpl->EmailLineEdit->setText(WString::fromUTF8(cgiEnv->GetInformation().Client.Session.Email));

            m_pimpl->PasswordLineEdit = new WLineEdit();
            m_pimpl->PasswordLineEdit->setEchoMode(WLineEdit::Password);
            m_pimpl->PasswordLineEdit->setPlaceholderText(tr("cms-change-email-password-placeholder"));
            WLengthValidator *passwordValidator = new WLengthValidator(Pool::Storage().MinPasswordLength(),
                                                                       Pool::Storage().MaxPasswordLength());
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

    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    try {
        auto conn = Pool::Database().Connection();
        conn->activate();
        pqxx::work txn(*conn.get());

        bool success = false;

        string query((format("SELECT pwd FROM \"%1%\""
                             " WHERE user_id = %2%;")
                      % Pool::Database().GetTableName("ROOT_CREDENTIALS")
                      % txn.quote(cgiEnv->GetInformation().Client.Session.UserId)).str());
        LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

        result r = txn.exec(query);

        if (!r.empty()) {
            const pqxx::row row(r[0]);

            string hashedPwd(row["pwd"].c_str());
            Pool::Crypto().Decrypt(hashedPwd, hashedPwd);

            if (Pool::Crypto().Argon2iVerify(PasswordLineEdit->text().toUTF8(), hashedPwd)) {
                success = true;
            }
        }

        if (!success) {
            LOG_ERROR("Invalid password!", cgiEnv->GetInformation().ToJson());
            m_parent->HtmlError(tr("cms-change-email-invalid-pwd-error"), ChangeEmailMessageArea);
            PasswordLineEdit->setFocus();
            return;
        }

        CDate::Now n(CDate::Timezone::UTC);

        string email(EmailLineEdit->text().toUTF8());

        query.assign((boost::format("UPDATE ONLY \"%1%\""
                                    " SET email = %2%, modification_time = TO_TIMESTAMP(%3%)::TIMESTAMPTZ"
                                    " WHERE user_id = %4%;")
                      % txn.esc(Service::Pool::Database().GetTableName("ROOT"))
                      % txn.quote(email)
                      % txn.esc(lexical_cast<string>(n.RawTime()))
                      % txn.quote(cgiEnv->GetInformation().Client.Session.UserId)).str());
        LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

        r = txn.exec(query);

        txn.commit();

        cgiEnv->SetSessionEmail(email);

        PasswordLineEdit->setText("");
        EmailLineEdit->setFocus();

        m_parent->HtmlInfo(tr("cms-change-email-success-message"), ChangeEmailMessageArea);

        return;
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
