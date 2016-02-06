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
 * A Gregorian to Jalali and vice-versa date converter with optional support
 * for arabic and farsi glyphs.
 * Note that it's only accurate over a 33 years time span which is fine by me
 * for now.
 */


#include <sstream>
#include <ctime>
#include <boost/lexical_cast.hpp>
#include "CDate.hpp"

using namespace std;
using namespace boost;
using namespace CoreLib::CDate;

Now::Now()
{
    time(&RawTime);
    TimeInfo = localtime(&RawTime);
    // Full date+time
    //asctime(timeInfo);

    Hour = TimeInfo->tm_hour; //  hour (0 - 23)
    DaylightSavingTime = TimeInfo->tm_isdst; //  Daylight saving time enabled (> 0), disabled (= 0), or unknown (< 0)
    DayOfMonth = TimeInfo->tm_mday; //  day of the month (1 - 31)
    Minutes = TimeInfo->tm_min; //  minutes (0 - 59)
    Month = TimeInfo->tm_mon + 1; //  month (0 - 11, 0 = January)
    Seconds = TimeInfo->tm_sec != 60 ? TimeInfo->tm_sec : 59; //  seconds (0 - 60, 60 = Leap second)
    /*
    A leap second is a plus or minus one-second adjustment to the Coordinated Universal Time (UTC) time scale that keeps it close to mean solar time.
    When a positive leap second is added at 23:59:60 UTC, it delays the start of the following UTC day (at 00:00:00 UTC) by one second, effectively slowing the UTC clock.
    */
    DayOfWeek = TimeInfo->tm_wday + 1; //  day of the week (0 - 6, 0 = Sunday)
    DayOfYear = TimeInfo->tm_yday + 1; //  day of the year (0 - 365)
    Year = TimeInfo->tm_year + 1900; //  year since 1900
}

Now::~Now() = default;

std::string DateConv::IntToStr(int num)
{
    return lexical_cast<string>(num);
}

std::string DateConv::CalcToG(int jYear, int dayOfYear)
{
    bool isLeapYear = IsLeapYearJ(jYear);
    int dayMatch[13] = { !isLeapYear ? 287 : 288, !isLeapYear ? 318 : 319, !isLeapYear && !IsLeapYearJ(jYear + 1) ? 346 : 347, !isLeapYear ? 12 : 13, !isLeapYear ? 42 : 43, !isLeapYear ? 73 : 74, !isLeapYear ? 103 : 104, !isLeapYear ? 134 : 135, !isLeapYear ? 165 : 166, !isLeapYear ? 195 : 196, !isLeapYear ? 226 : 227, !isLeapYear ? 256 : 257, 999 };

    string gDay;
    string gMonth;

    for (int i = 0; i < 12; ++i)
        if ((dayOfYear >= dayMatch[i] && dayOfYear < dayMatch[i + 1]) || ((dayOfYear >= dayMatch[i] || dayOfYear < dayMatch[i + 1]) && (i == 2))) {
            gDay = IntToStr(dayOfYear >= dayMatch[i] ? dayOfYear - dayMatch[i] + 1 : !isLeapYear ? dayOfYear + 20 : dayOfYear + 19);
            gMonth = IntToStr(i + 1);
            break;
        }

    return IntToStr(dayOfYear < dayMatch[0] ? jYear + 621 : jYear + 622) + "/" +
            (gMonth.size() == 1 ? "0" + gMonth : gMonth)
            + "/" +
            (gDay.size() != 1 ? gDay : "0" + gDay);
}

std::string DateConv::CalcToJ(int gYear, int dayOfYear)
{
    bool isLeapYear = IsLeapYearG(gYear - 1);
    int dayMatch[13] = { 80, 111, 142, 173, 204, 235, 266, 296, 326, 356, !isLeapYear ? 21 : 20, !isLeapYear ? 51 : 50, 999 };

    string jDay;
    string jMonth;

    for (int i = 0; i < 12; ++i)
        if ((dayOfYear >= dayMatch[i] && dayOfYear < dayMatch[i + 1]) || ((dayOfYear >= dayMatch[i] || dayOfYear < dayMatch[i + 1]) && (i == 9))) {
            jDay = IntToStr(dayOfYear >= dayMatch[i] ? dayOfYear - dayMatch[i] + 1 : !isLeapYear ? dayOfYear + 10 : dayOfYear + 11);
            jMonth = IntToStr(i + 1);
            break;
        }

    return IntToStr(dayOfYear > 79 ? gYear - 621 : gYear - 622) + "/" +
            (jMonth.size() == 1 ? "0" + jMonth : jMonth)
            + "/" +
            (jDay.size() != 1 ? jDay : "0" + jDay);
}

bool DateConv::IsRangeValidG(int gYear, int gMonth, int gDay)
{
    int gMonths[12] = { 31, !IsLeapYearG(gYear) ? 28 : 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if ((gYear < 10000) && (gYear > -10000))
        if ((gMonth < 13) && (gMonth > 0))
            if ((gDay <= gMonths[gMonth - 1]) && (gDay > 0))
                return true;

    return false;
}

bool DateConv::IsRangeValidJ(int jYear, int jMonth, int jDay)
{
    int jMonths[12] = { 31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, !IsLeapYearJ(jYear) ? 29 : 30 };

    if ((jYear < 10000) && (jYear > -100))
        if ((jMonth < 13) && (jMonth > 0))
            if ((jDay <= jMonths[jMonth - 1]) && (jDay > 0))
                return true;

    return false;
}

int DateConv::DayOfYearG(int gYear, int gMonth, int gDay)
{
    int gMonths[12] = { 31, !IsLeapYearG(gYear) ? 28 : 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int result = 0;

    for (int i = 0; i < gMonth - 1; ++i)
        result += gMonths[i];

    return result + gDay;
}

int DateConv::DayOfYearJ(int jYear, int jMonth, int jDay)
{
    int jMonths[12] = { 31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, !IsLeapYearJ(jYear) ? 29 : 30 };
    int result = 0;

    for (int i = 0; i < jMonth - 1; ++i)
        result += jMonths[i];

    return result + jDay;
}

bool DateConv::IsLeapYearG(int gYear)
{
    int modulus = gYear % 4;

    return gYear > 0 && modulus == 0 ? true : gYear < 0 && modulus == 0 ? true : gYear == 0 ? true : false;
}

bool DateConv::IsLeapYearJ(int jYear)
{
    int modulus = jYear % 4;

    return jYear > 0 && modulus == 3 ? true : jYear < 0 && modulus == -1 ? true : false;
}

std::string DateConv::ToGregorian(int jYear, int jMonth, int jDay)
{
    if (!IsRangeValidJ(jYear, jMonth, jDay))
        return "";

    return CalcToG(jYear, DayOfYearJ(jYear, jMonth, jDay));
}

std::string DateConv::ToGregorian()
{
    Now n;

    string m = IntToStr(n.Month);
    string d = IntToStr(n.DayOfMonth);

    return IntToStr(n.Year) + "/" +
            (m.size() == 1 ? "0" + m : m)
            + "/" +
            (d.size() != 1 ? d : "0" + d);
}

std::string DateConv::ToJalali(int gYear, int gMonth, int gDay)
{
    if (!IsRangeValidG(gYear, gMonth, gDay))
        return "";

    return CalcToJ(gYear, DayOfYearG(gYear, gMonth, gDay));
}

std::string DateConv::ToJalali()
{
    Now n;
    return CalcToJ(n.Year, n.DayOfYear);
}

std::string DateConv::ToGregorian(const CDate::Now &now)
{
    string m = IntToStr(now.Month);
    string d = IntToStr(now.DayOfMonth);

    return IntToStr(now.Year) + "/" +
            (m.size() == 1 ? "0" + m : m)
            + "/" +
            (d.size() != 1 ? d : "0" + d);
}

std::string DateConv::ToJalali(const CDate::Now &now)
{
    return CalcToJ(now.Year, now.DayOfYear);
}

std::string DateConv::Time(const CDate::Now &now)
{
    string s = IntToStr(now.Seconds);
    string m = IntToStr(now.Minutes);
    string h = IntToStr(now.Hour);

    return (h.size() == 1 ? "0" + h : h) + ":" +
            (m.size() == 1 ? "0" + m : m)
            + ":" +
            (s.size() != 1 ? s : "0" + s);
}

std::string DateConv::RawLocalDateTime(const CDate::Now &now)
{
    return asctime(now.TimeInfo);
}

std::wstring DateConv::GetPersianDayOfWeek(const CDate::Now &now)
{
    switch (now.DayOfWeek) {
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

