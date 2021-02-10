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


#ifndef CORELIB_CDATE_HPP
#define CORELIB_CDATE_HPP


#include <memory>
#include <string>
#include <ctime>

namespace CoreLib {
namespace CDate {
enum class Timezone : unsigned char {
    Local,
    UTC
};

class Now;
class DateConv;
}
}

class CoreLib::CDate::Now
{

private:
    struct Impl;
    std::unique_ptr<Impl> m_pimpl;

public:
    Now(const CDate::Timezone &tz);
    virtual ~Now();

public:
    const struct tm *TimeInfo() const;
    const Timezone &TimezoneOffset() const;
    time_t RawTime() const;
    int DaylightSavingTime() const;
    int DayOfMonth() const;
    int DayOfWeek() const;
    int DayOfYear() const;
    int Hour() const;
    int Minutes() const;
    int Month() const;
    int Seconds() const;
    int Year() const;
};


class CoreLib::CDate::DateConv
{
public:
    static std::string ToGregorian(const int jYear, const int jMonth, const int jDay);
    static std::string ToGregorian(const CDate::Timezone &tz = CDate::Timezone::Local);
    static std::string ToGregorian(const CDate::Now &now);
    static std::string ToJalali(const int gYear, const int gMonth, const int gDay);
    static std::string ToJalali(const CDate::Timezone &tz = CDate::Timezone::Local);
    static std::string ToJalali(const CDate::Now &now);
    static std::string ToJalali(const std::time_t rawTime, const CDate::Timezone &tz = CDate::Timezone::Local);
    static std::string Time(const CDate::Now &now);
    static std::string DateTimeString(const std::time_t rawTime, const CDate::Timezone &tz);
    static std::string DateTimeString(const CDate::Now &now);
    static std::wstring GetPersianDayOfWeek(const CDate::Now &now);
    static std::wstring FormatToPersianNums(const std::string &date);
    static std::wstring FormatToPersianNums(const std::wstring &date);
    static std::string SecondsToHumanReadableTime(const std::time_t seconds);
};


#endif /* CORELIB_CDATE_HPP */
