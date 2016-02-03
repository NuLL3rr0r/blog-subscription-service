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
 * Change admin password.
 */


#include <boost/exception/diagnostic_information.hpp>
#include <boost/format.hpp>
#include <cppdb/frontend.h>
#include <Wt/WLengthValidator>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WString>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WWidget>
#include <CoreLib/Crypto.hpp>
#include <CoreLib/Database.hpp>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Log.hpp>
#include "CgiEnv.hpp"
#include "CmsChangePassword.hpp"
#include "Div.hpp"
#include "Pool.hpp"

#define         UNKNOWN_ERROR       "Unknown error!"

using namespace std;
using namespace boost;
using namespace cppdb;
using namespace Wt;
using namespace Service;

struct CmsChangePassword::Impl : public Wt::WObject
{
public:
    CmsChangePassword *m_parent;

    WLineEdit *CurrentPasswordLineEdit;
    WLineEdit *NewPasswordLineEdit;
    WLineEdit *ConfirmPasswordLineEdit;
    WText *ChangePasswordMessageArea;

public:
    explicit Impl(CmsChangePassword *parent);

public:
    void OnPasswordChangeFormSubmitted();
};

CmsChangePassword::CmsChangePassword(CgiRoot *cgi) :
    Page(cgi),
    m_pimpl(make_unique<CmsChangePassword::Impl>(this))
{
    this->clear();
    this->setId("CmsChangePasswordPage");
    this->addWidget(Layout());
    m_htmlRoot->addWidget(this);
}

WWidget *CmsChangePassword::Layout()
{
    Div *container = new Div("CmsChangePassword", "container-fluid");

    try {
        string htmlData;
        string file;
        if (m_cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
            file = "../templates/cms-change-password-fa.wtml";
        } else {
            file = "../templates/cms-change-password.wtml";
        }

        if (CoreLib::FileSystem::Read(file, htmlData)) {
            /// Fill the template
            WTemplate *tmpl = new WTemplate(container);
            tmpl->setTemplateText(WString(htmlData), TextFormat::XHTMLUnsafeText);

            m_pimpl->CurrentPasswordLineEdit = new WLineEdit();
            m_pimpl->CurrentPasswordLineEdit->setEchoMode(WLineEdit::Password);
            m_pimpl->CurrentPasswordLineEdit->setPlaceholderText(tr("cms-change-password-current-pwd-placeholder"));
            WLengthValidator *currentPasswordValidator = new WLengthValidator(Pool::Storage()->MinPasswordLength(),
                                                                       Pool::Storage()->MaxPasswordLength());
            currentPasswordValidator->setMandatory(true);
            m_pimpl->CurrentPasswordLineEdit->setValidator(currentPasswordValidator);

            m_pimpl->NewPasswordLineEdit = new WLineEdit();
            m_pimpl->NewPasswordLineEdit->setEchoMode(WLineEdit::Password);
            m_pimpl->NewPasswordLineEdit->setPlaceholderText(tr("cms-change-password-new--pwdplaceholder"));
            WLengthValidator *newPasswordValidator = new WLengthValidator(Pool::Storage()->MinPasswordLength(),
                                                                       Pool::Storage()->MaxPasswordLength());
            newPasswordValidator->setMandatory(true);
            m_pimpl->NewPasswordLineEdit->setValidator(newPasswordValidator);

            m_pimpl->ConfirmPasswordLineEdit = new WLineEdit();
            m_pimpl->ConfirmPasswordLineEdit->setEchoMode(WLineEdit::Password);
            m_pimpl->ConfirmPasswordLineEdit->setPlaceholderText(tr("cms-change-password-confirm-pwd-placeholder"));
            WLengthValidator *confirmPasswordValidator = new WLengthValidator(Pool::Storage()->MinPasswordLength(),
                                                                       Pool::Storage()->MaxPasswordLength());
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

CmsChangePassword::Impl::Impl(CmsChangePassword *parent)
    : m_parent(parent)
{

}

void CmsChangePassword::Impl::OnPasswordChangeFormSubmitted()
{
    if (!m_parent->Validate(CurrentPasswordLineEdit)
            || !m_parent->Validate(NewPasswordLineEdit)
            || !m_parent->Validate(ConfirmPasswordLineEdit)) {
        return;
    }

    transaction guard(Service::Pool::Database()->Sql());

    try {
        string encryptedPwd;
        Pool::Crypto()->Hash(CurrentPasswordLineEdit->text().toUTF8(), encryptedPwd);
        Pool::Crypto()->Encrypt(encryptedPwd, encryptedPwd);

        result r = Pool::Database()->Sql()
                << (format("SELECT pwd FROM \"%1%\""
                                  " WHERE username=? AND pwd=?;")
                    % Pool::Database()->GetTableName("ROOT")).str()
                << m_parent->m_cgiEnv->SignedInUser.Username
                << encryptedPwd
                << row;

        if (r.empty()) {
            guard.rollback();
            m_parent->HtmlError(tr("cms-change-password-invalid-pwd-error"), ChangePasswordMessageArea);
            CurrentPasswordLineEdit->setFocus();
            return;
        }

        if (NewPasswordLineEdit->text() == CurrentPasswordLineEdit->text()) {
            guard.rollback();
            m_parent->HtmlError(tr("cms-change-password-same-pwd-error"), ChangePasswordMessageArea);
            NewPasswordLineEdit->setFocus();
            return;
        }

        if (NewPasswordLineEdit->text() != ConfirmPasswordLineEdit->text()) {
            guard.rollback();
            m_parent->HtmlError(tr("cms-change-password-confirm-pwd-error"), ChangePasswordMessageArea);
            ConfirmPasswordLineEdit->setFocus();
            return;
        }

        encryptedPwd.clear();
        Pool::Crypto()->Hash(NewPasswordLineEdit->text().toUTF8(), encryptedPwd);
        Pool::Crypto()->Encrypt(encryptedPwd, encryptedPwd);

        Pool::Database()->Update("ROOT",
                                 "username", m_parent->m_cgiEnv->SignedInUser.Username,
                                 "pwd=?",
                                 { encryptedPwd });

        guard.commit();

        CurrentPasswordLineEdit->setText("");
        NewPasswordLineEdit->setText("");
        ConfirmPasswordLineEdit->setText("");
        CurrentPasswordLineEdit->setFocus();

        m_parent->HtmlInfo(tr("cms-change-password-success-message"), ChangePasswordMessageArea);

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

    guard.rollback();
}

