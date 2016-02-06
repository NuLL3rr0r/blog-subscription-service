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


#include <boost/exception/diagnostic_information.hpp>
#include <boost/format.hpp>
#include <cppdb/frontend.h>
#include <Wt/WComboBox>
#include <Wt/WLengthValidator>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
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
#include "CgiEnv.hpp"
#include "CmsNewsletter.hpp"
#include "Div.hpp"
#include "Pool.hpp"

#define         UNKNOWN_ERROR       "Unknown error!"

using namespace std;
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
    WText *NewsletterMessageArea;

public:
    Impl();
    ~Impl();

public:
    void OnRecipientsComboBoxSelectionChanged(Wt::WString recipients);
    void OnSendButtonPressed();
    void OnSendConfirmDialogClosed(Wt::StandardButton button);
    void OnClearButtonPressed();
    void OnClearConfirmDialogClosed(Wt::StandardButton button);
};

CmsNewsletter::CmsNewsletter()
    : Page(),
    m_pimpl(make_unique<CmsNewsletter::Impl>())
{
    this->clear();
    this->setId("CmsNewsletterPage");
    this->addWidget(Layout());
}

CmsNewsletter::~CmsNewsletter() = default;

WWidget *CmsNewsletter::Layout()
{
    Div *container = new Div("CmsNewsletter", "container-fluid");

    try {
        CgiEnv *cgiEnv = CgiEnv::GetInstance();

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
            tmpl->setTemplateText(WString(htmlData), TextFormat::XHTMLUnsafeText);

            m_pimpl->RecipientsComboBox = new WComboBox();
            m_pimpl->RecipientsComboBox->setPlaceholderText(tr("cms-newsletter-recipients-placeholder"));

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
            /// Excludes: compat3x,bbcode
            /// Note: 'legacyoutput' plugin should be enabled due to the lack of HTML5 support in some mail clients
            m_pimpl->BodyTextEdit->setExtraPlugins("advlist,anchor,autolink,autoresize,autosave"
                                                   ",charmap,code,codesample,colorpicker,contextmenu"
                                                   ",directionality,emoticons,fullpage,fullscreen,hr"
                                                   ",image,imagetools,importcss,insertdatetime,layer"
                                                   ",legacyoutput,link,lists,media,nonbreaking,noneditable"
                                                   ",pagebreak,paste,preview,print,save,searchreplace,spellchecker"
                                                   ",tabfocus,table,template,textcolor,textpattern"
                                                   ",visualblocks,visualchars,wordcount");

            /// http://www.tinymce.com/wiki.php/Configuration:toolbar%3CN%3E
            m_pimpl->BodyTextEdit->setToolBar(0, "newdocument save restoredraft fullpage print"
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

            m_pimpl->NewsletterMessageArea = new WText();

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

            tmpl->bindWidget("newsletter-message-area", m_pimpl->NewsletterMessageArea);



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

CmsNewsletter::Impl::Impl()
{

}

CmsNewsletter::Impl::~Impl() = default;

void CmsNewsletter::Impl::OnRecipientsComboBoxSelectionChanged(Wt::WString recipients)
{
    (void)recipients;
}

void CmsNewsletter::Impl::OnSendButtonPressed()
{
}

void CmsNewsletter::Impl::OnSendConfirmDialogClosed(Wt::StandardButton button)
{
    (void)button;
}

void CmsNewsletter::Impl::OnClearButtonPressed()
{

}

void CmsNewsletter::Impl::OnClearConfirmDialogClosed(Wt::StandardButton button)
{
    (void)button;
}

