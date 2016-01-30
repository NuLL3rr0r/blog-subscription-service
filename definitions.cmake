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


# WARNING: YOU MUST CHANGE THESE VALUES !!
# ASCII to HEX converter  :  http://www.dolcevie.com/js/converter.html
# Random string generator :  https://strongpasswordgenerator.com/
# ROOT_USERNAME: 4 to 16 chars
# ROOT_INITIAL_EMAIL: ANY_VALID_EMAIL
# ROOT_INITIAL_PASSWORD: 8 to 24 chars
# CRYPTO_KEY: exactly 16, 24 or 32 chars
# CRYPTO_IV: exactly 16, 24 or 32 chars
SET ( ROOT_USERNAME         "72:6f:6f:74" CACHE STRING "" ) # root
SET ( ROOT_INITIAL_EMAIL    "6e:6f:2d:72:65:70:6c:79:40:62:61:62:61:65:69:2e:6e:65:74" CACHE STRING "" ) # no-reply@babaei.net
SET ( ROOT_INITIAL_PASSWORD "41:35:34:38:33:37:2e:65:50:58:32:72:42:39:39:42:34:34:38:53:24:36:50:54" CACHE STRING "" ) # A54837.ePX2rB99B448S$6PT
SET ( CRYPTO_KEY            "52:7c:62:5d:2b:2e:27:54:55:5d:7b:72:68:7e:2f:75:40:7a:32:7e:5f:3d:68:2b:3e:38:42:4a:37:7d:38:78" CACHE STRING "" ) # R|b]+.'TU]{rh~/u@z2~_=h+>8BJ7}8x
SET ( CRYPTO_IV             "45:2b:25:5d:2d:3b:32:3f:41:79:21:29:66:39:73:3e:7b:5e:4b:23:22:46:36:27:35:7d:2d:22:76:38:21:64" CACHE STRING "" ) # E+%]-;2?Ay!)f9s>{^K#"F6'5}-"v8!d

SET ( DATABASE_BACKEND "PGSQL" CACHE STRING "" )
SET_PROPERTY( CACHE DATABASE_BACKEND PROPERTY STRINGS "SQLITE3" "PGSQL" "MYSQL" )

SET ( SQLITE3_DATABASE_FILE_PATH "../db/" CACHE STRING "" )
SET ( SQLITE3_DATABASE_FILE_NAME "subscription.db" CACHE STRING "" )

SET ( PGSQL_HOST "localhost" CACHE STRING "" )
SET ( PGSQL_PORT "5432" CACHE STRING "" )
SET ( PGSQL_DATABASE "blog_subscription_service_db" CACHE STRING "" )
SET ( PGSQL_USER "blog_subscription_service_user" CACHE STRING "" )
SET ( PGSQL_PASSWORD "BE_SURE_TO_USE_A_STRONG_SECRET_PASSPHRASE_HERE" CACHE STRING "" )

SET ( MYSQL_HOST "localhost" CACHE STRING "" )
SET ( MYSQL_PORT "3306" CACHE STRING "" )
SET ( MYSQL_UNIX_SOCKET "" CACHE STRING "" )
SET ( MYSQL_DATABASE "blog_subscription_service_db" CACHE STRING "" )
SET ( MYSQL_USER "blog_subscription_service_user" CACHE STRING "" )
SET ( MYSQL_PASSWORD "BE_SURE_TO_USE_A_STRONG_SECRET_PASSPHRASE_HERE" CACHE STRING "" )

SET ( LIBB64_BUFFERSIZE "16777216" CACHE STRING "" )

SET ( GEO_LITE_COUNTRY_DB_URL "http://geolite.maxmind.com/download/geoip/database/GeoLiteCountry/GeoIP.dat.gz" CACHE STRING "" )
SET ( GEO_LITE_CITY_DB_URL "http://geolite.maxmind.com/download/geoip/database/GeoLiteCity.dat.gz" CACHE STRING "" )

IF ( WIN32 )
    SET ( CORELIB_DEFINES "_UNICODE; UNICODE; WIN32_LEAN_AND_MEAN" CACHE STRING "" )
ELSE (  )
    SET ( CORELIB_DEFINES "" CACHE STRING "" )
ENDIF (  )

IF ( WIN32 )
    SET ( SERVICE_DEFINES "_UNICODE; UNICODE; WIN32_LEAN_AND_MEAN" CACHE STRING "" )
ELSE (  )
    SET ( SERVICE_DEFINES "" CACHE STRING "" )
ENDIF (  )

IF ( WIN32 )
    SET ( UTILS_DEFINES "_UNICODE; UNICODE; WIN32_LEAN_AND_MEAN" CACHE STRING "" )
ELSE (  )
    SET ( UTILS_DEFINES "" CACHE STRING "" )
ENDIF (  )


