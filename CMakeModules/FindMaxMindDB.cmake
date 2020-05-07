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


FIND_PATH ( MAXMINDDB_INCLUDE_DIR NAMES maxminddb.h PATHS /usr/include /usr/local/include )
FIND_LIBRARY ( MAXMINDDB_LIBRARY NAMES maxminddb PATHS /usr/lib /usr/local/lib )
FIND_FILE ( GEOLITE2_CITY_MMDB NAMES GeoIP/GeoLite2-City.mmdb PATHS /usr/share /usr/local/share )
FIND_FILE ( GEOLITE2_COUNTRY_MMDB NAMES GeoIP/GeoLite2-Country.mmdb PATHS /usr/share /usr/local/share )
FIND_FILE ( GEOLITE2_ASN_MMDB NAMES GeoIP/GeoLite2-ASN.mmdb PATHS /usr/share /usr/local/share )


IF ( MAXMINDDB_INCLUDE_DIR AND MAXMINDDB_LIBRARY )
    SET ( MAXMINDDB_FOUND TRUE )
ENDIF (  )

IF ( GEOLITE2_CITY_MMDB)
    SET ( GEOLITE2_CITY_MMDB_FOUND TRUE )
ENDIF (  )

IF ( GEOLITE2_COUNTRY_MMDB)
    SET ( GEOLITE2_COUNTRY_MMDB_FOUND TRUE )
ENDIF (  )

IF ( GEOLITE2_ASN_MMDB)
    SET ( GEOLITE2_ASN_MMDB_FOUND TRUE )
ENDIF (  )

IF ( MAXMINDDB_FOUND )
    MESSAGE ( STATUS "Found MaxMind DB headers in ${MAXMINDDB_INCLUDE_DIR}" )
    MESSAGE ( STATUS "Found MaxMind DB library: ${MAXMINDDB_LIBRARY}" )
ELSE (  )
    IF ( MAXMINDDB_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find MaxMind DB" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find MaxMind DB" )
    ENDIF (  )
ENDIF (  )


IF ( GEOLITE2_CITY_MMDB )
    MESSAGE ( STATUS "Found GeoLite2-City database: ${GEOLITE2_CITY_MMDB}" )
ELSE (  )
    IF ( GEOLITE2_CITY_MMDB_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find GeoLite2-City database" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find GeoLite2-City database" )
    ENDIF (  )
ENDIF (  )


IF ( GEOLITE2_COUNTRY_MMDB )
    MESSAGE ( STATUS "Found GeoLite2-Country database: ${GEOLITE2_COUNTRY_MMDB}" )
ELSE (  )
    IF ( GEOLITE2_COUNTRY_MMDB_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find GeoLite2-Country database" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find GeoLite2-Country database" )
    ENDIF (  )
ENDIF (  )


IF ( GEOLITE2_ASN_MMDB_FOUND )
    MESSAGE ( STATUS "Found GeoLite2-ASN database: ${GEOLITE2_ASN_MMDB}" )
ELSE (  )
    IF ( GEOLITE2_ASN_MMDB_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find GeoLite2-ASN database" )
    ELSE (  )
        MESSAGE ( STATUS "Could not find GeoLite2-ASN database" )
    ENDIF (  )
ENDIF (  )
