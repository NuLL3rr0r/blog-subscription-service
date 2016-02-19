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
 * The root login page. This is the main entry to the CMS.
 */


#include <boost/algorithm/string.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <Wt/WApplication>
#include <Wt/WCheckBox>
#include <Wt/WContainerWidget>
#include <Wt/WEnvironment>
#include <Wt/WImage>
#include <Wt/WIntValidator>
#include <Wt/WLengthValidator>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WRegExpValidator>
#include <Wt/WSignalMapper>
#include <Wt/WString>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <cppdb/frontend.h>
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
#include "Cms.hpp"
#include "RootLogin.hpp"

using namespace std;
using namespace boost;
using namespace Wt;
using namespace CoreLib;
using namespace CoreLib::CDate;
using namespace Service;
using namespace cppdb;

struct RootLogin::Impl : public Wt::WObject
{
public:
    bool PasswordRecoveryFormFlag;
    std::string PasswordRecoveryHtmlData;
    Div *PasswordRecoveryDiv;

    WLineEdit *UsernameLineEdit;
    WLineEdit *PasswordLineEdit;
    WLineEdit *CaptchaLineEdit;
    WCheckBox *RememberMeCheckBox;
    Service::Captcha *Captcha;
    WIntValidator *CaptchaValidator;
    WImage *CaptchaImage;
    WLineEdit *ForgotPassword_EmailLineEdit;
    WLineEdit *ForgotPassword_CaptchaLineEdit;
    WIntValidator *ForgotPassword_CaptchaValidator;
    WText *LoginMessageArea;
    WText *PasswordRecoveryMessageArea;

private:
    RootLogin *m_parent;

public:
    explicit Impl(RootLogin *parent);
    ~Impl();

public:
    void OnLoginFormSubmitted();
    void OnPasswordRecoveryFormSubmitted();

    void OnGoToHomePageButtonPressed();
    void OnSignInAgainButtonPressed();

    void GenerateCaptcha();
    void PasswordRecoveryForm();
    void PreserveSessionData(const CDate::Now &n, const std::string &username, const bool saveLocally);

    void SendLoginAlertEmail(const std::string &email, const std::string &username, CDate::Now &n);
    void SendPasswordRecoveryEmail(const std::string &email,
                                   const std::string &username, const std::string &password,
                                   CDate::Now &n);

    Wt::WWidget *LogoutPage();
};

RootLogin::RootLogin()
    : Page(),
    m_pimpl(make_unique<RootLogin::Impl>(this))
{
    bool hasValidSession = false;

    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    try {
        if (cgiEnv->IsRootLogoutRequested()) {
            try {
                cgiRoot->removeCookie("cms-session-user");
            } catch (...) {
            }
            try {
                cgiRoot->removeCookie("cms-session-token");
            } catch (...) {
            }
            hasValidSession = false;
        } else {
            string user(cgiRoot->environment().getCookie("cms-session-user"));
            string token(cgiRoot->environment().getCookie("cms-session-token"));
            Pool::Crypto()->Decrypt(user, user);
            Pool::Crypto()->Decrypt(token, token);

            try {
                time_t rawTime = lexical_cast<time_t>(token);

                CDate::Now n;
                if (rawTime + Pool::Storage()->RootSessionLifespan() >= n.RawTime) {
                    transaction guard(Service::Pool::Database()->Sql());

                    try {
                        result r = Pool::Database()->Sql()
                                << (format("SELECT username, email,"
                                           " last_login_ip, last_login_location,"
                                           " last_login_rawtime,"
                                           " last_login_gdate, last_login_jdate,"
                                           " last_login_time,"
                                           " last_login_user_agent, last_login_referer"
                                           " FROM \"%1%\" WHERE username=?;")
                                    % Pool::Database()->GetTableName("ROOT")).str()
                                << user << row;

                        if (!r.empty()) {
                            r >> cgiEnv->SignedInUser.Username
                                    >> cgiEnv->SignedInUser.Email
                                    >> cgiEnv->SignedInUser.LastLogin.IP
                                    >> cgiEnv->SignedInUser.LastLogin.Location
                                    >> cgiEnv->SignedInUser.LastLogin.LoginRawTime
                                    >> cgiEnv->SignedInUser.LastLogin.LoginGDate
                                    >> cgiEnv->SignedInUser.LastLogin.LoginJDate
                                    >> cgiEnv->SignedInUser.LastLogin.LoginTime
                                    >> cgiEnv->SignedInUser.LastLogin.UserAgent
                                    >> cgiEnv->SignedInUser.LastLogin.Referer;

                            m_pimpl->PreserveSessionData(n, cgiEnv->SignedInUser.Username, true);
                            guard.commit();

                            m_pimpl->SendLoginAlertEmail(cgiEnv->SignedInUser.Email,
                                                         cgiEnv->SignedInUser.Username,
                                                         n);

                            hasValidSession = true;
                        }
                    }

                    catch (boost::exception &ex) {
                        guard.rollback();
                        LOG_ERROR(boost::diagnostic_information(ex));
                    }

                    catch (std::exception &ex) {
                        guard.rollback();
                        LOG_ERROR(ex.what());
                    }

                    catch (...) {
                        guard.rollback();
                        LOG_ERROR(UNKNOWN_ERROR);
                    }
                }
            }

            catch (...) {
                /// Invalid session!
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

    if (!hasValidSession) {
        if (cgiEnv->IsRootLogoutRequested()) {
            cgiRoot->setTitle(tr("root-logout-page-title"));
            this->clear();
            this->setId("RootLogoutPage");
            this->setStyleClass("root-logout-page full-width full-height");
            this->addWidget(m_pimpl->LogoutPage());
        } else {
            cgiRoot->setTitle(tr("root-login-page-title"));

            this->clear();
            this->setId("RootLoginPage");
            this->setStyleClass("root-login-page full-width full-height");
            this->addWidget(this->Layout());
        }
    } else {
        this->clear();
        this->addWidget(new Cms());
    }
}

RootLogin::~RootLogin() = default;

WWidget *RootLogin::Layout()
{
    Div *container = new Div("RootLogin", "root-login-layout full-width full-height");
    Div *noScript = new Div(container);
    noScript->addWidget(new WText(tr("no-script")));

    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    string htmlData;
    string file;
    if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
        file = "../templates/root-login-fa.wtml";
    } else {
        file = "../templates/root-login.wtml";
    }

    if (CoreLib::FileSystem::Read(file, htmlData)) {
        /// Fill the template
        WTemplate *tmpl = new WTemplate(container);
        tmpl->setStyleClass("container-table");
        tmpl->setTemplateText(WString(htmlData), TextFormat::XHTMLUnsafeText);

        m_pimpl->UsernameLineEdit = new WLineEdit();
        m_pimpl->UsernameLineEdit->setPlaceholderText(tr("root-login-username-placeholder"));
        WLengthValidator *usernameValidator = new WLengthValidator(Pool::Storage()->MinUsernameLength(),
                                                                   Pool::Storage()->MaxUsernameLength());
        usernameValidator->setMandatory(true);
        m_pimpl->UsernameLineEdit->setValidator(usernameValidator);

        m_pimpl->PasswordLineEdit = new WLineEdit();
        m_pimpl->PasswordLineEdit->setEchoMode(WLineEdit::Password);
        m_pimpl->PasswordLineEdit->setPlaceholderText(tr("root-login-password-placeholder"));
        WLengthValidator *passwordValidator = new WLengthValidator(Pool::Storage()->MinPasswordLength(),
                                                                   Pool::Storage()->MaxPasswordLength());
        passwordValidator->setMandatory(true);
        m_pimpl->PasswordLineEdit->setValidator(passwordValidator);

        m_pimpl->Captcha = new Service::Captcha();
        m_pimpl->CaptchaImage = m_pimpl->Captcha->Generate();
        m_pimpl->CaptchaImage->setAlternateText(tr("root-login-captcha-hint"));
        m_pimpl->CaptchaImage->setAttributeValue("title", tr("root-login-captcha-hint"));

        int captchaResult = (int)m_pimpl->Captcha->GetResult();

        m_pimpl->CaptchaLineEdit = new WLineEdit();
        m_pimpl->CaptchaLineEdit->setPlaceholderText(tr("root-login-captcha-hint"));
        m_pimpl->CaptchaValidator = new WIntValidator(captchaResult, captchaResult);
        m_pimpl->CaptchaValidator->setMandatory(true);
        m_pimpl->CaptchaLineEdit->setValidator(m_pimpl->CaptchaValidator);

        m_pimpl->RememberMeCheckBox = new WCheckBox();

        m_pimpl->LoginMessageArea = new WText();

        WPushButton *signInPushButton = new WPushButton(tr("root-login-sign-in"));
        signInPushButton->setStyleClass("btn btn-default");

        WText *forgotPasswordLink = new WText(tr("root-login-forgot-password"));
        forgotPasswordLink->setStyleClass("link");

        m_pimpl->PasswordRecoveryDiv = new Div("password-recovery", "password-recovery");

        tmpl->bindString("username-input-id", m_pimpl->UsernameLineEdit->id());
        tmpl->bindString("password-input-id", m_pimpl->PasswordLineEdit->id());
        tmpl->bindString("captcha-input-id", m_pimpl->CaptchaLineEdit->id());
        tmpl->bindString("captcha-input-id", m_pimpl->CaptchaLineEdit->id());

        tmpl->bindWidget("root-login-title", new WText(tr("root-login-page-title")));
        tmpl->bindWidget("username-label-text", new WText(tr("root-login-username")));
        tmpl->bindWidget("password-label-text", new WText(tr("root-login-password")));
        tmpl->bindWidget("captcha-label-text", new WText(tr("root-login-captcha")));
        tmpl->bindWidget("remember-me-text", new WText(tr("root-login-remember-me")));

        tmpl->bindWidget("username-input", m_pimpl->UsernameLineEdit);
        tmpl->bindWidget("password-input", m_pimpl->PasswordLineEdit);
        tmpl->bindWidget("captcha-input", m_pimpl->CaptchaLineEdit);
        tmpl->bindWidget("remember-me-input", m_pimpl->RememberMeCheckBox);
        tmpl->bindWidget("captcha-image", m_pimpl->CaptchaImage);
        tmpl->bindWidget("message-area", m_pimpl->LoginMessageArea);
        tmpl->bindWidget("sign-in-button", signInPushButton);
        tmpl->bindWidget("forgot-password-link", forgotPasswordLink);

        tmpl->bindWidget("password-recovery-div", m_pimpl->PasswordRecoveryDiv);

        container->addWidget(tmpl);

        m_pimpl->UsernameLineEdit->enterPressed().connect(m_pimpl.get(), &RootLogin::Impl::OnLoginFormSubmitted);
        m_pimpl->PasswordLineEdit->enterPressed().connect(m_pimpl.get(), &RootLogin::Impl::OnLoginFormSubmitted);
        m_pimpl->CaptchaLineEdit->enterPressed().connect(m_pimpl.get(), &RootLogin::Impl::OnLoginFormSubmitted);
        m_pimpl->RememberMeCheckBox->enterPressed().connect(m_pimpl.get(), &RootLogin::Impl::OnLoginFormSubmitted);
        signInPushButton->clicked().connect(m_pimpl.get(), &RootLogin::Impl::OnLoginFormSubmitted);

        WSignalMapper<WText *> *forgotPasswordSignalMapper = new WSignalMapper<WText *>(m_pimpl.get());
        forgotPasswordSignalMapper->mapped().connect(m_pimpl.get(), &RootLogin::Impl::PasswordRecoveryForm);
        forgotPasswordSignalMapper->mapConnect(forgotPasswordLink->clicked(), forgotPasswordLink);

        m_pimpl->PasswordRecoveryFormFlag = false;
    }

    return container;
}

RootLogin::Impl::Impl(RootLogin *parent)
    : m_parent(parent)
{
    PasswordRecoveryFormFlag = false;
}

RootLogin::Impl::~Impl() = default;

void RootLogin::Impl::OnLoginFormSubmitted()
{
    if (!m_parent->Validate(CaptchaLineEdit)
            || !m_parent->Validate(UsernameLineEdit)
            || !m_parent->Validate(PasswordLineEdit)) {
        GenerateCaptcha();
        return;
    }

    transaction guard(Service::Pool::Database()->Sql());

    try {
        string user = UsernameLineEdit->text().toUTF8();
        string pwd;
        Pool::Crypto()->Hash(PasswordLineEdit->text().toUTF8(), pwd);
        Pool::Crypto()->Encrypt(pwd, pwd);

        result r = Pool::Database()->Sql()
                << (format("SELECT username, email,"
                           " last_login_ip, last_login_location,"
                           " last_login_rawtime,"
                           " last_login_gdate, last_login_jdate,"
                           " last_login_time,"
                           " last_login_user_agent, last_login_referer"
                           " FROM \"%1%\" WHERE username=? AND pwd=?;")
                    % Pool::Database()->GetTableName("ROOT")).str()
                << user << pwd << row;

        // One-time passowrd
        if (r.empty()) {
            r = Pool::Database()->Sql()
                    << (format("SELECT username, email,"
                               " last_login_ip, last_login_location,"
                               " last_login_rawtime,"
                               " last_login_gdate, last_login_jdate,"
                               " last_login_time,"
                               " last_login_user_agent, last_login_referer"
                               " FROM \"%1%\" WHERE username=? AND recovery_pwd=?;")
                        % Pool::Database()->GetTableName("ROOT")).str()
                    << user << pwd << row;
            Pool::Database()->Update("ROOT",
                                     "username", user,
                                     "recovery_pwd=?",
            { "" });
        }

        if (r.empty()) {
            guard.rollback();
            m_parent->HtmlError(tr("root-login-fail"), LoginMessageArea);
            UsernameLineEdit->setFocus();
            GenerateCaptcha();
            return;
        }

        CDate::Now n;

        CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
        CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

        try {
            r >> cgiEnv->SignedInUser.Username
                    >> cgiEnv->SignedInUser.Email
                    >> cgiEnv->SignedInUser.LastLogin.IP
                    >> cgiEnv->SignedInUser.LastLogin.Location
                    >> cgiEnv->SignedInUser.LastLogin.LoginRawTime
                    >> cgiEnv->SignedInUser.LastLogin.LoginGDate
                    >> cgiEnv->SignedInUser.LastLogin.LoginJDate
                    >> cgiEnv->SignedInUser.LastLogin.LoginTime
                    >> cgiEnv->SignedInUser.LastLogin.UserAgent
                    >> cgiEnv->SignedInUser.LastLogin.Referer;
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

        PreserveSessionData(n, cgiEnv->SignedInUser.Username,
                            RememberMeCheckBox->checkState() == Wt::Checked);
        guard.commit();

        SendLoginAlertEmail(cgiEnv->SignedInUser.Email,
                            cgiEnv->SignedInUser.Username,
                            n);

        /// It's absolutely safe (even in case of throwing an exception in the constructor)
        /// since we attach it to Wt's WObject hierarchy in it's constructor.
        new Cms();

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

void RootLogin::Impl::OnPasswordRecoveryFormSubmitted()
{
    if (!m_parent->Validate(ForgotPassword_CaptchaLineEdit)
            || !m_parent->Validate(ForgotPassword_EmailLineEdit)) {
        GenerateCaptcha();
        return;
    }

    transaction guard(Service::Pool::Database()->Sql());

    try {
        string email = ForgotPassword_EmailLineEdit->text().toUTF8();

        result r = Pool::Database()->Sql()
                << (format("SELECT username FROM \"%1%\""
                           " WHERE email=?;")
                    % Pool::Database()->GetTableName("ROOT")).str()
                << email << row;

        if (r.empty()) {
            guard.rollback();
            m_parent->HtmlError(tr("root-login-password-recovery-fail"), PasswordRecoveryMessageArea);
            ForgotPassword_EmailLineEdit->setFocus();
            GenerateCaptcha();
            return;
        }

        CDate::Now n;

        string pwd;
        string encryptedPwd;
        Random::Characters(Random::Character::Alphanumeric,
                           (size_t)Pool::Storage()->MaxPasswordLength(), pwd);
        Pool::Crypto()->Hash(pwd, encryptedPwd);
        Pool::Crypto()->Encrypt(encryptedPwd, encryptedPwd);

        string user;
        r >> user;

        Pool::Database()->Update("ROOT",
                                 "email", email,
                                 "recovery_pwd=?",
                                 { encryptedPwd });

        guard.commit();

        SendPasswordRecoveryEmail(email, user, pwd, n);

        m_parent->HtmlInfo(tr("root-login-password-recovery-success"), PasswordRecoveryMessageArea);
        UsernameLineEdit->setFocus();
        GenerateCaptcha();

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

void RootLogin::Impl::OnGoToHomePageButtonPressed()
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    cgiRoot->Exit("/");
}

void RootLogin::Impl::OnSignInAgainButtonPressed()
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    cgiRoot->Exit("/?root");
}

void RootLogin::Impl::GenerateCaptcha()
{
    CaptchaImage->setImageRef(Captcha->Generate()->imageRef());
    int captchaResult = (int)Captcha->GetResult();

    CaptchaValidator->setRange(captchaResult, captchaResult);

    if (PasswordRecoveryFormFlag) {
        ForgotPassword_CaptchaValidator->setRange(captchaResult, captchaResult);
    }
}

void RootLogin::Impl::PasswordRecoveryForm()
{
    if (!PasswordRecoveryFormFlag) {
        PasswordRecoveryDiv->clear();

        CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
        CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

        if (PasswordRecoveryHtmlData == "") {
            string file;
            if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
                file = "../templates/root-login-password-recovery-fa.wtml";
            } else {
                file = "../templates/root-login-password-recovery.wtml";
            }

            if (!CoreLib::FileSystem::Read(file, PasswordRecoveryHtmlData)) {
                return;
            }
        }

        WTemplate *tmpl = new WTemplate(PasswordRecoveryDiv);
        tmpl->setTemplateText(WString(PasswordRecoveryHtmlData), TextFormat::XHTMLUnsafeText);

        ForgotPassword_EmailLineEdit = new WLineEdit();
        ForgotPassword_EmailLineEdit->setPlaceholderText(tr("root-login-password-recovery-email-placeholder"));
        WRegExpValidator *emailValidator = new WRegExpValidator(Pool::Storage()->RegexEmail());
        emailValidator->setFlags(MatchCaseInsensitive);
        emailValidator->setMandatory(true);
        ForgotPassword_EmailLineEdit->setValidator(emailValidator);

        int captchaResult = (int)Captcha->GetResult();

        ForgotPassword_CaptchaLineEdit = new WLineEdit();
        ForgotPassword_CaptchaLineEdit->setPlaceholderText(tr("root-login-captcha-hint"));
        ForgotPassword_CaptchaValidator = new WIntValidator(captchaResult, captchaResult);
        ForgotPassword_CaptchaValidator->setMandatory(true);
        ForgotPassword_CaptchaLineEdit->setValidator(ForgotPassword_CaptchaValidator);

        PasswordRecoveryMessageArea = new WText();

        WPushButton *recoverPushButton = new WPushButton(tr("root-login-password-recovery-recover"));
        recoverPushButton->setStyleClass("btn btn-default");

        tmpl->bindString("email-input-id", ForgotPassword_EmailLineEdit->id());
        tmpl->bindString("captcha-input-id", ForgotPassword_CaptchaLineEdit->id());

        tmpl->bindWidget("email-label-text", new WText(tr("root-login-password-recovery-email")));
        tmpl->bindWidget("captcha-label-text", new WText(tr("root-login-captcha")));

        tmpl->bindWidget("email-input", ForgotPassword_EmailLineEdit);
        tmpl->bindWidget("captcha-input", ForgotPassword_CaptchaLineEdit);
        tmpl->bindWidget("message-area", PasswordRecoveryMessageArea);
        tmpl->bindWidget("recover-button", recoverPushButton);

        ForgotPassword_EmailLineEdit->enterPressed().connect(this, &RootLogin::Impl::OnPasswordRecoveryFormSubmitted);
        ForgotPassword_CaptchaLineEdit->enterPressed().connect(this, &RootLogin::Impl::OnPasswordRecoveryFormSubmitted);
        recoverPushButton->clicked().connect(this, &RootLogin::Impl::OnPasswordRecoveryFormSubmitted);

        PasswordRecoveryFormFlag = true;
    } else {
        PasswordRecoveryDiv->clear();
        PasswordRecoveryFormFlag = false;
    }
}

void RootLogin::Impl::PreserveSessionData(const CDate::Now &n, const std::string &username, const bool saveLocally)
{
    try {
        CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
        CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

        Pool::Database()->Update("ROOT",
                                 "username", username,
                                 "last_login_ip=?, last_login_location=?,"
                                 " last_login_rawtime=?,"
                                 " last_login_gdate=?, last_login_jdate=?,"
                                 " last_login_time=?,"
                                 " last_login_user_agent=?,"
                                 " last_login_referer=?",
                                 {
                                     cgiEnv->GetClientInfo(CgiEnv::ClientInfo::IP),
                                     cgiEnv->GetClientInfo(CgiEnv::ClientInfo::Location),
                                     lexical_cast<std::string>(n.RawTime),
                                     DateConv::ToGregorian(n),
                                     DateConv::DateConv::ToJalali(n),
                                     DateConv::Time(n),
                                     cgiEnv->GetClientInfo(CgiEnv::ClientInfo::Browser),
                                     cgiEnv->GetClientInfo(CgiEnv::ClientInfo::Referer)
                                 });

        string user;
        string token;

        if (saveLocally) {
            Pool::Crypto()->Encrypt(username, user);
            Pool::Crypto()->Encrypt(lexical_cast<std::string>(n.RawTime), token);
        }

        if (cgiRoot->environment().supportsCookies()) {
            cgiRoot->setCookie("cms-session-user",
                           user,
                           Pool::Storage()->RootSessionLifespan());
            cgiRoot->setCookie("cms-session-token",
                           token,
                           Pool::Storage()->RootSessionLifespan());
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

void RootLogin::Impl::SendLoginAlertEmail(const std::string &email, const std::string &username, CDate::Now &n)
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    string htmlData;
    string file;
    if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
        file = "../templates/email-root-login-alert-fa.wtml";
    } else {
        file = "../templates/email-root-login-alert.wtml";
    }

    if (CoreLib::FileSystem::Read(file, htmlData)) {
        replace_all(htmlData, "${username}", username);
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

        CoreLib::Mail *mail = new CoreLib::Mail(
                    cgiEnv->GetServerInfo(CgiEnv::ServerInfo::NoReplyAddr), email,
                    (format(tr("root-login-alert-email-subject").toUTF8())
                     % cgiEnv->GetServerInfo(CgiEnv::ServerInfo::Host)
                     % username).str(),
                    htmlData);
        mail->SetDeleteLater(true);
        mail->SendAsync();
    }
}

void RootLogin::Impl::SendPasswordRecoveryEmail(const std::string &email,
                                                const std::string &username, const std::string &password,
                                                CDate::Now &n)
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    string htmlData;
    string file;
    if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
        file = "../templates/email-root-password-recovery-fa.wtml";
    } else {
        file = "../templates/email-root-password-recovery.wtml";
    }

    if (CoreLib::FileSystem::Read(file, htmlData)) {
        replace_all(htmlData, "${login-url}",
                           cgiEnv->GetServerInfo(CgiEnv::ServerInfo::RootLoginUrl));
        replace_all(htmlData, "${username}", username);
        replace_all(htmlData, "${password}", password);
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

        CoreLib::Mail *mail = new CoreLib::Mail(
                    cgiEnv->GetServerInfo(CgiEnv::ServerInfo::NoReplyAddr), email,
                    (format(tr("root-login-password-recovery-email-subject").toUTF8())
                     % cgiEnv->GetServerInfo(CgiEnv::ServerInfo::Host)
                     % username).str(),
                    htmlData);
        mail->SetDeleteLater(true);
        mail->SendAsync();
    }
}

Wt::WWidget *RootLogin::Impl::LogoutPage()
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    try {
        cgiRoot->removeCookie("cms-session-user");
    } catch (...) {
    }
    try {
        cgiRoot->removeCookie("cms-session-token");
    } catch (...) {
    }

    Div *container = new Div("RootLogout", "root-logout-layout full-width full-height");
    Div *noScript = new Div(container);
    noScript->addWidget(new WText(tr("no-script")));

    string htmlData;
    string file;
    if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
        file = "../templates/root-logout-fa.wtml";
    } else {
        file = "../templates/root-logout.wtml";
    }

    if (CoreLib::FileSystem::Read(file, htmlData)) {
        WTemplate *tmpl = new WTemplate(container);
        tmpl->setStyleClass("container-table");
        tmpl->setTemplateText(WString(htmlData), TextFormat::XHTMLUnsafeText);

        WPushButton *homePagePushButton = new WPushButton(tr("root-logout-go-to-home-page"));
        homePagePushButton->setStyleClass("btn btn-default");

        WPushButton *signInPushButton = new WPushButton(tr("root-logout-sign-in-again"));
        signInPushButton->setStyleClass("btn btn-default");

        tmpl->bindWidget("logout-message", new WText(tr("root-logout-message")));
        tmpl->bindWidget("home-page-button", homePagePushButton);
        tmpl->bindWidget("sign-in-button", signInPushButton);

        homePagePushButton->clicked().connect(this, &RootLogin::Impl::OnGoToHomePageButtonPressed);
        signInPushButton->clicked().connect(this, &RootLogin::Impl::OnSignInAgainButtonPressed);
    }

    return container;
}

