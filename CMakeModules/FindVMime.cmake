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


FIND_PATH ( VMIME_INCLUDE_DIR NAMES vmime PATHS /usr/include/ /usr/local/include/ )
FIND_LIBRARY ( VMIME_LIBRARY NAMES vmime PATHS /usr/lib /usr/local/lib ) 


IF ( VMIME_INCLUDE_DIR AND VMIME_LIBRARY )
    SET ( VMIME_FOUND TRUE )
ENDIF (  )


IF ( VMIME_FOUND )
    MESSAGE ( STATUS "Found VMime headers in ${VMIME_INCLUDE_DIR}" )
    MESSAGE ( STATUS "Found VMime library: ${VMIME_LIBRARY}" )
ELSE (  )
    IF ( VMIME_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find VMime" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find VMime" )
    ENDIF (  )
ENDIF (  )


