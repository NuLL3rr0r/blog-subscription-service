/**
 * @file
 * @author  Mamadou Babaei <info@babaei.net>
 * @version 0.1.0
 *
 * @section LICENSE
 *
 * (The MIT License)
 *
 * Copyright (c) 2016 - 2019 Mamadou Babaei
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
#include <cstdint>
#include <cstdio>
#include <cstring>
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
#include <maxminddb.h>
#include <CoreLib/Crypto.hpp>
#include <CoreLib/Defines.hpp>
#include <CoreLib/Exception.hpp>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Log.hpp>
#include <CoreLib/make_unique.hpp>
#include <CoreLib/Utility.hpp>
#include "CgiEnv.hpp"
#include "Exception.hpp"
#include "Pool.hpp"

#define     UNKNOWN_ERROR                   "Unknown error!"
#define     GEO_LOCATION_INITIALIZE_ERROR   "Failed to initialize GeoIP record!"

#define     CITY_DATABASE_NAME              "GeoLite2-City.mmdb"
#define     CITY_DATABASE_PATH_USR          "/usr/share/GeoIP/" CITY_DATABASE_NAME
#define     CITY_DATABASE_PATH_USR_LOCAL    "/usr/local/share/GeoIP/" CITY_DATABASE_NAME

#define     COUNTRY_DATABASE_NAME           "GeoLite2-Country.mmdb"
#define     COUNTRY_DATABASE_PATH_USR       "/usr/share/GeoIP/" COUNTRY_DATABASE_NAME
#define     COUNTRY_DATABASE_PATH_USR_LOCAL "/usr/local/share/GeoIP/" COUNTRY_DATABASE_NAME

#define     ASN_DATABASE_NAME               "GeoLite2-ASN.mmdb"
#define     ASN_DATABASE_PATH_USR           "/usr/share/GeoIP/" ASN_DATABASE_NAME
#define     ASN_DATABASE_PATH_USR_LOCAL     "/usr/local/share/GeoIP/" ASN_DATABASE_NAME

#if SIZE_MAX == UINT32_MAX
#define MAYBE_CHECK_SIZE_OVERFLOW(lhs, rhs, error) \
    if ((lhs) > (rhs)) {                           \
        return error;                              \
    }
#else
#define MAYBE_CHECK_SIZE_OVERFLOW(...)
#endif

using namespace std;
using namespace Wt;
using namespace boost;
using namespace CoreLib;
using namespace Service;

struct CgiEnv::Impl
{
public:
    FORCEINLINE static const char* GetGeoLite2CityDatabase()
    {
#if defined ( __FreeBSD__ )
        static const char* database = CITY_DATABASE_PATH_USR_LOCAL;
#elif defined ( __gnu_linux__ ) || defined ( __linux__ )
        static const char* database = CITY_DATABASE_PATH_USR;
#else /* defined ( __FreeBSD__ ) */
        static const char* database =
                FileSystem::FileExists(CITY_DATABASE_PATH_USR_LOCAL)
                ? FileSystem::FileExists(CITY_DATABASE_PATH_USR_LOCAL)
                : FileSystem::FileExists(CITY_DATABASE_PATH_USR);
#endif /* defined ( __FreeBSD__ ) */

        if (!FileSystem::FileExists(database)) {
            LOG_ERROR("Cannot find MaxMind database file!", database);
        }

        return database;
    }

    FORCEINLINE static const char* GetGeoLite2CountryDatabase()
    {
#if defined ( __FreeBSD__ )
        static const char* database = COUNTRY_DATABASE_PATH_USR_LOCAL;
#elif defined ( __gnu_linux__ ) || defined ( __linux__ )
        static const char* database = COUNTRY_DATABASE_PATH_USR;
#else /* defined ( __FreeBSD__ ) */
        static const char* database =
                FileSystem::FileExists(COUNTRY_DATABASE_PATH_USR_LOCAL)
                ? FileSystem::FileExists(COUNTRY_DATABASE_PATH_USR_LOCAL)
                : FileSystem::FileExists(COUNTRY_DATABASE_PATH_USR);
#endif /* defined ( __FreeBSD__ ) */

        if (!FileSystem::FileExists(database)) {
            LOG_ERROR("Cannot find MaxMind database file!", database);
        }

        return database;
    }

    FORCEINLINE static const char* GetGeoLite2ASNDatabase()
    {
#if defined ( __FreeBSD__ )
        static const char* database = ASN_DATABASE_PATH_USR_LOCAL;
#elif defined ( __gnu_linux__ ) || defined ( __linux__ )
        static const char* database = ASN_DATABASE_PATH_USR;
#else /* defined ( __FreeBSD__ ) */
        static const char* database =
                FileSystem::FileExists(ASN_DATABASE_PATH_USR_LOCAL)
                ? FileSystem::FileExists(ASN_DATABASE_PATH_USR_LOCAL)
                : FileSystem::FileExists(ASN_DATABASE_PATH_USR);
#endif /* defined ( __FreeBSD__ ) */

        if (!FileSystem::FileExists(database)) {
            LOG_ERROR("Cannot find MaxMind database file!", database);
        }

        return database;
    }

    static const char* TranslateMaxMindError(int errorCode);

    FORCEINLINE static char *BytesToHex(uint8_t *bytes, uint32_t size)
    {
        char *hexString;
        MAYBE_CHECK_SIZE_OVERFLOW(size, SIZE_MAX / 2 - 1, nullptr);

        hexString = static_cast<char *>(malloc((size * 2) + 1));
        if (!hexString) {
            return nullptr;
        }

        for (uint32_t i = 0; i < size; ++i) {
            sprintf(hexString + (2 * i), "%02X", bytes[i]);
        }

        return hexString;
    }

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

    MMDB_entry_data_list_s *DumpEntryDataList(
            boost::property_tree::ptree &out_entryTree,
            MMDB_entry_data_list_s *out_entryDataList,
            int& out_status) const;

    FORCEINLINE bool GetGeoData(const std::string &database,
                                const std::string &ipAddress,
                                boost::property_tree::ptree &out_tree) const
    {
        bool result = false;

        try {
            MMDB_entry_data_list_s *out_entryDataList = nullptr;

            MMDB_s mmdb;
            int openStatus = MMDB_open(database.c_str(), MMDB_MODE_MMAP, &mmdb);

            if (openStatus == MMDB_SUCCESS) {
                int gaiError;
                int mmdbError;

                MMDB_lookup_result_s lookupResult =
                        MMDB_lookup_string(&mmdb, ipAddress.c_str(),
                                           &gaiError, &mmdbError);

                if (gaiError == 0) {
                    if (mmdbError == MMDB_SUCCESS) {
                        if (lookupResult.found_entry) {
                            int statusGetEntryDataList =
                                    MMDB_get_entry_data_list(&lookupResult.entry,
                                                             &out_entryDataList);

                            if (statusGetEntryDataList == MMDB_SUCCESS) {
                                if (out_entryDataList) {
//                                    { /// DEBUG DUMP
//                                        FILE *stream;
//                                        char *buffer;
//                                        size_t length;

//                                        stream = open_memstream(&buffer, &length);

//                                        if (stream) {
//                                            int statusDumpEntryData =
//                                                    MMDB_dump_entry_data_list(stream, out_entryDataList, 2);

//                                            if (statusDumpEntryData == MMDB_SUCCESS) {
//                                                fflush(stream);

//                                                std::string data(buffer, length);

//                                                LOG_DEBUG(data);
//                                            } else {
//                                                LOG_ERROR(ipAddress, "Geo dump entry data error!");
//                                            }
//                                        } else {
//                                            LOG_ERROR(ipAddress, "Geo stream open error!");
//                                        }
//                                    }

                                    MMDB_entry_data_list_s *firstEntry = out_entryDataList;
                                    int status;

                                    (void)DumpEntryDataList(out_tree, firstEntry, status);

                                    if (status == MMDB_SUCCESS) {
                                        result = true;
                                    } else {
                                        LOG_ERROR(ipAddress, "Failed to dump geo entry data list!");
                                    }
                                } else {
                                    LOG_ERROR(ipAddress, "Geo null entry data list error!");
                                }

                                MMDB_free_entry_data_list(out_entryDataList);
                            } else {
                                LOG_ERROR(ipAddress,
                                          "Geo lookup error!",
                                          CgiEnv::Impl::TranslateMaxMindError(statusGetEntryDataList));
                            }
                        } else {
                            LOG_ERROR(ipAddress, "No geo data entry was found!");
                        }
                    } else {
                        LOG_ERROR(ipAddress,
                                  (boost::format("Geo error from libmaxminddb: '%1%!'")
                                   % gaiError).str())
                    }
                } else {
                    LOG_ERROR(ipAddress,
                              (boost::format("Geo error from getaddrinfo: '%1%'!")
                               % gaiError).str())
                }

                MMDB_close(&mmdb);
            } else {
                LOG_ERROR(ipAddress, CgiEnv::Impl::TranslateMaxMindError(openStatus));
            }
        }

        catch (const Service::Exception<std::string> &ex) {
            LOG_ERROR(GEO_LOCATION_INITIALIZE_ERROR, ex.What());
        }

        catch (const CoreLib::Exception<std::string> &ex) {
            LOG_ERROR(GEO_LOCATION_INITIALIZE_ERROR, ex.What());
        }

        catch (const boost::exception &ex) {
            LOG_ERROR(GEO_LOCATION_INITIALIZE_ERROR, boost::diagnostic_information(ex));
        }

        return result;
    }

    FORCEINLINE bool GetGeoCityData(const std::string &ipAddress,
                                    boost::property_tree::ptree &out_data) const
    {
        return GetGeoData(Impl::GetGeoLite2CityDatabase(),
                          ipAddress, out_data);
    }

    FORCEINLINE bool GetGeoCountryData(const std::string &ipAddress,
                                       boost::property_tree::ptree &out_data) const
    {
        return GetGeoData(Impl::GetGeoLite2CountryDatabase(),
                          ipAddress, out_data);
    }

    FORCEINLINE bool GetGeoASNData(const std::string &ipAddress,
                                   boost::property_tree::ptree &out_data) const
    {
        return GetGeoData(Impl::GetGeoLite2ASNDatabase(),
                          ipAddress, out_data);
    }

    template <typename _T>
    bool GetGeoValue(
            const boost::property_tree::ptree &tree,
            const std::string &key,
            _T &out_value) const
    {
        if (tree.get_child_optional("GeoLite2-City." + key)) {
            out_value = tree.get<_T>("GeoLite2-City." + key);
            return true;

        } else if (tree.get_child_optional("GeoLite2-Country." + key)) {
            out_value = tree.get<_T>("GeoLite2-Country." + key);
            return true;

        } else if (tree.get_child_optional("GeoLite2-ASN." + key)) {
            out_value = tree.get<_T>("GeoLite2-ASN." + key);
            return true;
        }

        return false;
    }

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

const char* CgiEnv::Impl::TranslateMaxMindError(int errorCode)
{
    switch (errorCode) {

    case MMDB_SUCCESS:
        return "Success (not an error)!";

    case MMDB_FILE_OPEN_ERROR:
        return "Error opening the specified MaxMind DB file!";

    case MMDB_CORRUPT_SEARCH_TREE_ERROR:
        return "The MaxMind DB file's search tree is corrupt!";

    case MMDB_INVALID_METADATA_ERROR:
        return "The MaxMind DB file contains invalid metadata!";

    case MMDB_IO_ERROR:
        return "An attempt to read data from the MaxMind DB file failed!";

    case MMDB_OUT_OF_MEMORY_ERROR:
        return "A memory allocation call failed!";

    case MMDB_UNKNOWN_DATABASE_FORMAT_ERROR:
        return "The MaxMind DB file is in a format this library can't handle"
               " (unknown record size or binary format version)!";

    case MMDB_INVALID_DATA_ERROR:
        return "The MaxMind DB file's data section contains bad data (unknown"
               " data type or corrupt data)!";

    case MMDB_INVALID_LOOKUP_PATH_ERROR:
        return "The lookup path contained an invalid value (like a negative"
               " integer for an array index)!";

    case MMDB_LOOKUP_PATH_DOES_NOT_MATCH_DATA_ERROR:
        return "The lookup path does not match the data (key that doesn't exist,"
               " array index bigger than the array, expected array or map where"
               " none exists)!";

    case MMDB_INVALID_NODE_NUMBER_ERROR:
        return "The MMDB_read_node function was called with a node number that"
               " does not exist in the search tree!";

    case MMDB_IPV6_LOOKUP_IN_IPV4_DATABASE_ERROR:
        return "You attempted to look up an IPv6 address in an IPv4-only"
               " database!";

    default:
        return "Unknown error code!";
    }
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

MMDB_entry_data_list_s *CgiEnv::Impl::DumpEntryDataList(
        boost::property_tree::ptree &out_entryTree,
        MMDB_entry_data_list_s *out_entryDataList,
        int &out_status) const
{
    switch (out_entryDataList->entry_data.type) {
    case MMDB_DATA_TYPE_ARRAY: {
        uint32_t size = out_entryDataList->entry_data.data_size;

        for (out_entryDataList = out_entryDataList->next;
             size && out_entryDataList; --size) {
            boost::property_tree::ptree itemTree;

            out_entryDataList = DumpEntryDataList(itemTree, out_entryDataList, out_status);

            out_entryTree.push_back(std::make_pair("", itemTree));

            if (out_status != MMDB_SUCCESS) {
                return nullptr;
            }
        }
    } break;
    case MMDB_DATA_TYPE_BOOLEAN: {
        bool value = out_entryDataList->entry_data.boolean;
        out_entryTree.put_value(boost::lexical_cast<std::string>(value));
        out_entryDataList = out_entryDataList->next;
    } break;
    case MMDB_DATA_TYPE_BYTES: {
        char *hexBuffer =
            Impl::BytesToHex((uint8_t *)out_entryDataList->entry_data.bytes,
                         out_entryDataList->entry_data.data_size);

        if (!hexBuffer) {
            out_status = MMDB_OUT_OF_MEMORY_ERROR;
            return nullptr;
        }

        std::string value(hexBuffer);
        free(hexBuffer);
        out_entryTree.put_value(value);

        out_entryDataList = out_entryDataList->next;
    } break;
    case MMDB_DATA_TYPE_DOUBLE: {
        double value = out_entryDataList->entry_data.double_value;
        out_entryTree.put_value(boost::lexical_cast<std::string>(value));
        out_entryDataList = out_entryDataList->next;
    } break;
    case MMDB_DATA_TYPE_FLOAT: {
        float value = out_entryDataList->entry_data.float_value;
        out_entryTree.put_value(boost::lexical_cast<std::string>(value));
        out_entryDataList = out_entryDataList->next;
    } break;
    case MMDB_DATA_TYPE_INT32: {
        int32_t value = out_entryDataList->entry_data.int32;
        out_entryTree.put_value(boost::lexical_cast<std::string>(value));
        out_entryDataList = out_entryDataList->next;
    } break;
    case MMDB_DATA_TYPE_MAP: {
        uint32_t size = out_entryDataList->entry_data.data_size;

        for (out_entryDataList = out_entryDataList->next;
             size && out_entryDataList; --size) {

            if (out_entryDataList->entry_data.type
                    != MMDB_DATA_TYPE_UTF8_STRING) {
                out_status = MMDB_INVALID_DATA_ERROR;
                return nullptr;
            }

            const char *keyBuffer = static_cast<const char *>(
                        out_entryDataList->entry_data.utf8_string);
            if (!keyBuffer) {
                out_status = MMDB_OUT_OF_MEMORY_ERROR;
                return nullptr;
            }

            const uint32_t keySize = out_entryDataList->entry_data.data_size;
            std::string key(keyBuffer, keySize);

            out_entryDataList = out_entryDataList->next;
            boost::property_tree::ptree subTree;
            out_entryDataList = DumpEntryDataList(subTree, out_entryDataList, out_status);

            if (out_status != MMDB_SUCCESS) {
                return nullptr;
            }

            out_entryTree.add_child(key, subTree);
        }
    } break;
    case MMDB_DATA_TYPE_UINT16: {
        uint16_t value = out_entryDataList->entry_data.uint16;
        out_entryTree.put_value(boost::lexical_cast<std::string>(value));
        out_entryDataList = out_entryDataList->next;
    } break;
    case MMDB_DATA_TYPE_UINT32: {
        uint32_t value = out_entryDataList->entry_data.uint32;
        out_entryTree.put_value(boost::lexical_cast<std::string>(value));
        out_entryDataList = out_entryDataList->next;
    } break;
    case MMDB_DATA_TYPE_UINT64: {
        uint64_t value = out_entryDataList->entry_data.uint64;
        out_entryTree.put_value(boost::lexical_cast<std::string>(value));
        out_entryDataList = out_entryDataList->next;
    } break;
    case MMDB_DATA_TYPE_UINT128: {
        mmdb_uint128_t value = out_entryDataList->entry_data.uint128;
        out_entryTree.put_value(boost::lexical_cast<std::string>(value));
        out_entryDataList = out_entryDataList->next;
    } break;
    case MMDB_DATA_TYPE_UTF8_STRING: {
        const char *buffer = static_cast<const char *>(
                    out_entryDataList->entry_data.utf8_string);

        if (!buffer) {
            out_status = MMDB_OUT_OF_MEMORY_ERROR;
            return nullptr;
        }

        const uint32_t size = out_entryDataList->entry_data.data_size;
        std::string value(buffer, size);
        out_entryTree.put_value(value);

        out_entryDataList = out_entryDataList->next;
    } break;
    default: {
        out_status = MMDB_INVALID_DATA_ERROR;
        return nullptr;
    } break;
    }

    out_status = MMDB_SUCCESS;

    return out_entryDataList;
}

void CgiEnv::Impl::FillGeoLocationRecord()
{
    try {
        const char *ipAddress = this->Information.Client.IPAddress.c_str();

        boost::property_tree::ptree cityTree;
        (void)this->GetGeoCityData(ipAddress, cityTree);

        boost::property_tree::ptree countryTree;
        (void)this->GetGeoCountryData(ipAddress, countryTree);

        boost::property_tree::ptree asnTree;
        (void)this->GetGeoASNData(ipAddress, asnTree);

        boost::property_tree::ptree fullTree;
        fullTree.add_child("GeoLite2-City", cityTree);
        fullTree.add_child("GeoLite2-Country", countryTree);
        fullTree.add_child("GeoLite2-ASN", asnTree);

        if (!this->GetGeoValue<std::string>(
                    fullTree, std::string("country.iso_code"),
                    this->Information.Client.GeoLocation.CountryCode)) {
            this->Information.Client.GeoLocation.CountryCode = "";
        }

        this->Information.Client.GeoLocation.CountryCode3 = "";

        if (!this->GetGeoValue<std::string>(
                    fullTree, std::string("country.names.en"),
                    this->Information.Client.GeoLocation.CountryName)) {
            this->Information.Client.GeoLocation.CountryName = "";
        }

        /// NOTE
        /// .. reads the arrays fist element
        if (!this->GetGeoValue<std::string>(
                    fullTree, std::string("subdivisions..names.en"),
                    this->Information.Client.GeoLocation.Region)) {
            this->Information.Client.GeoLocation.Region = "";
        }

        if (this->GetGeoValue<std::string>(
                    fullTree, std::string("city.names.en"),
                    this->Information.Client.GeoLocation.City)) {
            this->Information.Client.GeoLocation.City = "";
        }

        if (this->GetGeoValue<std::string>(
                    fullTree, std::string("postal.code"),
                    this->Information.Client.GeoLocation.PostalCode)) {
            this->Information.Client.GeoLocation.PostalCode = "";
        }

        if (!this->GetGeoValue<float>(
                    fullTree, std::string("location.latitude"),
                    this->Information.Client.GeoLocation.Latitude)) {
            this->Information.Client.GeoLocation.Latitude = 0.0f;
        }

        if (!this->GetGeoValue<float>(
                    fullTree, std::string("location.longitude"),
                    this->Information.Client.GeoLocation.Longitude)) {
            this->Information.Client.GeoLocation.Longitude = 0.0f;
        }

        if (!this->GetGeoValue<int>(
                    fullTree, std::string("location.metro_code"),
                    this->Information.Client.GeoLocation.MetroCode)) {
            this->Information.Client.GeoLocation.MetroCode = -1;
        }

        this->Information.Client.GeoLocation.DmaCode = -1;
        this->Information.Client.GeoLocation.AreaCode = -1;
        this->Information.Client.GeoLocation.Charset = -1;

        if (!this->GetGeoValue<std::string>(
                    fullTree, std::string("continent.code"),
                    this->Information.Client.GeoLocation.ContinentCode)) {
            this->Information.Client.GeoLocation.ContinentCode = "";
        }

        this->Information.Client.GeoLocation.Netmask = -1;

        if (!this->GetGeoValue<int>(
                    fullTree, std::string("autonomous_system_number"),
                    this->Information.Client.GeoLocation.ASN)) {
            this->Information.Client.GeoLocation.ASN = -1;
        }

        if (!this->GetGeoValue<std::string>(
                    fullTree, std::string("autonomous_system_organization"),
                    this->Information.Client.GeoLocation.ASO)) {
            this->Information.Client.GeoLocation.ASO = "";
        }

        std::stringstream ss;
        boost::property_tree::write_json(ss, fullTree, false);
        this->Information.Client.GeoLocation.RawData.assign(ss.str());
    }

    catch (const Service::Exception<std::string> &ex) {
        LOG_ERROR(GEO_LOCATION_INITIALIZE_ERROR, ex.What());
    }

    catch (const CoreLib::Exception<std::string> &ex) {
        LOG_ERROR(GEO_LOCATION_INITIALIZE_ERROR, ex.What());
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
