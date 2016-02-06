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


SET ( CORELIB_LINK_TYPE "STATIC" CACHE STRING "" )
SET_PROPERTY( CACHE CORELIB_LINK_TYPE PROPERTY STRINGS "STATIC" "SHARED" )

SET ( WT_APPLICATION_TYPE "WTHTTPD" CACHE STRING "" )
SET_PROPERTY( CACHE WT_APPLICATION_TYPE PROPERTY STRINGS "WTHTTPD" "FASTCGI" )

SET ( CMAKE_BUILD_TYPE "Release" CACHE STRING "" )
SET_PROPERTY( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "None" "Debug" "Release" "RelWithDebInfo" "MinSizeRel" "GCCMinSizeRel" "GCCMaxSpeedRel" "GCCNativeMinSizeRel" "GCCNativeMaxSpeedRel" "ClangMinSizeRel" "ClangMaxSpeedRel" "ClangNativeMinSizeRel" "ClangNativeMaxSpeedRel" )

# Reduces executables size by using GNU strip (GNU Binutils).
# Note that this is GNU toolchain only"
SET ( GCC_STRIP_EXECUTABLES "YES" CACHE STRINGS "" )
SET_PROPERTY( CACHE GCC_STRIP_EXECUTABLES PROPERTY STRINGS "YES" "NO" )

SET ( APP_ROOT_DIR /srv/babaei.net/subscribe CACHE PATH "" )

SET ( BUILD_SERVICE "YES" CACHE STRING "" )
SET_PROPERTY( CACHE BUILD_SERVICE PROPERTY STRINGS "YES" "NO" )

SET ( BUILD_UTILS_GEOIP_UPDATER "YES" CACHE STRING "" )
SET_PROPERTY( CACHE BUILD_UTILS_GEOIP_UPDATER PROPERTY STRINGS "YES" "NO" )

SET ( BUILD_UTILS_SPAWN_FASTCGI "YES" CACHE STRING "" )
SET_PROPERTY( CACHE BUILD_UTILS_SPAWN_FASTCGI PROPERTY STRINGS "YES" "NO" )

SET ( BUILD_UTILS_SPAWN_WTHTTPD "YES" CACHE STRING "" )
SET_PROPERTY( CACHE BUILD_UTILS_SPAWN_WTHTTPD PROPERTY STRINGS "YES" "NO" )

SET ( CORELIB_BIN_NAME "core" CACHE STRING "" )
SET ( SERVICE_BIN_NAME "subscribe.app" CACHE STRING "" )
SET ( UTILS_GEOIP_UPDATER_BIN_NAME "geoip-updater" CACHE STRING "" )
SET ( UTILS_SPAWN_FASTCGI_BIN_NAME "spawn-fastcgi" CACHE STRING "" )
SET ( UTILS_SPAWN_WTHTTPD_BIN_NAME "spawn-wthttpd" CACHE STRING "" )

