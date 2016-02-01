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
    CmsNewsletter *m_parent;

    WComboBox *RecipientsComboBox;
    WLineEdit *SubjectLineEdit;
    WTextEdit *BodyTextEdit;
    WPushButton *SendPushButton;
    WPushButton *ClearPushButton;
    WText *NewsletterMessageArea;

public:
    explicit Impl(CmsNewsletter *parent);

public:
};

CmsNewsletter::CmsNewsletter(CgiRoot *cgi) :
    Page(cgi),
    m_pimpl(std::make_unique<CmsNewsletter::Impl>(this))
{
    this->clear();
    this->setId("CmsNewsletterPage");
    this->addWidget(Layout());
    m_htmlRoot->addWidget(this);
}

WWidget *CmsNewsletter::Layout()
{
    Div *container = new Div("CmsNewsletter", "container-fluid");

    try {
        std::string htmlData;
        std::string file;
        if (m_cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
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

            m_pimpl->BodyTextEdit = new WTextEdit();
            m_pimpl->BodyTextEdit->setPlaceholderText(tr("cms-newsletter-body-placeholder"));
            WLengthValidator *bodyValidator = new WLengthValidator(Pool::Storage()->MinEmailSubjectLength(),
                                                                   Pool::Storage()->MaxEmailSubjectLength_RFC());
            bodyValidator->setMandatory(true);
            //m_pimpl->BodyTextEdit->
            m_pimpl->BodyTextEdit->setValidator(bodyValidator);

            m_pimpl->SendPushButton = new WPushButton("cms-newsletter-send");
            m_pimpl->SendPushButton->setStyleClass("btn btn-default");

            m_pimpl->ClearPushButton = new WPushButton("cms-newsletter-clear");
            m_pimpl->ClearPushButton->setStyleClass("btn btn-default");

            m_pimpl->NewsletterMessageArea = new WText();

            tmpl->bindWidget("newsletter-title", new WText(tr("newsletter-page-title")));

            tmpl->bindString("recipients-input-id", m_pimpl->RecipientsComboBox->id());
            tmpl->bindString("subject-input-id", m_pimpl->SubjectLineEdit->id());
            tmpl->bindString("body-textarea-id", m_pimpl->BodyTextEdit->id());

            tmpl->bindWidget("recipients-label-text", new WText(tr("cms-newsletter-recipients")));
            tmpl->bindWidget("subject-label-text", new WText(tr("cms-newsletter-subject")));
            tmpl->bindWidget("body-label-text", new WText(tr("cms-newsletter-body")));

            tmpl->bindWidget("recipients-input", m_pimpl->RecipientsComboBox);
            tmpl->bindWidget("subject-input", m_pimpl->SubjectLineEdit);
            tmpl->bindWidget("body-input", m_pimpl->BodyTextEdit);

            tmpl->bindWidget("add-button", m_pimpl->SendPushButton);
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

CmsNewsletter::Impl::Impl(CmsNewsletter *parent)
    : m_parent(parent)
{

}

