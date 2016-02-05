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
 * The subscribe form shown to the end-user.
 */


#include <Wt/WContainerWidget>
#include <Wt/WString>
#include <Wt/WText>
#include <CoreLib/make_unique.hpp>
#include "Div.hpp"
#include "SubscribeForm.hpp"

using namespace std;
using namespace boost;
using namespace Wt;
using namespace Service;

struct SubscribeForm::Impl : public Wt::WObject
{
private:
    SubscribeForm *m_parent;

public:
    Impl(SubscribeForm *parent);
    ~Impl();
};

SubscribeForm::SubscribeForm(CgiRoot *cgi) :
    Page(cgi),
    m_pimpl(make_unique<SubscribeForm::Impl>(this))
{
    m_cgiRoot->setTitle(replace_all_copy(tr("SubscribeForm-page-title").value(), L"&amp;", "&"));

    this->clear();
    this->setId("SubscribeFormPage");
    this->addWidget(Layout());
}

SubscribeForm::~SubscribeForm() = default;

WWidget *SubscribeForm::Layout()
{
    Div *container = new Div("SubscribeForm", "container");
    Div *noScript = new Div(container);
    noScript->addWidget(new WText(tr("no-script")));

    return container;
}

SubscribeForm::Impl::Impl(SubscribeForm *parent)
    : m_parent(parent)
{

}

SubscribeForm::Impl::~Impl() = default;
