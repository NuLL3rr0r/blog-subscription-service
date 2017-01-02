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
 * General settings such as website title, url, etc.
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
#include "CmsSettings.hpp"
#include "Div.hpp"
#include "Pool.hpp"

using namespace std;
using namespace boost;
using namespace cppdb;
using namespace Wt;
using namespace CoreLib;
using namespace Service;

struct CmsSettings::Impl : public Wt::WObject
{
public:
    WLineEdit *EnHomePageUrlLineEdit;
    WLineEdit *FaHomePageUrlLineEdit;
    WLineEdit *EnHomePageTitleLineEdit;
    WLineEdit *FaHomePageTitleLineEdit;
    WText *SettingsMessageArea;

private:
    CmsSettings *m_parent;

public:
    Impl(CmsSettings *parent);
    ~Impl();

public:
    void OnSaveSettingsFormSubmitted();
};

CmsSettings::CmsSettings()
    : Page(),
    m_pimpl(make_unique<CmsSettings::Impl>(this))
{
    this->clear();
    this->setId("CmsSettingsPage");
    this->addWidget(this->Layout());
}

CmsSettings::~CmsSettings() = default;

WWidget *CmsSettings::Layout()
{
    Div *container = new Div("CmsSettings", "container-fluid");

    try {
        CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
        CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

        string htmlData;
        string file;
        if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
            file = "../templates/cms-settings-fa.wtml";
        } else {
            file = "../templates/cms-settings.wtml";
        }

        if (CoreLib::FileSystem::Read(file, htmlData)) {
            /// Fill the template
            WTemplate *tmpl = new WTemplate(container);
            tmpl->setTemplateText(WString::fromUTF8(htmlData), TextFormat::XHTMLUnsafeText);

            m_pimpl->EnHomePageUrlLineEdit = new WLineEdit();
            m_pimpl->EnHomePageUrlLineEdit->setPlaceholderText(tr("cms-settings-home-page-url-en-placeholder"));
            WRegExpValidator *enHomePageUrlValidator = new WRegExpValidator(Pool::Storage()->RegexHttpUrl());
            enHomePageUrlValidator->setMandatory(true);
            m_pimpl->EnHomePageUrlLineEdit->setValidator(enHomePageUrlValidator);

            m_pimpl->FaHomePageUrlLineEdit = new WLineEdit();
            m_pimpl->FaHomePageUrlLineEdit->setPlaceholderText(tr("cms-settings-home-page-url-fa-placeholder"));
            WRegExpValidator *faHomePageUrlValidator = new WRegExpValidator(Pool::Storage()->RegexHttpUrl());
            faHomePageUrlValidator->setMandatory(true);
            m_pimpl->FaHomePageUrlLineEdit->setValidator(faHomePageUrlValidator);

            m_pimpl->EnHomePageTitleLineEdit = new WLineEdit();
            m_pimpl->EnHomePageTitleLineEdit->setPlaceholderText(tr("cms-settings-home-page-title-en-placeholder"));
            WLengthValidator *enHomePageTitleValidator = new WLengthValidator(Pool::Storage()->MinHomePageTitleLength(),
                                                                            Pool::Storage()->MaxHomePageTitleLength());
            enHomePageTitleValidator->setMandatory(true);
            m_pimpl->EnHomePageTitleLineEdit->setValidator(enHomePageTitleValidator);

            m_pimpl->FaHomePageTitleLineEdit = new WLineEdit();
            m_pimpl->FaHomePageTitleLineEdit->setPlaceholderText(tr("cms-settings-home-page-title-fa-placeholder"));
            WLengthValidator *faHomePageTitleValidator = new WLengthValidator(Pool::Storage()->MinHomePageTitleLength(),
                                                                            Pool::Storage()->MaxHomePageTitleLength());
            faHomePageTitleValidator->setMandatory(true);
            m_pimpl->FaHomePageTitleLineEdit->setValidator(faHomePageTitleValidator);

            WPushButton *saveSettingsPushButton = new WPushButton(tr("cms-settings-save-settings"));
            saveSettingsPushButton->setStyleClass("btn btn-default");

            m_pimpl->SettingsMessageArea = new WText();

            result r = Pool::Database()->Sql()
                    << (format("SELECT homepage_url_en, homepage_url_fa, homepage_title_en, homepage_title_fa"
                               " FROM \"%1%\" WHERE pseudo_id = '0';")
                        % Pool::Database()->GetTableName("SETTINGS")).str()
                    << row;

            if (!r.empty()) {
                string homepage_url_en;
                string homepage_url_fa;
                string homepage_title_en;
                string homepage_title_fa;

                r >> homepage_url_en >> homepage_url_fa >> homepage_title_en >> homepage_title_fa;

                m_pimpl->EnHomePageUrlLineEdit->setText(WString::fromUTF8(homepage_url_en));
                m_pimpl->FaHomePageUrlLineEdit->setText(WString::fromUTF8(homepage_url_fa));
                m_pimpl->EnHomePageTitleLineEdit->setText(WString::fromUTF8(homepage_title_en));
                m_pimpl->FaHomePageTitleLineEdit->setText(WString::fromUTF8(homepage_title_fa));
            }

            tmpl->bindString("home-page-url-en-input-id", m_pimpl->EnHomePageUrlLineEdit->id());
            tmpl->bindString("home-page-url-fa-input-id", m_pimpl->FaHomePageUrlLineEdit->id());
            tmpl->bindString("home-page-title-en-input-id", m_pimpl->EnHomePageTitleLineEdit->id());
            tmpl->bindString("home-page-title-da-input-id", m_pimpl->FaHomePageTitleLineEdit->id());

            tmpl->bindWidget("settings-title", new WText(tr("cms-settings-page-title")));
            tmpl->bindWidget("home-page-url-en-label-text", new WText(tr("cms-settings-home-page-url-en")));
            tmpl->bindWidget("home-page-url-fa-label-text", new WText(tr("cms-settings-home-page-url-fa")));
            tmpl->bindWidget("home-page-title-en-label-text", new WText(tr("cms-settings-home-page-title-en")));
            tmpl->bindWidget("home-page-title-fa-label-text", new WText(tr("cms-settings-home-page-title-fa")));

            tmpl->bindWidget("home-page-url-en-input", m_pimpl->EnHomePageUrlLineEdit);
            tmpl->bindWidget("home-page-url-fa-input", m_pimpl->FaHomePageUrlLineEdit);
            tmpl->bindWidget("home-page-title-en-input", m_pimpl->EnHomePageTitleLineEdit);
            tmpl->bindWidget("home-page-title-fa-input", m_pimpl->FaHomePageTitleLineEdit);

            tmpl->bindWidget("save-settings-button", saveSettingsPushButton);
            tmpl->bindWidget("settings-message-area", m_pimpl->SettingsMessageArea);

            m_pimpl->EnHomePageUrlLineEdit->enterPressed().connect(m_pimpl.get(), &CmsSettings::Impl::OnSaveSettingsFormSubmitted);
            m_pimpl->FaHomePageUrlLineEdit->enterPressed().connect(m_pimpl.get(), &CmsSettings::Impl::OnSaveSettingsFormSubmitted);
            m_pimpl->EnHomePageTitleLineEdit->enterPressed().connect(m_pimpl.get(), &CmsSettings::Impl::OnSaveSettingsFormSubmitted);
            m_pimpl->FaHomePageTitleLineEdit->enterPressed().connect(m_pimpl.get(), &CmsSettings::Impl::OnSaveSettingsFormSubmitted);
            saveSettingsPushButton->clicked().connect(m_pimpl.get(), &CmsSettings::Impl::OnSaveSettingsFormSubmitted);

            m_pimpl->EnHomePageUrlLineEdit->setFocus();
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

CmsSettings::Impl::Impl(CmsSettings *parent)
    : m_parent(parent)
{

}

CmsSettings::Impl::~Impl() = default;

void CmsSettings::Impl::OnSaveSettingsFormSubmitted()
{
    if (!m_parent->Validate(EnHomePageUrlLineEdit)
            || !m_parent->Validate(FaHomePageUrlLineEdit)
            || !m_parent->Validate(EnHomePageTitleLineEdit)
            || !m_parent->Validate(FaHomePageTitleLineEdit)) {
        return;
    }

    transaction guard(Service::Pool::Database()->Sql());

    try {
        string homepage_url_en(EnHomePageUrlLineEdit->text().toUTF8());
        string homepage_url_fa(FaHomePageUrlLineEdit->text().toUTF8());
        string homepage_title_en(EnHomePageTitleLineEdit->text().toUTF8());
        string homepage_title_fa(FaHomePageTitleLineEdit->text().toUTF8());

        Pool::Database()->Update("SETTINGS",
                                 "pseudo_id", "0",
                                 "homepage_url_en=?, homepage_url_fa=?, homepage_title_en=?, homepage_title_fa=?",
                                 {
                                     homepage_url_en,
                                     homepage_url_fa,
                                     homepage_title_en,
                                     homepage_title_fa
                                 });

        guard.commit();

        EnHomePageUrlLineEdit->setFocus();

        m_parent->HtmlInfo(tr("cms-settings-save-settings-success-message"), SettingsMessageArea);

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
