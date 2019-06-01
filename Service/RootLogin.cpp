/**
 * @file
 * @author  Mamadou Babaei <info@babaei.net>
 * @version 0.1.0
 *
 * @section LICENSE
 *
 * (The MIT License)
 *
 * Copyright (c) 2016 - 2019 Mamadou Babaei
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


#include <ctime>
#include <boost/algorithm/string.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <pqxx/pqxx>
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
#include "Cms.hpp"
#include "Div.hpp"
#include "Pool.hpp"
#include "RootLogin.hpp"

using namespace std;
using namespace boost;
using namespace pqxx;
using namespace Wt;
using namespace CoreLib;
using namespace CoreLib::CDate;
using namespace Service;

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
    void PreserveSessionData(const CDate::Now &n, const bool saveLocally);

    void SendLoginAlertEmail(const CDate::Now &n);
    void SendPasswordRecoveryEmail(const std::string &email,
                                   const std::string &username, const std::string &password,
                                   const CDate::Now &n);

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
        if (cgiEnv->GetInformation().Client.Request.Root.Logout) {
            try {
                cgiRoot->removeCookie("cms-session-token");
                LOG_INFO("Root logout request succeed!", cgiEnv->GetInformation().ToJson());
            } catch (...) {
                LOG_ERROR("Root logout request failed!", cgiEnv->GetInformation().ToJson());
            }
            hasValidSession = false;
        } else {
            string token(cgiRoot->environment().getCookie("cms-session-token"));
            Pool::Crypto().Decrypt(token, token);

            try {
                auto conn = Pool::Database().Connection();
                conn->activate();
                pqxx::work txn(*conn.get());

                string query((boost::format("SELECT EXTRACT ( EPOCH FROM expiry::TIMESTAMPTZ ) as expiry"
                                            " FROM \"%1%\" WHERE token = %2%;")
                              % txn.esc(Service::Pool::Database().GetTableName("ROOT_SESSIONS"))
                              % txn.quote(token)).str());
                LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

                result r = txn.exec(query);

                string expiry("0");
                if (!r.empty()) {
                    const pqxx::row row(r[0]);
                    expiry = row["expiry"].c_str();
                }

                time_t rawTime = lexical_cast<time_t>(expiry);

                CDate::Now n(CDate::Timezone::UTC);
                if (rawTime >= n.RawTime()) {
                    try {
                        query.assign((format("SELECT t1.user_id, t1.username, t1.email,"
                                             " EXTRACT ( EPOCH FROM t2.login_time::TIMESTAMPTZ ) as login_time,"
                                             " t2.ip_address, t2.location_country_code, t2.location_country_code3,"
                                             " t2.location_country_name, t2.location_region, t2.location_city,"
                                             " t2.location_postal_code, t2.location_latitude, t2.location_longitude,"
                                             " t2.location_metro_code, t2.location_dma_code, t2.location_area_code,"
                                             " t2.location_charset, t2.location_continent_code, t2.location_netmask,"
                                             " t2.location_asn, t2.location_aso, t2.location_raw_data, "
                                             " t2.user_agent, t2.referer"
                                             " FROM \"%1%\" t1"
                                             " INNER JOIN \"%2%\" t2 ON t1.user_id = t2.user_id"
                                             " WHERE t1.username = %3%"
                                             " ORDER BY t2.login_time DESC LIMIT 1;")
                                      % txn.esc(Pool::Database().GetTableName("ROOT"))
                                      % txn.esc(Pool::Database().GetTableName("ROOT_SESSIONS"))
                                      % txn.quote(Pool::Storage().RootUsername())).str());
                        LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

                        r = txn.exec(query);

                        if (!r.empty()) {
                            const pqxx::row row(r[0]);

                            CgiEnv::InformationRecord::ClientRecord::SessionRecord record;
                            record.UserId = row["user_id"].c_str();
                            record.Username = row["username"].c_str();
                            record.Email = row["email"].c_str();
                            record.LastLogin.Time = lexical_cast<time_t>(row["login_time"].c_str());
                            record.LastLogin.IPAddress = row["ip_address"].c_str();
                            record.LastLogin.GeoLocation.CountryCode = row["location_country_code"].c_str();
                            record.LastLogin.GeoLocation.CountryCode3 = row["location_country_code3"].c_str();
                            record.LastLogin.GeoLocation.CountryName = row["location_country_name"].c_str();
                            record.LastLogin.GeoLocation.Region = row["location_region"].c_str();
                            record.LastLogin.GeoLocation.City = row["location_city"].c_str();
                            record.LastLogin.GeoLocation.PostalCode = row["location_postal_code"].c_str();
                            record.LastLogin.GeoLocation.Latitude = lexical_cast<float>(row["location_latitude"].c_str());
                            record.LastLogin.GeoLocation.Longitude = lexical_cast<float>(row["location_longitude"].c_str());
                            record.LastLogin.GeoLocation.MetroCode = lexical_cast<int>(row["location_metro_code"].c_str());
                            record.LastLogin.GeoLocation.DmaCode = lexical_cast<int>(row["location_dma_code"].c_str());
                            record.LastLogin.GeoLocation.AreaCode = lexical_cast<int>(row["location_area_code"].c_str());
                            record.LastLogin.GeoLocation.Charset = lexical_cast<int>(row["location_charset"].c_str());
                            record.LastLogin.GeoLocation.ContinentCode = row["location_continent_code"].c_str();
                            record.LastLogin.GeoLocation.Netmask = lexical_cast<int>(row["location_netmask"].c_str());
                            record.LastLogin.GeoLocation.ASN = lexical_cast<int>(row["location_asn"].c_str());
                            record.LastLogin.GeoLocation.ASO = row["location_aso"].c_str();
                            record.LastLogin.GeoLocation.RawData = row["location_raw_data"].c_str();
                            record.LastLogin.UserAgent = row["user_agent"].c_str();
                            record.LastLogin.Referer = row["referer"].c_str();

                            cgiEnv->SetSessionRecord(record);

                            LOG_INFO("Successful login!", cgiEnv->GetInformation().ToJson());

                            txn.abort();

                            m_pimpl->PreserveSessionData(n, true);

                            m_pimpl->SendLoginAlertEmail(n);

                            hasValidSession = true;
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
            }

            catch (...) {
                /// Invalid session!
                LOG_ERROR("Invalid session!", cgiEnv->GetInformation().ToJson());
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

    if (!hasValidSession) {
        if (cgiEnv->GetInformation().Client.Request.Root.Logout) {
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
    if (cgiEnv->GetInformation().Client.Language.Code
            == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
        file = "../templates/root-login-fa.wtml";
    } else {
        file = "../templates/root-login.wtml";
    }

    if (CoreLib::FileSystem::Read(file, htmlData)) {
        /// Fill the template
        WTemplate *tmpl = new WTemplate(container);
        tmpl->setStyleClass("container-table");
        tmpl->setTemplateText(WString::fromUTF8(htmlData), TextFormat::XHTMLUnsafeText);

        m_pimpl->UsernameLineEdit = new WLineEdit();
        m_pimpl->UsernameLineEdit->setPlaceholderText(tr("root-login-username-placeholder"));
        WLengthValidator *usernameValidator = new WLengthValidator(Pool::Storage().MinUsernameLength(),
                                                                   Pool::Storage().MaxUsernameLength());
        usernameValidator->setMandatory(true);
        m_pimpl->UsernameLineEdit->setValidator(usernameValidator);

        m_pimpl->PasswordLineEdit = new WLineEdit();
        m_pimpl->PasswordLineEdit->setEchoMode(WLineEdit::Password);
        m_pimpl->PasswordLineEdit->setPlaceholderText(tr("root-login-password-placeholder"));
        WLengthValidator *passwordValidator = new WLengthValidator(Pool::Storage().MinPasswordLength(),
                                                                   Pool::Storage().MaxPasswordLength());
        passwordValidator->setMandatory(true);
        m_pimpl->PasswordLineEdit->setValidator(passwordValidator);

        m_pimpl->Captcha = new Service::Captcha();
        m_pimpl->CaptchaImage = m_pimpl->Captcha->Generate();
        m_pimpl->CaptchaImage->setAlternateText(tr("root-login-captcha-hint"));
        m_pimpl->CaptchaImage->setAttributeValue("title", tr("root-login-captcha-hint"));

        int captchaResult = static_cast<int>(m_pimpl->Captcha->GetResult());

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

    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    try {
        auto conn = Pool::Database().Connection();
        conn->activate();
        pqxx::work txn(*conn.get());

        string userId;
        string username = UsernameLineEdit->text().toUTF8();
        string email;
        bool success = false;

        string query((format("SELECT t1.user_id, t1.username, t1.email, t2.pwd, t3.new_pwd,"
                             " EXTRACT ( EPOCH FROM t3.expiry::TIMESTAMPTZ ) as expiry"
                             " FROM \"%1%\" t1"
                             " INNER JOIN \"%2%\" t2 ON t1.user_id = t2.user_id"
                             " LEFT OUTER JOIN \"%3%\" t3 ON t1.user_id = t3.user_id"
                             " WHERE t1.username = %4%"
                             " ORDER BY t3.request_time DESC LIMIT 1;")
                      % txn.esc(Pool::Database().GetTableName("ROOT"))
                      % txn.esc(Pool::Database().GetTableName("ROOT_CREDENTIALS"))
                      % txn.esc(Pool::Database().GetTableName("ROOT_CREDENTIALS_RECOVERY"))
                      % txn.quote(username)).str());
        LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

        result r = txn.exec(query);

        if (!r.empty()) {
            const pqxx::row row(r[0]);

            userId = row["user_id"].c_str();
            username = row["username"].c_str();
            email = row["email"].c_str();
            string encryptedPwd(row["pwd"].c_str());
            string encryptedRecoveryPwd(row["new_pwd"].c_str());

            time_t expiry = 0;
            try {
                expiry = lexical_cast<time_t>(row["expiry"].c_str());
            } catch (...) {
            }

            string hashedPwd;
            string hashedRecoveryPwd;
            Pool::Crypto().Decrypt(encryptedPwd, hashedPwd);
            Pool::Crypto().Decrypt(encryptedRecoveryPwd, hashedRecoveryPwd);

            CDate::Now n(CDate::Timezone::UTC);

            if (Pool::Crypto().Argon2iVerify(PasswordLineEdit->text().toUTF8(), hashedPwd)) {
                success = true;
                LOG_INFO("Legit login password!", username, cgiEnv->GetInformation().ToJson());
            } else if (expiry >= n.RawTime() && Pool::Crypto().Argon2iVerify(PasswordLineEdit->text().toUTF8(), hashedRecoveryPwd)) {
                success = true;
                LOG_INFO("Legit recovery password!", username, cgiEnv->GetInformation().ToJson());

                query.assign((boost::format("UPDATE ONLY \"%1%\""
                                            " SET expiry = '19700101'::TIMESTAMPTZ,"
                                            " utilization_time = TO_TIMESTAMP( %2% )::TIMESTAMPTZ, utilization_ip_address = %3%,"
                                            " utilization_location_country_code = %4%, utilization_location_country_code3 = %5%,"
                                            " utilization_location_country_name = %6%, utilization_location_region = %7%,"
                                            " utilization_location_city = %8%, utilization_location_postal_code = %9%,"
                                            " utilization_location_latitude = %10%, utilization_location_longitude = %11%,"
                                            " utilization_location_metro_code = %12%, utilization_location_dma_code = %13%,"
                                            " utilization_location_area_code = %14%, utilization_location_charset = %15%,"
                                            " utilization_location_continent_code = %16%, utilization_location_netmask = %17%,"
                                            " utilization_location_asn = %18%, utilization_location_aso = %19%,"
                                            " utilization_location_raw_data = %20%,"
                                            " utilization_user_agent = %21%, utilization_referer = %22%"
                                            " WHERE user_id = %23%;")
                              % txn.esc(Service::Pool::Database().GetTableName("ROOT_CREDENTIALS_RECOVERY"))
                              % txn.esc(lexical_cast<string>(n.RawTime()))
                              % txn.quote(cgiEnv->GetInformation().Client.IPAddress)
                              % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.CountryCode)
                              % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.CountryCode3)
                              % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.CountryName)
                              % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.Region)
                              % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.City)
                              % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.PostalCode)
                              % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.Latitude))
                              % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.Longitude))
                              % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.MetroCode))
                              % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.DmaCode))
                              % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.AreaCode))
                              % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.Charset))
                              % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.ContinentCode)
                              % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.Netmask))
                              % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.ASN))
                              % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.ASO)
                              % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.RawData)
                              % txn.quote(cgiEnv->GetInformation().Client.UserAgent)
                              % txn.quote(cgiEnv->GetInformation().Client.Referer)
                              % txn.quote(userId)).str());
                LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

                r = txn.exec(query);

                Pool::Database().Update("ROOT_CREDENTIALS",
                                         "user_id", userId,
                                         "pwd=?",
                                        { encryptedRecoveryPwd });
            }
        } else {
            LOG_ERROR("Login query does not match!", username, cgiEnv->GetInformation().ToJson());
        }

        if (!success) {
            LOG_ERROR("Login failed!", username, cgiEnv->GetInformation().ToJson());
            txn.abort();
            m_parent->HtmlError(tr("root-login-fail"), LoginMessageArea);
            UsernameLineEdit->setFocus();
            GenerateCaptcha();
            return;
        }

        CDate::Now n(CDate::Timezone::UTC);

        CgiEnv::InformationRecord::ClientRecord::SessionRecord record;
        record.UserId = userId;
        record.Username = username;
        record.Email = email;

        try {
            query.assign((format("SELECT t1.email,"
                                 " EXTRACT ( EPOCH FROM t2.login_time::TIMESTAMPTZ ) as login_time,"
                                 " t2.ip_address, t2.location_country_code, t2.location_country_code3,"
                                 " t2.location_country_name, t2.location_region, t2.location_city,"
                                 " t2.location_postal_code, t2.location_latitude, t2.location_longitude,"
                                 " t2.location_metro_code, t2.location_dma_code, t2.location_area_code,"
                                 " t2.location_charset, t2.location_continent_code, t2.location_netmask,"
                                 " t2.location_asn, t2.location_aso, t2.location_raw_data,"
                                 " t2.user_agent, t2.referer"
                                 " FROM \"%1%\" t1"
                                 " INNER JOIN \"%2%\" t2 ON t1.user_id = t2.user_id"
                                 " WHERE t1.user_id = %3%"
                                 " ORDER BY t2.login_time DESC LIMIT 1;")
                          % txn.esc(Pool::Database().GetTableName("ROOT"))
                          % txn.esc(Pool::Database().GetTableName("ROOT_SESSIONS"))
                          % txn.quote(userId)).str());
            LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

            r = txn.exec(query);

            if (!r.empty()) {
                const pqxx::row row(r[0]);

                record.LastLogin.Time = lexical_cast<time_t>(row["login_time"].c_str());
                record.LastLogin.IPAddress = row["ip_address"].c_str();
                record.LastLogin.GeoLocation.ContinentCode = row["location_country_code"].c_str();
                record.LastLogin.GeoLocation.CountryCode3 = row["location_country_code3"].c_str();
                record.LastLogin.GeoLocation.CountryName = row["location_country_name"].c_str();
                record.LastLogin.GeoLocation.Region = row["location_region"].c_str();
                record.LastLogin.GeoLocation.City = row["location_city"].c_str();
                record.LastLogin.GeoLocation.PostalCode = row["location_postal_code"].c_str();
                record.LastLogin.GeoLocation.Latitude = lexical_cast<float>(row["location_latitude"].c_str());
                record.LastLogin.GeoLocation.Longitude = lexical_cast<float>(row["location_longitude"].c_str());
                record.LastLogin.GeoLocation.MetroCode = lexical_cast<int>(row["location_metro_code"].c_str());
                record.LastLogin.GeoLocation.DmaCode = lexical_cast<int>(row["location_dma_code"].c_str());
                record.LastLogin.GeoLocation.AreaCode = lexical_cast<int>(row["location_area_code"].c_str());
                record.LastLogin.GeoLocation.Charset = lexical_cast<int>(row["location_charset"].c_str());
                record.LastLogin.GeoLocation.ContinentCode = row["location_continent_code"].c_str();
                record.LastLogin.GeoLocation.Netmask = lexical_cast<int>(row["location_netmask"].c_str());
                record.LastLogin.GeoLocation.ASN = lexical_cast<int>(row["location_asn"].c_str());
                record.LastLogin.GeoLocation.ASO = row["location_aso"].c_str();
                record.LastLogin.GeoLocation.RawData = row["location_raw_data"].c_str();
                record.LastLogin.UserAgent = row["user_agent"].c_str();
                record.LastLogin.Referer = row["referer"].c_str();
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

        cgiEnv->SetSessionRecord(record);

        LOG_INFO("Successful login!", cgiEnv->GetInformation().ToJson());

        txn.commit();

        PreserveSessionData(n, RememberMeCheckBox->checkState() == Wt::Checked);

        SendLoginAlertEmail(n);

        /// It's absolutely safe (even in case of throwing an exception in the constructor)
        /// since we attach it to Wt's WObject hierarchy in it's constructor.
        new Cms();

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

    LOG_ERROR("Internal server error!", cgiEnv->GetInformation().ToJson());
    m_parent->HtmlError(tr("internal-server-error"), LoginMessageArea);
    ForgotPassword_EmailLineEdit->setFocus();
    GenerateCaptcha();
}

void RootLogin::Impl::OnPasswordRecoveryFormSubmitted()
{
    if (!m_parent->Validate(ForgotPassword_CaptchaLineEdit)
            || !m_parent->Validate(ForgotPassword_EmailLineEdit)) {
        GenerateCaptcha();
        return;
    }

    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    auto conn = Pool::Database().Connection();
    conn->activate();
    pqxx::work txn(*conn.get());

    try {
        string email(ForgotPassword_EmailLineEdit->text().toUTF8());

        string query((boost::format("SELECT user_id, username FROM \"%1%\" WHERE email = %2%;")
                      % txn.esc(Service::Pool::Database().GetTableName("ROOT"))
                      % txn.quote(email)).str());
        LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

        result r = txn.exec(query);

        if (r.empty()) {
            LOG_ERROR("Password recovery request failed!", cgiEnv->GetInformation().ToJson());
            m_parent->HtmlError(tr("root-login-password-recovery-fail"), PasswordRecoveryMessageArea);
            ForgotPassword_EmailLineEdit->setFocus();
            GenerateCaptcha();
            return;
        }

        CDate::Now n(CDate::Timezone::UTC);
        time_t expiry = n.RawTime() + Pool::Storage().ResetPwdLifespan();

        const pqxx::row row(r[0]);
        string userId(row["user_id"].c_str());
        string username(row["username"].c_str());

        LOG_INFO("Generating a new password...", email, username, cgiEnv->GetInformation().ToJson());

        string pwd;
        string encryptedPwd;
        Random::Characters(Random::Character::Alphanumeric,
                           static_cast<size_t>(Pool::Storage().MaxPasswordLength()), pwd);
        Pool::Crypto().Argon2i(pwd, encryptedPwd,
                                CoreLib::Crypto::Argon2iOpsLimit::Interactive,
                                CoreLib::Crypto::Argon2iMemLimit::Interactive);
        Pool::Crypto().Encrypt(encryptedPwd, encryptedPwd);

        string token;
        while (true) {
            CoreLib::Random::Uuid(token);

            query.assign((boost::format("SELECT token FROM \"%1%\" WHERE token = %2%;")
                          % txn.esc(Service::Pool::Database().GetTableName("ROOT_CREDENTIALS_RECOVERY"))
                          % txn.quote(token)).str());
            LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

            r = txn.exec(query);

            if (r.empty()) {
                break;
            }
        }

        query.assign((boost::format("INSERT INTO \"%1%\""
                                    " ( token, user_id, expiry, new_pwd, request_time, request_ip_address,"
                                    " request_location_country_code, request_location_country_code3,"
                                    " request_location_country_name, request_location_region, request_location_city,"
                                    " request_location_postal_code, request_location_latitude, request_location_longitude,"
                                    " request_location_metro_code, request_location_dma_code, request_location_area_code,"
                                    " request_location_charset, request_location_continent_code, request_location_netmask,"
                                    " request_location_asn, request_location_aso, request_location_raw_data,"
                                    " request_user_agent, request_referer )"
                                    " VALUES ( %2%, %3%, TO_TIMESTAMP( %4% )::TIMESTAMPTZ, %5%, TO_TIMESTAMP( %6% )::TIMESTAMPTZ,"
                                    " %7%, %8%, %9%, %10%, %11%, %12%, %13%, %14%, %15%, %16%, %17%, %18%, %19%, %20%, %21%, %22%, %23%, %24%, %25%, %26% );")
                      % txn.esc(Service::Pool::Database().GetTableName("ROOT_CREDENTIALS_RECOVERY"))
                      % txn.quote(token)
                      % txn.quote(userId)
                      % txn.esc(lexical_cast<string>(expiry))
                      % txn.quote(encryptedPwd)
                      % txn.esc(lexical_cast<string>(n.RawTime()))
                      % txn.quote(cgiEnv->GetInformation().Client.IPAddress)
                      % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.CountryCode)
                      % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.CountryCode3)
                      % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.CountryName)
                      % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.Region)
                      % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.City)
                      % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.PostalCode)
                      % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.Latitude))
                      % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.Longitude))
                      % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.MetroCode))
                      % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.DmaCode))
                      % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.AreaCode))
                      % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.Charset))
                      % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.ContinentCode)
                      % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.Netmask))
                      % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.ASN))
                      % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.ASO)
                      % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.RawData)
                      % txn.quote(cgiEnv->GetInformation().Client.UserAgent)
                      % txn.quote(cgiEnv->GetInformation().Client.Referer)).str());
        LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

        r = txn.exec(query);

        txn.commit();

        SendPasswordRecoveryEmail(email, username, pwd, n);

        m_parent->HtmlInfo(tr("root-login-password-recovery-success"), PasswordRecoveryMessageArea);
        UsernameLineEdit->setFocus();
        GenerateCaptcha();

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

    LOG_ERROR("Internal server error!", cgiEnv->GetInformation().ToJson());
    m_parent->HtmlError(tr("internal-server-error"), PasswordRecoveryMessageArea);
    ForgotPassword_EmailLineEdit->setFocus();
    GenerateCaptcha();
}

void RootLogin::Impl::OnGoToHomePageButtonPressed()
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    try {
        string homePageFields;

        if (cgiEnv->GetInformation().Client.Language.Code
                == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
            homePageFields = "homepage_url_fa";
        } else {
            homePageFields = "homepage_url_en";
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
        if (!r.empty()) {
            const pqxx::row row(r[0]);
            homePageUrl.assign(row[0].c_str());
        }

        cgiRoot->Exit(homePageUrl);
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

void RootLogin::Impl::OnSignInAgainButtonPressed()
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    cgiRoot->Exit("/?root");
}

void RootLogin::Impl::GenerateCaptcha()
{
    CaptchaImage->setImageRef(Captcha->Generate()->imageRef());
    int captchaResult = static_cast<int>(Captcha->GetResult());

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
            if (cgiEnv->GetInformation().Client.Language.Code
                    == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
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
        WRegExpValidator *emailValidator = new WRegExpValidator(Pool::Storage().RegexEmail());
        emailValidator->setFlags(MatchCaseInsensitive);
        emailValidator->setMandatory(true);
        ForgotPassword_EmailLineEdit->setValidator(emailValidator);

        int captchaResult = static_cast<int>(Captcha->GetResult());

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

void RootLogin::Impl::PreserveSessionData(const CDate::Now &n, const bool saveLocally)
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    try {
        auto conn = Pool::Database().Connection();
        conn->activate();
        pqxx::work txn(*conn.get());

        string token;
        while (true) {
            CoreLib::Random::Uuid(token);

            string query((boost::format("SELECT token FROM \"%1%\" WHERE token = %2%;")
                         % txn.esc(Service::Pool::Database().GetTableName("ROOT_SESSIONS"))
                         % txn.quote(token)).str());
            LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

            result r = txn.exec(query);

            if (r.empty()) {
                break;
            }
        }

        cgiEnv->SetSessionToken(token);

        time_t expiry;
        if (saveLocally) {
            /// expiry > login_time --> a recurring session
            /// due to checking "Remember Me" on the login form
            expiry = n.RawTime() + Pool::Storage().RootSessionLifespan();
        } else {
            /// expiry == login_time --> a temporary session
            expiry = n.RawTime(); // ,
        }
        /// expiry == 0 --> force exit the session

        string query((boost::format("INSERT INTO \"%1%\""
                                    " ( token, user_id, expiry, login_time,"
                                    " ip_address, location_country_code, location_country_code3,"
                                    " location_country_name, location_region, location_city,"
                                    " location_postal_code, location_latitude, location_longitude,"
                                    " location_metro_code, location_dma_code, location_area_code,"
                                    " location_charset, location_continent_code, location_netmask,"
                                    " location_asn, location_aso, location_raw_data,"
                                    " user_agent, referer )"
                                    " VALUES ( %2%, %3%, TO_TIMESTAMP( %4% )::TIMESTAMPTZ, TO_TIMESTAMP( %5% )::TIMESTAMPTZ,"
                                    " %6%, %7%, %8%, %9%, %10%, %11%, %12%, %13%, %14%, %15%, %16%, %17%, %18%, %19%, %20%, %21%, %22%, %23%, %24%, %25% );")
                      % txn.esc(Service::Pool::Database().GetTableName("ROOT_SESSIONS"))
                      % txn.quote(token)
                      % txn.quote(cgiEnv->GetInformation().Client.Session.UserId)
                      % txn.esc(lexical_cast<string>(expiry))
                      % txn.esc(lexical_cast<string>(n.RawTime()))
                      % txn.quote(cgiEnv->GetInformation().Client.IPAddress)
                      % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.CountryCode)
                      % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.CountryCode3)
                      % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.CountryName)
                      % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.Region)
                      % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.City)
                      % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.PostalCode)
                      % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.Latitude))
                      % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.Longitude))
                      % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.MetroCode))
                      % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.DmaCode))
                      % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.AreaCode))
                      % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.Charset))
                      % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.ContinentCode)
                      % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.Netmask))
                      % txn.quote(lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.ASN))
                      % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.ASO)
                      % txn.quote(cgiEnv->GetInformation().Client.GeoLocation.RawData)
                      % txn.quote(cgiEnv->GetInformation().Client.UserAgent)
                      % txn.quote(cgiEnv->GetInformation().Client.Referer)).str());
        LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

        result r = txn.exec(query);

        txn.commit();

        if (saveLocally) {
            Pool::Crypto().Encrypt(token, token);

            if (cgiRoot->environment().supportsCookies()) {
                cgiRoot->setCookie("cms-session-token",
                                   token,
                                   Pool::Storage().RootSessionLifespan());
                LOG_ERROR("Saved session token on client!", cgiEnv->GetInformation().ToJson(););
            } else {
                LOG_ERROR("Client has no cookie support!", cgiEnv->GetInformation().ToJson(););
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

void RootLogin::Impl::SendLoginAlertEmail(const CDate::Now &n)
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    string htmlData;
    string file;
    if (cgiEnv->GetInformation().Client.Language.Code
            == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
        file = "../templates/email-root-login-alert-fa.wtml";
    } else {
        file = "../templates/email-root-login-alert.wtml";
    }

    if (CoreLib::FileSystem::Read(file, htmlData)) {
        replace_all(htmlData, "${username}", cgiEnv->GetInformation().Client.Session.Username);
        replace_all(htmlData, "${client-ip}",
                    cgiEnv->GetInformation().Client.IPAddress);
        replace_all(htmlData, "${client-user-agent}",
                    cgiEnv->GetInformation().Client.UserAgent);
        replace_all(htmlData, "${client-referer}",
                    cgiEnv->GetInformation().Client.Referer);
        replace_all(htmlData, "${time}",
                    (format("%1% ~ %2%")
                     % WString(DateConv::FormatToPersianNums(DateConv::ToJalali(n))).toUTF8()
                     % algorithm::trim_copy(DateConv::DateTimeString(n))).str());
        replace_all(htmlData, "${client-location-country-code}",
                    cgiEnv->GetInformation().Client.GeoLocation.CountryCode);
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
        replace_all(htmlData, "${client-location-continent-code}",
                    cgiEnv->GetInformation().Client.GeoLocation.ContinentCode);
        replace_all(htmlData, "${client-location-asn}",
                    lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.ASN));
        replace_all(htmlData, "${client-location-aso}",
                    cgiEnv->GetInformation().Client.GeoLocation.ASO);
        replace_all(htmlData, "${client-location-raw-data}",
                    cgiEnv->GetInformation().Client.GeoLocation.RawData);

        LOG_INFO("Sending login alert email...", cgiEnv->GetInformation().ToJson(););

        CoreLib::Mail *mail = new CoreLib::Mail(
                    cgiEnv->GetInformation().Server.NoReplyAddress, cgiEnv->GetInformation().Client.Session.Email,
                    (format(tr("root-login-alert-email-subject").toUTF8())
                     % cgiEnv->GetInformation().Server.Hostname
                     % cgiEnv->GetInformation().Client.Session.Username).str(),
                    htmlData);
        mail->SetDeleteLater(true);
        mail->SendAsync();
    }
}

void RootLogin::Impl::SendPasswordRecoveryEmail(const std::string &email,
                                                const std::string &username, const std::string &password,
                                                const CDate::Now &n)
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    string htmlData;
    string file;
    if (cgiEnv->GetInformation().Client.Language.Code
            == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
        file = "../templates/email-root-password-recovery-fa.wtml";
    } else {
        file = "../templates/email-root-password-recovery.wtml";
    }

    if (CoreLib::FileSystem::Read(file, htmlData)) {
        replace_all(htmlData, "${login-url}",
                    cgiEnv->GetInformation().Server.RootLoginUrl);
        replace_all(htmlData, "${username}", username);
        replace_all(htmlData, "${password}", password);
        replace_all(htmlData, "${client-ip}",
                    cgiEnv->GetInformation().Client.IPAddress);
        replace_all(htmlData, "${client-user-agent}",
                    cgiEnv->GetInformation().Client.UserAgent);
        replace_all(htmlData, "${client-referer}",
                    cgiEnv->GetInformation().Client.Referer);
        replace_all(htmlData, "${time}",
                    algorithm::trim_copy(DateConv::DateTimeString(n)));
        replace_all(htmlData, "${client-location-country-code}",
                    cgiEnv->GetInformation().Client.GeoLocation.CountryCode);
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
        replace_all(htmlData, "${client-location-continent-code}",
                    cgiEnv->GetInformation().Client.GeoLocation.ContinentCode);
        replace_all(htmlData, "${client-location-asn}",
                    lexical_cast<string>(cgiEnv->GetInformation().Client.GeoLocation.ASN));
        replace_all(htmlData, "${client-location-aso}",
                    cgiEnv->GetInformation().Client.GeoLocation.ASO);
        replace_all(htmlData, "${client-location-raw-data}",
                    cgiEnv->GetInformation().Client.GeoLocation.RawData);

        LOG_INFO("Sending password recovery email...", email, username, cgiEnv->GetInformation().ToJson(););

        CoreLib::Mail *mail = new CoreLib::Mail(
                    cgiEnv->GetInformation().Server.NoReplyAddress, email,
                    (format(tr("root-login-password-recovery-email-subject").toUTF8())
                     % cgiEnv->GetInformation().Server.Hostname
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
        cgiRoot->removeCookie("cms-session-token");
        LOG_INFO("Root logout succeed!", cgiEnv->GetInformation().ToJson());
    } catch (...) {
        LOG_ERROR("Root logout failed!", cgiEnv->GetInformation().ToJson());
    }

    Div *container = new Div("RootLogout", "root-logout-layout full-width full-height");
    Div *noScript = new Div(container);
    noScript->addWidget(new WText(tr("no-script")));

    string htmlData;
    string file;
    if (cgiEnv->GetInformation().Client.Language.Code
            == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
        file = "../templates/root-logout-fa.wtml";
    } else {
        file = "../templates/root-logout.wtml";
    }

    if (CoreLib::FileSystem::Read(file, htmlData)) {
        WTemplate *tmpl = new WTemplate(container);
        tmpl->setStyleClass("container-table");
        tmpl->setTemplateText(WString::fromUTF8(htmlData), TextFormat::XHTMLUnsafeText);

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
