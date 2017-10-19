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


#include <vector>
#include <cmath>
#include <ctime>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <pqxx/pqxx>
#include <Wt/WApplication>
#include <Wt/WComboBox>
#include <Wt/WPushButton>
#include <Wt/WSignalMapper>
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
#include <CoreLib/make_unique.hpp>
#include "CgiEnv.hpp"
#include "CgiRoot.hpp"
#include "CmsSubscribers.hpp"
#include "Div.hpp"
#include "Pool.hpp"

using namespace std;
using namespace boost;
using namespace pqxx;
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

    std::vector<std::pair<Wt::WString, int>> PaginationOptions;
    int PaginationOptionsDefaultIndex;
    Table PaginationTableType;
    uint_fast64_t PaginationTotalItems;
    int PaginationItemsPerPageLimit;
    uint_fast64_t PaginationItemOffset;
    uint_fast64_t PaginationPageOffset;
    Div *PaginationButtonsContainer;

public:
    Impl();
    ~Impl();

public:
    void OnAllButtonPressed();
    void OnEnFaButtonPressed();
    void OnEnButtonPressed();
    void OnFaButtonPressed();
    void OnInactiveButtonPressed();

    void OnItemsPerPageComboBoxChanged(Wt::WComboBox *comboBox);
    void OnPaginiationButtonPressed(Wt::WPushButton *button);

private:
    void GetDate(const std::string &timeSinceEpoch, Wt::WString &out_date);
    void GetSubscriptionTypeName(const std::string &type, Wt::WString &out_name);

    void FillDataTable(const CmsSubscribers::Impl::Table &table);

    void ReEvaluatePaginationButtons();
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
        if (cgiEnv->GetInformation().Client.Language.Code
                == CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa) {
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

            WComboBox* itemsPerPageComboBox = new WComboBox();
            itemsPerPageComboBox->setWidth(WLength(200.0, WLength::Pixel));

            for (const pair<WString, int> &it : m_pimpl->PaginationOptions) {
                itemsPerPageComboBox->addItem(it.first);
            }

            itemsPerPageComboBox->setCurrentIndex(m_pimpl->PaginationOptionsDefaultIndex);
            m_pimpl->PaginationItemsPerPageLimit = boost::lexical_cast<int>(itemsPerPageComboBox->currentText());

            WSignalMapper<WComboBox *> *itemsPerPageSignalMapper = new WSignalMapper<WComboBox *>(this);
            itemsPerPageSignalMapper->mapped().connect(m_pimpl.get(), &CmsSubscribers::Impl::OnItemsPerPageComboBoxChanged);
            itemsPerPageSignalMapper->mapConnect(itemsPerPageComboBox->changed(), itemsPerPageComboBox);

            m_pimpl->PaginationButtonsContainer = new Div("PaginationButtonsContainer", "pagination-buttons-container");

            m_pimpl->SubscribersTableContainer = new Div("SubscribersTableContainer", "subscribers-table-container");

            tmpl->bindWidget("subscribers-title", new WText(tr("cms-subscribers-page-title")));

            tmpl->bindWidget("subscribers-table", m_pimpl->SubscribersTableContainer);

            tmpl->bindWidget("all-subscribers-button", allSubscribersPushButton);
            tmpl->bindWidget("english-farsi-subscribers-button", englishFarsiSubscribersPushButton);
            tmpl->bindWidget("english-subscribers-button", englishSubscribersPushButton);
            tmpl->bindWidget("farsi-subscribers-button", farsiSubscribersPushButton);
            tmpl->bindWidget("inactive-subscribers-button", inactiveSubscribersPushButton);

            tmpl->bindWidget("items-per-page-text", new WText(tr("cms-subscribers-number-of-items-per-page")));
            tmpl->bindWidget("items-per-page-select", itemsPerPageComboBox);
            tmpl->bindString("items-per-page-select-id", itemsPerPageComboBox->id());
            tmpl->bindWidget("pagination-buttons", m_pimpl->PaginationButtonsContainer);

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
    : PaginationOptions {
            { tr("cms-subscribers-number-of-items-per-page-10"), 10 },
            { tr("cms-subscribers-number-of-items-per-page-25"), 25 },
            { tr("cms-subscribers-number-of-items-per-page-50"), 50 },
            { tr("cms-subscribers-number-of-items-per-page-100"), 100 },
            { tr("cms-subscribers-number-of-items-per-page-500"), 500 },
            { tr("cms-subscribers-number-of-items-per-page-1000"), 1000 },
            { tr("cms-subscribers-number-of-items-per-page-all"), -1 }
      },
      PaginationOptionsDefaultIndex (2),
      PaginationTableType(Table::All),
      PaginationTotalItems(0),
      PaginationItemsPerPageLimit(-1),
      PaginationItemOffset(0),
      PaginationPageOffset(0)
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

void CmsSubscribers::Impl::OnItemsPerPageComboBoxChanged(Wt::WComboBox *comboBox)
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    try {
        WString value(comboBox->currentText());

        bool found = false;
        for (const pair<WString, int> &it : this->PaginationOptions) {
            if (value == it.first) {
                this->PaginationItemsPerPageLimit = it.second;
                found = true;
            }
        }

        if (!found) {
            return;
        }

        if (this->PaginationItemsPerPageLimit < 0) {
            this->PaginationPageOffset = 0;
            this->PaginationItemOffset = 0;
            this->FillDataTable(this->PaginationTableType);
            return;
        }

        this->PaginationPageOffset = static_cast<uint_fast64_t>(
                    std::floor(this->PaginationItemOffset
                               / static_cast<double>(this->PaginationItemsPerPageLimit)));
        this->PaginationItemOffset = this->PaginationPageOffset
                * static_cast<uint_fast64_t>(this->PaginationItemsPerPageLimit);

        this->FillDataTable(this->PaginationTableType);
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

void CmsSubscribers::Impl::OnPaginiationButtonPressed(Wt::WPushButton *button)
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    try {
        this->PaginationPageOffset =
                boost::lexical_cast<uint_fast64_t>(button->text().toUTF8()) - 1;
        this->PaginationItemOffset = this->PaginationPageOffset
                * static_cast<uint_fast64_t>(this->PaginationItemsPerPageLimit);

        this->FillDataTable(this->PaginationTableType);
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

        if (cgiEnv->GetInformation().Client.Language.Code
                == CgiEnv::InformationRecord::ClientRecord::LanguageCode::En) {
            out_date = (boost::wformat(L"%1%/%2%/%3%")
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
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    try {
        this->PaginationTableType = tableType;

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

        std::string paginationPhrase;
        if (this->PaginationItemsPerPageLimit > -1) {
            paginationPhrase = (format("LIMIT %1% OFFSET %2%")
                                % boost::lexical_cast<std::string>(this->PaginationItemsPerPageLimit)
                                % boost::lexical_cast<std::string>(this->PaginationItemOffset)).str();
        }

        string query;
        switch (tableType) {
        case Table::All:
            query.assign((format("SELECT count(*) over(), inbox, uuid, subscription, pending_confirm, pending_cancel, join_date, update_date"
                                 " FROM \"%1%\" ORDER BY inbox COLLATE \"en_US.UTF-8\" ASC %2%;")
                          % Pool::Database().GetTableName("SUBSCRIBERS") % paginationPhrase).str());
            break;
        case Table::EnFa:
            query.assign((format("SELECT count(*) over(), inbox, uuid, subscription, pending_confirm, pending_cancel, join_date, update_date"
                                 " FROM \"%1%\" WHERE subscription = 'en_fa' ORDER BY inbox COLLATE \"en_US.UTF-8\" ASC %2%;")
                          % Pool::Database().GetTableName("SUBSCRIBERS") % paginationPhrase).str());
            break;
        case Table::En:
            query.assign((format("SELECT count(*) over(), inbox, uuid, subscription, pending_confirm, pending_cancel, join_date, update_date"
                                 " FROM \"%1%\" WHERE subscription = 'en' ORDER BY inbox COLLATE \"en_US.UTF-8\" ASC %2%;")
                          % Pool::Database().GetTableName("SUBSCRIBERS") % paginationPhrase).str());
            break;
        case Table::Fa:
            query.assign((format("SELECT count(*) over(), inbox, uuid, subscription, pending_confirm, pending_cancel, join_date, update_date"
                                 " FROM \"%1%\" WHERE subscription = 'fa' ORDER BY inbox COLLATE \"en_US.UTF-8\" ASC %2%;")
                          % Pool::Database().GetTableName("SUBSCRIBERS") % paginationPhrase).str());
            break;
        case Table::Inactive:
            query.assign((format("SELECT count(*) over(), inbox, uuid, subscription, pending_confirm, pending_cancel, join_date, update_date"
                                 " FROM \"%1%\" WHERE subscription = 'none' ORDER BY inbox COLLATE \"en_US.UTF-8\" ASC %2%;")
                          % Pool::Database().GetTableName("SUBSCRIBERS") % paginationPhrase).str());
            break;
        }

        LOG_INFO("Running query...", query, cgiEnv->GetInformation().ToJson());

        auto conn = Pool::Database().Connection();
        conn->activate();
        pqxx::work txn(*conn.get());

        result r = txn.exec(query);

        int i = 0;
        for (const auto & row : r) {
            ++i;

            this->PaginationTotalItems = boost::lexical_cast<uint_fast64_t>(row["count"].c_str());

            const string inbox(row["inbox"].c_str());
            const string uuid(row["uuid"].c_str());
            const string subscription(row["subscription"].c_str());
            const string pendingConfirm(row["pending_confirm"].c_str());
            const string pendingCancel(row["pending_cancel"].c_str());
            const string joinDate(row["join_date"].c_str());
            const string updateDate(row["update_date"].c_str());

            WString subscriptionTypeName;
            WString pendingConfirmTypeName;
            WString pendingCancelTypeName;

            this->GetSubscriptionTypeName(subscription, subscriptionTypeName);
            this->GetSubscriptionTypeName(pendingConfirm, pendingConfirmTypeName);
            this->GetSubscriptionTypeName(pendingCancel, pendingCancelTypeName);

            WString joinDateFormatted;
            WString updateDateFormatted;

            this->GetDate(joinDate, joinDateFormatted);
            this->GetDate(updateDate, updateDateFormatted);

            table->elementAt(i, 0)->addWidget(new WText(WString::fromUTF8(lexical_cast<string>(this->PaginationItemOffset + static_cast<uint_fast64_t>(i)))));
            table->elementAt(i, 1)->addWidget(new WText(WString::fromUTF8(inbox)));
            table->elementAt(i, 2)->addWidget(new WText(subscriptionTypeName));
            table->elementAt(i, 3)->addWidget(new WText(pendingConfirmTypeName));
            table->elementAt(i, 4)->addWidget(new WText(pendingCancelTypeName));
            table->elementAt(i, 5)->addWidget(new WText(joinDateFormatted));
            table->elementAt(i, 6)->addWidget(new WText(updateDateFormatted));
            table->elementAt(i, 7)->addWidget(new WText(WString::fromUTF8(uuid)));
        }

        this->ReEvaluatePaginationButtons();
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

void CmsSubscribers::Impl::ReEvaluatePaginationButtons()
{
    CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
    CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

    try {
        this->PaginationButtonsContainer->clear();

        if (this->PaginationItemsPerPageLimit < 0) {
            return;
        }

        uint_fast64_t numberOfPages = static_cast<uint_fast64_t>(
                    std::ceil(this->PaginationTotalItems
                              / static_cast<double>(this->PaginationItemsPerPageLimit)));
        for (uint_fast64_t i = 0; i < numberOfPages; ++i) {
            WPushButton *button = new WPushButton(
                        WString::fromUTF8(boost::lexical_cast<std::string>(i + 1)));

            if (i != this->PaginationPageOffset) {
                button->setStyleClass("btn btn-default");
            } else {
                button->setStyleClass("btn btn-primary");
            }

            this->PaginationButtonsContainer->addWidget(button);

            WSignalMapper<WPushButton *> *buttonSignalMapper = new WSignalMapper<WPushButton *>(this);
            buttonSignalMapper->mapped().connect(this, &CmsSubscribers::Impl::OnPaginiationButtonPressed);
            buttonSignalMapper->mapConnect(button->clicked(), button);
        }
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
