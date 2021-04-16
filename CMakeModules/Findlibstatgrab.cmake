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


FIND_PATH ( LIBSTATGRAB_INCLUDE_DIR NAMES statgrab.h PATHS /usr/include/ /usr/local/include/ )
FIND_LIBRARY ( LIBSTATGRAB_LIBRARY NAMES statgrab PATHS /usr/lib /usr/local/lib )
IF (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    FIND_LIBRARY ( DEVSTAT_LIBRARY NAMES devstat PATHS /usr/lib /usr/local/lib )
ENDIF (  )


IF ( LIBSTATGRAB_INCLUDE_DIR AND LIBSTATGRAB_LIBRARY )
    SET ( LIBSTATGRAB_FOUND TRUE )
ENDIF (  )

IF (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    IF ( DEVSTAT_LIBRARY )
        SET ( DEVSTAT_FOUND TRUE )
    ENDIF (  )
ENDIF (  )


IF ( LIBSTATGRAB_FOUND )
    MESSAGE ( STATUS "Found libstatgrab headers in ${LIBSTATGRAB_INCLUDE_DIR}" )
    MESSAGE ( STATUS "Found libstatgrab library: ${LIBSTATGRAB_LIBRARY}" )
ELSE (  )
    IF ( LIBSTATGRAB_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find libstatgrab" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find libstatgrab" )
    ENDIF (  )
ENDIF (  )

IF (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    IF ( DEVSTAT_FOUND )
        MESSAGE ( STATUS "Found devstat library: ${DEVSTAT_LIBRARY}" )
    ELSE (  )
        IF ( DEVSTAT_FIND_REQUIRED )
            MESSAGE ( FATAL_ERROR "Could not find devstat" )
        ELSE (  )
            MESSAGE ( STATUS "Could not find devstat" )
        ENDIF (  )
    ENDIF (  )
ENDIF (  )
