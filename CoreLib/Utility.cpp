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
 * Common utility funtions.
 */


#include <vector>
#include <boost/format.hpp>
#include "Utility.hpp"

using namespace std;
using namespace boost;
using namespace CoreLib;

std::string Utility::CalculateSize(const std::size_t size)
{
    static const vector<std::string> units { "EB", "PB", "TB", "GB", "MB", "KB", "B" };
    static const size_t exbibytes = 1024ull * 1024ull * 1024ull * 1024ull * 1024ull * 1024ull;

    size_t multiplier = exbibytes;
    string result;

    if (size != 0) {
        for (size_t i = 0; i < units.size(); i++, multiplier /= 1024) {
            if (size < multiplier)
                continue;

            if (size % multiplier == 0) {
                result.assign((format("%s %s") % (size / multiplier) % units[i]).str());
            } else {
                result.assign((format("%.2f %s") % ((float)size / (float)multiplier) % units[i]).str());
            }

            return result;
        }
    }

    result.assign("0");
    return result;
}

