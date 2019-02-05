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
 * Main entry for the web application. When a user visits the website, this is
 * the first thing that runs.
 */


#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>
#include <Wt/WApplication>
#include <Wt/WBootstrapTheme>
#include <Wt/WContainerWidget>
#include <Wt/WCssStyleSheet>
#include <Wt/WEnvironment>
#include <Wt/WText>
#include <CoreLib/Log.hpp>
#include <CoreLib/make_unique.hpp>
#include "CgiRoot.hpp"
#include "CgiEnv.hpp"
#include "Exception.hpp"
#include "Home.hpp"
#include "Pool.hpp"
#include "RootLogin.hpp"

#define         ALICE               L"<pre>Alice is not in Wonderland!!</pre>"

using namespace std;
using namespace boost;
using namespace Wt;
using namespace CoreLib;
using namespace Service;

struct CgiRoot::Impl
{
public:
    boost::mutex *CgiEnvMutex;
    unique_ptr<CgiEnv> CgiEnvInstance;

private:
    CgiRoot *m_parent;

public:
    explicit Impl(CgiRoot *parent);
    ~Impl();

public:
    Wt::WWidget *GetHomePage();
    Wt::WWidget *GetRootLoginPage();

    void ReloadWithLanguage(const std::string &lang);
};

WApplication *CgiRoot::CreateApplication(const WEnvironment &env)
{
    return new CgiRoot(env);
}

CgiRoot::CgiRoot(const WEnvironment &env)
    : WApplication(env),
      m_pimpl(make_unique<CgiRoot::Impl>(this))
{
    try {
        this->setInternalPathDefaultValid(false);

        WBootstrapTheme *bootstrapTheme = new WBootstrapTheme();
        bootstrapTheme->setVersion(WBootstrapTheme::Version3);
        bootstrapTheme->setResponsive(true);
        bootstrapTheme->setFormControlStyleEnabled(true);
        setTheme(bootstrapTheme);

        CgiEnv *cgiEnv = this->GetCgiEnvInstance();

        if (cgiEnv->GetInformation().Client.Security.XssAttackDetected)
            throw Service::Exception<std::wstring>(ALICE);

        root()->clear();

        switch (cgiEnv->GetInformation().Client.Language.Code) {
        case CgiEnv::InformationRecord::ClientRecord::LanguageCode::None:
        case CgiEnv::InformationRecord::ClientRecord::LanguageCode::Invalid:
            try {
                m_pimpl->ReloadWithLanguage(env.getCookie("lang"));
            } catch (...) {
                if (algorithm::contains(
                        cgiEnv->GetInformation().Client.GeoLocation.CountryName,
                        "Iran")
                    || algorithm::starts_with(locale().name(), "fa")) {
                    m_pimpl->ReloadWithLanguage("fa");
                } else {
                    m_pimpl->ReloadWithLanguage("en");
                }
            }
            return;

        case CgiEnv::InformationRecord::ClientRecord::LanguageCode::En:
        case CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa:
            if (env.supportsCookies()) {
                setCookie("lang",cgiEnv->GetInformation().Client.Language.CodeAsString,
                          Pool::Storage().LanguageCookieLifespan());
            }
        }

        setLocale(cgiEnv->GetInformation().Client.Language.CodeAsString);
        messageResourceBundle().use(appRoot() + "../i18n/localization");

        if (cgiEnv->GetInformation().Client.Language.PageDirection
                == CgiEnv::InformationRecord::ClientRecord::PageDirection::RightToLeft) {
            setLayoutDirection(Wt::LayoutDirection::RightToLeft);
        }

        if (!cgiEnv->GetInformation().Client.Request.Root.Login) {
            root()->addWidget(m_pimpl->GetHomePage());
        } else {
            root()->addWidget(m_pimpl->GetRootLoginPage());
        }
    }

    catch (Service::Exception<std::wstring> &ex) {
        root()->clear();
        root()->addWidget(new WText(ex.What()));
    }

    catch (Service::Exception<std::string> &ex) {
        root()->clear();
        root()->addWidget(new WText(WString::fromUTF8(ex.What())));
    }

    catch (CoreLib::Exception<std::wstring> &ex) {
        LOG_ERROR(Wt::WString(ex.What()).toUTF8(), GetCgiEnvInstance()->GetInformation().ToJson());
    }

    catch (CoreLib::Exception<std::string> &ex) {
        LOG_ERROR(ex.What(), GetCgiEnvInstance()->GetInformation().ToJson());
    }

    catch (...) {
        LOG_ERROR(UNKNOWN_ERROR, GetCgiEnvInstance()->GetInformation().ToJson());
    }
}

CgiRoot::~CgiRoot() = default;

CgiEnv *CgiRoot::GetCgiEnvInstance()
{
    if (m_pimpl->CgiEnvInstance == nullptr) {
        struct make_unique_enabler : public CgiEnv {
            /// if it has non-default constructors,
            /// you will also need to expose them
            //template <typename ..._ARGS>
            //make_unique_enabler(_ARGS &&...args)
            //    : CgiEnv(std::forward<_ARGS>(args)...)
            //{
            //}
        };
        m_pimpl->CgiEnvInstance = make_unique<make_unique_enabler>();
    }

    return m_pimpl->CgiEnvInstance.get();
}

void CgiRoot::Exit(const std::string &url)
{
    redirect(url);
    quit();
}

CgiRoot::Impl::Impl(CgiRoot *parent) :
    m_parent(parent)
{

}

CgiRoot::Impl::~Impl() = default;

Wt::WWidget *CgiRoot::Impl::GetHomePage()
{
    CgiEnv *cgiEnv = m_parent->GetCgiEnvInstance();

    m_parent->useStyleSheet("css/home.css");

    switch (cgiEnv->GetInformation().Client.Language.Code) {
    case CgiEnv::InformationRecord::ClientRecord::LanguageCode::En:
        m_parent->useStyleSheet("css/home-ltr.css");
        m_parent->useStyleSheet("css/home-en.css");

        /// Google Webfonts (Monserrat 400/700, Open Sans 400/600)
        m_parent->useStyleSheet("css/wf-montserrat-v6-latin.css");
        m_parent->useStyleSheet("css/wf-open-sans-v13-latin.css");

        /// Load our fonts individually if IE8+, to avoid faux bold & italic rendering
        m_parent->useStyleSheet(Wt::WCssStyleSheet("css/wf-montserrat-v6-latin-regular.css"), "IE");
        m_parent->useStyleSheet(Wt::WCssStyleSheet("css/wf-montserrat-v6-latin-bold.css"), "IE");
        m_parent->useStyleSheet(Wt::WCssStyleSheet("css/wf-open-sans-v13-latin-regular.css"), "IE");
        m_parent->useStyleSheet(Wt::WCssStyleSheet("css/wf-open-sans-v13-latin-semibold.css"), "IE");
        break;
    case CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa:
        m_parent->useStyleSheet("css/home-rtl.css");
        m_parent->useStyleSheet("css/home-fa.css");

        /// Farsi Webfont (Yekan 400)
        m_parent->useStyleSheet("css/wf-yekan.css");
        break;
    case CgiEnv::InformationRecord::ClientRecord::LanguageCode::None:
    case CgiEnv::InformationRecord::ClientRecord::LanguageCode::Invalid:
        break;
    }

    m_parent->useStyleSheet("resources/font-awesome/css/font-awesome.min.css");

    return new Home();
}

Wt::WWidget *CgiRoot::Impl::GetRootLoginPage()
{
    CgiEnv *cgiEnv = m_parent->GetCgiEnvInstance();

    m_parent->useStyleSheet("css/root.css");

    switch (cgiEnv->GetInformation().Client.Language.Code) {
    case CgiEnv::InformationRecord::ClientRecord::LanguageCode::En:
        m_parent->useStyleSheet("css/root-ltr.css");
        m_parent->useStyleSheet("css/root-en.css");
        break;
    case CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa:
        m_parent->useStyleSheet("css/root-rtl.css");
        m_parent->useStyleSheet("css/root-fa.css");
        break;
    case CgiEnv::InformationRecord::ClientRecord::LanguageCode::None:
    case CgiEnv::InformationRecord::ClientRecord::LanguageCode::Invalid:
        break;
    }

    /// Google Webfonts (Monserrat 400/700, Open Sans 400/600)
    m_parent->useStyleSheet("css/wf-montserrat-v6-latin.css");
    m_parent->useStyleSheet("css/wf-open-sans-v13-latin.css");

    /// Load our fonts individually if IE8+, to avoid faux bold & italic rendering
    m_parent->useStyleSheet(Wt::WCssStyleSheet("css/wf-montserrat-v6-latin-regular.css"), "IE");
    m_parent->useStyleSheet(Wt::WCssStyleSheet("css/wf-montserrat-v6-latin-bold.css"), "IE");
    m_parent->useStyleSheet(Wt::WCssStyleSheet("css/wf-open-sans-v13-latin-regular.css"), "IE");
    m_parent->useStyleSheet(Wt::WCssStyleSheet("css/wf-open-sans-v13-latin-semibold.css"), "IE");

    /// Farsi Webfont (Yekan 400)
    m_parent->useStyleSheet("css/wf-yekan.css");

    m_parent->useStyleSheet("resources/font-awesome/css/font-awesome.min.css");

    m_parent->require("js/jquery.min.js");
    m_parent->require("js/bootstrap.min.js");

    return new RootLogin();
}

void CgiRoot::Impl::ReloadWithLanguage(const std::string &lang)
{
    string queryString(m_parent->environment().getCgiValue("QUERY_STRING"));

    if (algorithm::starts_with(queryString, "?")) {
        boost::replace_first(queryString, "?", "");
    }

    string newQueryString;

    bool langFlag = false;
    int argsCount = 0;

    vector<string> pairs;
    boost::split(pairs, queryString, boost::is_any_of("&"));
    for (const auto &p : pairs) {
        vector<string> pair;
        boost::split(pair, p, boost::is_any_of("="));
        if (pair.size() > 0) {
            if (pair[0] != "lang") {
                if (pair.size() == 1) {
                    string key = trim_copy(pair[0]);
                    if (key != "") {
                        if (argsCount == 0) {
                            newQueryString += (format("?%1%") % key).str();
                        } else {
                            newQueryString += (format("&%1%") % key).str();
                        }
                        ++argsCount;
                    }
                }
                else {
                    string value;
                    string key = trim_copy(pair[0]);
                    if (key != "") {
                        for (size_t i = 1; i < pair.size(); ++i) {
                            value += pair[i];
                        }
                        if (argsCount == 0) {
                            newQueryString += (format("?%1%=%2%") % key % value).str();
                        } else {
                            newQueryString += (format("&%1%=%2%") % key % value).str();
                        }
                        ++argsCount;
                    }
                }
            } else if (!langFlag) {
                if (argsCount == 0) {
                    newQueryString += (format("?lang=%1%") % lang).str();
                } else {
                    newQueryString += (format("&lang=%1%") % lang).str();
                }
                langFlag = true;
            }
        }
    }

    if (!langFlag) {
        if (argsCount == 0) {
            newQueryString += (format("?lang=%1%") % lang).str();
        } else {
            newQueryString += (format("&lang=%1%") % lang).str();
        }
    }

    m_parent->Exit(newQueryString);
}
