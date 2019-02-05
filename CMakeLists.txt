#  (The MIT License)
#
#  Copyright (c) 2016 - 2019 Mamadou Babaei
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


CMAKE_MINIMUM_REQUIRED ( VERSION 3.7.1 FATAL_ERROR )

IF ( NOT ${CMAKE_VERSION} VERSION_LESS 3.0 )
    IF ( POLICY CMP0026 )
        CMAKE_POLICY ( SET CMP0026 OLD )
    ENDIF(  )
    IF ( POLICY CMP0045 )
        CMAKE_POLICY ( SET CMP0045 OLD )
    ENDIF(  )
ENDIF (  )

IF ( POLICY CMP0058)
    CMAKE_POLICY ( SET CMP0058 NEW)
ENDIF (  )

PROJECT ( BLOG_SUBSCRIPTION_SERVICE )
SET ( CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules" "${CMAKE_MODULE_PATH}" )


SET ( SUPPORTED_PLATFORM "" )
SET ( SUPPORTED_COMPILER "" )
SET ( CXX_COMPILE_MODE "" )
SET ( CXX_GCC "" )
SET ( CXX_MINGW "" )
SET ( CXX_CLANG "" )
SET ( CXX_MSVC "" )
SET ( COMPILER "" )
SET ( COMPILER_NAME "" )
SET ( COMPILER_VERSION "" )

SET ( CXX_COMPILE_MODE_98 "1" )
SET ( CXX_COMPILE_MODE_03 "1" ) # Deliberately set to 1 due to the fact that it won't make any difference in the current project
SET ( CXX_COMPILE_MODE_0x "3" )
SET ( CXX_COMPILE_MODE_11 "4" )
SET ( CXX_COMPILE_MODE_1y "5" )
SET ( CXX_COMPILE_MODE_14 "6" )
SET ( CXX_COMPILE_MODE_1z "7" )


MESSAGE ( STATUS "Checking if your OS (${CMAKE_SYSTEM}) is supported for building" )

IF ( UNIX )
    IF ( ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD" )
        # FreeBSD
        SET ( SUPPORTED_PLATFORM "YES" )
    ELSEIF ( ${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD" )
        # OpenBSD
        SET ( SUPPORTED_PLATFORM "YES" )
    ELSEIF ( ${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" )
        # NetBSD
        SET ( SUPPORTED_PLATFORM "YES" )
    ELSEIF ( ${CMAKE_SYSTEM_NAME} MATCHES "DragonFly" )
        # DragonFly BSD
        SET ( SUPPORTED_PLATFORM "YES" )
    ELSEIF ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
        # Linux
        SET ( SUPPORTED_PLATFORM "YES" )
    ELSEIF ( APPLE )
        IF ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
            # Apple Mac OS X
            SET ( SUPPORTED_PLATFORM "YES" )
        ELSE (  )
            # Unsupported
            SET ( SUPPORTED_PLATFORM "NO" )
        ENDIF (  )
    ENDIF (  )
ELSEIF ( WIN32 )
    IF ( CYGWIN )
        # CygWin
        SET ( SUPPORTED_PLATFORM "NO" )
    ELSE (  )
        # Microsoft Windows
        SET ( SUPPORTED_PLATFORM "YES" )
    ENDIF (  )
ELSE (  )
    # Unsupported OS
    SET ( SUPPORTED_PLATFORM "NO" )
ENDIF (  )


IF ( SUPPORTED_PLATFORM )
    MESSAGE ( STATUS "Checking if your OS (${CMAKE_SYSTEM}) is supported for building -- yes" )
ELSE (  )
    MESSAGE ( STATUS "Checking if your OS (${CMAKE_SYSTEM}) is supported for building -- no" )
    MESSAGE ( FATAL_ERROR "\nUnfortunately your system doesn't meet the minimum requirements for building."
    "\nThe only supported platforms are *BSD, Linux, Windows and Mac OS X." )
ENDIF (  )


MESSAGE ( STATUS "Checking if your compiler (${CMAKE_CXX_COMPILER}) is supported for building" )


IF ( CMAKE_COMPILER_IS_GNUCXX )
    EXEC_PROGRAM ( "${CMAKE_C_COMPILER}" ARGS "-dumpversion" OUTPUT_VARIABLE GCC_VERSION_INFO )
    STRING ( REGEX REPLACE "^([0-9]+).*$"                   "\\1" GCC_MAJOR "${GCC_VERSION_INFO}" )
    STRING ( REGEX REPLACE "^[0-9]+\\.([0-9]+).*$"          "\\1" GCC_MINOR "${GCC_VERSION_INFO}" )
    STRING ( REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" GCC_PATCH "${GCC_VERSION_INFO}" )

    IF ( GCC_MAJOR MATCHES "\\.+" )
        SET ( GCC_MAJOR "" )
    ENDIF (  )
    IF ( GCC_MINOR MATCHES "\\.+" )
        SET ( GCC_MINOR "" )
    ENDIF (  )
    IF ( GCC_PATCH MATCHES "\\.+" )
        SET ( GCC_PATCH "" )
    ENDIF (  )

    SET ( CXX_GCC "YES" )

    IF ( NOT MINGW )
        SET ( COMPILER_NAME "gcc" )
    ELSE (  )
        SET ( CXX_MINGW "YES" )
        SET ( COMPILER_NAME "mingw" )
    ENDIF (  )

    SET ( COMPILER_VERSION "${GCC_MAJOR}.${GCC_MINOR}.${GCC_PATCH}" )

    SET ( COMPILER "${COMPILER_NAME}-${COMPILER_VERSION}" )
ELSE (  )
    SET ( GCC_MAJOR 0 )
    SET ( GCC_MINOR 0 )
    SET ( GCC_PATCH 0 )
    SET ( CXX_GCC "NO" )
ENDIF (  )

IF ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
    EXEC_PROGRAM ( "${CMAKE_C_COMPILER}" ARGS "-v" OUTPUT_VARIABLE CLANG_VERSION_INFO )
    STRING ( REGEX REPLACE "^.*[ ]([0-9]+)\\.[0-9].*$" "\\1" CLANG_MAJOR "${CLANG_VERSION_INFO}" )
    STRING ( REGEX REPLACE "^.*[ ][0-9]+\\.([0-9]).*$" "\\1" CLANG_MINOR "${CLANG_VERSION_INFO}" )
    STRING ( REGEX REPLACE "^.*[ ][0-9]+\\.[0-9]+\\.([0-9]).*$" "\\1" CLANG_PATCH "${CLANG_VERSION_INFO}" )

    SET ( CXX_CLANG "YES" )
    SET ( COMPILER_NAME "clang" )
    SET ( COMPILER_VERSION "${CLANG_MAJOR}.${CLANG_MINOR}.${CLANG_PATCH}" )
    SET ( COMPILER "${COMPILER_NAME}-${COMPILER_VERSION}" )
ELSE (  )
    SET ( CLANG_MAJOR 0 )
    SET ( CLANG_MINOR 0 )
    SET ( CXX_CLANG "NO" )
ENDIF (  )

IF ( WIN32 AND MSVC )
    EXEC_PROGRAM ( "${CMAKE_C_COMPILER}" ARGS "" OUTPUT_VARIABLE MSVC_VERSION_INFO )
    STRING ( REGEX REPLACE "^.*[ ]([0-9]+)\\.[0-9].*$" "\\1" MSVC_MAJOR "${MSVC_VERSION_INFO}" )
    STRING ( REGEX REPLACE "^.*[ ][0-9]+\\.([0-9]).*$" "\\1" MSVC_MINOR "${MSVC_VERSION_INFO}" )
    STRING ( REGEX REPLACE "^.*[ ][0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" MSVC_BUILD "${MSVC_VERSION_INFO}" )
    STRING ( REGEX REPLACE "^.*[ ][0-9]+\\.[0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" MSVC_REVISION "${MSVC_VERSION_INFO}" )

    SET ( CXX_MSVC "YES" )
    SET ( COMPILER_NAME "msvc" )
    SET ( COMPILER_VERSION "${MSVC_MAJOR}.${MSVC_MINOR}.${MSVC_BUILD}.${MSVC_REVISION}" )
    SET ( COMPILER "${COMPILER_NAME}-${COMPILER_VERSION}" )    
ELSE (  )
    SET ( MSVC_MAJOR 0 )
    SET ( MSVC_MINOR 0 )
    SET ( MSVC_BUILD 0 )
    SET ( MSVC_REVISION 0 )
    SET ( CXX_MSVC "NO" )
ENDIF (  )


IF ( CXX_GCC )
    IF ( GCC_MAJOR GREATER 4 )
        SET ( SUPPORTED_COMPILER "YES" )
    ELSEIF ( GCC_MAJOR EQUAL 4 )
        IF ( GCC_MINOR GREATER 6 OR  GCC_MINOR EQUAL 6 )
            SET ( SUPPORTED_COMPILER "YES" )
        ENDIF (  )
    ENDIF (  )
ELSEIF ( CXX_CLANG )
    IF ( CLANG_MAJOR GREATER 4 OR CLANG_MAJOR EQUAL 4 )
        SET ( SUPPORTED_COMPILER "YES" )
    ELSEIF ( CLANG_MAJOR GREATER 3 OR CLANG_MAJOR EQUAL 3 )
        IF ( CLANG_MINOR GREATER 2 OR  CLANG_MINOR EQUAL 2 )
            SET ( SUPPORTED_COMPILER "YES" )
        ENDIF (  )
    ENDIF (  )
ELSEIF ( CXX_MSVC )
    IF ( MSVC_MAJOR GREATER 17 OR MSVC_MAJOR EQUAL 17 )
        SET ( SUPPORTED_COMPILER "YES" )
    ENDIF (  )
ELSE (  )
    SET ( SUPPORTED_COMPILER "NO" )
ENDIF (  )


IF ( SUPPORTED_COMPILER )
    MESSAGE ( STATUS "Checking if your compiler (${CMAKE_CXX_COMPILER}) is supported for building -- yes" )
ELSE (  )
    MESSAGE ( STATUS "Checking if your compiler (${CMAKE_CXX_COMPILER}) is supported for building -- no" )
    MESSAGE ( FATAL_ERROR "\nUnfortunately your system doesn't meet the minimum requirements for building."
    "\nThe only supported compilers are gcc-4.6+, mingw-4.6+, clang-3.2+, msvc-17+." )
ENDIF (  )


INCLUDE ( CheckCXX11Features )

INCLUDE ( dependencies.cmake )

INCLUDE ( build-config.cmake )


IF ( CXX_GCC )
    IF ( GCC_MAJOR GREATER 4 )
        SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14" )
        SET ( CXX_COMPILE_MODE "${CXX_COMPILE_MODE_14}" )        
    ELSE(  )
        IF ( GCC_MINOR EQUAL 9 )
            SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14" )
            SET ( CXX_COMPILE_MODE "${CXX_COMPILE_MODE_14}" )
        ELSEIF ( GCC_MINOR EQUAL 8 )
            SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y" )
            SET ( CXX_COMPILE_MODE "${CXX_COMPILE_MODE_14y}" )
        ELSEIF ( GCC_MINOR GREATER 6 )
            SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
            SET ( CXX_COMPILE_MODE "${CXX_COMPILE_MODE_11}" )
        ELSE (  )
            SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x" )
            SET ( CXX_COMPILE_MODE "${CXX_COMPILE_MODE_0x}" )
        ENDIF (  )
    ENDIF (  )

    SET ( GCC_CXX_FLAGS_WARN_LEVEL "-Wall -Wextra -Wno-unused-parameter -Wno-date-time" )
    SET ( GCC_CXX_FLAGS_ISO_CPP "-pedantic" )
    SET ( GCC_CXX_FLAGS_USE_PIPE "-pipe" )

    IF ( CMAKE_BUILD_TYPE MATCHES "GCCMinSizeRel"
            OR CMAKE_BUILD_TYPE MATCHES "GCCMaxSpeedRel"
            OR CMAKE_BUILD_TYPE MATCHES "GCCNativeMinSizeRel"
            OR CMAKE_BUILD_TYPE MATCHES "GCCNativeMaxSpeedRel" )
        SET ( GCC_CXX_FLAGS_OPT_LEVEL_SIZE "-Os" )
        SET ( GCC_CXX_FLAGS_OPT_LEVEL_SPEED "-O2" )
        SET ( GCC_CXX_FLAGS_OPT_NATIVE "-march=native -mtune=native" )
        SET ( GCC_CXX_FLAGS_OMIT_FP "-fomit-frame-pointer" )
        SET ( GCC_CXX_FLAGS_SIZE_OPT "-ffunction-sections -fdata-sections" )
        SET ( GCC_LD_FLAGS_SIZE_OPT "-Wl,--gc-sections" )

        SET ( CMAKE_CXX_FLAGS_GCCMinSizeRel "${GCC_CXX_FLAGS_WARN_LEVEL} ${GCC_CXX_FLAGS_ISO_CPP} ${GCC_CXX_FLAGS_USE_PIPE} ${GCC_CXX_FLAGS_OPT_LEVEL_SIZE} ${GCC_CXX_FLAGS_OMIT_FP} ${GCC_CXX_FLAGS_SIZE_OPT}" )
        SET ( CMAKE_CXX_FLAGS_GCCMaxSpeedRel "${GCC_CXX_FLAGS_WARN_LEVEL} ${GCC_CXX_FLAGS_ISO_CPP} ${GCC_CXX_FLAGS_USE_PIPE} ${GCC_CXX_FLAGS_OPT_LEVEL_SPEED} ${GCC_CXX_FLAGS_OMIT_FP} ${GCC_CXX_FLAGS_SIZE_OPT}" )
        SET ( CMAKE_CXX_FLAGS_GCCNativeMinSizeRel "${GCC_CXX_FLAGS_WARN_LEVEL} ${GCC_CXX_FLAGS_ISO_CPP} ${GCC_CXX_FLAGS_USE_PIPE} ${GCC_CXX_FLAGS_OPT_LEVEL_SIZE} ${GCC_CXX_FLAGS_OPT_NATIVE} ${GCC_CXX_FLAGS_OMIT_FP} ${GCC_CXX_FLAGS_SIZE_OPT}" )
        SET ( CMAKE_CXX_FLAGS_GCCNativeMaxSpeedRel "${GCC_CXX_FLAGS_WARN_LEVEL} ${GCC_CXX_FLAGS_ISO_CPP} ${GCC_CXX_FLAGS_USE_PIPE} ${GCC_CXX_FLAGS_OPT_LEVEL_SPEED} ${GCC_CXX_FLAGS_OPT_NATIVE} ${GCC_CXX_FLAGS_OMIT_FP} ${GCC_CXX_FLAGS_SIZE_OPT}" )

        SET ( CMAKE_C_FLAGS_GCCMinSizeRel "${CMAKE_CXX_FLAGS_GCCMinSizeRel}" )
        SET ( CMAKE_C_FLAGS_GCCMaxSpeedRel "${CMAKE_CXX_FLAGS_GCCMaxSpeedRel}" )
        SET ( CMAKE_C_FLAGS_GCCNativeMinSizeRel "${CMAKE_CXX_FLAGS_GCCNativeMinSizeRel}" )
        SET ( CMAKE_C_FLAGS_GCCNativeMaxSpeedRel "${CMAKE_CXX_FLAGS_GCCNativeMaxSpeedRel}" )

        IF ( CMAKE_BUILD_TYPE MATCHES "GCCMinSizeRel")
            SET ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_GCCMinSizeRel}" )
            SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_GCCMinSizeRel}" )
        ENDIF (  )

        IF ( CMAKE_BUILD_TYPE MATCHES "GCCMaxSpeedRel")
            SET ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_GCCMaxSpeedRel}" )
            SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_GCCMaxSpeedRel}" )
        ENDIF (  )

        IF ( CMAKE_BUILD_TYPE MATCHES "GCCNativeMinSizeRel")
            SET ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_GCCNativeMinSizeRel}" )
            SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_GCCNativeMinSizeRel}" )
        ENDIF (  )

        IF ( CMAKE_BUILD_TYPE MATCHES "GCCNativeMaxSpeedRel")
            SET ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_GCCNativeMaxSpeedRel}" )
            SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_GCCNativeMaxSpeedRel}" )
        ENDIF (  )

        SET ( CMAKE_EXE_LINKER_FLAGS "${GCC_LD_FLAGS_SIZE_OPT}" )
    ELSE (  )
        SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_CXX_FLAGS_WARN_LEVEL} ${GCC_CXX_FLAGS_ISO_CPP}" )
        SET ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_CXX_FLAGS_WARN_LEVEL} ${GCC_CXX_FLAGS_ISO_CPP}" )
    ENDIF (  )
ENDIF (  )

IF ( CXX_CLANG )
    IF ( CLANG_MAJOR GREATER 3 )
        SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14" )
        SET ( CXX_COMPILE_MODE "${CXX_COMPILE_MODE_14}" )
    ELSE (  )
        IF ( CLANG_MINOR GREATER 4 )
            SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14" )
            SET ( CXX_COMPILE_MODE "${CXX_COMPILE_MODE_14}" )
        ELSEIF ( CLANG_MINOR GREATER 3 )
            SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y" )
            SET ( CXX_COMPILE_MODE "${CXX_COMPILE_MODE_1y}" )
        ELSE (  )
            SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
            SET ( CXX_COMPILE_MODE "${CXX_COMPILE_MODE_11}" )
        ENDIF (  )
    ENDIF (  )

    SET ( CLANG_CXX_FLAGS_WARN_LEVEL "-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-global-constructors -Wno-exit-time-destructors -Wno-padded -Wno-weak-vtables -Wno-date-time" )
    SET ( CLANG_CXX_FLAGS_ISO_CPP "-pedantic" )

    IF ( CMAKE_BUILD_TYPE MATCHES "ClangMinSizeRel"
            OR CMAKE_BUILD_TYPE MATCHES "ClangMaxSpeedRel"
            OR CMAKE_BUILD_TYPE MATCHES "ClangNativeMinSizeRel"
            OR CMAKE_BUILD_TYPE MATCHES "ClangNativeMaxSpeedRel" )
        SET ( CLANG_CXX_FLAGS_OPT_LEVEL_SIZE "-Oz" )
        SET ( CLANG_CXX_FLAGS_OPT_LEVEL_SPEED "-O3" )
        SET ( CLANG_CXX_FLAGS_OMIT_FP "-fno-omit-frame-pointer" )

        SET ( CMAKE_CXX_FLAGS_ClangMinSizeRel "${CLANG_CXX_FLAGS_WARN_LEVEL} ${CLANG_CXX_FLAGS_ISO_CPP} ${CLANG_CXX_FLAGS_OPT_LEVEL_SIZE} ${CLANG_CXX_FLAGS_OMIT_FP}" )
        SET ( CMAKE_CXX_FLAGS_ClangMaxSpeedRel "${CLANG_CXX_FLAGS_WARN_LEVEL} ${CLANG_CXX_FLAGS_ISO_CPP} ${CLANG_CXX_FLAGS_OPT_LEVEL_SPEED} ${CLANG_CXX_FLAGS_OMIT_FP}" )
        SET ( CMAKE_CXX_FLAGS_ClangNativeMinSizeRel "${CLANG_CXX_FLAGS_WARN_LEVEL} ${CLANG_CXX_FLAGS_ISO_CPP} ${CLANG_CXX_FLAGS_OPT_LEVEL_SIZE} ${CLANG_CXX_FLAGS_OMIT_FP}" )
        SET ( CMAKE_CXX_FLAGS_ClangNativeMaxSpeedRel "${CLANG_CXX_FLAGS_WARN_LEVEL} ${CLANG_CXX_FLAGS_ISO_CPP} ${CLANG_CXX_FLAGS_OPT_LEVEL_SPEED} ${CLANG_CXX_FLAGS_OMIT_FP}" )

        SET ( CMAKE_C_FLAGS_ClangMinSizeRel "${CMAKE_CXX_FLAGS_ClangMinSizeRel}" )
        SET ( CMAKE_C_FLAGS_ClangMaxSpeedRel "${CMAKE_CXX_FLAGS_ClangMaxSpeedRel}" )
        SET ( CMAKE_C_FLAGS_ClangNativeMinSizeRel "${CMAKE_CXX_FLAGS_ClangNativeMinSizeRel}" )
        SET ( CMAKE_C_FLAGS_ClangNativeMaxSpeedRel "${CMAKE_CXX_FLAGS_ClangNativeMaxSpeedRel}" )

        IF ( CMAKE_BUILD_TYPE MATCHES "ClangMinSizeRel")
            SET ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_ClangMinSizeRel}" )
            SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_ClangMinSizeRel}" )
        ENDIF (  )

        IF ( CMAKE_BUILD_TYPE MATCHES "ClangMaxSpeedRel")
            SET ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_ClangMaxSpeedRel}" )
            SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_ClangMaxSpeedRel}" )
        ENDIF (  )

        IF ( CMAKE_BUILD_TYPE MATCHES "ClangNativeMinSizeRel")
            SET ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_ClangNativeMinSizeRel}" )
            SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_ClangNativeMinSizeRel}" )
        ENDIF (  )

        IF ( CMAKE_BUILD_TYPE MATCHES "ClangNativeMaxSpeedRel")
            SET ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_ClangNativeMaxSpeedRel}" )
            SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_ClangNativeMaxSpeedRel}" )
        ENDIF (  )
    ELSE (  )
        SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANG_CXX_FLAGS_WARN_LEVEL} ${CLANG_CXX_FLAGS_ISO_CPP}" )
        SET ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CLANG_CXX_FLAGS_WARN_LEVEL} ${CLANG_CXX_FLAGS_ISO_CPP}" )
    ENDIF (  )
ENDIF (  )

IF ( CXX_MSVC )
    IF ( MSVC_MAJOR GREATER 17 )
        SET ( CXX_COMPILE_MODE "${CXX_COMPILE_MODE_14}" )
    ELSE (  )
        SET ( CXX_COMPILE_MODE "${CXX_COMPILE_MODE_11}" )
    ENDIF (  )
ENDIF (  )


SET ( BUILD_INFO_COMPILER "${COMPILER}" )
SET ( BUILD_INFO_PROCESSOR "${CMAKE_SYSTEM_PROCESSOR}" )
SET ( BUILD_INFO_SYSTEM "${CMAKE_SYSTEM}" )

SITE_NAME( BUILD_INFO_HOST )

IF ( NOT WIN32 )
    SET ( BUILD_INFO_HOST "$ENV{USER}@${BUILD_INFO_HOST}" )    
ELSE (  )
    SET ( BUILD_INFO_HOST "$ENV{USERNAME}@${BUILD_INFO_HOST}" )    
ENDIF(  )

INCLUDE ( product-info.cmake )


MESSAGE ( STATUS "Build type: ${CMAKE_BUILD_TYPE}" )
MESSAGE ( STATUS "Compiler flags: ${CMAKE_CXX_FLAGS}" )
MESSAGE ( STATUS "Linker flags: ${CMAKE_EXE_LINKER_FLAGS}" )

INCLUDE_DIRECTORIES(.)

IF ( WIN32 AND MINGW )
    IF ( CMAKE_BUILD_TYPE MATCHES "Release"
            OR CMAKE_BUILD_TYPE MATCHES "GCCMinSizeRel"
            OR CMAKE_BUILD_TYPE MATCHES "GCCMaxSpeedRel"
            OR CMAKE_BUILD_TYPE MATCHES "GCCNativeMinSizeRel"
            OR CMAKE_BUILD_TYPE MATCHES "GCCNativeMaxSpeedRel" )
        SET ( IS_DEBUG_BUILD FALSE )

        GET_FILENAME_COMPONENT( BOOST_DATE_TIME_LIBRARY_PATH "${Boost_DATE_TIME_LIBRARY_RELEASE}" NAME )
        GET_FILENAME_COMPONENT( BOOST_FILESYSTEM_LIBRARY_PATH "${Boost_FILESYSTEM_LIBRARY_RELEASE}" NAME )
        GET_FILENAME_COMPONENT( BOOST_RANDOM_LIBRARY_PATH "${Boost_RANDOM_LIBRARY_RELEASE}" NAME )
        GET_FILENAME_COMPONENT( BOOST_REGEX_LIBRARY_PATH "${Boost_REGEX_LIBRARY_RELEASE}" NAME )
        GET_FILENAME_COMPONENT( BOOST_SYSTEM_LIBRARY_PATH "${Boost_SYSTEM_LIBRARY_RELEASE}" NAME )
        GET_FILENAME_COMPONENT( BOOST_THREAD_LIBRARY_PATH "${Boost_THREAD_LIBRARY_RELEASE}" NAME )
    ELSE (  )
        SET ( IS_DEBUG_BUILD TRUE )

        GET_FILENAME_COMPONENT( BOOST_DATE_TIME_LIBRARY_PATH "${Boost_DATE_TIME_LIBRARY_DEBUG}" NAME )
        GET_FILENAME_COMPONENT( BOOST_FILESYSTEM_LIBRARY_PATH "${Boost_FILESYSTEM_LIBRARY_DEBUG}" NAME )
        GET_FILENAME_COMPONENT( BOOST_RANDOM_LIBRARY_PATH "${Boost_RANDOM_LIBRARY_DEBUG}" NAME )
        GET_FILENAME_COMPONENT( BOOST_REGEX_LIBRARY_PATH "${Boost_REGEX_LIBRARY_DEBUG}" NAME )
        GET_FILENAME_COMPONENT( BOOST_SYSTEM_LIBRARY_PATH "${Boost_SYSTEM_LIBRARY_DEBUG}" NAME )
        GET_FILENAME_COMPONENT( BOOST_THREAD_LIBRARY_PATH "${Boost_THREAD_LIBRARY_DEBUG}" NAME )
    ENDIF (  )

    SET ( BOOST_DATE_TIME_LIBRARY_PATH "${Boost_LIBRARY_DIRS}/${BOOST_DATE_TIME_LIBRARY_PATH}" )
    SET ( BOOST_FILESYSTEM_LIBRARY_PATH "${Boost_LIBRARY_DIRS}/${BOOST_FILESYSTEM_LIBRARY_PATH}" )
    SET ( BOOST_RANDOM_LIBRARY_PATH "${Boost_LIBRARY_DIRS}/${BOOST_RANDOM_LIBRARY_PATH}" )
    SET ( BOOST_REGEX_LIBRARY_PATH "${Boost_LIBRARY_DIRS}/${BOOST_REGEX_LIBRARY_PATH}" )
    SET ( BOOST_SYSTEM_LIBRARY_PATH "${Boost_LIBRARY_DIRS}/${BOOST_SYSTEM_LIBRARY_PATH}" )
    SET ( BOOST_THREAD_LIBRARY_PATH "${Boost_LIBRARY_DIRS}/${BOOST_THREAD_LIBRARY_PATH}" )
ENDIF (  )

INCLUDE ( definitions.cmake )
INCLUDE ( deploy.cmake )
INCLUDE ( resources.cmake )

INCLUDE ( cotire )

SUBDIRS( CoreLib Utils Service )
