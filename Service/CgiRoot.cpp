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
 * Main entry for the web application. When a user visits the website, this is
 * the first thing that runs.
 */


#include <boost/algorithm/string.hpp>
#include <Wt/WApplication>
#include <Wt/WBootstrapTheme>
#include <Wt/WContainerWidget>
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

#define         UNKNOWN_ERROR       "Unknown error!"
#define         ALICE               L"<pre>Alice is not in Wonderland!!</pre>"

using namespace std;
using namespace Wt;
using namespace CoreLib;
using namespace Service;

struct CgiRoot::Impl
{
private:
    CgiRoot *m_cgiRoot;

public:
    explicit Impl(CgiRoot *cgiRoot);

    Wt::WWidget *GetHomePage();
    Wt::WWidget *GetRootLoginPage();

    void ReloadWithLanguage(const std::string &lang);
};

WApplication *CgiRoot::CreateApplication(const WEnvironment &env)
{
    return new CgiRoot(env);
}

CgiRoot::CgiRoot(const WEnvironment &env) :
    WApplication(env),
    m_pimpl(std::make_unique<CgiRoot::Impl>(this))
{
    try {
        this->setInternalPathDefaultValid(false);

        WBootstrapTheme *bootstrapTheme = new WBootstrapTheme();
        bootstrapTheme->setVersion(WBootstrapTheme::Version3);
        bootstrapTheme->setResponsive(true);
        bootstrapTheme->setFormControlStyleEnabled(true);
        setTheme(bootstrapTheme);

        CgiEnvInstance = std::make_shared<CgiEnv>(env);
        if (CgiEnvInstance->FoundXSS())
            throw Service::Exception(ALICE);

        root()->clear();
        HtmlRoot = root();

        switch (CgiEnvInstance->GetCurrentLanguage()) {
        case CgiEnv::Language::None:
        case CgiEnv::Language::Invalid:
            try {
                m_pimpl->ReloadWithLanguage(env.getCookie("lang"));
            } catch (...) {
                if (boost::algorithm::contains(
                            CgiEnvInstance->GetClientInfo(CgiEnv::ClientInfo::Location),
                            "Iran")
                        || boost::algorithm::starts_with(locale().name(), "fa")) {
                    m_pimpl->ReloadWithLanguage("fa");
                } else {
                    m_pimpl->ReloadWithLanguage("en");
                }
            }
            return;

        case CgiEnv::Language::En:
        case CgiEnv::Language::Fa:
            if (env.supportsCookies()) {
                setCookie("lang", CgiEnvInstance->GetCurrentLanguageString(),
                          Pool::Storage()->LanguageCookieLifespan());
            }
        }

        setLocale(CgiEnvInstance->GetCurrentLanguageString());
        messageResourceBundle().use(appRoot() + "../i18n/localization");

        if (CgiEnvInstance->GetCurrentLanguageDirection() == CgiEnv::LanguageDirection::RightToLeft) {
            setLayoutDirection(Wt::LayoutDirection::RightToLeft);
        }

        if (!CgiEnvInstance->IsRootLoginRequested()) {
            root()->addWidget(m_pimpl->GetHomePage());
        } else {
            root()->addWidget(m_pimpl->GetRootLoginPage());
        }
    }

    catch (const Service::Exception &ex) {
        root()->clear();
        root()->addWidget(new WText(ex.What()));
    }

    catch (const CoreLib::Exception &ex) {
        LOG_ERROR(ex.what());
    }

    catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }
}

void CgiRoot::Redirect(const std::string &url)
{
    redirect(url);
}

void CgiRoot::Exit(const std::string &url)
{
    redirect(url);
    quit();
}

void CgiRoot::Exit()
{
    quit();
}

CgiRoot::Impl::Impl(CgiRoot *cgiRoot) :
    m_cgiRoot(cgiRoot)
{

}

Wt::WWidget *CgiRoot::Impl::GetHomePage()
{
    m_cgiRoot->useStyleSheet("css/home.css");

    switch (m_cgiRoot->CgiEnvInstance->GetCurrentLanguage()) {
    case CgiEnv::Language::En:
        m_cgiRoot->useStyleSheet("css/home-ltr.css");
        m_cgiRoot->useStyleSheet("css/home-en.css");
        break;
    case CgiEnv::Language::Fa:
        m_cgiRoot->useStyleSheet("css/home-rtl.css");
        m_cgiRoot->useStyleSheet("css/home-fa.css");
        break;
    case CgiEnv::Language::None:
    case CgiEnv::Language::Invalid:
        break;
    }

    return new Home(m_cgiRoot);
}

Wt::WWidget *CgiRoot::Impl::GetRootLoginPage()
{
    m_cgiRoot->useStyleSheet("css/root.css");
    m_cgiRoot->useStyleSheet("resources/font-awesome/css/font-awesome.min.css");
    m_cgiRoot->require("js/jquery.min.js");
    m_cgiRoot->require("js/bootstrap.min.js");

    switch (m_cgiRoot->CgiEnvInstance->GetCurrentLanguage()) {
    case CgiEnv::Language::En:
        m_cgiRoot->useStyleSheet("css/root-ltr.css");
        m_cgiRoot->useStyleSheet("css/root-en.css");
        break;
    case CgiEnv::Language::Fa:
        m_cgiRoot->useStyleSheet("css/root-rtl.css");
        m_cgiRoot->useStyleSheet("css/root-fa.css");
        break;
    case CgiEnv::Language::None:
    case CgiEnv::Language::Invalid:
        break;
    }

    return new RootLogin(m_cgiRoot);
}

void CgiRoot::Impl::ReloadWithLanguage(const std::string &lang)
{
    std::string parameters(m_cgiRoot->CgiEnvInstance->GetInitialQueryString());

    if (parameters.size() > 0) {
        parameters += "&lang=";
    } else {
        parameters = "?lang=";
    }

    parameters = parameters + lang;

    if (!boost::starts_with(parameters, "?")) {
        parameters = "?" + parameters;
    }

    m_cgiRoot->Exit(parameters);
}

