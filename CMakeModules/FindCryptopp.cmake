#  (The MIT License)
#
#  Copyright (c) 2016 - 2020 Mamadou Babaei
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


FIND_PATH ( CRYPTOPP_INCLUDE_DIR NAMES cryptopp PATHS /usr/include/ /usr/local/include/ )
FIND_LIBRARY ( CRYPTOPP_LIBRARY NAMES cryptopp PATHS /usr/lib /usr/local/lib )


IF ( CRYPTOPP_INCLUDE_DIR AND CRYPTOPP_LIBRARY )
    SET ( CRYPTOPP_FOUND TRUE )
ENDIF (  )


IF ( CRYPTOPP_FOUND )
    MESSAGE ( STATUS "Found Crypto++ headers in ${CRYPTOPP_INCLUDE_DIR}" )
    MESSAGE ( STATUS "Found Crypto++ library: ${CRYPTOPP_LIBRARY}" )
ELSE (  )
    IF ( CRYPTOPP_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find Crypto++" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find Crypto++" )
    ENDIF (  )
ENDIF (  )
