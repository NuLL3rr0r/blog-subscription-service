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


FIND_PATH ( GEOIP_INCLUDE_DIR NAMES GeoIP.h PATHS /usr/include/ /usr/local/include/ )
FIND_LIBRARY ( GEOIP_LIBRARY NAMES GeoIP PATHS /usr/lib /usr/local/lib )
FIND_FILE ( GEOLITE_COUNTRY_DB NAMES GeoIP/GeoIP.dat PATHS /usr/share/ /usr/local/share/ )
FIND_FILE ( GEOLITE_CITY_DB NAMES GeoIP/GeoLiteCity.dat PATHS /usr/share/ /usr/local/share/ )


IF ( GEOIP_INCLUDE_DIR AND GEOIP_LIBRARY )
    SET ( GEOIP_FOUND TRUE )
ENDIF (  )

IF ( GEOLITE_COUNTRY_DB)
    SET ( GEOLITE_COUNTRY_FOUND TRUE )
ENDIF (  )

IF ( GEOLITE_CITY_DB)
    SET ( GEOLITE_CITY_FOUND TRUE )
ENDIF (  )


IF ( GEOIP_FOUND )
    MESSAGE ( STATUS "Found GeoIP headers in ${GEOIP_INCLUDE_DIR}" )
    MESSAGE ( STATUS "Found GeoIP library: ${GEOIP_LIBRARY}" )
ELSE (  )
    IF ( GEOIP_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find GeoIP" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find GeoIP" )
    ENDIF (  )
ENDIF (  )


IF ( GEOLITE_COUNTRY_FOUND )
    MESSAGE ( STATUS "Found GeoLite-Country database: ${GEOLITE_COUNTRY_DB}" )
ELSE (  )
    IF ( GEOLITE_COUNTRY_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find GeoLite-Country database" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find GeoLite-Country database" )
    ENDIF (  )
ENDIF (  )


IF ( GEOLITE_CITY_FOUND )
    MESSAGE ( STATUS "Found GeoLite-City database: ${GEOLITE_CITY_DB}" )
ELSE (  )
    IF ( GEOLITE_CITY_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find GeoLite-City database" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find GeoLite-City database" )
    ENDIF (  )
ENDIF (  )


