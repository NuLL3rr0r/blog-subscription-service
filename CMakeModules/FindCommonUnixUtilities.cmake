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


FIND_FILE ( AWK_EXECUTABLE NAMES awk PATHS /usr/bin /usr/local/bin )
FIND_FILE ( CUT_EXECUTABLE NAMES cut PATHS /usr/bin /usr/local/bin )
FIND_FILE ( GREP_EXECUTABLE NAMES grep PATHS /usr/bin /usr/local/bin )
FIND_FILE ( PERL_EXECUTABLE NAMES perl PATHS /usr/bin /usr/local/bin )
FIND_FILE ( SED_EXECUTABLE NAMES sed PATHS /usr/bin /usr/local/bin )
FIND_FILE ( TR_EXECUTABLE NAMES tr PATHS /usr/bin /usr/local/bin )


IF ( AWK_EXECUTABLE  )
    SET ( AWK_FOUND TRUE )
ENDIF (  )

IF ( CUT_EXECUTABLE  )
    SET ( CUT_FOUND TRUE )
ENDIF (  )

IF ( GREP_EXECUTABLE  )
    SET ( GREP_FOUND TRUE )
ENDIF (  )

IF ( PERL_EXECUTABLE  )
    SET ( PERL_FOUND TRUE )
ENDIF (  )

IF ( SED_EXECUTABLE  )
    SET ( SED_FOUND TRUE )
ENDIF (  )

IF ( TR_EXECUTABLE  )
    SET ( TR_FOUND TRUE )
ENDIF (  )


IF ( AWK_FOUND )
    MESSAGE ( STATUS "Found awk executable: ${AWK_EXECUTABLE}" )
ELSE (  )
    IF ( AWK_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find awk executable" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find awk executable" )
    ENDIF (  )
ENDIF (  )


IF ( CUT_FOUND )
    MESSAGE ( STATUS "Found cut executable: ${CUT_EXECUTABLE}" )
ELSE (  )
    IF ( CUT_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find cut executable" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find cut executable" )
    ENDIF (  )
ENDIF (  )


IF ( GREP_FOUND )
    MESSAGE ( STATUS "Found grep executable: ${GREP_EXECUTABLE}" )
ELSE (  )
    IF ( GREP_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find grep executable" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find grep executable" )
    ENDIF (  )
ENDIF (  )


IF ( PERL_FOUND )
    MESSAGE ( STATUS "Found perl executable: ${PERL_EXECUTABLE}" )
ELSE (  )
    IF ( PERL_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find perl executable" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find perl executable" )
    ENDIF (  )
ENDIF (  )


IF ( SED_FOUND )
    MESSAGE ( STATUS "Found sed executable: ${SED_EXECUTABLE}" )
ELSE (  )
    IF ( SED_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find sed executable" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find sed executable" )
    ENDIF (  )
ENDIF (  )


IF ( TR_FOUND )
    MESSAGE ( STATUS "Found tr executable: ${TR_EXECUTABLE}" )
ELSE (  )
    IF ( TR_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find tr executable" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find tr executable" )
    ENDIF (  )
ENDIF (  )
