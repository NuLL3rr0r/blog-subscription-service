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
 * Manage website contacts.
 */


#include <boost/exception/diagnostic_information.hpp>
#include <boost/format.hpp>
#include <cppdb/frontend.h>
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
#include "CmsContacts.hpp"
#include "Div.hpp"
#include "Pool.hpp"

#define         UNKNOWN_ERROR       "Unknown error!"

using namespace std;
using namespace Wt;
using namespace Service;

struct CmsContacts::Impl : public Wt::WObject
{
public:
    CmsContacts *m_parent;

    WLineEdit *RecipientEnLineEdit;
    WLineEdit *RecipientFaLineEdit;
    WLineEdit *EmailLineEdit;
    WText *EditContactsMessageArea;
    WContainerWidget *ContactsTableContainer;

public:
    explicit Impl(CmsContacts *parent);

public:
};

CmsContacts::CmsContacts(CgiRoot *cgi) :
    Page(cgi),
    m_pimpl(std::make_unique<CmsContacts::Impl>(this))
{
    this->clear();
    this->setId("CmsContactsPage");
    this->addWidget(Layout());
    m_htmlRoot->addWidget(this);
}

WWidget *CmsContacts::Layout()
{
    Div *container = new Div("CmsContacts", "container");

    try {
        std::string htmlData;
        std::string file;
        if (m_cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
            file = "../templates/cms-contacts-fa.wtml";
        } else {
            file = "../templates/cms-contacts.wtml";
        }

        if (CoreLib::FileSystem::Read(file, htmlData)) {
            WTemplate *tmpl = new WTemplate(container);
            tmpl->setTemplateText(WString(htmlData), TextFormat::XHTMLUnsafeText);

            m_pimpl->RecipientEnLineEdit = new WLineEdit();
            m_pimpl->RecipientEnLineEdit->setPlaceholderText(tr("cms-contacts-recipient-name-en-placeholder"));
            WLengthValidator *recipientEnValidator = new WLengthValidator(Pool::Storage()->MinEmailRecipientNameLength(),
                                                                         Pool::Storage()->MaxEmailRecipientNameLength());
            recipientEnValidator->setMandatory(true);
            m_pimpl->RecipientEnLineEdit->setValidator(recipientEnValidator);

            m_pimpl->RecipientFaLineEdit = new WLineEdit();
            m_pimpl->RecipientFaLineEdit->setPlaceholderText(tr("cms-contacts-recipient-name-fa-placeholder"));
            WLengthValidator *recipientFaValidator = new WLengthValidator(Pool::Storage()->MinEmailRecipientNameLength(),
                                                                         Pool::Storage()->MaxEmailRecipientNameLength());
            recipientFaValidator->setMandatory(true);
            m_pimpl->RecipientFaLineEdit->setValidator(recipientFaValidator);

            m_pimpl->EmailLineEdit = new WLineEdit();
            m_pimpl->EmailLineEdit->setPlaceholderText(tr("cms-contacts-email-address-placeholder"));
            WRegExpValidator *emailValidator = new WRegExpValidator(Pool::Storage()->RegexEmail());
            emailValidator->setMandatory(true);
            m_pimpl->EmailLineEdit->setValidator(emailValidator);

            WPushButton *addPushButton = new WPushButton(tr("cms-contacts-add-save"));
            addPushButton->setStyleClass("btn btn-default");

            WPushButton *cancelPushButton = new WPushButton(tr("cms-contacts-add-cancel"));
            cancelPushButton->setStyleClass("btn btn-default");

            m_pimpl->EditContactsMessageArea = new WText();
            HtmlInfo(tr("cms-contacts-edit-hint").value(), m_pimpl->EditContactsMessageArea);

            m_pimpl->ContactsTableContainer = new Div("ContactsTableContainer", "contacts-table-container");

            tmpl->bindString("recipient-en-input-id", m_pimpl->RecipientEnLineEdit->id());
            tmpl->bindString("recipient-fa-input-id", m_pimpl->RecipientFaLineEdit->id());
            tmpl->bindString("email-input-id", m_pimpl->EmailLineEdit->id());

            tmpl->bindWidget("edit-contacts-title", new WText(tr("cms-contacts-page-title")));

            tmpl->bindWidget("recipient-en-label-text", new WText(tr("cms-contacts-recipient-name-en")));
            tmpl->bindWidget("recipient-fa-label-text", new WText(tr("cms-contacts-recipient-name-fa")));
            tmpl->bindWidget("email-label-text", new WText(tr("cms-contacts-email-address")));

            tmpl->bindWidget("recipient-en-input", m_pimpl->RecipientEnLineEdit);
            tmpl->bindWidget("recipient-fa-input", m_pimpl->RecipientFaLineEdit);
            tmpl->bindWidget("email-input", m_pimpl->EmailLineEdit);

            tmpl->bindWidget("add-button", addPushButton);
            tmpl->bindWidget("cancel-button", cancelPushButton);

            tmpl->bindWidget("edit-contacts-message-area", m_pimpl->EditContactsMessageArea);

            tmpl->bindWidget("contacts-table", m_pimpl->ContactsTableContainer);



            m_pimpl->RecipientEnLineEdit->setFocus();
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

CmsContacts::Impl::Impl(CmsContacts *parent)
    : m_parent(parent)
{

}

