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
 * Manage website pages.
 */


#include <ctime>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <cppdb/frontend.h>
#include <Wt/WApplication>
#include <Wt/WPushButton>
#include <Wt/WString>
#include <Wt/WTable>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WWidget>
#include <CoreLib/CDate.hpp>
#include <CoreLib/Crypto.hpp>
#include <CoreLib/Database.hpp>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Log.hpp>
#include "CgiEnv.hpp"
#include "CgiRoot.hpp"
#include "CmsSubscribers.hpp"
#include "Div.hpp"
#include "Pool.hpp"

using namespace std;
using namespace boost;
using namespace cppdb;
using namespace Wt;
using namespace CoreLib;
using namespace Service;

struct CmsSubscribers::Impl : public Wt::WObject
{
public:
    enum class Table : unsigned char {
        All,
        EnFa,
        En,
        Fa,
        Inactive
    };

public:
    WContainerWidget *SubscribersTableContainer;

public:
    Impl();
    ~Impl();

public:
    void OnAllButtonPressed();
    void OnEnFaButtonPressed();
    void OnEnButtonPressed();
    void OnFaButtonPressed();
    void OnInactiveButtonPressed();

private:
    void GetDate(const std::string &timeSinceEpoch, Wt::WString &out_date);
    void GetSubscriptionTypeName(const std::string &type, Wt::WString &out_name);

    void FillDataTable(const CmsSubscribers::Impl::Table &table);
};

CmsSubscribers::CmsSubscribers()
    : Page(),
    m_pimpl(make_unique<CmsSubscribers::Impl>())
{
    this->clear();
    this->setId("CmsSubscribersPage");
    this->addWidget(this->Layout());
}

CmsSubscribers::~CmsSubscribers() = default;

WWidget *CmsSubscribers::Layout()
{
    Div *container = new Div("CmsSubscribers", "container-fluid");

    try {
        CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
        CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

        string htmlData;
        string file;
        if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
            file = "../templates/cms-subscribers-fa.wtml";
        } else {
            file = "../templates/cms-subscribers.wtml";
        }

        if (CoreLib::FileSystem::Read(file, htmlData)) {
            /// Fill the template
            WTemplate *tmpl = new WTemplate(container);
            tmpl->setTemplateText(WString::fromUTF8(htmlData), TextFormat::XHTMLUnsafeText);

            WPushButton *allSubscribersPushButton = new WPushButton(tr("cms-subscribers-all"));
            allSubscribersPushButton->setStyleClass("btn btn-default");

            WPushButton *englishFarsiSubscribersPushButton = new WPushButton(tr("cms-subscribers-english-farsi"));
            englishFarsiSubscribersPushButton->setStyleClass("btn btn-default");

            WPushButton *englishSubscribersPushButton = new WPushButton(tr("cms-subscribers-english"));
            englishSubscribersPushButton->setStyleClass("btn btn-default");

            WPushButton *farsiSubscribersPushButton = new WPushButton(tr("cms-subscribers-farsi"));
            farsiSubscribersPushButton->setStyleClass("btn btn-default");

            WPushButton *inactiveSubscribersPushButton = new WPushButton(tr("cms-subscribers-inactive"));
            inactiveSubscribersPushButton->setStyleClass("btn btn-default");

            m_pimpl->SubscribersTableContainer = new Div("SubscribersTableContainer", "subscribers-table-container");

            tmpl->bindWidget("subscribers-title", new WText(tr("cms-subscribers-page-title")));

            tmpl->bindWidget("subscribers-table", m_pimpl->SubscribersTableContainer);

            tmpl->bindWidget("all-subscribers-button", allSubscribersPushButton);
            tmpl->bindWidget("english-farsi-subscribers-button", englishFarsiSubscribersPushButton);
            tmpl->bindWidget("english-subscribers-button", englishSubscribersPushButton);
            tmpl->bindWidget("farsi-subscribers-button", farsiSubscribersPushButton);
            tmpl->bindWidget("inactive-subscribers-button", inactiveSubscribersPushButton);

            allSubscribersPushButton->clicked().connect(m_pimpl.get(), &CmsSubscribers::Impl::OnAllButtonPressed);
            englishFarsiSubscribersPushButton->clicked().connect(m_pimpl.get(), &CmsSubscribers::Impl::OnEnFaButtonPressed);
            englishSubscribersPushButton->clicked().connect(m_pimpl.get(), &CmsSubscribers::Impl::OnEnButtonPressed);
            farsiSubscribersPushButton->clicked().connect(m_pimpl.get(), &CmsSubscribers::Impl::OnFaButtonPressed);
            inactiveSubscribersPushButton->clicked().connect(m_pimpl.get(), &CmsSubscribers::Impl::OnInactiveButtonPressed);

            allSubscribersPushButton->setFocus();
        }
    }

    catch (const boost::exception &ex) {
        LOG_ERROR(boost::diagnostic_information(ex));
    }

    catch (const std::exception &ex) {
        LOG_ERROR(ex.what());
    }

    catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    return container;
}

CmsSubscribers::Impl::Impl()
{

}

CmsSubscribers::Impl::~Impl() = default;

void CmsSubscribers::Impl::OnAllButtonPressed()
{
    FillDataTable(Table::All);
}

void CmsSubscribers::Impl::OnEnFaButtonPressed()
{
    FillDataTable(Table::EnFa);
}

void CmsSubscribers::Impl::OnEnButtonPressed()
{
    FillDataTable(Table::En);
}

void CmsSubscribers::Impl::OnFaButtonPressed()
{
    FillDataTable(Table::Fa);
}

void CmsSubscribers::Impl::OnInactiveButtonPressed()
{
    FillDataTable(Table::Inactive);
}

void CmsSubscribers::Impl::GetDate(const std::string &timeSinceEpoch, Wt::WString &out_date)
{
    try {
        time_t tt = lexical_cast<time_t>(timeSinceEpoch);

        struct tm *utc_tm = gmtime(&tt);

        int year = utc_tm->tm_year + 1900;
        int month = utc_tm->tm_mon + 1;
        int day = utc_tm->tm_mday;

        CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
        CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

        if (cgiEnv->GetCurrentLanguage() != CgiEnv::Language::Fa) {
            out_date = (boost::wformat(L"%1%/%2%/%3%/")
                        % lexical_cast<wstring>(year)
                        % lexical_cast<wstring>(month)
                        % lexical_cast<wstring>(day)).str();
        } else {
            out_date = CDate::DateConv::FormatToPersianNums(CDate::DateConv::ToJalali(year, month, day));
        }
    } catch (...) {
        out_date = L"-";
    }
}

void CmsSubscribers::Impl::GetSubscriptionTypeName(const std::string &type, Wt::WString &out_name)
{
    static const std::string EN_FA_TYPE("en_fa");
    static const std::string EN_TYPE("en");
    static const std::string FA_TYPE("fa");
    static const std::string NONE_TYPE("none");

    static const std::string EN_FA_NAME(tr("cms-subscribers-subscription-en-fa").toUTF8());
    static const std::string EN_NAME(tr("cms-subscribers-subscription-en").toUTF8());
    static const std::string FA_NAME(tr("cms-subscribers-subscription-fa").toUTF8());
    static const std::string NONE_NAME(tr("cms-subscribers-subscription-none").toUTF8());

    if (type == EN_FA_TYPE) {
        out_name = WString::fromUTF8(EN_FA_NAME);
    } else if (type == EN_TYPE) {
        out_name = WString::fromUTF8(EN_NAME);
    } else if (type == FA_TYPE) {
        out_name = WString::fromUTF8(FA_NAME);
    } else if (type == NONE_TYPE) {
        out_name = WString::fromUTF8(NONE_NAME);
    }
}

void CmsSubscribers::Impl::FillDataTable(const CmsSubscribers::Impl::Table &tableType)
{
    try {
        SubscribersTableContainer->clear();

        WTable *table = new WTable(SubscribersTableContainer);
        table->setStyleClass("table table-striped table-hover");
        table->setHeaderCount(1, Orientation::Horizontal);

        table->elementAt(0, 0)->addWidget(new WText(tr("cms-subscribers-no")));
        table->elementAt(0, 1)->addWidget(new WText(tr("cms-subscribers-inbox")));
        table->elementAt(0, 2)->addWidget(new WText(tr("cms-subscribers-subscription")));
        table->elementAt(0, 3)->addWidget(new WText(tr("cms-subscribers-pending-confirm")));
        table->elementAt(0, 4)->addWidget(new WText(tr("cms-subscribers-pending-cancel")));
        table->elementAt(0, 5)->addWidget(new WText(tr("cms-subscribers-join-date")));
        table->elementAt(0, 6)->addWidget(new WText(tr("cms-subscribers-update-date")));
        table->elementAt(0, 7)->addWidget(new WText(tr("cms-subscribers-uuid")));

        result r;
        switch (tableType) {
        case Table::All:
            r = Pool::Database()->Sql()
                    << (format("SELECT inbox, uuid, subscription, pending_confirm, pending_cancel, join_date, update_date"
                               " FROM \"%1%\" ORDER BY inbox COLLATE \"en_US.UTF-8\" ASC;")
                        % Pool::Database()->GetTableName("SUBSCRIBERS")).str();
            break;
        case Table::EnFa:
            r = Pool::Database()->Sql()
                    << (format("SELECT inbox, uuid, subscription, pending_confirm, pending_cancel, join_date, update_date"
                               " FROM \"%1%\" WHERE subscription = 'en_fa' ORDER BY inbox COLLATE \"en_US.UTF-8\" ASC;")
                        % Pool::Database()->GetTableName("SUBSCRIBERS")).str();
            break;
        case Table::En:
            r = Pool::Database()->Sql()
                    << (format("SELECT inbox, uuid, subscription, pending_confirm, pending_cancel, join_date, update_date"
                               " FROM \"%1%\" WHERE subscription = 'en' ORDER BY inbox COLLATE \"en_US.UTF-8\" ASC;")
                        % Pool::Database()->GetTableName("SUBSCRIBERS")).str();
            break;
        case Table::Fa:
            r = Pool::Database()->Sql()
                    << (format("SELECT inbox, uuid, subscription, pending_confirm, pending_cancel, join_date, update_date"
                               " FROM \"%1%\" WHERE subscription = 'fa' ORDER BY inbox COLLATE \"en_US.UTF-8\" ASC;")
                        % Pool::Database()->GetTableName("SUBSCRIBERS")).str();
            break;
        case Table::Inactive:
            r = Pool::Database()->Sql()
                    << (format("SELECT inbox, uuid, subscription, pending_confirm, pending_cancel, join_date, update_date"
                               " FROM \"%1%\" WHERE subscription = 'none' ORDER BY inbox COLLATE \"en_US.UTF-8\" ASC;")
                        % Pool::Database()->GetTableName("SUBSCRIBERS")).str();
            break;
        }

        int i = 0;
        while(r.next()) {
            ++i;
            string inbox;
            string uuid;
            string subscription;
            string pending_confirm;
            string pending_cancel;
            string join_date;
            string update_date;

            r >> inbox >> uuid >> subscription >> pending_confirm >> pending_cancel >> join_date >> update_date;

            WString subscriptionTypeName;
            WString pendingConfirmTypeName;
            WString pendingCancelTypeName;

            this->GetSubscriptionTypeName(subscription, subscriptionTypeName);
            this->GetSubscriptionTypeName(pending_confirm, pendingConfirmTypeName);
            this->GetSubscriptionTypeName(pending_cancel, pendingCancelTypeName);

            WString joinDate;
            WString updateDate;

            this->GetDate(join_date, joinDate);
            this->GetDate(join_date, updateDate);

            table->elementAt(i, 0)->addWidget(new WText(WString::fromUTF8(lexical_cast<string>(i))));
            table->elementAt(i, 1)->addWidget(new WText(WString::fromUTF8(inbox)));
            table->elementAt(i, 2)->addWidget(new WText(subscriptionTypeName));
            table->elementAt(i, 3)->addWidget(new WText(pendingConfirmTypeName));
            table->elementAt(i, 4)->addWidget(new WText(pendingCancelTypeName));
            table->elementAt(i, 5)->addWidget(new WText(joinDate));
            table->elementAt(i, 6)->addWidget(new WText(updateDate));
            table->elementAt(i, 7)->addWidget(new WText(WString::fromUTF8(uuid)));
        }
    }

    catch (const boost::exception &ex) {
        LOG_ERROR(boost::diagnostic_information(ex));
    }

    catch (const std::exception &ex) {
        LOG_ERROR(ex.what());
    }

    catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }
}
