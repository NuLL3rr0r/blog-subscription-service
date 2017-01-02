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
 * Provides basic captcha generation and validation.
 */


#include <list>
#include <Wt/WImage>
#include <Wt/WMemoryResource>
#if MAGICKPP_BACKEND == MAGICKPP_GM
#include <GraphicsMagick/Magick++.h>
#elif MAGICKPP_BACKEND == MAGICKPP_IM
#include <ImageMagick-6/Magick++.h>
#endif // MAGICKPP_BACKEND == MAGICKPP_GM
#include <CoreLib/make_unique.hpp>
#include <CoreLib/FileSystem.hpp>
#include <CoreLib/Random.hpp>
#include <CoreLib/System.hpp>
#include "Captcha.hpp"

using namespace std;
using namespace boost;
using namespace Magick;
using namespace Wt;
using namespace CoreLib;
using namespace Service;

struct Captcha::Impl
{
public:
    std::size_t Result;
};

Captcha::Captcha()
    : m_pimpl(make_unique<Captcha::Impl>())
{
    m_pimpl->Result = 0;
}

Captcha::~Captcha() = default;

Wt::WImage *Captcha::Generate()
{
    size_t n1 = static_cast<size_t>(Random::Number(1, 10));
    size_t n2 = static_cast<size_t>(Random::Number(1, 10));
    int rotate = Random::Number(-3, 3);
    int skew = Random::Number(-4, 4);

    m_pimpl->Result = n1 * n2;

    string captcha(lexical_cast<string>(n1));
    captcha += " X ";
    captcha += lexical_cast<string>(n2);

    Image img(Geometry(115, 35), Color("white"));
    list<Drawable> drawList;

    drawList.push_back(DrawableTextAntialias(true));
    drawList.push_back(DrawableFont("../fonts/hazeln.ttf"));
    drawList.push_back(DrawablePointSize(32));
    drawList.push_back(DrawableStrokeColor(Color("black")));
    drawList.push_back(DrawableFillColor(Color(0, 0, 0, MaxRGB)));
    drawList.push_back(DrawableTextDecoration(UnderlineDecoration));
    drawList.push_back(DrawableGravity(CenterGravity));


    drawList.push_back(DrawableRotation(rotate));
    drawList.push_back(DrawableRotation(skew));
    drawList.push_back(DrawableText(0, 0, captcha));

    img.draw(drawList);


    string captchaPath;

    do {
        captchaPath = "../tmp/captcha-";
        captchaPath += Random::Characters(Random::Character::Alphanumeric, 24);
        captchaPath += ".png";
    } while (FileSystem::FileExists(captchaPath));


    img.write(captchaPath);

    WMemoryResource *captchaResource = new WMemoryResource("image/png");

    string buffer;
    FileSystem::Read(captchaPath, buffer);

    captchaResource->setData(reinterpret_cast<const unsigned char*>(buffer.c_str()),
                             static_cast<int>(buffer.size()));

    FileSystem::Erase(captchaPath);

    WImage *captchaImage = new WImage(captchaResource, "Captcha");
    captchaImage->setStyleClass("captcha");

    return captchaImage;
}

std::size_t Captcha::GetResult() const
{
    return m_pimpl->Result;
}
