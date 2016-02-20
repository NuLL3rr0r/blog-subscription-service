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
 * General settings such as website title, url, etc.
 */


#include <cppdb/frontend.h>
#include <boost/exception/diagnostic_information.hpp>
#include <Wt/WApplication>
#include <Wt/WString>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WWidget>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Log.hpp>
#include "CgiEnv.hpp"
#include "CgiRoot.hpp"
#include "CmsSettings.hpp"
#include <CoreLib/Database.hpp>
#include "Div.hpp"
#include "Pool.hpp"

using namespace std;
using namespace boost;
using namespace cppdb;
using namespace Wt;
using namespace CoreLib;
using namespace Service;

struct CmsSettings::Impl : public Wt::WObject
{
public:
    Impl();
    ~Impl();
};

CmsSettings::CmsSettings()
    : Page(),
    m_pimpl(make_unique<CmsSettings::Impl>())
{
    this->clear();
    this->setId("CmsSettingsPage");
    this->addWidget(this->Layout());
}

CmsSettings::~CmsSettings() = default;

WWidget *CmsSettings::Layout()
{
    Div *container = new Div("CmsSettings", "container-fluid");

    try {
        CgiRoot *cgiRoot = static_cast<CgiRoot *>(WApplication::instance());
        CgiEnv *cgiEnv = cgiRoot->GetCgiEnvInstance();

        string htmlData;
        string file;
        if (cgiEnv->GetCurrentLanguage() == CgiEnv::Language::Fa) {
            file = "../templates/cms-settings-fa.wtml";
        } else {
            file = "../templates/cms-settings.wtml";
        }

        if (CoreLib::FileSystem::Read(file, htmlData)) {
            /// Fill the template
            WTemplate *tmpl = new WTemplate(container);
            tmpl->setTemplateText(WString::fromUTF8(htmlData), TextFormat::XHTMLUnsafeText);

            tmpl->bindWidget("settings-title", new WText(tr("cms-settings-page-title")));
        }
    }

    catch (boost::exception &ex) {
        LOG_ERROR(boost::diagnostic_information(ex));
    }

    catch (std::exception &ex) {
        LOG_ERROR(ex.what());
    }

    catch (...) {
        LOG_ERROR(UNKNOWN_ERROR);
    }

    return container;
}

CmsSettings::Impl::Impl()
{

}

CmsSettings::Impl::~Impl() = default;

