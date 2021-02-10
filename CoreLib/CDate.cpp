/**
 * @file
 * @author  Mamadou Babaei <info@babaei.net>
 * @version 0.1.0
 *
 * @section LICENSE
 *
 * (The MIT License)
 *
 * Copyright (c) 2016 - 2021 Mamadou Babaei
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
 * A Gregorian to Jalali and vice-versa date converter with optional support
 * for arabic and farsi glyphs.
 * Note that it's only accurate over a 33 years time span which is fine by me
 * for now.
 */


#include <sstream>
#include <ctime>
#include <boost/lexical_cast.hpp>
#include <boost/locale.hpp>
#include "CDate.hpp"
#include "make_unique.hpp"

using namespace std;
using namespace boost;
using namespace CoreLib::CDate;

struct Now::Impl
{
    struct tm *TimeInfo;
    CDate::Timezone Timezone;
    time_t RawTime;
    int DaylightSavingTime;
    int DayOfMonth;
    int DayOfWeek;
    int DayOfYear;
    int Hour;
    int Minutes;
    int Month;
    int Seconds;
    int Year;
};

Now::Now(const Timezone &tz)
    : m_pimpl(make_unique<Now::Impl>())
{
    m_pimpl->Timezone = tz;

    time(&m_pimpl->RawTime);
    if (tz == Timezone::UTC) {
        m_pimpl->TimeInfo = gmtime(&m_pimpl->RawTime);
    } else {
        m_pimpl->TimeInfo = localtime(&m_pimpl->RawTime);
    }

    m_pimpl->Hour = m_pimpl->TimeInfo->tm_hour; //  hour (0 - 23)
    m_pimpl->Minutes = m_pimpl->TimeInfo->tm_min; //  minutes (0 - 59)
    /*
    A leap second is a plus or minus one-second adjustment to the Coordinated Universal Time (UTC) time scale that keeps it close to mean solar time.
    When a positive leap second is added at 23:59:60 UTC, it delays the start of the following UTC day (at 00:00:00 UTC) by one second, effectively slowing the UTC clock.
    */
    m_pimpl->Seconds = m_pimpl->TimeInfo->tm_sec != 60 ? m_pimpl->TimeInfo->tm_sec : 59; //  seconds (0 - 60, 60 = Leap second)

    m_pimpl->DayOfWeek = m_pimpl->TimeInfo->tm_wday + 1; //  day of the week (0 - 6, 0 = Sunday)
    m_pimpl->DayOfMonth = m_pimpl->TimeInfo->tm_mday; //  day of the month (1 - 31)
    m_pimpl->DayOfYear = m_pimpl->TimeInfo->tm_yday + 1; //  day of the year (0 - 365)
    m_pimpl->Month = m_pimpl->TimeInfo->tm_mon + 1; //  month (0 - 11, 0 = January)
    m_pimpl->Year = m_pimpl->TimeInfo->tm_year + 1900; //  year since 1900

    m_pimpl->DaylightSavingTime = m_pimpl->TimeInfo->tm_isdst; //  Daylight saving time enabled (> 0), disabled (= 0), or unknown (< 0)
}

Now::~Now() = default;

const struct tm *Now::TimeInfo() const
{
    return m_pimpl->TimeInfo;
}

const CoreLib::CDate::Timezone &Now::TimezoneOffset() const
{
    return m_pimpl->Timezone;
}

time_t Now::RawTime() const
{
    return m_pimpl->RawTime;
}

int Now::DaylightSavingTime() const
{
    return m_pimpl->DaylightSavingTime;
}

int Now::DayOfMonth() const
{
    return m_pimpl->DayOfMonth;
}

int Now::DayOfWeek() const
{
    return m_pimpl->DayOfWeek;
}

int Now::DayOfYear() const
{
    return m_pimpl->DayOfYear;
}

int Now::Hour() const
{
    return m_pimpl->Hour;
}

int Now::Minutes() const
{
    return m_pimpl->Minutes;
}

int Now::Month() const
{
    return m_pimpl->Month;
}

int Now::Seconds() const
{
    return m_pimpl->Seconds;
}

int Now::Year() const
{
    return m_pimpl->Year;
}

std::string DateConv::ToGregorian(const int jYear, const int jMonth, const int jDay)
{
    boost::locale::generator generator;

    std::locale locale_gregorian = generator("en_US.UTF-8");
    std::locale locale_jalali = generator("en_US.UTF-8@calendar=persian");

    boost::locale::date_time jalali(
            boost::locale::period::year(jYear)
            + boost::locale::period::month(jMonth - 1)
            + boost::locale::period::day(jDay),
            locale_jalali);

    boost::locale::date_time gregorian(jalali.time(), locale_gregorian);

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4)
        << gregorian / boost::locale::period::year()
        << "/" << std::setfill('0') << std::setw(2)
        << (gregorian / boost::locale::period::month()) + 1
        << "/" << std::setfill('0') << std::setw(2)
        << gregorian / boost::locale::period::day();

    return ss.str();
}

std::string DateConv::ToGregorian(const CDate::Timezone &tz)
{
    Now n(tz);

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4)
        << n.Year()
        << "/" << std::setfill('0') << std::setw(2)
        << n.Month()
        << "/" << std::setfill('0') << std::setw(2)
        << n.DayOfMonth();

    return ss.str();
}

std::string DateConv::ToGregorian(const CDate::Now &now)
{
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4)
        << now.Year()
        << "/" << std::setfill('0') << std::setw(2)
        << now.Month()
        << "/" << std::setfill('0') << std::setw(2)
        << now.DayOfMonth();

    return ss.str();
}

std::string DateConv::ToJalali(int gYear, int gMonth, int gDay)
{
    boost::locale::generator generator;

    std::locale locale_gregorian = generator("en_US.UTF-8");
    std::locale locale_jalali = generator("en_US.UTF-8@calendar=persian");

    boost::locale::date_time gregorian(
            boost::locale::period::year(gYear)
            + boost::locale::period::month(gMonth - 1)
            + boost::locale::period::day(gDay),
            locale_gregorian);

    boost::locale::date_time jalali(gregorian.time(), locale_jalali);

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4)
        << jalali / boost::locale::period::year()
        << "/" << std::setfill('0') << std::setw(2)
        << (jalali / boost::locale::period::month()) + 1
        << "/" << std::setfill('0') << std::setw(2)
        << jalali / boost::locale::period::day();

    return ss.str();
}

std::string DateConv::ToJalali(const CDate::Timezone &tz)
{
    Now n(tz);

    return DateConv::ToJalali(n.DayOfMonth(), n.Month(), n.Year());
}

std::string DateConv::ToJalali(const CDate::Now &now)
{
    return DateConv::ToJalali(now.DayOfMonth(), now.Month(), now.Year());
}

std::string DateConv::ToJalali(const std::time_t rawTime, const CDate::Timezone &tz)
{
    struct tm *timeInfo;
    if (tz == Timezone::UTC) {
        timeInfo = gmtime(&rawTime);
    } else {
        timeInfo = localtime(&rawTime);
    }

    return DateConv::ToJalali(timeInfo->tm_year + 1900,
            timeInfo->tm_mon + 1, timeInfo->tm_mday);
}

std::string DateConv::Time(const CDate::Now &now)
{
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2)
        << now.Hour()
        << "/" << std::setfill('0') << std::setw(2)
        << now.Minutes()
        << "/" << std::setfill('0') << std::setw(2)
        << now.Seconds();

    return ss.str();
}

std::string DateConv::DateTimeString(const std::time_t rawTime, const CDate::Timezone &tz)
{
    if (tz == CDate::Timezone::UTC) {
        return asctime(gmtime(&rawTime));
    } else {
        return asctime(localtime(&rawTime));
    }
}

std::string DateConv::DateTimeString(const CDate::Now &now)
{
    return asctime(now.TimeInfo());
}

std::wstring DateConv::GetPersianDayOfWeek(const CDate::Now &now)
{
    switch (now.DayOfWeek()) {
    case 7:
        return L"شنبه";
    case 1:
        return L"یکشنبه";
    case 2:
        return L"دوشنبه";
    case 3:
        return L"سه شنبه";
    case 4:
        return L"چهارشنبه";
    case 5:
        return L"پنج شنبه";
    case 6:
        return L"جمعه";
    default:
        break;
    }

    return L"";
}

std::wstring DateConv::FormatToPersianNums(const std::string &date)
{
    wstring res;

    for (size_t i = 0; i < date.length(); ++i) {
        switch(date[i]) {
        case 0x30:
            res += 0x06F0; // ۰
            break;
        case 0x31:
            res += 0x06F1; // ۱
            break;
        case 0x32:
            res += 0x06F2; // ۲
            break;
        case 0x33:
            res += 0x06F3; // ۳
            break;
        case 0x34:
            res += 0x06F4; // ۴
            break;
        case 0x35:
            res += 0x06F5; // ۵
            break;
        case 0x36:
            res += 0x06F6; // ۶
            break;
        case 0x37:
            res += 0x06F7; // ۷
            break;
        case 0x38:
            res += 0x06F8; // ۸
            break;
        case 0x39:
            res += 0x06F9; // ۹
            break;
        default:
            res += date[i];
            break;
        }
    }

    return res;
}

std::wstring DateConv::FormatToPersianNums(const std::wstring &date)
{
    wstring res;

    for (size_t i = 0; i < date.length(); ++i) {
        switch(date[i]) {
        case 0x30:
            res += 0x06F0; // ۰
            break;
        case 0x31:
            res += 0x06F1; // ۱
            break;
        case 0x32:
            res += 0x06F2; // ۲
            break;
        case 0x33:
            res += 0x06F3; // ۳
            break;
        case 0x34:
            res += 0x06F4; // ۴
            break;
        case 0x35:
            res += 0x06F5; // ۵
            break;
        case 0x36:
            res += 0x06F6; // ۶
            break;
        case 0x37:
            res += 0x06F7; // ۷
            break;
        case 0x38:
            res += 0x06F8; // ۸
            break;
        case 0x39:
            res += 0x06F9; // ۹
            break;
        default:
            res += date[i];
            break;
        }
    }

    return res;
}

std::string DateConv::SecondsToHumanReadableTime(const std::time_t seconds)
{
    tm *timeInfo = gmtime(&seconds);

    stringstream ss;

    int years = timeInfo->tm_year - 70; // years since 1900

    if (years > 0) {
        ss << years;
        if (years == 1) {
            ss << " year";
        } else {
            ss << " years";
        }
    }

    int months = timeInfo->tm_mon; // months since January, 0-11

    if (months > 0) {
        if (ss.str().size() > 0) {
            ss << ", ";
        }

        ss << months;
        if (months == 1) {
            ss << " month";
        } else {
            ss << " months";
        }
    }

    int days = timeInfo->tm_mday - 1; // day of the month, 1-31

    if (days > 0) {
        if (ss.str().size() > 0) {
            ss << ", ";
        }

        ss << days;
        if (days == 1) {
            ss << " day";
        } else {
            ss << " days";
        }
    }

    int hours = timeInfo->tm_hour; // hours since midnight, 0-23

    if (hours > 0) {
        if (ss.str().size() > 0) {
            ss << ", ";
        }

        ss << hours;
        if (hours == 1) {
            ss << " hour";
        } else {
            ss << " hours";
        }
    }

    int minutes = timeInfo->tm_min; // minutes after the hour, 0-59

    if (minutes > 0) {
        if (ss.str().size() > 0) {
            ss << ", ";
        }

        ss << minutes;
        if (minutes == 1) {
            ss << " minute";
        } else {
            ss << " minutes";
        }
    }

    int seconds_ = timeInfo->tm_sec; // seconds after the minute, 0-61

    if (seconds_ > 0) {
        if (ss.str().size() > 0) {
            ss << ", ";
        }

        ss << seconds_;
        if (seconds_ == 1) {
            ss << " second";
        } else {
            ss << " seconds";
        }
    }

    return ss.str();
}
