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


FIND_PATH ( MYSQL_INCLUDE_DIR NAMES mysql PATHS /usr/include /usr/local/include
    NO_DEFAULT_PATH
    NO_CMAKE_ENVIRONMENT_PATH
    NO_CMAKE_PATH
    NO_SYSTEM_ENVIRONMENT_PATH
    NO_CMAKE_SYSTEM_PATH
    CMAKE_FIND_ROOT_PATH_BOTH
    ONLY_CMAKE_FIND_ROOT_PATH
    NO_CMAKE_FIND_ROOT_PATH
)
FIND_LIBRARY ( MYSQL_LIBRARY
                NAMES mysqlclient
                PATHS /usr/lib /usr/local/lib
                PATH_SUFFIXES mysql )


IF ( MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY )
    SET ( MYSQL_FOUND TRUE )
ENDIF (  )


IF ( MYSQL_FOUND )
    MESSAGE ( STATUS "Found MySQL headers in ${MYSQL_INCLUDE_DIR}" )
    MESSAGE ( STATUS "Found MySQL library: ${MYSQL_LIBRARY}" )
ELSE (  )
    IF ( MYSQL_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find MySQL" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find MySQL" )
    ENDIF (  )
ENDIF (  )
