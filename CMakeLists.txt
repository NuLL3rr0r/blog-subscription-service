#  (The MIT License)
#
#  Copyright (c) 2016 - 2020 Mamadou Babaei
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in
#  all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#  THE SOFTWARE.


cmake_minimum_required(VERSION 3.1...3.17)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMakeModules" ${CMAKE_MODULE_PATH})

include(ccache-initialize)

project(BLOG_SUBSCRIPTION_SERVICE
    VERSION 0.9.0
    DESCRIPTION "A multilingual blog subscription service written in C++ on top of Wt webtoolkit"
    HOMEPAGE_URL "https://github.com/NuLL3rr0r/blog-subscription-service"
    LANGUAGES C CXX
)

include(ccache-finalize)

include(CheckIPOSupported)
include(FeatureSummary)

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

check_ipo_supported(RESULT IS_IPO_SUPPORTED)
if (IS_IPO_SUPPORTED)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
endif()

site_name(BUILD_INFO_HOST)

if (NOT WIN32)
    set(BUILD_INFO_HOST "$ENV{USER}@${BUILD_INFO_HOST}")
else ()
    set(BUILD_INFO_HOST "$ENV{USERNAME}@${BUILD_INFO_HOST}")
endif ()

include(default-build-type)
include(dependencies)
include(build-config)
include(product-info)
include(definitions)
include(deploy)
include(resources)

include(cotire)

include_directories(.)

add_subdirectory(CoreLib)
add_subdirectory(Utils)
add_subdirectory(Service)
