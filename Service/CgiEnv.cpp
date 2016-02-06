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


#include <unordered_map>
#include <boost/algorithm/string.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/thread/once.hpp>
#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <GeoIP.h>
#include <GeoIPCity.h>
#include <CoreLib/Crypto.hpp>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Log.hpp>
#include <CoreLib/make_unique.hpp>
#include <CoreLib/Utility.hpp>
#include "CgiEnv.hpp"
#include "Pool.hpp"

using namespace std;
using namespace Wt;
using namespace boost;
using namespace CoreLib;
using namespace Service;

struct CgiEnv::Impl
{
public:
    typedef boost::bimap<boost::bimaps::unordered_set_of<CgiEnv::Language>,
        boost::bimaps::unordered_set_of<std::string>> LanguageBiMap;
    typedef std::unordered_map<CgiEnv::Language, CgiEnv::LanguageDirection,
        CoreLib::Utility::Hasher<CgiEnv::Language>> LanguageDirectionHashTable;

public:
    std::string ClientInfoIP;
    std::string ClientInfoBrowser;
    std::string ClientInfoReferer;
    std::string ClientInfoLocation;

    std::string ServerInfoHost;
    std::string ServerInfoURL;
    std::string ServerInfoRootLoginUrl;
    std::string ServerInfoNoReplyAddr;

    CgiEnv::Language CurrentLanguage;
    LanguageBiMap LanguageMapper;
    LanguageDirectionHashTable LanguageDirectionMapper;

    bool FoundXSS;
    bool IsRootLoginRequested;
    bool IsRootLogoutRequested;
    bool IsContactFormRequested;

private:
    CgiEnv *m_parent;

public:
    explicit Impl(CgiEnv *cgiEnv);
    ~Impl();

    static std::string CStrToStr(const char *cstr);

    void ExtractClientInfoDetail();
};

CgiEnv::CgiEnv()
    : m_pimpl(make_unique<CgiEnv::Impl>(this))
{
    WApplication *app = WApplication::instance();

    m_pimpl->ClientInfoIP = app->environment().clientAddress();
    m_pimpl->ClientInfoBrowser = app->environment().userAgent();
    m_pimpl->ClientInfoReferer = app->environment().referer();

    m_pimpl->ExtractClientInfoDetail();

    m_pimpl->ClientInfoLocation =
            (format("%1% %2% %3% %4% %5% %6%")
             % ClientInfoRecord.city
             % ClientInfoRecord.region
             % ClientInfoRecord.country_code
             % ClientInfoRecord.country_code3
             % ClientInfoRecord.country_name
             % ClientInfoRecord.continent_code).str();
    if (ClientInfoRecord.latitude != "" && ClientInfoRecord.longitude != "") {
        m_pimpl->ClientInfoLocation +=
                (format("%1%,%2%")
                 % ClientInfoRecord.latitude
                 % ClientInfoRecord.longitude).str();
    }
    algorithm::trim(m_pimpl->ClientInfoLocation);

    m_pimpl->ServerInfoHost = app->environment().hostName();
    m_pimpl->ServerInfoURL = app->environment().urlScheme() + "://" + m_pimpl->ServerInfoHost;
    m_pimpl->ServerInfoRootLoginUrl = m_pimpl->ServerInfoURL
            + (algorithm::ends_with(m_pimpl->ServerInfoURL, "/") ? "" : "/")
            + "?root";
    m_pimpl->ServerInfoNoReplyAddr = "no-reply@" + m_pimpl->ServerInfoHost;

    string queryStr = app->environment().getCgiValue("QUERY_STRING");
    m_pimpl->FoundXSS = (queryStr.find("<") != string::npos ||
            queryStr.find(">") != string::npos ||
            queryStr.find("%3C") != string::npos ||
            queryStr.find("%3E") != string::npos ||
            queryStr.find("%3c") != string::npos ||
            queryStr.find("%3e") != string::npos)
            ? true : false;

    bool logout = false;

    SubscriptionData.Subscribe = Subscription::Action::None;

    Http::ParameterMap map = app->environment().getParameterMap();
    for (std::map<string, Http::ParameterValues>::const_iterator it = map.begin(); it != map.end(); ++it) {
        if (it->first == "lang") {
            auto itLang = m_pimpl->LanguageMapper.right.find(it->second[0]);
            if (itLang != m_pimpl->LanguageMapper.right.end()) {
                m_pimpl->CurrentLanguage = itLang->second;
            } else {
                m_pimpl->CurrentLanguage = Language::Invalid;
            }
        }

        if (it->first == "root") {
            m_pimpl->IsRootLoginRequested = true;
        }

        if (it->first == "logout") {
            logout = true;
        }

        if (it->first == "subscribe" && it->second[0] != "") {
            try {
                if (it->second[0] == "1" || it->second[0] == "2"
                        || it->second[0] == "-1" || it->second[0] == "-2") {
                    auto action = lexical_cast<short>(it->second[0]);
                    SubscriptionData.Subscribe = static_cast<Subscription::Action>(action);
                }
            } catch (...) {
                SubscriptionData.Subscribe = Subscription::Action::Subscribe;
            }
        }

        if (it->first == "inbox" && it->second[0] != "") {
            static const regex REGEX(Pool::Storage()->RegexEmail());
            smatch result;
            if (regex_search(it->second[0], result, REGEX)) {
                SubscriptionData.Inbox.assign(it->second[0]);
            }
        }

        if (it->first == "subscription" && it->second[0] != "") {
            static const regex REGEX(Pool::Storage()->RegexLanguageArray());
            smatch result;
            if (regex_search(it->second[0], result, REGEX)) {
                vector<string> vec;
                split(vec, it->second[0], boost::is_any_of(","));
                vector<Subscription::Language> langs;
                for (const auto &s : vec) {
                    if (s == "en") {
                        langs.push_back(Subscription::Language::En);
                    } else if (s == "fa") {
                        langs.push_back(Subscription::Language::Fa);
                    }
                }
                SubscriptionData.Languages = std::move(langs);
            }
        }

        if (it->first == "recipient" && it->second[0] != "") {
            static const regex REGEX(Pool::Storage()->RegexUuid());
            smatch result;
            if (regex_search(it->second[0], result, REGEX)) {
                SubscriptionData.Uuid.assign(it->second[0]);
            }
        }

        if (it->first == "token" && it->second[0] != "") {
            try {
                string token;
                Pool::Crypto()->Decrypt(it->second[0], token);
                SubscriptionData.Timestamp = lexical_cast<time_t>(token);
            } catch (...) {
            }
        }

        if (it->first == "contact-form") {
            m_pimpl->IsContactFormRequested = true;
        }
    }

    if (m_pimpl->IsRootLoginRequested && logout) {
        m_pimpl->IsRootLogoutRequested = true;
    }
}

CgiEnv::~CgiEnv() = default;

string CgiEnv::GetClientInfo(const CgiEnv::ClientInfo &key) const
{
    switch(key) {

    case ClientInfo::IP:
        return m_pimpl->ClientInfoIP;

    case ClientInfo::Browser:
        return m_pimpl->ClientInfoBrowser;

    case ClientInfo::Referer:
        return m_pimpl->ClientInfoReferer;

    case ClientInfo::Location:
        return m_pimpl->ClientInfoLocation;

    }
}

string CgiEnv::GetServerInfo(const CgiEnv::ServerInfo &key) const
{
    switch(key) {

    case ServerInfo::Host:
        return m_pimpl->ServerInfoHost;

    case ServerInfo::URL:
        return m_pimpl->ServerInfoURL;

    case ServerInfo::RootLoginUrl:
        return m_pimpl->ServerInfoRootLoginUrl;

    case ServerInfo::NoReplyAddr:
        return m_pimpl->ServerInfoNoReplyAddr;

    }
}

const CgiEnv::Language &CgiEnv::GetCurrentLanguage() const
{
    return m_pimpl->CurrentLanguage;
}

std::string CgiEnv::GetCurrentLanguageString() const
{
    auto it = m_pimpl->LanguageMapper.left.find(m_pimpl->CurrentLanguage);
    return it->second;
}

const CgiEnv::LanguageDirection &CgiEnv::GetLanguageDirection(
        const CgiEnv::Language &language) const
{
    return m_pimpl->LanguageDirectionMapper[language];
}

const CgiEnv::LanguageDirection &CgiEnv::GetCurrentLanguageDirection() const
{
    return m_pimpl->LanguageDirectionMapper[m_pimpl->CurrentLanguage];
}

bool CgiEnv::FoundXSS() const
{
    return m_pimpl->FoundXSS;
}

bool CgiEnv::IsRootLoginRequested() const
{
    return m_pimpl->IsRootLoginRequested;
}

bool CgiEnv::IsRootLogoutRequested() const
{
    return m_pimpl->IsRootLogoutRequested;
}

bool CgiEnv::IsContactFormRequested() const
{
    return m_pimpl->IsContactFormRequested;
}

CgiEnv::Impl::Impl(CgiEnv *cgiEnv)
    : CurrentLanguage(Language::None),
      LanguageDirectionMapper {
          { Language::None, LanguageDirection::None },
          { Language::Invalid, LanguageDirection::None },
          { Language::En, LanguageDirection::LeftToRight },
          { Language::Fa, LanguageDirection::RightToLeft }
      },
      FoundXSS(false),
      IsRootLoginRequested(false),
      IsRootLogoutRequested(false),
      IsContactFormRequested(false),
      m_parent(cgiEnv)
{
    LanguageMapper.insert({ Language::None, "none" });
    LanguageMapper.insert({ Language::Invalid, "invalid" });
    LanguageMapper.insert({ Language::En, "en" });
    LanguageMapper.insert({ Language::Fa, "fa" });
}

CgiEnv::Impl::~Impl() = default;

string CgiEnv::Impl::CStrToStr(const char *cstr)
{
    return cstr != NULL ? cstr : "";
}

void CgiEnv::Impl::ExtractClientInfoDetail()
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
#endif
        } else {
            return;
        }

        GeoIPRecordTag *record = GeoIP_record_by_name(geoLiteCity, ClientInfoIP.c_str());

        if (record != NULL) {
            m_parent->ClientInfoRecord.country_code = CStrToStr(record->country_code);
            m_parent->ClientInfoRecord.country_code3 = CStrToStr(record->country_code3);
            m_parent->ClientInfoRecord.country_name = CStrToStr(record->country_name);
            m_parent->ClientInfoRecord.region = CStrToStr(record->region);
            m_parent->ClientInfoRecord.city = CStrToStr(record->city);
            m_parent->ClientInfoRecord.postal_code = CStrToStr(record->postal_code);
            m_parent->ClientInfoRecord.latitude = lexical_cast<string>(record->latitude);
            m_parent->ClientInfoRecord.longitude = lexical_cast<string>(record->longitude);
            m_parent->ClientInfoRecord.metro_code = lexical_cast<string>(record->metro_code);
            m_parent->ClientInfoRecord.dma_code = lexical_cast<string>(record->dma_code);
            m_parent->ClientInfoRecord.area_code = lexical_cast<string>(record->area_code);
            m_parent->ClientInfoRecord.charset = lexical_cast<string>(record->charset);
            m_parent->ClientInfoRecord.continent_code = CStrToStr(record->continent_code);
            m_parent->ClientInfoRecord.netmask = lexical_cast<string>(record->netmask);
        }
    } catch(...) {
    }
}

