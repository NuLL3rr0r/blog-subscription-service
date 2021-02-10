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


FIND_PATH ( GRAPHICS_MAGICKPP_INCLUDE_DIR NAMES GraphicsMagick/Magick++ PATHS /usr/include/ /usr/local/include/ )
FIND_PATH ( GRAPHICS_MAGICK_INCLUDE_DIR NAMES GraphicsMagick/magick PATHS /usr/include/ /usr/local/include/ )
FIND_PATH ( GRAPHICS_MAGICK_WAND_INCLUDE_DIR NAMES GraphicsMagick/wand PATHS /usr/include/ /usr/local/include/ )
FIND_LIBRARY ( GRAPHICS_MAGICKPP_LIBRARY NAMES GraphicsMagick++ PATHS /usr/lib /usr/local/lib )
FIND_LIBRARY ( GRAPHICS_MAGICK_LIBRARY NAMES GraphicsMagick PATHS /usr/lib /usr/local/lib )
FIND_LIBRARY ( GRAPHICS_MAGICK_WAND_LIBRARY NAMES GraphicsMagickWand PATHS /usr/lib /usr/local/lib )
FIND_FILE ( GRAPHICS_MAGICKPP_CONFIG_SCRIPT NAMES GraphicsMagick++-config PATHS /usr/bin /usr/local/bin )
FIND_FILE ( GRAPHICS_MAGICK_CONFIG_SCRIPT NAMES GraphicsMagick-config PATHS /usr/bin /usr/local/bin )
FIND_FILE ( GRAPHICS_MAGICK_WAND_CONFIG_SCRIPT NAMES GraphicsMagickWand-config PATHS /usr/bin /usr/local/bin )

FIND_PATH ( IMAGE_MAGICKPP_INCLUDE_DIR NAMES ImageMagick/Magick++ ImageMagick-7/Magick++ ImageMagick-6/Magick++ PATHS /usr/include/ /usr/local/include/ )
FIND_PATH ( IMAGE_MAGICK_INCLUDE_DIR NAMES ImageMagick/magick ImageMagick-7/magick ImageMagick-6/magick PATHS /usr/include/ /usr/local/include/ )
FIND_PATH ( IMAGE_MAGICK_WAND_INCLUDE_DIR NAMES ImageMagick/wand ImageMagick-7/wand ImageMagick-6/wand PATHS /usr/include/ /usr/local/include/ )
FIND_LIBRARY ( IMAGE_MAGICKPP_LIBRARY NAMES Magick++ Magick++-7 Magick++-7.Q8 Magick++-7.Q16 Magick++-6 Magick++-6.Q8 Magick++-6.Q16 PATHS /usr/lib /usr/local/lib )
FIND_LIBRARY ( IMAGE_MAGICK_CORE_LIBRARY NAMES MagickCore MagickCore-7 MagickCore-7.Q8  MagickCore-7.Q16 MagickCore-6 MagickCore-6.Q8  MagickCore-6.Q16 PATHS /usr/lib /usr/local/lib )
FIND_LIBRARY ( IMAGE_MAGICK_WAND_LIBRARY NAMES MagickWand MagickWand-7 MagickWand-7.Q8 MagickWand-7.Q16 MagickWand-6 MagickWand-6.Q8 MagickWand-6.Q16 PATHS /usr/lib /usr/local/lib )
FIND_FILE ( IMAGE_MAGICKPP_CONFIG_SCRIPT NAMES Magick++-config PATHS /usr/bin /usr/local/bin )
FIND_FILE ( IMAGE_MAGICK_CONFIG_SCRIPT NAMES Magick-config PATHS /usr/bin /usr/local/bin )
FIND_FILE ( IMAGE_MAGICK_CORE_CONFIG_SCRIPT NAMES MagickCore-config PATHS /usr/bin /usr/local/bin )
FIND_FILE ( IMAGE_MAGICK_WAND_CONFIG_SCRIPT NAMES MagickWand-config PATHS /usr/bin /usr/local/bin )


IF ( GRAPHICS_MAGICKPP_INCLUDE_DIR AND GRAPHICS_MAGICKPP_LIBRARY )
    SET ( GRAPHICS_MAGICKPP_FOUND TRUE )
ENDIF (  )

IF ( GRAPHICS_MAGICK_INCLUDE_DIR AND GRAPHICS_MAGICK_LIBRARY )
    SET ( GRAPHICS_MAGICK_FOUND TRUE )
ENDIF (  )

IF ( GRAPHICS_MAGICK_WAND_INCLUDE_DIR AND GRAPHICS_MAGICK_WAND_LIBRARY )
    SET ( GRAPHICS_MAGICK_WAND_FOUND TRUE )
ENDIF (  )

IF ( GRAPHICS_MAGICKPP_CONFIG_SCRIPT )
    SET ( GRAPHICS_MAGICKPP_CONFIG_FOUND TRUE )
ENDIF (  )

IF ( GRAPHICS_MAGICK_CONFIG_SCRIPT )
    SET ( GRAPHICS_MAGICK_CONFIG_FOUND TRUE )
ENDIF (  )

IF ( GRAPHICS_MAGICK_WAND_CONFIG_SCRIPT )
    SET ( GRAPHICS_MAGICK_WAND_CONFIG_FOUND TRUE )
ENDIF (  )

IF ( IMAGE_MAGICKPP_INCLUDE_DIR AND IMAGE_MAGICKPP_LIBRARY )
    SET ( IMAGE_MAGICKPP_FOUND TRUE )
ENDIF (  )

IF ( IMAGE_MAGICK_INCLUDE_DIR AND IMAGE_MAGICK_CORE_LIBRARY )
    SET ( IMAGE_MAGICK_FOUND TRUE )
ENDIF (  )

IF ( IMAGE_MAGICK_WAND_INCLUDE_DIR AND IMAGE_MAGICK_WAND_LIBRARY )
    SET ( IMAGE_MAGICK_WAND_FOUND TRUE )
ENDIF (  )

IF ( IMAGE_MAGICKPP_CONFIG_SCRIPT )
    SET ( IMAGE_MAGICKPP_CONFIG_FOUND TRUE )
ENDIF (  )

IF ( IMAGE_MAGICK_CONFIG_SCRIPT )
    SET ( IMAGE_MAGICK_CONFIG_FOUND TRUE )
ENDIF (  )

IF ( IMAGE_MAGICK_CORE_CONFIG_SCRIPT )
    SET ( IMAGE_MAGICK_CORE_CONFIG_FOUND TRUE )
ENDIF (  )

IF ( IMAGE_MAGICK_WAND_CONFIG_SCRIPT )
    SET ( IMAGE_MAGICK_WAND_CONFIG_FOUND TRUE )
ENDIF (  )


IF ( GRAPHICS_MAGICK_FOUND )
    MESSAGE ( STATUS "Found GraphicsMagick headers in ${GRAPHICS_MAGICK_INCLUDE_DIR}" )
    MESSAGE ( STATUS "Found GraphicsMagick library: ${GRAPHICS_MAGICK_LIBRARY}" )
ELSE (  )
    IF ( GRAPHICS_MAGICK_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find GraphicsMagick" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find GraphicsMagick" )
    ENDIF (  )
ENDIF (  )

IF ( GRAPHICS_MAGICKPP_FOUND )
    MESSAGE ( STATUS "Found GraphicsMagick++ headers in ${GRAPHICS_MAGICKPP_INCLUDE_DIR}" )
    MESSAGE ( STATUS "Found GraphicsMagick++ library: ${GRAPHICS_MAGICKPP_LIBRARY}" )
ELSE (  )
    IF ( GRAPHICS_MAGICKPP_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find GraphicsMagick++" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find GraphicsMagick++" )
    ENDIF (  )
ENDIF (  )

IF ( GRAPHICS_MAGICK_WAND_FOUND )
    MESSAGE ( STATUS "Found GraphicsMagickWand headers in ${GRAPHICS_MAGICK_WAND_INCLUDE_DIR}" )
    MESSAGE ( STATUS "Found GraphicsMagickWand library: ${GRAPHICS_MAGICK_WAND_LIBRARY}" )
ELSE (  )
    IF ( GRAPHICS_MAGICK_WAND_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find GraphicsMagickWand" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find GraphicsMagickWand" )
    ENDIF (  )
ENDIF (  )

IF ( GRAPHICS_MAGICK_CONFIG_FOUND )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICK_CONFIG_SCRIPT} ARGS --cflags
            OUTPUT_VARIABLE GRAPHICS_MAGICK_CFLAGS )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICK_CONFIG_SCRIPT} ARGS --cppflags
            OUTPUT_VARIABLE GRAPHICS_MAGICK_CPPFLAGS )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICK_CONFIG_SCRIPT} ARGS --exec-prefix
            OUTPUT_VARIABLE GRAPHICS_MAGICK_EXEC_PREFIX )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICK_CONFIG_SCRIPT} ARGS --ldflags
            OUTPUT_VARIABLE GRAPHICS_MAGICK_LDFLAGS )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICK_CONFIG_SCRIPT} ARGS --libs
            OUTPUT_VARIABLE GRAPHICS_MAGICK_LIBS )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICK_CONFIG_SCRIPT} ARGS --prefix
            OUTPUT_VARIABLE GRAPHICS_MAGICK_PREFIX )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICK_CONFIG_SCRIPT} ARGS --version
            OUTPUT_VARIABLE GRAPHICS_MAGICK_VERSION )

    MESSAGE ( STATUS "Found GraphicsMagick configuration helper script ${GRAPHICS_MAGICK_CONFIG_SCRIPT}" )
    MESSAGE ( STATUS "  GraphicsMagick cflags: ${GRAPHICS_MAGICK_CFLAGS}" )
    MESSAGE ( STATUS "  GraphicsMagick cppflags: ${GRAPHICS_MAGICK_CPPFLAGS}" )
    MESSAGE ( STATUS "  GraphicsMagick exec-prefix: ${GRAPHICS_MAGICK_EXEC_PREFIX}" )
    MESSAGE ( STATUS "  GraphicsMagick ldflags: ${GRAPHICS_MAGICK_LDFLAGS}" )
    MESSAGE ( STATUS "  GraphicsMagick libs: ${GRAPHICS_MAGICK_LIBS}" )
    MESSAGE ( STATUS "  GraphicsMagick prefix: ${GRAPHICS_MAGICK_PREFIX}" )
    MESSAGE ( STATUS "  GraphicsMagick version: ${GRAPHICS_MAGICK_VERSION}" )

    EXEC_PROGRAM ( ${GRAPHICS_MAGICK_CONFIG_SCRIPT} ARGS --cppflags | cut -d ' ' -f '1-1' | sed 's/-I//g' | sed 's/-I//g'
            OUTPUT_VARIABLE GRAPHICS_MAGICK_CONFIG_HEADERS )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICK_CONFIG_SCRIPT} ARGS --libs
            OUTPUT_VARIABLE GRAPHICS_MAGICK_CONFIG_FLAGS )
ELSE (  )
    IF ( GRAPHICS_MAGICK_CONFIG_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find GraphicsMagick configuration helper script" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find GraphicsMagick configuration helper script" )
    ENDIF (  )
ENDIF (  )

IF ( GRAPHICS_MAGICKPP_CONFIG_FOUND )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICKPP_CONFIG_SCRIPT} ARGS --cppflags
            OUTPUT_VARIABLE GRAPHICS_MAGICKPP_CPPFLAGS )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICKPP_CONFIG_SCRIPT} ARGS --cxxflags
            OUTPUT_VARIABLE GRAPHICS_MAGICKPP_CXXFLAGS )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICKPP_CONFIG_SCRIPT} ARGS --exec-prefix
            OUTPUT_VARIABLE GRAPHICS_MAGICKPP_EXEC_PREFIX )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICKPP_CONFIG_SCRIPT} ARGS --ldflags
            OUTPUT_VARIABLE GRAPHICS_MAGICKPP_LDFLAGS )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICKPP_CONFIG_SCRIPT} ARGS --libs
            OUTPUT_VARIABLE GRAPHICS_MAGICKPP_LIBS )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICKPP_CONFIG_SCRIPT} ARGS --prefix
            OUTPUT_VARIABLE GRAPHICS_MAGICKPP_PREFIX )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICKPP_CONFIG_SCRIPT} ARGS --version
            OUTPUT_VARIABLE GRAPHICS_MAGICKPP_VERSION )

    MESSAGE ( STATUS "Found GraphicsMagick++ configuration helper script ${GRAPHICS_MAGICKPP_CONFIG_SCRIPT}" )
    MESSAGE ( STATUS "  GraphicsMagick++ cppflags: ${GRAPHICS_MAGICKPP_CPPFLAGS}" )
    MESSAGE ( STATUS "  GraphicsMagick++ cxxflags: ${GRAPHICS_MAGICKPP_CXXFLAGS}" )
    MESSAGE ( STATUS "  GraphicsMagick++ exec-prefix: ${GRAPHICS_MAGICKPP_EXEC_PREFIX}" )
    MESSAGE ( STATUS "  GraphicsMagick++ ldflags: ${GRAPHICS_MAGICKPP_LDFLAGS}" )
    MESSAGE ( STATUS "  GraphicsMagick++ libs: ${GRAPHICS_MAGICKPP_LIBS}" )
    MESSAGE ( STATUS "  GraphicsMagick++ prefix: ${GRAPHICS_MAGICKPP_PREFIX}" )
    MESSAGE ( STATUS "  GraphicsMagick++ version: ${GRAPHICS_MAGICKPP_VERSION}" )

    EXEC_PROGRAM ( ${GRAPHICS_MAGICKPP_CONFIG_SCRIPT} ARGS --cppflags | cut -d ' ' -f '1-1' | sed 's/-I//g' | sed 's/-I//g'
            OUTPUT_VARIABLE GRAPHICS_MAGICKPP_CONFIG_HEADERS )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICKPP_CONFIG_SCRIPT} ARGS --libs
            OUTPUT_VARIABLE GRAPHICS_MAGICKPP_CONFIG_FLAGS )
ELSE (  )
    IF ( GRAPHICS_MAGICKPP_CONFIG_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find GraphicsMagick++ configuration helper script" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find GraphicsMagick++ configuration helper script" )
    ENDIF (  )
ENDIF (  )

IF ( GRAPHICS_MAGICK_WAND_CONFIG_FOUND )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICK_WAND_CONFIG_SCRIPT} ARGS --cflags
            OUTPUT_VARIABLE GRAPHICS_MAGICK_WAND_CFLAGS )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICK_WAND_CONFIG_SCRIPT} ARGS --cppflags
            OUTPUT_VARIABLE GRAPHICS_MAGICK_WAND_CPPFLAGS )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICK_WAND_CONFIG_SCRIPT} ARGS --exec-prefix
            OUTPUT_VARIABLE GRAPHICS_MAGICK_WAND_EXEC_PREFIX )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICK_WAND_CONFIG_SCRIPT} ARGS --ldflags
            OUTPUT_VARIABLE GRAPHICS_MAGICK_WAND_LDFLAGS )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICK_WAND_CONFIG_SCRIPT} ARGS --libs
            OUTPUT_VARIABLE GRAPHICS_MAGICK_WAND_LIBS )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICK_WAND_CONFIG_SCRIPT} ARGS --prefix
            OUTPUT_VARIABLE GRAPHICS_MAGICK_WAND_PREFIX )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICK_WAND_CONFIG_SCRIPT} ARGS --version
            OUTPUT_VARIABLE GRAPHICS_MAGICK_WAND_VERSION )

    MESSAGE ( STATUS "Found GraphicsMagickWand configuration helper script ${GRAPHICS_MAGICK_WAND_CONFIG_SCRIPT}" )
    MESSAGE ( STATUS "  GraphicsMagickWand cflags: ${GRAPHICS_MAGICK_WAND_CFLAGS}" )
    MESSAGE ( STATUS "  GraphicsMagickWand cppflags: ${GRAPHICS_MAGICK_WAND_CPPFLAGS}" )
    MESSAGE ( STATUS "  GraphicsMagickWand exec-prefix: ${GRAPHICS_MAGICK_WAND_EXEC_PREFIX}" )
    MESSAGE ( STATUS "  GraphicsMagickWand ldflags: ${GRAPHICS_MAGICK_WAND_LDFLAGS}" )
    MESSAGE ( STATUS "  GraphicsMagickWand libs: ${GRAPHICS_MAGICK_WAND_LIBS}" )
    MESSAGE ( STATUS "  GraphicsMagickWand prefix: ${GRAPHICS_MAGICK_WAND_PREFIX}" )
    MESSAGE ( STATUS "  GraphicsMagickWand version: ${GRAPHICS_MAGICK_WAND_VERSION}" )

    EXEC_PROGRAM ( ${GRAPHICS_MAGICK_WAND_CONFIG_SCRIPT} ARGS --cppflags | cut -d ' ' -f '1-1' | sed 's/-I//g' | sed 's/-I//g'
            OUTPUT_VARIABLE GRAPHICS_MAGICK_WAND_CONFIG_HEADERS )
    EXEC_PROGRAM ( ${GRAPHICS_MAGICK_WAND_CONFIG_SCRIPT} ARGS --libs
            OUTPUT_VARIABLE GRAPHICS_MAGICK_WAND_CONFIG_FLAGS )
ELSE (  )
    IF ( GRAPHICS_MAGICK_WAND_CONFIG_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find GraphicsMagickWand configuration helper script" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find GraphicsMagickWand configuration helper script" )
    ENDIF (  )
ENDIF (  )


IF ( IMAGE_MAGICK_FOUND )
    MESSAGE ( STATUS "Found ImageMagick headers in ${IMAGE_MAGICK_INCLUDE_DIR}" )
    MESSAGE ( STATUS "Found ImageMagickCore library: ${IMAGE_MAGICK_CORE_LIBRARY}" )
ELSE (  )
    IF ( IMAGE_MAGICK_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find ImageMagick" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find ImageMagick" )
    ENDIF (  )
ENDIF (  )

IF ( IMAGE_MAGICKPP_FOUND )
    MESSAGE ( STATUS "Found ImageMagick++ headers in ${IMAGE_MAGICKPP_INCLUDE_DIR}" )
    MESSAGE ( STATUS "Found ImageMagick++ library: ${IMAGE_MAGICKPP_LIBRARY}" )
ELSE (  )
    IF ( IMAGE_MAGICKPP_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find ImageMagick++" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find ImageMagick++" )
    ENDIF (  )
ENDIF (  )

IF ( IMAGE_MAGICK_WAND_FOUND )
    MESSAGE ( STATUS "Found ImageMagickWand headers in ${IMAGE_MAGICK_WAND_INCLUDE_DIR}" )
    MESSAGE ( STATUS "Found ImageMagickWand library: ${IMAGE_MAGICK_WAND_LIBRARY}" )
ELSE (  )
    IF ( IMAGE_MAGICK_WAND_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find ImageMagickWand" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find ImageMagickWand" )
    ENDIF (  )
ENDIF (  )


IF ( IMAGE_MAGICK_CONFIG_FOUND )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_CONFIG_SCRIPT} ARGS --cflags
            OUTPUT_VARIABLE IMAGE_MAGICK_CFLAGS )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_CONFIG_SCRIPT} ARGS --cppflags
            OUTPUT_VARIABLE IMAGE_MAGICK_CPPFLAGS )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_CONFIG_SCRIPT} ARGS --exec-prefix
            OUTPUT_VARIABLE IMAGE_MAGICK_EXEC_PREFIX )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_CONFIG_SCRIPT} ARGS --ldflags
            OUTPUT_VARIABLE IMAGE_MAGICK_LDFLAGS )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_CONFIG_SCRIPT} ARGS --libs
            OUTPUT_VARIABLE IMAGE_MAGICK_LIBS )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_CONFIG_SCRIPT} ARGS --prefix
            OUTPUT_VARIABLE IMAGE_MAGICK_PREFIX )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_CONFIG_SCRIPT} ARGS --version
            OUTPUT_VARIABLE IMAGE_MAGICK_VERSION )

    MESSAGE ( STATUS "Found ImageMagick configuration helper script ${IMAGE_MAGICK_CONFIG_SCRIPT}" )
    MESSAGE ( STATUS "  ImageMagick cflags: ${IMAGE_MAGICK_CFLAGS}" )
    MESSAGE ( STATUS "  ImageMagick cppflags: ${IMAGE_MAGICK_CPPFLAGS}" )
    MESSAGE ( STATUS "  ImageMagick exec-prefix: ${IMAGE_MAGICK_EXEC_PREFIX}" )
    MESSAGE ( STATUS "  ImageMagick ldflags: ${IMAGE_MAGICK_LDFLAGS}" )
    MESSAGE ( STATUS "  ImageMagick libs: ${IMAGE_MAGICK_LIBS}" )
    MESSAGE ( STATUS "  ImageMagick prefix: ${IMAGE_MAGICK_PREFIX}" )
    MESSAGE ( STATUS "  ImageMagick version: ${IMAGE_MAGICK_VERSION}" )

    EXEC_PROGRAM ( ${IMAGE_MAGICK_CONFIG_SCRIPT} ARGS --cppflags | cut -d ' ' -f '1-1' | sed 's/-I//g' | sed 's/-I//g'
            OUTPUT_VARIABLE IMAGE_MAGICK_CONFIG_HEADERS )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_CONFIG_SCRIPT} ARGS --libs
            OUTPUT_VARIABLE IMAGE_MAGICK_CONFIG_FLAGS )
ELSE (  )
    IF ( IMAGE_MAGICK_CONFIG_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find ImageMagick configuration helper script" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find ImageMagick configuration helper script" )
    ENDIF (  )
ENDIF (  )

IF ( IMAGE_MAGICK_CORE_CONFIG_FOUND )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_CORE_CONFIG_SCRIPT} ARGS --cflags
            OUTPUT_VARIABLE IMAGE_MAGICK_CORE_CFLAGS )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_CORE_CONFIG_SCRIPT} ARGS --cppflags
            OUTPUT_VARIABLE IMAGE_MAGICK_CORE_CPPFLAGS )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_CORE_CONFIG_SCRIPT} ARGS --exec-prefix
            OUTPUT_VARIABLE IMAGE_MAGICK_CORE_EXEC_PREFIX )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_CORE_CONFIG_SCRIPT} ARGS --ldflags
            OUTPUT_VARIABLE IMAGE_MAGICK_CORE_LDFLAGS )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_CORE_CONFIG_SCRIPT} ARGS --libs
            OUTPUT_VARIABLE IMAGE_MAGICK_CORE_LIBS )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_CORE_CONFIG_SCRIPT} ARGS --prefix
            OUTPUT_VARIABLE IMAGE_MAGICK_CORE_PREFIX )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_CORE_CONFIG_SCRIPT} ARGS --version
            OUTPUT_VARIABLE IMAGE_MAGICK_CORE_VERSION )

    MESSAGE ( STATUS "Found ImageMagickCore configuration helper script ${IMAGE_MAGICK_CORE_CONFIG_SCRIPT}" )
    MESSAGE ( STATUS "  ImageMagickCore cflags: ${IMAGE_MAGICK_CORE_CFLAGS}" )
    MESSAGE ( STATUS "  ImageMagickCore cppflags: ${IMAGE_MAGICK_CORE_CPPFLAGS}" )
    MESSAGE ( STATUS "  ImageMagickCore exec-prefix: ${IMAGE_MAGICK_CORE_EXEC_PREFIX}" )
    MESSAGE ( STATUS "  ImageMagickCore ldflags: ${IMAGE_MAGICK_CORE_LDFLAGS}" )
    MESSAGE ( STATUS "  ImageMagickCore libs: ${IMAGE_MAGICK_CORE_LIBS}" )
    MESSAGE ( STATUS "  ImageMagickCore prefix: ${IMAGE_MAGICK_CORE_PREFIX}" )
    MESSAGE ( STATUS "  ImageMagickCore version: ${IMAGE_MAGICK_CORE_VERSION}" )

    EXEC_PROGRAM ( ${IMAGE_MAGICK_CORE_CONFIG_SCRIPT} ARGS --cppflags | cut -d ' ' -f '1-1' | sed 's/-I//g' | sed 's/-I//g'
            OUTPUT_VARIABLE IMAGE_MAGICK_CORE_CONFIG_HEADERS )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_CORE_CONFIG_SCRIPT} ARGS --libs
            OUTPUT_VARIABLE IMAGE_MAGICK_CORE_CONFIG_FLAGS )
ELSE (  )
    IF ( IMAGE_MAGICK_CORE_CONFIG_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find ImageMagickCore configuration helper script" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find ImageMagickCore configuration helper script" )
    ENDIF (  )
ENDIF (  )

IF ( IMAGE_MAGICKPP_CONFIG_FOUND )
    EXEC_PROGRAM ( ${IMAGE_MAGICKPP_CONFIG_SCRIPT} ARGS --cppflags
            OUTPUT_VARIABLE IMAGE_MAGICKPP_CPPFLAGS )
    EXEC_PROGRAM ( ${IMAGE_MAGICKPP_CONFIG_SCRIPT} ARGS --cxxflags
            OUTPUT_VARIABLE IMAGE_MAGICKPP_CXXFLAGS )
    EXEC_PROGRAM ( ${IMAGE_MAGICKPP_CONFIG_SCRIPT} ARGS --exec-prefix
            OUTPUT_VARIABLE IMAGE_MAGICKPP_EXEC_PREFIX )
    EXEC_PROGRAM ( ${IMAGE_MAGICKPP_CONFIG_SCRIPT} ARGS --ldflags
            OUTPUT_VARIABLE IMAGE_MAGICKPP_LDFLAGS )
    EXEC_PROGRAM ( ${IMAGE_MAGICKPP_CONFIG_SCRIPT} ARGS --libs
            OUTPUT_VARIABLE IMAGE_MAGICKPP_LIBS )
    EXEC_PROGRAM ( ${IMAGE_MAGICKPP_CONFIG_SCRIPT} ARGS --prefix
            OUTPUT_VARIABLE IMAGE_MAGICKPP_PREFIX )
    EXEC_PROGRAM ( ${IMAGE_MAGICKPP_CONFIG_SCRIPT} ARGS --version
            OUTPUT_VARIABLE IMAGE_MAGICKPP_VERSION )

    MESSAGE ( STATUS "Found ImageMagick++ configuration helper script ${IMAGE_MAGICKPP_CONFIG_SCRIPT}" )
    MESSAGE ( STATUS "  ImageMagick++ cppflags: ${IMAGE_MAGICKPP_CPPFLAGS}" )
    MESSAGE ( STATUS "  ImageMagick++ cxxflags: ${IMAGE_MAGICKPP_CXXFLAGS}" )
    MESSAGE ( STATUS "  ImageMagick++ exec-prefix: ${IMAGE_MAGICKPP_EXEC_PREFIX}" )
    MESSAGE ( STATUS "  ImageMagick++ ldflags: ${IMAGE_MAGICKPP_LDFLAGS}" )
    MESSAGE ( STATUS "  ImageMagick++ libs: ${IMAGE_MAGICKPP_LIBS}" )
    MESSAGE ( STATUS "  ImageMagick++ prefix: ${IMAGE_MAGICKPP_PREFIX}" )
    MESSAGE ( STATUS "  ImageMagick++ version: ${IMAGE_MAGICKPP_VERSION}" )

    EXEC_PROGRAM ( ${IMAGE_MAGICKPP_CONFIG_SCRIPT} ARGS --cppflags | cut -d ' ' -f '1-1' | sed 's/-I//g' | sed 's/-I//g'
            OUTPUT_VARIABLE IMAGE_MAGICKPP_CONFIG_HEADERS )
    EXEC_PROGRAM ( ${IMAGE_MAGICKPP_CONFIG_SCRIPT} ARGS --libs
            OUTPUT_VARIABLE IMAGE_MAGICKPP_CONFIG_FLAGS )
ELSE (  )
    IF ( IMAGE_MAGICKPP_CONFIG_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find ImageMagick++ configuration helper script" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find ImageMagick++ configuration helper script" )
    ENDIF (  )
ENDIF (  )

IF ( IMAGE_MAGICK_WAND_CONFIG_FOUND )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_WAND_CONFIG_SCRIPT} ARGS --cflags
            OUTPUT_VARIABLE IMAGE_MAGICK_WAND_CFLAGS )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_WAND_CONFIG_SCRIPT} ARGS --cppflags
            OUTPUT_VARIABLE IMAGE_MAGICK_WAND_CPPFLAGS )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_WAND_CONFIG_SCRIPT} ARGS --exec-prefix
            OUTPUT_VARIABLE IMAGE_MAGICK_WAND_EXEC_PREFIX )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_WAND_CONFIG_SCRIPT} ARGS --ldflags
            OUTPUT_VARIABLE IMAGE_MAGICK_WAND_LDFLAGS )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_WAND_CONFIG_SCRIPT} ARGS --libs
            OUTPUT_VARIABLE IMAGE_MAGICK_WAND_LIBS )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_WAND_CONFIG_SCRIPT} ARGS --prefix
            OUTPUT_VARIABLE IMAGE_MAGICK_WAND_PREFIX )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_WAND_CONFIG_SCRIPT} ARGS --version
            OUTPUT_VARIABLE IMAGE_MAGICK_WAND_VERSION )

    MESSAGE ( STATUS "Found ImageMagickWand configuration helper script ${IMAGE_MAGICK_WAND_CONFIG_SCRIPT}" )
    MESSAGE ( STATUS "  ImageMagickWand cflags: ${IMAGE_MAGICK_WAND_CFLAGS}" )
    MESSAGE ( STATUS "  ImageMagickWand cppflags: ${IMAGE_MAGICK_WAND_CPPFLAGS}" )
    MESSAGE ( STATUS "  ImageMagickWand exec-prefix: ${IMAGE_MAGICK_WAND_EXEC_PREFIX}" )
    MESSAGE ( STATUS "  ImageMagickWand ldflags: ${IMAGE_MAGICK_WAND_LDFLAGS}" )
    MESSAGE ( STATUS "  ImageMagickWand libs: ${IMAGE_MAGICK_WAND_LIBS}" )
    MESSAGE ( STATUS "  ImageMagickWand prefix: ${IMAGE_MAGICK_WAND_PREFIX}" )
    MESSAGE ( STATUS "  ImageMagickWand version: ${IMAGE_MAGICK_WAND_VERSION}" )

    EXEC_PROGRAM ( ${IMAGE_MAGICK_WAND_CONFIG_SCRIPT} ARGS --cppflags | cut -d ' ' -f '1-1' | sed 's/-I//g' | sed 's/-I//g'
            OUTPUT_VARIABLE IMAGE_MAGICK_WAND_CONFIG_HEADERS )
    EXEC_PROGRAM ( ${IMAGE_MAGICK_WAND_CONFIG_SCRIPT} ARGS --libs
            OUTPUT_VARIABLE IMAGE_MAGICK_WAND_CONFIG_FLAGS )
ELSE (  )
    IF ( IMAGE_MAGICK_WAND_CONFIG_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find ImageMagickWand configuration helper script" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find ImageMagickWand configuration helper script" )
    ENDIF (  )
ENDIF (  )


IF ( GRAPHICS_MAGICK_FOUND AND IMAGE_MAGICK_FOUND )
    SET ( MAGICK_VARIANT BOTH )
ELSEIF ( GRAPHICS_MAGICK_FOUND )
    SET ( MAGICK_VARIANT GM )
ELSEIF ( IMAGE_MAGICK_FOUND )
    SET ( MAGICK_VARIANT IM )
ELSE (  )
    IF ( MAGICK_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find Magick" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find Magick" )
        SET ( MAGICK_VARIANT NONE )
    ENDIF (  )
ENDIF (  )

IF ( GRAPHICS_MAGICKPP_FOUND AND IMAGE_MAGICKPP_FOUND )
    SET ( MAGICKPP_VARIANT BOTH )
ELSEIF ( GRAPHICS_MAGICKPP_FOUND )
    SET ( MAGICKPP_VARIANT GM )
ELSEIF ( IMAGE_MAGICKPP_FOUND )
    SET ( MAGICKPP_VARIANT IM )
ELSE (  )
    IF ( MAGICKPP_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find Magick++" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find Magick++" )
        SET ( MAGICKPP_VARIANT NONE )
    ENDIF (  )
ENDIF (  )

IF ( GRAPHICS_MAGICK_WAND_FOUND AND IMAGE_MAGICK_WAND_FOUND )
    SET ( MAGICK_WAND_VARIANT BOTH )
ELSEIF ( GRAPHICS_MAGICK_WAND_FOUND )
    SET ( MAGICK_WAND_VARIANT GM )
ELSEIF ( IMAGE_MAGICK_WAND_FOUND )
    SET ( MAGICK_WAND_VARIANT IM )
ELSE (  )
    IF ( MAGICK_WAND_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find MagickWand" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find MagickWand" )
        SET ( MAGICK_WAND_VARIANT NONE )
    ENDIF (  )
ENDIF (  )


FUNCTION ( SET_MAGICK_VARS VARIANT )
    IF ( ${VARIANT} MATCHES "IM" )
        IF ( DEFINED IMAGE_MAGICKPP_CONFIG_HEADERS )
            SET ( MAGICKPP_INCLUDE_DIR ${IMAGE_MAGICKPP_CONFIG_HEADERS} PARENT_SCOPE )
        ENDIF (  )
        IF ( DEFINED IMAGE_MAGICKPP_CONFIG_FLAGS  )
            SET ( MAGICKPP_LIBRARIES ${IMAGE_MAGICKPP_CONFIG_FLAGS} PARENT_SCOPE )
        ENDIF (  )

        IF ( DEFINED IMAGE_MAGICK_CONFIG_HEADERS )
            SET ( MAGICK_INCLUDE_DIR ${IMAGE_MAGICK_CONFIG_HEADERS} PARENT_SCOPE )
        ENDIF (  )
        IF ( DEFINED IMAGE_MAGICK_CONFIG_FLAGS  )
            SET ( MAGICK_LIBRARIES ${IMAGE_MAGICK_CONFIG_FLAGS} PARENT_SCOPE )
        ENDIF (  )

        IF ( DEFINED IMAGE_MAGICK_CORE_CONFIG_HEADERS )
            SET ( MAGICK_CORE_INCLUDE_DIR ${IMAGE_MAGICK_CORE_CONFIG_HEADERS} PARENT_SCOPE )
        ENDIF (  )
        IF ( DEFINED IMAGE_MAGICK_CORE_CONFIG_FLAGS  )
            SET ( MAGICK_CORE_LIBRARIES ${IMAGE_MAGICK_CORE_CONFIG_FLAGS} PARENT_SCOPE )
        ENDIF (  )

        IF ( DEFINED IMAGE_MAGICK_WAND_CONFIG_HEADERS )
            SET ( MAGICK_WAND_INCLUDE_DIR ${IMAGE_MAGICK_WAND_CONFIG_HEADERS} PARENT_SCOPE )
        ENDIF (  )
        IF ( DEFINED IMAGE_MAGICK_WAND_CONFIG_FLAGS  )
            SET ( MAGICK_WAND_LIBRARIES ${IMAGE_MAGICK_WAND_CONFIG_FLAGS} PARENT_SCOPE )
        ENDIF (  )
    ELSE (  )
        IF ( DEFINED GRAPHICS_MAGICKPP_CONFIG_HEADERS )
            SET ( MAGICKPP_INCLUDE_DIR ${GRAPHICS_MAGICKPP_CONFIG_HEADERS} PARENT_SCOPE )
        ENDIF (  )
        IF ( DEFINED GRAPHICS_MAGICKPP_CONFIG_FLAGS  )
            SET ( MAGICKPP_LIBRARIES ${GRAPHICS_MAGICKPP_CONFIG_FLAGS} PARENT_SCOPE )
        ENDIF (  )

        IF ( DEFINED GRAPHICS_MAGICK_CONFIG_HEADERS )
            SET ( MAGICK_INCLUDE_DIR ${GRAPHICS_MAGICK_CONFIG_HEADERS} PARENT_SCOPE )
        ENDIF (  )
        IF ( DEFINED GRAPHICS_MAGICK_CONFIG_FLAGS  )
            SET ( MAGICK_LIBRARIES ${GRAPHICS_MAGICK_CONFIG_FLAGS} PARENT_SCOPE )
        ENDIF (  )

        IF ( DEFINED GRAPHICS_MAGICK_WAND_CONFIG_HEADERS )
            SET ( MAGICK_WAND_INCLUDE_DIR ${GRAPHICS_MAGICK_WAND_CONFIG_HEADERS} PARENT_SCOPE )
        ENDIF (  )
        IF ( DEFINED GRAPHICS_MAGICK_WAND_CONFIG_FLAGS  )
            SET ( MAGICK_WAND_LIBRARIES ${GRAPHICS_MAGICK_WAND_CONFIG_FLAGS} PARENT_SCOPE )
        ENDIF (  )
    ENDIF (  )
ENDFUNCTION ( SET_MAGICK_VARS )


IF ( ${MAGICKPP_VARIANT} MATCHES "BOTH"
        OR ${MAGICK_VARIANT} MATCHES "BOTH"
        OR ${MAGICK_WAND_VARIANT} MATCHES "BOTH" )
    IF ( ${PREFERRED_MAGICK_IMPLEMENTATION} MATCHES "GM" )
        MESSAGE ( STATUS "Preferred GraphicsMagick over ImageMagick" )
        SET_MAGICK_VARS ( GM )
    ELSEIF ( ${PREFERRED_MAGICK_IMPLEMENTATION} MATCHES "IM" )
        MESSAGE ( STATUS "Preferred ImageMagick over GraphicsMagick" )
        SET_MAGICK_VARS ( IM )
    ELSE (  )
        MESSAGE ( STATUS "Preferred GraphicsMagick over ImageMagick" )
        SET ( PREFERRED_MAGICK_IMPLEMENTATION "GM" )
        SET_MAGICK_VARS ( GM )
    ENDIF (  )
ELSEIF ( ${MAGICKPP_VARIANT} MATCHES "GM"
        OR ${MAGICK_VARIANT} MATCHES "GM"
        OR ${MAGICK_WAND_VARIANT} MATCHES "GM" )
        SET_MAGICK_VARS ( GM )
ELSEIF ( ${MAGICKPP_VARIANT} MATCHES "IM"
        OR ${MAGICK_VARIANT} MATCHES "IM"
        OR ${MAGICK_WAND_VARIANT} MATCHES "IM" )
        SET_MAGICK_VARS ( IM )
ENDIF (  )
