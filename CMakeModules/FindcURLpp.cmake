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


FIND_PATH ( CURLPP_INCLUDE_DIR NAMES curlpp PATHS /usr/include/ /usr/local/include/ )
FIND_LIBRARY ( CURLPP_LIBRARY NAMES curlpp PATHS /usr/lib /usr/local/lib ) 


IF ( CURLPP_INCLUDE_DIR AND CURLPP_LIBRARY )
    SET ( CURLPP_FOUND TRUE )
ENDIF (  )


IF ( CURLPP_FOUND )
    MESSAGE ( STATUS "Found cURLpp headers in ${CURLPP_INCLUDE_DIR}" )
    MESSAGE ( STATUS "Found cURLpp library: ${CURLPP_LIBRARY}" )
ELSE (  )
    IF ( CURLPP_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find cURLpp" )
    ENDIF (  )
ENDIF (  )


