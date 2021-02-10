#  (The MIT License)
#
#  Copyright (c) 2016 - 2021 Mamadou Babaei
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

find_program(AWK_PROGRAM awk)
find_program(CCACHE_PROGRAM ccache)
find_program(GREP_PROGRAM grep)
find_program(SH_PROGRAM sh)

if(AWK_PROGRAM AND CCACHE_PROGRAM AND GREP_PROGRAM AND SH_PROGRAM)
    message(STATUS "Found ccache: ${CCACHE_PROGRAM}")

    execute_process(COMMAND
        ${SH_PROGRAM} -c "${CCACHE_PROGRAM} -s | ${GREP_PROGRAM} \"cache directory\" | ${AWK_PROGRAM} '{print \$3}'"
        RESULT_VARIABLE CCACHE_CACHE_DIR_RESULT
        OUTPUT_VARIABLE CCACHE_CACHE_DIR)

    string(REGEX REPLACE "\n$" "" CCACHE_CACHE_DIR "${CCACHE_CACHE_DIR}")

    if(CCACHE_CACHE_DIR_RESULT EQUAL 0)
        message(STATUS "ccache directory: ${CCACHE_CACHE_DIR}")
    else()
        message(STATUS "Could not get ccache directory")
    endif()

    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_PROGRAM})
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE_PROGRAM})
else()
    message(STATUS "Could not find ccache")
    set(CCACHE_PROGRAM      "")
    set(CCACHE_CACHE_DIR    "")
endif()
