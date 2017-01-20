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
 * Provides some useful information about both the web server and client.
 */


#include <sstream>
#include <unordered_map>
#include <boost/algorithm/string.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/regex.hpp>
#include <boost/thread/once.hpp>
#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <cereal/archives/json.hpp>
#include <cereal/types/common.hpp>
#include <GeoIP.h>
#include <GeoIPCity.h>
#include <CoreLib/Crypto.hpp>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Log.hpp>
#include <CoreLib/make_unique.hpp>
#include <CoreLib/Utility.hpp>
#include "CgiEnv.hpp"
#include "Pool.hpp"

#define     UNKNOWN_ERROR                   "Unknown error!"
#define     GEO_LOCATION_INITIALIZE_ERROR   "Failed to initialize GeoIP record!"

using namespace std;
using namespace Wt;
using namespace boost;
using namespace CoreLib;
using namespace Service;

struct CgiEnv::Impl
{
public:
    typedef boost::bimap<boost::bimaps::unordered_set_of<Service::CgiEnv::InformationRecord::ClientRecord::LanguageCode>,
    boost::bimaps::unordered_set_of<std::string>> LanguageStringBiMap;
    typedef std::unordered_map<Service::CgiEnv::InformationRecord::ClientRecord::LanguageCode,
    Service::CgiEnv::InformationRecord::ClientRecord::PageDirection,
    CoreLib::Utility::Hasher<Service::CgiEnv::InformationRecord::ClientRecord::LanguageCode>> LanguageDirectionHashTable;

public:
    LanguageStringBiMap LanguageStringMapper;
    LanguageDirectionHashTable LanguageDirectionMapper;

public:
    CgiEnv::InformationRecord Information;

public:
    static std::string CStrToStr(const char *cstr);

    template <typename _T>
    static std::string RecordToJson(const _T instance, bool pretty = false)
    {
        try {
            std::stringstream ss;

            /// The curly braces are mandatory!
            /// http://uscilab.github.io/cereal/quickstart.html
            /// "Archives are designed to be used in an RAII manner and are guaranteed to flush their contents only on destruction..."
            {
                cereal::JSONOutputArchive archive(ss);
                archive(*instance);
            }

            using boost::property_tree::ptree;

            ptree pt;
            read_json(ss, pt);

            if (pt.get_child_optional("value0")) {
                pt.add_child("Information", pt.get_child("value0"));
                pt.erase("value0");
            }

            /// Clear the stringstream
            ss.str(std::string());
            ss.clear();

            write_json(ss, pt, pretty);

            return ss.str();
        }

        catch (const boost::exception &ex) {
            LOG_ERROR(boost::diagnostic_information(ex));
        }

        catch (const std::exception &ex) {
            LOG_ERROR(ex.what());
        }

        catch(...) {
            LOG_ERROR(UNKNOWN_ERROR);
        }

        return "";
    }


public:
    explicit Impl();
    ~Impl();

    void Initialize();

    void FillGeoLocationRecord();
};

void CgiEnv::InformationRecord::ToJson(std::string &out_string) const
{
    out_string.assign(Service::CgiEnv::Impl::RecordToJson(this));
}

std::string CgiEnv::InformationRecord::ToJson() const
{
    string json;
    ToJson(json);
    return json;
}

void CgiEnv::InformationRecord::ClientRecord::ToJson(std::string &out_string) const
{
    out_string.assign(Service::CgiEnv::Impl::RecordToJson(this));
}

std::string CgiEnv::InformationRecord::ClientRecord::ToJson() const
{
    string json;
    ToJson(json);
    return json;
}

void CgiEnv::InformationRecord::ClientRecord::LanguageRecord::ToJson(std::string &out_string) const
{
    out_string.assign(Service::CgiEnv::Impl::RecordToJson(this));
}

std::string CgiEnv::InformationRecord::ClientRecord::LanguageRecord::ToJson() const
{
    string json;
    ToJson(json);
    return json;
}

void CgiEnv::InformationRecord::ClientRecord::GeoLocationRecord::ToJson(std::string &out_string) const
{
    out_string.assign(Service::CgiEnv::Impl::RecordToJson(this));
}

std::string CgiEnv::InformationRecord::ClientRecord::GeoLocationRecord::ToJson() const
{
    string json;
    ToJson(json);
    return json;
}

void CgiEnv::InformationRecord::ClientRecord::RequestRecord::ToJson(std::string &out_string) const
{
    out_string.assign(Service::CgiEnv::Impl::RecordToJson(this));
}

std::string CgiEnv::InformationRecord::ClientRecord::RequestRecord::ToJson() const
{
    string json;
    ToJson(json);
    return json;
}

void CgiEnv::InformationRecord::ClientRecord::RequestRecord::RootRecord::ToJson(std::string &out_string) const
{
    out_string.assign(Service::CgiEnv::Impl::RecordToJson(this));
}

std::string CgiEnv::InformationRecord::ClientRecord::RequestRecord::RootRecord::ToJson() const
{
    string json;
    ToJson(json);
    return json;
}

void CgiEnv::InformationRecord::ClientRecord::SecurityRecord::ToJson(std::string &out_string) const
{
    out_string.assign(Service::CgiEnv::Impl::RecordToJson(this));
}

std::string CgiEnv::InformationRecord::ClientRecord::SecurityRecord::ToJson() const
{
    string json;
    ToJson(json);
    return json;
}

void CgiEnv::InformationRecord::ClientRecord::SessionRecord::ToJson(std::string &out_string) const
{
    out_string.assign(Service::CgiEnv::Impl::RecordToJson(this));
}

std::string CgiEnv::InformationRecord::ClientRecord::SessionRecord::ToJson() const
{
    string json;
    ToJson(json);
    return json;
}

void CgiEnv::InformationRecord::ServerRecord::ToJson(std::string &out_string) const
{
    out_string.assign(Service::CgiEnv::Impl::RecordToJson(this));
}

std::string CgiEnv::InformationRecord::ServerRecord::ToJson() const
{
    string json;
    ToJson(json);
    return json;
}

void CgiEnv::InformationRecord::SubscriptionRecord::ToJson(std::string &out_string) const
{
    out_string.assign(Service::CgiEnv::Impl::RecordToJson(this));
}

std::string CgiEnv::InformationRecord::SubscriptionRecord::ToJson() const
{
    string json;
    ToJson(json);
    return json;
}

CgiEnv::CgiEnv()
    : m_pimpl(make_unique<CgiEnv::Impl>())
{
    m_pimpl->Initialize();
}

CgiEnv::~CgiEnv() = default;

const CgiEnv::InformationRecord &CgiEnv::GetInformation() const
{
    return m_pimpl->Information;
}

void CgiEnv::SetSessionRecord(const Service::CgiEnv::InformationRecord::ClientRecord::SessionRecord &record)
{
    m_pimpl->Information.Client.Session = record;
}

void CgiEnv::SetSessionToken(const std::string &token)
{
    m_pimpl->Information.Client.Session.Token.assign(token);
}

void CgiEnv::SetSessionEmail(const std::string &email)
{
    m_pimpl->Information.Client.Session.Email.assign(email);
}

void CgiEnv::AddSubscriptionLanguage(const CgiEnv::InformationRecord::SubscriptionRecord::Language &lang)
{
     m_pimpl->Information.Subscription.Languages.push_back(lang);
}

void CgiEnv::SetSubscriptionAction(const CgiEnv::InformationRecord::SubscriptionRecord::Action &action)
{
    m_pimpl->Information.Subscription.Subscribe = action;
}

void CgiEnv::SetSubscriptionInbox(const std::string &inbox)
{
    m_pimpl->Information.Subscription.Inbox.assign(inbox);
}

string CgiEnv::Impl::CStrToStr(const char *cstr)
{
    return cstr != NULL ? cstr : "";
}

CgiEnv::Impl::Impl()
    : LanguageDirectionMapper {
{ Service::CgiEnv::InformationRecord::ClientRecord::LanguageCode::None,
          Service::CgiEnv::InformationRecord::ClientRecord::PageDirection::None },
{ Service::CgiEnv::InformationRecord::ClientRecord::LanguageCode::Invalid,
          Service::CgiEnv::InformationRecord::ClientRecord::PageDirection::None },
{ Service::CgiEnv::InformationRecord::ClientRecord::LanguageCode::En,
          Service::CgiEnv::InformationRecord::ClientRecord::PageDirection::LeftToRight },
{ Service::CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa,
          Service::CgiEnv::InformationRecord::ClientRecord::PageDirection::RightToLeft }
          }
{
    LanguageStringMapper.insert({ Service::CgiEnv::InformationRecord::ClientRecord::LanguageCode::None, "none" });
    LanguageStringMapper.insert({ Service::CgiEnv::InformationRecord::ClientRecord::LanguageCode::Invalid, "invalid" });
    LanguageStringMapper.insert({ Service::CgiEnv::InformationRecord::ClientRecord::LanguageCode::En, "en" });
    LanguageStringMapper.insert({ Service::CgiEnv::InformationRecord::ClientRecord::LanguageCode::Fa, "fa" });

    this->Information.Client.Language.Code = Service::CgiEnv::InformationRecord::ClientRecord::LanguageCode::None;
    this->Information.Client.Language.CodeAsString =
            LanguageStringMapper.left.find(Service::CgiEnv::InformationRecord::ClientRecord::LanguageCode::None)->second;
    this->Information.Client.Language.PageDirection = Service::CgiEnv::InformationRecord::ClientRecord::PageDirection::None;
    this->Information.Client.Request.Root.Login = false;
    this->Information.Client.Request.Root.Logout = false;
    this->Information.Client.Request.ContactForm = false;
    this->Information.Client.Security.XssAttackDetected = false;
}

CgiEnv::Impl::~Impl() = default;

void CgiEnv::Impl::Initialize()
{
    WApplication *app = WApplication::instance();

    this->Information.Server.Hostname = app->environment().hostName();
    this->Information.Server.Url = app->environment().urlScheme() + "://" + this->Information.Server.Hostname;
    this->Information.Server.RootLoginUrl = this->Information.Server.Url
            + (algorithm::ends_with(this->Information.Server.Url, "/") ? "" : "/")
            + "?root";
    this->Information.Server.NoReplyAddress = "no-reply@" + this->Information.Server.Hostname;

    this->Information.Client.IPAddress = app->environment().clientAddress();
    this->Information.Client.UserAgent = app->environment().userAgent();
    this->Information.Client.Referer = app->environment().referer();

    string queryStr = app->environment().getCgiValue("QUERY_STRING");
    this->Information.Client.Security.XssAttackDetected =
            (queryStr.find("<") != string::npos
            || queryStr.find(">") != string::npos
            || queryStr.find("%3C") != string::npos
            || queryStr.find("%3E") != string::npos
            || queryStr.find("%3c") != string::npos
            || queryStr.find("%3e") != string::npos)
            ? true : false;

    this->Information.Subscription.Subscribe = InformationRecord::SubscriptionRecord::Action::None;

    bool logout = false;

    Http::ParameterMap map = app->environment().getParameterMap();
    for (std::map<string, Http::ParameterValues>::const_iterator it = map.begin(); it != map.end(); ++it) {
        if (it->first == "lang") {
            auto itLang = this->LanguageStringMapper.right.find(it->second[0]);
            if (itLang != this->LanguageStringMapper.right.end()) {
                this->Information.Client.Language.Code = itLang->second;
            } else {
                this->Information.Client.Language.Code = Service::CgiEnv::InformationRecord::ClientRecord::LanguageCode::Invalid;
            }
            this->Information.Client.Language.CodeAsString = LanguageStringMapper.left.find(this->Information.Client.Language.Code)
                    ->second;
            this->Information.Client.Language.PageDirection = this->LanguageDirectionMapper[this->Information.Client.Language.Code];
        }

        if (it->first == "root") {
            this->Information.Client.Request.Root.Login = true;
        }

        if (it->first == "logout") {
            logout = true;
        }

        if (it->first == "subscribe" && it->second[0] != "") {
            try {
                if (it->second[0] == "1" || it->second[0] == "2"
                        || it->second[0] == "-1" || it->second[0] == "-2") {
                    auto action = lexical_cast<short>(it->second[0]);
                    this->Information.Subscription.Subscribe = static_cast<InformationRecord::SubscriptionRecord::Action>(action);
                }
            } catch (...) {
                this->Information.Subscription.Subscribe = InformationRecord::SubscriptionRecord::Action::Subscribe;
            }
        }

        if (it->first == "inbox" && it->second[0] != "") {
            static const regex REGEX(Pool::Storage().RegexEmail());
            smatch result;
            if (regex_search(it->second[0], result, REGEX)) {
                this->Information.Subscription.Inbox.assign(it->second[0]);
            }
        }

        if (it->first == "subscription" && it->second[0] != "") {
            static const regex REGEX(Pool::Storage().RegexLanguageArray());
            smatch result;
            if (regex_search(it->second[0], result, REGEX)) {
                vector<string> vec;
                split(vec, it->second[0], boost::is_any_of(","));
                vector<InformationRecord::SubscriptionRecord::Language> langs;
                for (const auto &s : vec) {
                    if (s == "en") {
                        langs.push_back(InformationRecord::SubscriptionRecord::Language::En);
                    } else if (s == "fa") {
                        langs.push_back(InformationRecord::SubscriptionRecord::Language::Fa);
                    }
                }
                this->Information.Subscription.Languages = std::move(langs);
            }
        }

        if (it->first == "recipient" && it->second[0] != "") {
            static const regex REGEX(Pool::Storage().RegexUuid());
            smatch result;
            if (regex_search(it->second[0], result, REGEX)) {
                this->Information.Subscription.Uuid.assign(it->second[0]);
            }
        }

        if (it->first == "token" && it->second[0] != "") {
            try {
                string token;
                Pool::Crypto().Decrypt(it->second[0], token);
                this->Information.Subscription.Timestamp = lexical_cast<time_t>(token);
            } catch (...) {
            }
        }

        if (it->first == "contact-form") {
            this->Information.Client.Request.ContactForm = true;
        }
    }

    if (this->Information.Client.Request.Root.Login && logout) {
        this->Information.Client.Request.Root.Logout = true;
    }

    // Set IP before calling this
    this->FillGeoLocationRecord();
}

void CgiEnv::Impl::FillGeoLocationRecord()
{
    try {
        GeoIP *geoLiteCity;
#if defined ( __FreeBSD__ )
        if (FileSystem::FileExists("/usr/local/share/GeoIP/GeoLiteCity.dat")) {
            geoLiteCity = GeoIP_open("/usr/local/share/GeoIP/GeoLiteCity.dat", GEOIP_STANDARD);
#elif  defined ( __gnu_linux__ ) || defined ( __linux__ )
        if (FileSystem::FileExists("/usr/share/GeoIP/GeoLiteCity.dat")) {
            geoLiteCity = GeoIP_open("/usr/share/GeoIP/GeoLiteCity.dat", GEOIP_STANDARD);
#else
        if (FileSystem::FileExists("/usr/local/share/GeoIP/GeoLiteCity.dat")) {
            geoLiteCity = GeoIP_open("/usr/local/share/GeoIP/GeoLiteCity.dat", GEOIP_STANDARD);
        } else if (FileSystem::FileExists("/usr/share/GeoIP/GeoLiteCity.dat")) {
            geoLiteCity = GeoIP_open("/usr/share/GeoIP/GeoLiteCity.dat", GEOIP_STANDARD);
#endif  // defined ( __FreeBSD__ )
        } else {
            LOG_ERROR(GEO_LOCATION_INITIALIZE_ERROR, "");
            return;
        }

        GeoIPRecordTag *record = GeoIP_record_by_name(geoLiteCity, this->Information.Client.IPAddress.c_str());

        if (record != NULL) {
            this->Information.Client.GeoLocation.CountryCode = CStrToStr(record->country_code);
            this->Information.Client.GeoLocation.CountryCode3 = CStrToStr(record->country_code3);
            this->Information.Client.GeoLocation.CountryName = CStrToStr(record->country_name);
            this->Information.Client.GeoLocation.Region = CStrToStr(record->region);
            this->Information.Client.GeoLocation.City = CStrToStr(record->city);
            this->Information.Client.GeoLocation.PostalCode = CStrToStr(record->postal_code);
            this->Information.Client.GeoLocation.Latitude = record->latitude;
            this->Information.Client.GeoLocation.Longitude = record->longitude;
            this->Information.Client.GeoLocation.MetroCode = record->metro_code;
            this->Information.Client.GeoLocation.DmaCode = record->dma_code;
            this->Information.Client.GeoLocation.AreaCode = record->area_code;
            this->Information.Client.GeoLocation.Charset = record->charset;
            this->Information.Client.GeoLocation.ContinentCode = record->continent_code;
            this->Information.Client.GeoLocation.Netmask = record->netmask;
        }
    }

    catch (const boost::exception &ex) {
        LOG_ERROR(GEO_LOCATION_INITIALIZE_ERROR, boost::diagnostic_information(ex));
    }

    catch (const std::exception &ex) {
        LOG_ERROR(GEO_LOCATION_INITIALIZE_ERROR, ex.what());
    }

    catch(...) {
        LOG_ERROR(GEO_LOCATION_INITIALIZE_ERROR, UNKNOWN_ERROR);
    }
}
