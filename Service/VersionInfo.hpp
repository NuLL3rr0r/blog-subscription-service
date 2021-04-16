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
 * Provides application information.
 */


#ifndef SERVICE_VERSIONINFO_HPP
#define SERVICE_VERSIONINFO_HPP


#define     VERSION_INFO_BUILD_COMPILER             PRODUCT_BUILD_COMPILER
#define     VERSION_INFO_BUILD_DATE                 __DATE__ " " __TIME__
#define     VERSION_INFO_BUILD_HOST                 PRODUCT_BUILD_HOST
#define     VERSION_INFO_BUILD_PROCESSOR            PRODUCT_BUILD_PROCESSOR
#define     VERSION_INFO_BUILD_SYSTEM               PRODUCT_BUILD_SYSTEM

#define     VERSION_INFO_PRODUCT_COMPANY_NAME       PRODUCT_COMPANY_NAME
#define     VERSION_INFO_PRODUCT_COPYRIGHT_HOLDER   PRODUCT_COPYRIGHT_HOLDER
#define     VERSION_INFO_PRODUCT_COPYRIGHT_YEAR     PRODUCT_COPYRIGHT_YEAR
#define     VERSION_INFO_PRODUCT_COPYRIGHT          PRODUCT_COPYRIGHT

#define     VERSION_INFO_PRODUCT_INTERNAL_NAME      PRODUCT_INTERNAL_NAME
#define     VERSION_INFO_PRODUCT_NAME               PRODUCT_NAME
#define     VERSION_INFO_PRODUCT_VERSION_MAJOR      PRODUCT_VERSION_MAJOR
#define     VERSION_INFO_PRODUCT_VERSION_MINOR      PRODUCT_VERSION_MINOR
#define     VERSION_INFO_PRODUCT_VERSION_PATCH      PRODUCT_VERSION_PATCH
#define     VERSION_INFO_PRODUCT_VERSION_REVISION   PRODUCT_VERSION_REVISION
#define     VERSION_INFO_PRODUCT_VERSION            PRODUCT_VERSION
#define     VERSION_INFO_PRODUCT_DESCRIPTION        PRODUCT_DESCRIPTION


#endif /* SERVICE_VERSIONINFO_HPP */
