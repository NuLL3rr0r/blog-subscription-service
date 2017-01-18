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

    IF ( DEFINED GREP_EXECUTABLE )
        EXEC_PROGRAM ( ${GREP_EXECUTABLE}
            ARGS "-n \"VMIME_VERSION\" ${VMIME_INCLUDE_DIR}/vmime/config.hpp"
            OUTPUT_VARIABLE VMIME_VERSION_DEFINE_STRING )
        EXEC_PROGRAM ( ${GREP_EXECUTABLE}
            ARGS "-n \"VMIME_API\" ${VMIME_INCLUDE_DIR}/vmime/config.hpp"
            OUTPUT_VARIABLE VMIME_API_DEFINE_STRING )
    ELSE (  )
        MESSAGE ( SEND_ERROR "Could not determine VMime version" )
        MESSAGE ( FATAL_ERROR "grep is required" )
    ENDIF (  )


    IF ( DEFINED PERL_EXECUTABLE )
        EXEC_PROGRAM ( "echo"
            ARGS "${VMIME_VERSION_DEFINE_STRING} | ${PERL_EXECUTABLE} -pe 'if(($_)=/([0-9]+([.][0-9]+)+)/){}'"
            OUTPUT_VARIABLE VMIME_VERSION_RAW_STRING )
        EXEC_PROGRAM ( "echo"
            ARGS "${VMIME_API_DEFINE_STRING} | ${PERL_EXECUTABLE} -pe 'if(($_)=/([0-9]+([.][0-9]+)+)/){}'"
            OUTPUT_VARIABLE VMIME_API_RAW_STRING )
    ELSE (  )
        IF ( DEFINED AWK_EXECUTABLE )
            EXEC_PROGRAM ( "echo"
                ARGS "${VMIME_VERSION_DEFINE_STRING} | ${AWK_EXECUTABLE} '{sub(/-.*/,\"\",$3);print $3}'"
                OUTPUT_VARIABLE VMIME_VERSION_RAW_STRING )
            EXEC_PROGRAM ( "echo"
                ARGS "${VMIME_API_DEFINE_STRING} | ${AWK_EXECUTABLE} '{sub(/-.*/,\"\",$3);print $3}'"
                OUTPUT_VARIABLE VMIME_API_RAW_STRING )
        ELSE (  )
            IF ( DEFINED CUT_EXECUTABLE )
                EXEC_PROGRAM ( "echo"
                    ARGS "${VMIME_VERSION_DEFINE_STRING} | ${CUT_EXECUTABLE} -d' ' -f3 | ${CUT_EXECUTABLE} -d'-' -f1"
                    OUTPUT_VARIABLE VMIME_VERSION_RAW_STRING )
                EXEC_PROGRAM ( "echo"
                    ARGS "${VMIME_API_DEFINE_STRING} | ${CUT_EXECUTABLE} -d' ' -f3 | ${CUT_EXECUTABLE} -d'-' -f1"
                    OUTPUT_VARIABLE VMIME_API_RAW_STRING )
            ELSE (  )
                MESSAGE ( SEND_ERROR "Could not determine VMime version" )
                MESSAGE ( FATAL_ERROR "One of perl, awk, or cut executables is required" )
            ENDIF (  )
        ENDIF (  )
    ENDIF (  )

    IF ( DEFINED PERL_EXECUTABLE )
        EXEC_PROGRAM ( "echo"
            ARGS "${VMIME_VERSION_RAW_STRING} | ${PERL_EXECUTABLE} -pe 's/\\\"//g;s/ //g;s/\\t//g;s/\\R\\z//g;'"
            OUTPUT_VARIABLE VMIME_VERSION_STRING )
        EXEC_PROGRAM ( "echo"
            ARGS "${VMIME_API_RAW_STRING} | ${PERL_EXECUTABLE} -pe 's/\\\"//g;s/ //g;s/\\t//g;s/\\R\\z//g;'"
            OUTPUT_VARIABLE VMIME_API_STRING )
    ELSE (  )
        IF ( DEFINED TR_EXECUTABLE )
            EXEC_PROGRAM ( "echo"
                ARGS "${VMIME_VERSION_RAW_STRING} | ${TR_EXECUTABLE} -d '\"' | ${TR_EXECUTABLE} -d ' ' |  ${TR_EXECUTABLE} -d '\t'  |  ${TR_EXECUTABLE} -d '\r' | ${TR_EXECUTABLE} -d '\n'"
                OUTPUT_VARIABLE VMIME_VERSION_STRING )
            EXEC_PROGRAM ( "echo"
                ARGS "${VMIME_API_RAW_STRING} | ${TR_EXECUTABLE} -d '\"' | ${TR_EXECUTABLE} -d ' ' |  ${TR_EXECUTABLE} -d '\t'  |  ${TR_EXECUTABLE} -d '\r' | ${TR_EXECUTABLE} -d '\n'"
                OUTPUT_VARIABLE VMIME_API_STRING )
        ELSE (  )
            IF ( DEFINED SED_EXECUTABLE )
                EXEC_PROGRAM ( "echo"
                    ARGS "${VMIME_VERSION_RAW_STRING} | ${SED_EXECUTABLE} ':a;N;$!ba;s/[\" \\t\\r\\n]//g'"
                    OUTPUT_VARIABLE VMIME_VERSION_STRING )
                EXEC_PROGRAM ( "echo"
                    ARGS "${VMIME_API_RAW_STRING} | ${SED_EXECUTABLE} ':a;N;$!ba;s/[\" \\t\\r\\n]//g'"
                    OUTPUT_VARIABLE VMIME_API_STRING )
            ELSE (  )
                MESSAGE ( SEND_ERROR "Could not determine VMime version" )
                MESSAGE ( FATAL_ERROR "One of perl, tr, or sed executables is required" )
            ENDIF (  )
        ENDIF (  )
    ENDIF (  )

    STRING ( REPLACE "." ";" VMIME_VERSION_LIST ${VMIME_VERSION_STRING} )
    STRING ( REPLACE "." ";" VMIME_API_LIST ${VMIME_API_STRING} )

    LIST ( LENGTH VMIME_VERSION_LIST VMIME_VERSION_LIST_LENGTH )
    LIST ( LENGTH VMIME_API_LIST VMIME_API_LIST_LENGTH )

    IF ( NOT VMIME_VERSION_LIST_LENGTH EQUAL 3 AND NOT VMIME_API_LIST_LENGTH EQUAL 3 )
        MESSAGE ( FATAL_ERROR "Could not determine VMime version" )
    ENDIF (  )

    LIST ( GET VMIME_VERSION_LIST 0 VMIME_VERSION_MAJOR )
    LIST ( GET VMIME_VERSION_LIST 1 VMIME_VERSION_MINOR )
    LIST ( GET VMIME_VERSION_LIST 2 VMIME_VERSION_PATCH )

    LIST ( GET VMIME_API_LIST 0 VMIME_API_MAJOR )
    LIST ( GET VMIME_API_LIST 1 VMIME_API_MINOR )
    LIST ( GET VMIME_API_LIST 2 VMIME_API_PATCH )

    MESSAGE (STATUS  "VMime version: ${VMIME_VERSION_STRING};\n\tMajor: ${VMIME_VERSION_MAJOR};\n\tMinor: ${VMIME_VERSION_MINOR};\n\tPatch: ${VMIME_VERSION_PATCH};")
    MESSAGE (STATUS  "VMime API: ${VMIME_API_STRING};\n\tMajor: ${VMIME_API_MAJOR};\n\tMinor: ${VMIME_API_MINOR};\n\tPatch: ${VMIME_API_PATCH};")

    SET ( VMIME_LEGACY_API "0" )
    SET ( VMIME_NEW_API "1" )

    IF ( ( ${VMIME_VERSION_MAJOR} GREATER_EQUAL 0 AND ${VMIME_VERSION_MINOR} GREATER_EQUAL 9 AND ${VMIME_VERSION_PATCH} GREATER_EQUAL 2 )
            AND ( ${VMIME_API_MAJOR} GREATER_EQUAL 1 AND ${VMIME_API_MINOR} GREATER_EQUAL 0 AND ${VMIME_API_PATCH} GREATER_EQUAL 0 ) )
        MESSAGE ( STATUS "Using new VMime API" )
        SET ( VMIME_API_MODE ${VMIME_NEW_API} )
    ELSE (  )
        MESSAGE ( STATUS "Using legacy VMime API" )
        SET ( VMIME_API_MODE ${VMIME_LEGACY_API} )
    ENDIF (  )
ELSE (  )
    IF ( VMIME_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find VMime" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find VMime" )
    ENDIF (  )
ENDIF (  )
