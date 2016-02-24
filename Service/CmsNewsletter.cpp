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
 * Manage news section of the website.
 */


#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/format.hpp>
#include <cppdb/frontend.h>
#include <Wt/WApplication>
#include <Wt/WComboBox>
#include <Wt/WLengthValidator>
#include <Wt/WLineEdit>
#include <Wt/WMessageBox>
#include <Wt/WPushButton>
#include <Wt/WSignalMapper>
#include <Wt/WString>
#include <Wt/WTable>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WTextEdit>
#include <Wt/WWidget>
#include <CoreLib/Crypto.hpp>
#include <CoreLib/Database.hpp>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Log.hpp>
#include <CoreLib/Mail.hpp>
#include "CgiEnv.hpp"
#include "CgiRoot.hpp"
#include "CmsNewsletter.hpp"
#include "Div.hpp"
#include "Pool.hpp"

using namespace std;
using namespace boost;
using namespace cppdb;
using namespace Wt;
using namespace Service;

struct CmsNewsletter::Impl : public Wt::WObject
{
public:
    WComboBox *RecipientsComboBox;
    WLineEdit *SubjectLineEdit;
    WTextEdit *BodyTextEdit;
    WPushButton *SendPushButton;
    WPushButton *ClearPushButton;

    std::unique_ptr<Wt::WMessageBox> SendMessageBox;
    std::unique_ptr<Wt::WMessageBox> ClearMessageBox;
    std::unique_ptr<Wt::WMessageBox> SuccessMessageBox;

private:
    CmsNewsletter *m_parent;

public:
    explicit Impl(CmsNewsletter *parent);
    ~Impl();

public:
    void OnRecipientsComboBoxSelectionChanged(Wt::WString recipients);
    void OnSendButtonPressed();
    void OnSendConfirmDialogClosed(Wt::StandardButton button);
    void OnClearButtonPressed();
    void OnClearConfirmDialogClosed(Wt::StandardButton button);
    void OnSendSuccessDialogClosed(Wt::StandardButton button);

public:
    void SetFormEnable(bool status);
    void ResetTheForm();
};

CmsNewsletter::CmsNewsletter()
    : Page(),
    m_pimpl(make_unique<CmsNewsletter::Impl>(this))
{
    this->clear();
    this->setId("CmsNewsletterPage");
    this->addWidget(this->Layout());
}

CmsNewsletter::~CmsNewsletter() = default;

WWidget *CmsNewsletter::Layout()
{
    Div *container = new Div("CmsNewsletter", "container-fluid");

    try {
        CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
        CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

        string htmlData;
        string file;
        if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
            file = "../templates/cms-newsletter-fa.wtml";
        } else {
            file = "../templates/cms-newsletter.wtml";
        }

        if (CoreLib::FileSystem::Read(file, htmlData)) {
            /// Fill the template
            WTemplate *tmpl = new WTemplate(container);
            tmpl->setTemplateText(WString::fromUTF8(htmlData), TextFormat::XHTMLUnsafeText);

            m_pimpl->RecipientsComboBox = new WComboBox();
            m_pimpl->RecipientsComboBox->setPlaceholderText(tr("cms-newsletter-recipients-placeholder"));
            m_pimpl->RecipientsComboBox->addItem(tr("cms-newsletter-choose-recipients"));
            m_pimpl->RecipientsComboBox->addItem(tr("cms-newsletter-all-recipients"));
            m_pimpl->RecipientsComboBox->addItem(tr("cms-newsletter-english-recipients"));
            m_pimpl->RecipientsComboBox->addItem(tr("cms-newsletter-farsi-recipients"));

            m_pimpl->SubjectLineEdit = new WLineEdit();
            m_pimpl->SubjectLineEdit->setPlaceholderText(tr("cms-newsletter-subject-placeholder"));
            WLengthValidator *subjectValidator = new WLengthValidator(Pool::Storage()->MinEmailSubjectLength(),
                                                                      Pool::Storage()->MaxEmailSubjectLength_RFC());
            subjectValidator->setMandatory(true);
            m_pimpl->SubjectLineEdit->setValidator(subjectValidator);


            /// Do not call
            /// 'setPlaceholderText' on 'm_pimpl->BodyTextEdit'
            /// or, it will throw.
            m_pimpl->BodyTextEdit = new WTextEdit();

            /// http://tinymce.moxiecode.com/wiki.php/Configuration
            m_pimpl->BodyTextEdit->setConfigurationSetting("valid_elements", std::string("*[*]"));

            switch (cgiEnv->GetCurrentLanguage()) {
            case CgiEnv::Language::Fa:
                m_pimpl->BodyTextEdit->setConfigurationSetting("language", string("fa_IR"));
                break;
            case CgiEnv::Language::En:
            case CgiEnv::Language::Invalid:
            case CgiEnv::Language::None:
                break;
            }

            /// http://www.tinymce.com/wiki.php/Plugins
            /// Excludes: compat3x,bbcode,fullpage for good reasons
            /// Note: 'legacyoutput' plugin should be enabled due to the lack of HTML5 support in some mail clients
            m_pimpl->BodyTextEdit->setExtraPlugins("advlist,anchor,autolink,autoresize,autosave"
                                                   ",charmap,code,codesample,colorpicker,contextmenu"
                                                   ",directionality,emoticons,fullscreen,hr"
                                                   ",image,imagetools,importcss,insertdatetime,layer"
                                                   ",legacyoutput,link,lists,media,nonbreaking,noneditable"
                                                   ",pagebreak,paste,preview,print,save,searchreplace,spellchecker"
                                                   ",tabfocus,table,template,textcolor,textpattern"
                                                   ",visualblocks,visualchars,wordcount");

            /// http://www.tinymce.com/wiki.php/Configuration:toolbar%3CN%3E
            m_pimpl->BodyTextEdit->setToolBar(0, "newdocument save restoredraft print"
                                              " | undo redo | cut copy paste pastetext | searchreplace"
                                              " | visualchars visualblocks | fullscreen preview | code"
                                              " | spellchecker | help");
            m_pimpl->BodyTextEdit->setToolBar(1, "link unlink anchor table image media codesample"
                                              " | emoticons charmap nonbreaking hr pagebreak"
                                              " | insertdatetime | template");
            m_pimpl->BodyTextEdit->setToolBar(2, "formatselect fontselect fontsizeselect | forecolor backcolor"
                                              " | styleselect removeformat");
            m_pimpl->BodyTextEdit->setToolBar(3, " bold italic underline strikethrough | superscript subscript"
                                              " | alignleft aligncenter alignright alignjustify"
                                              " | outdent indent bullist numlist blockquote | ltr rtl");

            m_pimpl->BodyTextEdit->setConfigurationSetting(
                        "contextmenu",
                        std::string("link anchor image media"
                                    " | inserttable cell row column deletetable | code"));

            m_pimpl->SendPushButton = new WPushButton(tr("cms-newsletter-send"));
            m_pimpl->SendPushButton->setStyleClass("btn btn-default");

            m_pimpl->ClearPushButton = new WPushButton(tr("cms-newsletter-clear"));
            m_pimpl->ClearPushButton->setStyleClass("btn btn-default");

            m_pimpl->SetFormEnable(false);
            m_pimpl->ResetTheForm();

            tmpl->bindWidget("newsletter-title", new WText(tr("cms-newsletter-page-title")));

            tmpl->bindString("recipients-select-id", m_pimpl->RecipientsComboBox->id());
            tmpl->bindString("subject-input-id", m_pimpl->SubjectLineEdit->id());
            tmpl->bindString("body-textarea-id", m_pimpl->BodyTextEdit->id());

            tmpl->bindWidget("recipients-label-text", new WText(tr("cms-newsletter-recipients")));
            tmpl->bindWidget("subject-label-text", new WText(tr("cms-newsletter-subject")));
            tmpl->bindWidget("body-label-text", new WText(tr("cms-newsletter-body")));

            tmpl->bindWidget("recipients-select", m_pimpl->RecipientsComboBox);
            tmpl->bindWidget("subject-input", m_pimpl->SubjectLineEdit);
            tmpl->bindWidget("body-textarea", m_pimpl->BodyTextEdit);

            tmpl->bindWidget("send-button", m_pimpl->SendPushButton);
            tmpl->bindWidget("clear-button", m_pimpl->ClearPushButton);

            m_pimpl->RecipientsComboBox->sactivated().connect(
                        m_pimpl.get(), &CmsNewsletter::Impl::OnRecipientsComboBoxSelectionChanged);
            m_pimpl->SendPushButton->clicked().connect(
                        m_pimpl.get(), &CmsNewsletter::Impl::OnSendButtonPressed);
            m_pimpl->ClearPushButton->clicked().connect(
                        m_pimpl.get(), &CmsNewsletter::Impl::OnClearButtonPressed);

            m_pimpl->RecipientsComboBox->setFocus();
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

CmsNewsletter::Impl::Impl(CmsNewsletter *parent)
    : m_parent(parent)
{

}

CmsNewsletter::Impl::~Impl() = default;

void CmsNewsletter::Impl::OnRecipientsComboBoxSelectionChanged(Wt::WString recipients)
{
    if (recipients == tr("cms-newsletter-all-recipients")
            || recipients == tr("cms-newsletter-english-recipients")
            || recipients == tr("cms-newsletter-farsi-recipients")) {
        this->SetFormEnable(true);
    } else {
        this->SetFormEnable(false);
    }
}

void CmsNewsletter::Impl::OnSendButtonPressed()
{
    SendMessageBox =
            std::make_unique<WMessageBox>(tr("cms-newsletter-send-confirm-title"),
                                          tr("cms-newsletter-send-confirm-message"),
                                          Warning, NoButton);
    SendMessageBox->addButton(tr("cms-newsletter-send-confirm-ok"), Ok);
    SendMessageBox->addButton(tr("cms-newsletter-send-confirm-cancel"), Cancel);

    SendMessageBox->buttonClicked().connect(
                this, &CmsNewsletter::Impl::OnSendConfirmDialogClosed);

    SendMessageBox->show();
}

void CmsNewsletter::Impl::OnSendConfirmDialogClosed(Wt::StandardButton button)
{
    if (button == Ok) {
        string recipients(RecipientsComboBox->currentText().toUTF8());

        if (recipients != tr("cms-newsletter-all-recipients")
                && recipients != tr("cms-newsletter-english-recipients")
                && recipients != tr("cms-newsletter-farsi-recipients")) {
            RecipientsComboBox->setFocus();
            return;
        }

        if (!m_parent->Validate(SubjectLineEdit)) {
            return;
        }

        if ((BodyTextEdit->text().toUTF8() == "")) {
            BodyTextEdit->setFocus();
            return;
        }

        string bodyHtmlText(BodyTextEdit->text().toUTF8());

        if (bodyHtmlText == "") {
            BodyTextEdit->setFocus();
            return;
        }

        try {
            CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
            CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

            string htmlData;
            string file;
            if (recipients == tr("cms-newsletter-all-recipients")) {
                file = "../templates/email-newsletter-template.wtml";
            } else if (recipients == tr("cms-newsletter-english-recipients")) {
                file = "../templates/email-newsletter-template.wtml";
            } else if (recipients == tr("cms-newsletter-farsi-recipients")) {
                file = "../templates/email-newsletter-template-fa.wtml";
            } else {
                return;
            }

            if (CoreLib::FileSystem::Read(file, htmlData)) {
                string subject(SubjectLineEdit->text().toUTF8());

                replace_all(htmlData, "${newsletter}", bodyHtmlText);

                string homePageFields;
                if (recipients == tr("cms-newsletter-all-recipients")) {
                    homePageFields = "homepage_url_en, homepage_title_en";
                } else if (recipients == tr("cms-newsletter-english-recipients")) {
                    homePageFields = "homepage_url_en, homepage_title_en";
                } else if (recipients == tr("cms-newsletter-farsi-recipients")) {
                    homePageFields = "homepage_url_fa, homepage_title_fa";
                } else {
                    return;
                }

                result r = Pool::Database()->Sql()
                        << (format("SELECT %1%"
                                   " FROM \"%2%\" WHERE pseudo_id = '0';")
                            % homePageFields
                            % Pool::Database()->GetTableName("SETTINGS")).str()
                        << row;

                string homePageUrl;
                string homePageTitle;
                if (!r.empty()) {
                    r >> homePageUrl >> homePageTitle;
                }

                replace_all(htmlData, "${home-page-url}", homePageUrl);
                replace_all(htmlData, "${home-page-title}", homePageTitle);

                string unsubscribeLink(cgiEnv->GetServerInfo(CgiEnv::ServerInfo::URL));
                if (!ends_with(unsubscribeLink, "/"))
                    unsubscribeLink += "/";

                if (recipients == tr("cms-newsletter-all-recipients")) {
                    unsubscribeLink += "?subscribe=-1&recipient=${uuid},&subscription=en,fa";

                    r = Pool::Database()->Sql()
                            << (format("SELECT inbox, uuid FROM \"%1%\""
                                       " WHERE subscription <> 'none';")
                                % Pool::Database()->GetTableName("SUBSCRIBERS")).str();
                } else if (recipients == tr("cms-newsletter-english-recipients")) {
                    unsubscribeLink += "?lang=${lang}&subscribe=-1&recipient=${uuid}&subscription=en";

                    r = Pool::Database()->Sql()
                            << (format("SELECT inbox, uuid FROM \"%1%\""
                                       " WHERE subscription = 'en_fa' OR subscription = 'en';")
                                % Pool::Database()->GetTableName("SUBSCRIBERS")).str();
                } else if (recipients == tr("cms-newsletter-farsi-recipients")) {
                    unsubscribeLink += "?lang=${lang}&subscribe=-1&recipient=${uuid}&subscription=fa";

                    r = Pool::Database()->Sql()
                            << (format("SELECT inbox, uuid FROM \"%1%\""
                                       " WHERE subscription = 'en_fa' OR subscription = 'fa';")
                                % Pool::Database()->GetTableName("SUBSCRIBERS")).str();
                } else {
                    return;
                }

                string enUnsubscribeLink(replace_all_copy(unsubscribeLink, "${lang}", "en"));
                string faUnsubscribeLink(replace_all_copy(unsubscribeLink, "${lang}", "fa"));

                string message;
                string inbox;
                string uuid;

                while(r.next()) {
                    r >> inbox >> uuid;

                    message.assign(htmlData);
                    replace_all(message, "unsubscribe-link-en", replace_all_copy(enUnsubscribeLink, "${uuid}", uuid));
                    replace_all(message, "unsubscribe-link-fa", replace_all_copy(faUnsubscribeLink, "${uuid}", uuid));

                    CoreLib::Mail *mail = new CoreLib::Mail(
                                cgiEnv->GetServerInfo(CgiEnv::ServerInfo::NoReplyAddr), inbox,
                                subject, message);
                    mail->SetDeleteLater(true);
                    mail->SendAsync();
                }

                message.assign(htmlData);
                replace_all(message, "unsubscribe-link-en", "javascript:;");
                replace_all(message, "unsubscribe-link-fa", "javascript:;");

                CoreLib::Mail *mail = new CoreLib::Mail(
                            cgiEnv->GetServerInfo(CgiEnv::ServerInfo::NoReplyAddr),
                            cgiEnv->SignedInUser.Email,
                            subject, message);
                mail->SetDeleteLater(true);
                mail->SendAsync();

                SuccessMessageBox =
                        std::make_unique<WMessageBox>(tr("cms-newsletter-sent-successfully-title"),
                                                      tr("cms-newsletter-sent-successfully-message"),
                                                      Warning, NoButton);
                SuccessMessageBox->addButton(tr("cms-newsletter-sent-successfully-ok"), Ok);

                SuccessMessageBox->buttonClicked().connect(
                            this, &CmsNewsletter::Impl::OnSendSuccessDialogClosed);

                SuccessMessageBox->show();

                this->ResetTheForm();
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

    SendMessageBox.reset();
}

void CmsNewsletter::Impl::OnClearButtonPressed()
{
    ClearMessageBox =
            std::make_unique<WMessageBox>(tr("cms-newsletter-clear-confirm-title"),
                                          tr("cms-newsletter-clear-confirm-message"),
                                          Warning, NoButton);
    ClearMessageBox->addButton(tr("cms-newsletter-clear-confirm-ok"), Ok);
    ClearMessageBox->addButton(tr("cms-newsletter-clear-confirm-cancel"), Cancel);

    ClearMessageBox->buttonClicked().connect(
                this, &CmsNewsletter::Impl::OnClearConfirmDialogClosed);

    ClearMessageBox->show();
}

void CmsNewsletter::Impl::OnClearConfirmDialogClosed(Wt::StandardButton button)
{
    if (button == Ok) {
        this->ResetTheForm();
    }

    ClearMessageBox.reset();
}

void CmsNewsletter::Impl::OnSendSuccessDialogClosed(Wt::StandardButton button)
{
    (void)button;

    SuccessMessageBox.reset();
}

void CmsNewsletter::Impl::ResetTheForm()
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    RecipientsComboBox->setCurrentIndex(0);
    SubjectLineEdit->setText("");

    if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
        BodyTextEdit->setText("<div style=\"direction: rtl;\"></div>");
    } else {
        BodyTextEdit->setText("<div style=\"direction: ltr;\"></div>");
    }

    this->SetFormEnable(false);
}

void CmsNewsletter::Impl::SetFormEnable(bool status)
{
    if (status) {
        SendPushButton->setEnabled(true);
    } else {
        SendPushButton->setDisabled(true);
    }
}

