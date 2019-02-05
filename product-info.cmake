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


SET ( PRODUCT_COMPANY_NAME "babaei.net" )
SET ( PRODUCT_COPYRIGHT_HOLDER "Mamadou Babaei" )
SET ( PRODUCT_COPYRIGHT_YEAR "2016" )
SET ( PRODUCT_COPYRIGHT "(C) ${PRODUCT_COPYRIGHT_YEAR} ${PRODUCT_COPYRIGHT_HOLDER}" )

SET ( SERVICE_PRODUCT_INTERNAL_NAME "Blog Subscription Service" )
SET ( SERVICE_PRODUCT_NAME "Blog Subscription Service" )
SET ( SERVICE_PRODUCT_VERSION_MAJOR "0" )
SET ( SERVICE_PRODUCT_VERSION_MINOR "1" )
SET ( SERVICE_PRODUCT_VERSION_PATCH "0" )
SET ( SERVICE_PRODUCT_DESCRIPTION "A multilingual blog subscription service written in C++ on top of Wt webtoolkit" )

SET ( UTILS_GEOIP_UPDATER_PRODUCT_INTERNAL_NAME "GeoIP Updater" )
SET ( UTILS_GEOIP_UPDATER_PRODUCT_NAME "GeoIP database updater" )
SET ( UTILS_GEOIP_UPDATER_PRODUCT_VERSION_MAJOR "0" )
SET ( UTILS_GEOIP_UPDATER_PRODUCT_VERSION_MINOR "1" )
SET ( UTILS_GEOIP_UPDATER_PRODUCT_VERSION_PATCH "0" )
SET ( UTILS_GEOIP_UPDATER_PRODUCT_DESCRIPTION "The GeoIP database updater utility" )

SET ( UTILS_SPAWN_FASTCGI_PRODUCT_INTERNAL_NAME "Spawn FastCGI" )
SET ( UTILS_SPAWN_FASTCGI_PRODUCT_NAME "FastCGI spawn utility" )
SET ( UTILS_SPAWN_FASTCGI_PRODUCT_VERSION_MAJOR "0" )
SET ( UTILS_SPAWN_FASTCGI_PRODUCT_VERSION_MINOR "1" )
SET ( UTILS_SPAWN_FASTCGI_PRODUCT_VERSION_PATCH "0" )
SET ( UTILS_SPAWN_FASTCGI_PRODUCT_DESCRIPTION "The FastCGI application spawn utility" )

SET ( UTILS_SPAWN_WTHTTPD_PRODUCT_INTERNAL_NAME "Spawn wthttpd" )
SET ( UTILS_SPAWN_WTHTTPD_PRODUCT_NAME "wthttpd spawn utility" )
SET ( UTILS_SPAWN_WTHTTPD_PRODUCT_VERSION_MAJOR "0" )
SET ( UTILS_SPAWN_WTHTTPD_PRODUCT_VERSION_MINOR "1" )
SET ( UTILS_SPAWN_WTHTTPD_PRODUCT_VERSION_PATCH "0" )
SET ( UTILS_SPAWN_WTHTTPD_PRODUCT_DESCRIPTION "The wthttpd application spawn utility" )

IF ( EXISTS "${PROJECT_SOURCE_DIR}/.git" )
    EXEC_PROGRAM ( ${GIT_EXECUTABLE}
        ARGS "--git-dir=\"${PROJECT_SOURCE_DIR}/.git\" --work-tree=\"${PROJECT_SOURCE_DIR}\" describe --always"
        OUTPUT_VARIABLE GIT_SHORT_REVISION_HASH )
    SET ( PRODUCT_VERSION_REVISION ${GIT_SHORT_REVISION_HASH} )
ELSE (  )
    SET ( PRODUCT_VERSION_REVISION "" )
ENDIF (  )

SET ( SERVICE_PRODUCT_VERSION "${SERVICE_PRODUCT_VERSION_MAJOR}.${SERVICE_PRODUCT_VERSION_MINOR}.${SERVICE_PRODUCT_VERSION_PATCH}" )
SET ( UTILS_GEOIP_UPDATER_PRODUCT_VERSION "${UTILS_GEOIP_UPDATER_PRODUCT_VERSION_MAJOR}.${UTILS_GEOIP_UPDATER_PRODUCT_VERSION_MINOR}.${UTILS_GEOIP_UPDATER_PRODUCT_VERSION_PATCH}" )
SET ( UTILS_SPAWN_FASTCGI_PRODUCT_VERSION "${UTILS_SPAWN_FASTCGI_PRODUCT_VERSION_MAJOR}.${UTILS_SPAWN_FASTCGI_PRODUCT_VERSION_MINOR}.${UTILS_SPAWN_FASTCGI_PRODUCT_VERSION_PATCH}" )
SET ( UTILS_SPAWN_WTHTTPD_PRODUCT_VERSION "${UTILS_SPAWN_WTHTTPD_PRODUCT_VERSION_MAJOR}.${UTILS_SPAWN_WTHTTPD_PRODUCT_VERSION_MINOR}.${UTILS_SPAWN_WTHTTPD_PRODUCT_VERSION_PATCH}" )

IF ( NOT PRODUCT_VERSION_REVISION STREQUAL "" )
    SET ( SERVICE_PRODUCT_VERSION "${SERVICE_PRODUCT_VERSION}-${PRODUCT_VERSION_REVISION}" )
    SET ( UTILS_GEOIP_UPDATER_PRODUCT_VERSION "${UTILS_GEOIP_UPDATER_PRODUCT_VERSION}-${PRODUCT_VERSION_REVISION}" )
    SET ( UTILS_SPAWN_FASTCGI_PRODUCT_VERSION "${UTILS_SPAWN_FASTCGI_PRODUCT_VERSION}-${PRODUCT_VERSION_REVISION}" )
    SET ( UTILS_SPAWN_WTHTTPD_PRODUCT_VERSION "${UTILS_SPAWN_WTHTTPD_PRODUCT_VERSION}-${PRODUCT_VERSION_REVISION}" )
ENDIF (  )
