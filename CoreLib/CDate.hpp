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


#ifndef CORELIB_CDATE_HPP
#define CORELIB_CDATE_HPP


#include <string>
#include <ctime>

namespace CoreLib {
namespace CDate {
class DateConv;
class Now;
}
}

class CoreLib::CDate::DateConv
{
public:
    static std::string CalcToG(int jYear, int dayOfYear);
    static std::string CalcToJ(int gYear, int dayOfYear);
    static bool IsRangeValidG(int gYear, int gMonth, int gDay);
    static bool IsRangeValidJ(int jYear, int jMonth, int jDay);
    static int DayOfYearG(int gYear, int gMonth, int gDay);
    static int DayOfYearJ(int jYear, int jMonth, int jDay);
    static bool IsLeapYearG(int gYear);
    static bool IsLeapYearJ(int jYear);
    static std::string ToGregorian(int jYear, int jMonth, int jDay);
    static std::string ToGregorian();
    static std::string ToJalali(int gYear, int gMonth, int gDay);
    static std::string ToJalali();
    static std::string ToGregorian(const CDate::Now &now);
    static std::string ToJalali(const CDate::Now &now);
    static std::string Time(const CDate::Now &now);
    static std::string RawLocalDateTime(const CDate::Now &now);
    static std::wstring GetPersianDayOfWeek(const CDate::Now &now);
    static std::wstring FormatToPersianNums(const std::string &date);
    static std::wstring FormatToPersianNums(const std::wstring &date);

private:
    static std::string IntToStr(int num);
};

class CoreLib::CDate::Now
{
public:
    Now();

    time_t RawTime;
    int Hour;
    int DaylightSavingTime;
    int DayOfMonth;
    int Minutes;
    int Month;
    int Seconds;
    int DayOfWeek;
    int DayOfYear;
    int Year;
    struct tm *TimeInfo;
};


#endif /* CORELIB_CDATE_HPP */

