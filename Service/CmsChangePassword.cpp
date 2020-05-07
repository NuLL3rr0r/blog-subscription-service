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
 * Change admin password.
 */


#include <boost/exception/diagnostic_information.hpp>
#include <boost/format.hpp>
#include <pqxx/pqxx>
#include <Wt/WApplication>
#include <Wt/WLengthValidator>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
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
#include "CmsChangePassword.hpp"
#include "Div.hpp"
#include "Pool.hpp"

using namespace std;
using namespace boost;
using namespace pqxx;
using namespace Wt;
using namespace CoreLib;
using namespace CoreLib::CDate;
using namespace Service;

struct CmsChangePassword::Impl : public Wt::WObject
{
public:
    WLineEdit *CurrentPasswordLineEdit;
    WLineEdit *NewPasswordLineEdit;
    WLineEdit *ConfirmPasswordLineEdit;
    WText *ChangePasswordMessageArea;

private:
    CmsChangePassword *m_parent;

public:
    explicit Impl(CmsChangePassword *parent);
    ~Impl();

public:
    void OnPasswordChangeFormSubmitted();
};

CmsChangePassword::CmsChangePassword()
    : Page(),
    m_pimpl(make_unique<CmsChangePassword::Impl>(this))
{
    this->clear();
    this->setId("CmsChangePasswordPage");
    this->addWidget(this->Layout());
}

CmsChangePassword::~CmsChangePassword() = default;

WWidget *CmsChangePassword::Layout()
{
    Div *container = new Div("CmsChangePassword", "container-fluid");

    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    try {
        string htmlData;
        string file;
        if (cgiEnv->GetInformation().Client.Language.Code
                == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
            file = "../templates/cms-change-password-fa.wtml";
        } else {
            file = "../templates/cms-change-password.wtml";
        }

        if (CoreLib::FileSystem::Read(file, htmlData)) {
            /// Fill the template
            WTemplate *tmpl = new WTemplate(container);
            tmpl->setTemplateText(WString::fromUTF8(htmlData), TextFormat::XHTMLUnsafeText);

            m_pimpl->CurrentPasswordLineEdit = new WLineEdit();
            m_pimpl->CurrentPasswordLineEdit->setEchoMode(WLineEdit::Password);
            m_pimpl->CurrentPasswordLineEdit->setPlaceholderText(tr("cms-change-password-current-pwd-placeholder"));
            WLengthValidator *currentPasswordValidator = new WLengthValidator(Pool::Storage().MinPasswordLength(),
                                                                              Pool::Storage().MaxPasswordLength());
            currentPasswordValidator->setMandatory(true);
            m_pimpl->CurrentPasswordLineEdit->setValidator(currentPasswordValidator);

            m_pimpl->NewPasswordLineEdit = new WLineEdit();
            m_pimpl->NewPasswordLineEdit->setEchoMode(WLineEdit::Password);
            m_pimpl->NewPasswordLineEdit->setPlaceholderText(tr("cms-change-password-new--pwdplaceholder"));
            WLengthValidator *newPasswordValidator = new WLengthValidator(Pool::Storage().MinPasswordLength(),
                                                                          Pool::Storage().MaxPasswordLength());
            newPasswordValidator->setMandatory(true);
            m_pimpl->NewPasswordLineEdit->setValidator(newPasswordValidator);

            m_pimpl->ConfirmPasswordLineEdit = new WLineEdit();
            m_pimpl->ConfirmPasswordLineEdit->setEchoMode(WLineEdit::Password);
            m_pimpl->ConfirmPasswordLineEdit->setPlaceholderText(tr("cms-change-password-confirm-pwd-placeholder"));
            WLengthValidator *confirmPasswordValidator = new WLengthValidator(Pool::Storage().MinPasswordLength(),
                                                                              Pool::Storage().MaxPasswordLength());
            confirmPasswordValidator->setMandatory(true);
            m_pimpl->ConfirmPasswordLineEdit->setValidator(confirmPasswordValidator);

            WPushButton *changePasswordPushButton = new WPushButton(tr("cms-change-password-change-pwd"));
            changePasswordPushButton->setStyleClass("btn btn-default");

            m_pimpl->ChangePasswordMessageArea = new WText();

            tmpl->bindString("current-password-input-id", m_pimpl->CurrentPasswordLineEdit->id());
            tmpl->bindString("new-password-input-id", m_pimpl->NewPasswordLineEdit->id());
            tmpl->bindString("confirm-password-input-id", m_pimpl->ConfirmPasswordLineEdit->id());

            tmpl->bindWidget("change-password-title", new WText(tr("cms-change-password-page-title")));
            tmpl->bindWidget("current-password-label-text", new WText(tr("cms-change-password-current-pwd")));
            tmpl->bindWidget("new-password-label-text", new WText(tr("cms-change-password-new-pwd")));
            tmpl->bindWidget("confirm-password-label-text", new WText(tr("cms-change-password-confirm-pwd")));

            tmpl->bindWidget("current-password-input", m_pimpl->CurrentPasswordLineEdit);
            tmpl->bindWidget("new-password-input", m_pimpl->NewPasswordLineEdit);
            tmpl->bindWidget("confirm-password-input", m_pimpl->ConfirmPasswordLineEdit);

            tmpl->bindWidget("change-password-button", changePasswordPushButton);
            tmpl->bindWidget("change-password-message-area", m_pimpl->ChangePasswordMessageArea);

            m_pimpl->CurrentPasswordLineEdit->enterPressed().connect(m_pimpl.get(), &CmsChangePassword::Impl::OnPasswordChangeFormSubmitted);
            m_pimpl->NewPasswordLineEdit->enterPressed().connect(m_pimpl.get(), &CmsChangePassword::Impl::OnPasswordChangeFormSubmitted);
            m_pimpl->ConfirmPasswordLineEdit->enterPressed().connect(m_pimpl.get(), &CmsChangePassword::Impl::OnPasswordChangeFormSubmitted);
            changePasswordPushButton->clicked().connect(m_pimpl.get(), &CmsChangePassword::Impl::OnPasswordChangeFormSubmitted);

            m_pimpl->CurrentPasswordLineEdit->setFocus();
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

CmsChangePassword::Impl::Impl(CmsChangePassword *parent)
    : m_parent(parent)
{

}

CmsChangePassword::Impl::~Impl() = default;

void CmsChangePassword::Impl::OnPasswordChangeFormSubmitted()
{
    if (!m_parent->Validate(CurrentPasswordLineEdit)
            || !m_parent->Validate(NewPasswordLineEdit)
            || !m_parent->Validate(ConfirmPasswordLineEdit)) {
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

            if (Pool::Crypto().Argon2Verify(CurrentPasswordLineEdit->text().toUTF8(), hashedPwd)) {
                success = true;
            }
        }

        if (!success) {
            LOG_ERROR("Invalid password!", cgiEnv->GetInformation().ToJson());
            m_parent->HtmlError(tr("cms-change-password-invalid-pwd-error"), ChangePasswordMessageArea);
            CurrentPasswordLineEdit->setFocus();
            return;
        }

        if (NewPasswordLineEdit->text() == CurrentPasswordLineEdit->text()) {
            LOG_ERROR("Password must be different from the current password!", cgiEnv->GetInformation().ToJson());
            m_parent->HtmlError(tr("cms-change-password-same-pwd-error"), ChangePasswordMessageArea);
            NewPasswordLineEdit->setFocus();
            return;
        }

        if (NewPasswordLineEdit->text() != ConfirmPasswordLineEdit->text()) {
            LOG_ERROR("Password mismatch!", cgiEnv->GetInformation().ToJson());
            m_parent->HtmlError(tr("cms-change-password-confirm-pwd-error"), ChangePasswordMessageArea);
            ConfirmPasswordLineEdit->setFocus();
            return;
        }

        string encryptedPwd;
        Pool::Crypto().Argon2(NewPasswordLineEdit->text().toUTF8(), encryptedPwd,
                                CoreLib::Crypto::Argon2OpsLimit::Sensitive,
                                CoreLib::Crypto::Argon2MemLimit::Sensitive);
        Pool::Crypto().Encrypt(encryptedPwd, encryptedPwd);

        CDate::Now n(CDate::Timezone::UTC);

        query.assign((boost::format("UPDATE ONLY \"%1%\""
                                    " SET pwd = %2%, modification_time = TO_TIMESTAMP(%3%)::TIMESTAMPTZ"
                                    " WHERE user_id = %4%;")
                      % txn.esc(Service::Pool::Database().GetTableName("ROOT_CREDENTIALS"))
                      % txn.quote(encryptedPwd)
                      % txn.esc(lexical_cast<string>(n.RawTime()))
                      % txn.quote(cgiEnv->GetInformation().Client.Session.UserId)).str());
        LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

        r = txn.exec(query);

        txn.commit();

        CurrentPasswordLineEdit->setText("");
        NewPasswordLineEdit->setText("");
        ConfirmPasswordLineEdit->setText("");
        CurrentPasswordLineEdit->setFocus();

        m_parent->HtmlInfo(tr("cms-change-password-success-message"), ChangePasswordMessageArea);

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
