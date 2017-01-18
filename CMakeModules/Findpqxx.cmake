#  (The MIT License)
#
#  Copyright (c) 2016 Mohammad S. Babaei
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


FIND_PATH ( PQXX_INCLUDE_DIR NAMES pqxx PATHS /usr/include/ /usr/local/include/ )
FIND_LIBRARY ( PQXX_LIBRARY
                NAMES pqxx
                PATHS /usr/lib /usr/local/lib )


IF ( PQXX_INCLUDE_DIR AND PQXX_LIBRARY )
    SET ( PQXX_FOUND TRUE )
ENDIF (  )  


IF ( PQXX_FOUND )
    MESSAGE ( STATUS "Found pqxx headers in ${PQXX_INCLUDE_DIR}" )
    MESSAGE ( STATUS "Found pqxx library: ${PQXX_LIBRARY}" )
ELSE (  )
    IF ( PQXX_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find pqxx" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find pqxx" )
    ENDIF (  )
ENDIF (  )
