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


#ifndef SERVICE_CGIENV_HPP
#define SERVICE_CGIENV_HPP


#include <memory>
#include <string>
#include <vector>
#include <ctime>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>

namespace Wt {
class WEnvironment;
}

namespace Service {
class CgiEnv;
}

class Service::CgiEnv
{
public:
    struct InformationRecord {
        struct ClientRecord {
            enum class LanguageCode : unsigned char {
                None,
                Invalid,
                En,
                Fa
            };

            enum class PageDirection : unsigned char {
                None,
                LeftToRight,
                RightToLeft
            };

            struct LanguageRecord {
                Service::CgiEnv::InformationRecord::ClientRecord::LanguageCode Code;
                std::string CodeAsString;
                Service::CgiEnv::InformationRecord::ClientRecord::PageDirection PageDirection;

            public:
                void ToJson(std::string &out_string) const;
                std::string ToJson() const;

            private:
                friend class cereal::access;
                template<class Archive>
                void serialize(Archive & archive)
                {
                    archive(CEREAL_NVP(Code), CEREAL_NVP(CodeAsString), CEREAL_NVP(PageDirection));
                }
            };

            struct GeoLocationRecord {
                std::string CountryCode;
                std::string CountryCode3;
                std::string CountryName;
                std::string Region;
                std::string City;
                std::string PostalCode;
                float Latitude;
                float Longitude;
                int MetroCode;
                int DmaCode;
                int AreaCode;
                int Charset;
                std::string ContinentCode;
                int Netmask;
                int ASN;
                std::string ASO;
                std::string RawData;

            public:
                void ToJson(std::string &out_string) const;
                std::string ToJson() const;

            private:
                friend class cereal::access;
                template<class Archive>
                void serialize(Archive & archive)
                {
                    archive(CEREAL_NVP(CountryCode), CEREAL_NVP(CountryCode3), CEREAL_NVP(CountryName),
                            CEREAL_NVP(Region), CEREAL_NVP(City), CEREAL_NVP(PostalCode),
                            CEREAL_NVP(Latitude), CEREAL_NVP(Longitude), CEREAL_NVP(MetroCode), CEREAL_NVP(DmaCode),
                            CEREAL_NVP(AreaCode), CEREAL_NVP(Charset), CEREAL_NVP(ContinentCode), CEREAL_NVP(Netmask),
                            CEREAL_NVP(ASN), CEREAL_NVP(ASO), CEREAL_NVP(RawData));
                }
            };

            struct RequestRecord {
                struct RootRecord {
                    bool Login;
                    bool Logout;

                public:
                    void ToJson(std::string &out_string) const;
                    std::string ToJson() const;

                private:
                    friend class cereal::access;
                    template<class Archive>
                    void serialize(Archive & archive)
                    {
                        archive(CEREAL_NVP(Login), CEREAL_NVP(Logout));
                    }
                };

                RootRecord Root;
                bool ContactForm;

            public:
                void ToJson(std::string &out_string) const;
                std::string ToJson() const;

            private:
                friend class cereal::access;
                template<class Archive>
                void serialize(Archive & archive)
                {
                    archive(CEREAL_NVP(Root), CEREAL_NVP(ContactForm));
                }
            };

            struct SecurityRecord {
                bool XssAttackDetected;

            public:
                void ToJson(std::string &out_string) const;
                std::string ToJson() const;

            private:
                friend class cereal::access;
                template<class Archive>
                void serialize(Archive & archive)
                {
                    archive(CEREAL_NVP(XssAttackDetected));
                }
            };

            struct SessionRecord {
                struct LastLoginRecord {
                    GeoLocationRecord GeoLocation;
                    std::string IPAddress;
                    std::string Referer;
                    std::time_t Time;
                    std::string UserAgent;

                public:
                    void ToJson(std::string &out_string) const;
                    std::string ToJson() const;

                private:
                    friend class cereal::access;
                    template<class Archive>
                    void serialize(Archive & archive)
                    {
                        archive(CEREAL_NVP(GeoLocation), CEREAL_NVP(IPAddress), CEREAL_NVP(Referer), CEREAL_NVP(Time),
                                CEREAL_NVP(UserAgent));
                    }
                };

                std::string Email;
                LastLoginRecord LastLogin;
                std::string Token;
                std::string UserId;
                std::string Username;

            public:
                void ToJson(std::string &out_string) const;
                std::string ToJson() const;

            private:
                friend class cereal::access;
                template<class Archive>
                void serialize(Archive & archive)
                {
                    archive(CEREAL_NVP(Email), CEREAL_NVP(LastLogin), CEREAL_NVP(Token), CEREAL_NVP(UserId), CEREAL_NVP(Username));
                }
            };

            GeoLocationRecord GeoLocation;
            std::string IPAddress;
            LanguageRecord Language;
            std::string Referer;
            RequestRecord Request;
            SecurityRecord Security;
            SessionRecord Session;
            std::string UserAgent;

        public:
            void ToJson(std::string &out_string) const;
            std::string ToJson() const;

        private:
            friend class cereal::access;
            template<class Archive>
            void serialize(Archive & archive)
            {
                archive(CEREAL_NVP(GeoLocation), CEREAL_NVP(IPAddress), CEREAL_NVP(Language), CEREAL_NVP(Referer),
                        CEREAL_NVP(Request), CEREAL_NVP(Security), CEREAL_NVP(Session), CEREAL_NVP(UserAgent));
            }
        };

        struct ServerRecord {
            std::string Hostname;
            std::string NoReplyAddress;
            std::string RootLoginUrl;
            std::string Url;

        public:
            void ToJson(std::string &out_string) const;
            std::string ToJson() const;

        private:
            friend class cereal::access;
            template<class Archive>
            void serialize(Archive & archive)
            {
                archive(CEREAL_NVP(Hostname), CEREAL_NVP(NoReplyAddress), CEREAL_NVP(RootLoginUrl), CEREAL_NVP(Url));
            }
        };

        struct SubscriptionRecord {
        public:
            enum class Action : char {
                Subscribe = 1,
                Confirm = 2,
                None = 0,
                Unsubscribe = -1,
                Cancel = -2,
            };

            enum class Language : unsigned char {
                En,
                Fa
            };

        public:
            Action Subscribe;
            std::string Inbox;
            std::vector<Language> Languages;
            std::string Uuid;
            std::time_t Timestamp;

        public:
            void ToJson(std::string &out_string) const;
            std::string ToJson() const;

        private:
            friend class cereal::access;
            template<class Archive>
            void serialize(Archive & archive)
            {
                archive(CEREAL_NVP(Subscribe), CEREAL_NVP(Inbox), CEREAL_NVP(Languages), CEREAL_NVP(Uuid), CEREAL_NVP(Timestamp));
            }
        };

        ClientRecord Client;
        ServerRecord Server;
        SubscriptionRecord Subscription;

    public:
        void ToJson(std::string &out_string) const;
        std::string ToJson() const;

    private:
        friend class cereal::access;
        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(Client), CEREAL_NVP(Server), CEREAL_NVP(Subscription));
        }
    };

private:
    struct Impl;
    std::unique_ptr<Impl> m_pimpl;

protected:
    CgiEnv();
public:
    virtual ~CgiEnv();

public:
    const InformationRecord &GetInformation() const;

    void SetSessionRecord(const Service::CgiEnv::InformationRecord::ClientRecord::SessionRecord &record);
    void SetSessionToken(const std::string &token);
    void SetSessionEmail(const std::string &email);

    void AddSubscriptionLanguage(const CgiEnv::InformationRecord::SubscriptionRecord::Language &lang);
    void SetSubscriptionAction(const CgiEnv::InformationRecord::SubscriptionRecord::Action &action);
    void SetSubscriptionInbox(const std::string &inbox);
};


#endif /* SERVICE_CGIENV_HPP */
