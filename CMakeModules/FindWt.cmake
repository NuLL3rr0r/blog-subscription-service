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


FIND_PATH ( WT_INCLUDE_DIR NAMES Wt PATHS /usr/include/ /usr/local/include/ )
FIND_LIBRARY ( WT_LIBRARY NAMES wt PATHS /usr/lib/ /usr/local/lib/ )
FIND_LIBRARY ( WT_FCGI_LIBRARY NAMES wtfcgi PATHS /usr/lib/ /usr/local/lib/ )
FIND_LIBRARY ( WT_HTTPD_LIBRARY NAMES wthttp PATHS /usr/lib/ /usr/local/lib/ )
FIND_PATH ( WT_RESOURCES_DIR NAMES Wt/resources PATHS /usr/share/ /usr/local/share/ )


IF ( WT_INCLUDE_DIR AND WT_LIBRARY )
    SET ( WT_FOUND TRUE )
ENDIF (  )

IF ( WT_FCGI_LIBRARY )
    SET ( WT_FCGI_FOUND TRUE )
ENDIF (  )

IF ( WT_HTTPD_LIBRARY )
    SET ( WT_HTTPD_FOUND TRUE )
ENDIF (  )

IF ( WT_RESOURCES_DIR )
    SET ( WT_RESOURCES_FOUND TRUE )
ENDIF (  )


IF ( WT_FOUND )
    MESSAGE ( STATUS "Found Wt headers in ${WT_INCLUDE_DIR}" )
    MESSAGE ( STATUS "Found Wt library: ${WT_LIBRARY}" )
ELSE (  )
    IF ( WT_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find Wt" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find Wt" )
    ENDIF (  )
ENDIF (  )


IF ( WT_FCGI_FOUND )
    MESSAGE ( STATUS "Found Wt-FastCGI library: ${WT_FCGI_LIBRARY}" )
ELSE ( WT_FCGI_FOUND )
    IF ( WT_FCGI_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find Wt-FastCGI" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find Wt-FastCGI" )
    ENDIF (  )
ENDIF (  )


IF ( WT_HTTPD_FOUND )
    MESSAGE ( STATUS "Found Wt-Httpd library: ${WT_HTTPD_LIBRARY}" )
ELSE ( WT_HTTPD_FOUND )
    IF ( WT_HTTPD_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find Wt-Httpd" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find Wt-Httpd" )
    ENDIF (  )
ENDIF (  )


IF ( WT_RESOURCES_FOUND )
    SET ( WT_RESOURCES_DIR "${WT_RESOURCES_DIR}/Wt/resources" )
    MESSAGE ( STATUS "Found Wt resources: ${WT_RESOURCES_DIR}" )
ELSE ( WT_RESOURCES_FOUND )
    IF ( WT_RESOURCES_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find Wt resources" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find Wt resources" )
    ENDIF (  )
ENDIF (  )
