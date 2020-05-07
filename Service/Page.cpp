/**
 * @file
 * @author  Mamadou Babaei <info@babaei.net>
 * @version 0.1.0
 *
 * @section LICENSE
 *
 * (The MIT License)
 *
 * Copyright (c) 2016 - 2020 Mamadou Babaei
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
 * An abstract base class for each web page in the application.
 */


#include<Wt/WApplication>
#include<Wt/WFormWidget>
#include<Wt/WText>
#include <Wt/WWidget>
#include "Page.hpp"

using namespace std;
using namespace Wt;
using namespace Service;

Page::Page() : WContainerWidget()
{

}

Page::~Page() = default;

bool Page::Validate(WFormWidget *widget) const
{
    switch (widget->validate()) {
    case WValidator::Valid:
        return true;
    case WValidator::InvalidEmpty:
        break;
    case WValidator::Invalid:
        break;
    }

    widget->setFocus();
    return false;
}

void Page::HtmlError(const string& err, WText *txt) const
{
    txt->setText(err);
    txt->setStyleClass("boldTextErr");
}

void Page::HtmlError(const wstring& err, WText *txt) const
{
    txt->setText(err);
    txt->setStyleClass("boldTextErr");
}

void Page::HtmlInfo(const string& msg, WText *txt) const
{
    txt->setText(msg);
    txt->setStyleClass("boldText");
}

void Page::HtmlInfo(const wstring& msg, WText *txt) const
{
    txt->setText(msg);
    txt->setStyleClass("boldText");
}
