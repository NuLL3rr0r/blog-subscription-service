Blog Subscription Service
=========================

A multilingual blog subscription service written in C++ on top of Wt webtoolkit with high performance in mind.

I developed this subscription service based on my own requirements, for my [English](http://www.babaei.net) and [Farsi](http://fa.babaei.net) blogs. It should work elsewhere on other websites or blogs out of the box, without any hassle.


## Documentation

I used a bit of doxygen inside the code. But, I'll try to add more documentation if my schedule allows me. For now, I'll put the minimum required documentation to build, configure and execute the server.

More comes when it's ready.


## Dependencies

* C++11 is the minimum required version of C++
* Bootsrap 3.x (Included)
* Boost (bzip2, date_time, filesystem, iostreams, random, regex, system, thread, zlib)
* cereal (Included)
* CMake (Ninja, BSD Make and GNU Make are supported)
* cotire (Included)
* Crypto++
* cURLpp
* Git (Optional, required by CMake during the build process to extract the commit hash as part of the version number)
* Gulp < 4.0.0 (Upon finding Gulp automatically installs: autoprefixer, minify-html, imagemin, minify-css, sass, uglify)
* jQuery (Included)
* libarchive
* libb64
* libcurl
* libmaxminddb
* libpq
* libpqxx
* libstatgrab >= 0.90+
* libzip
* Magick++ (either ImageMagick or GraphicsMagick)
* node.js (Required by Gulp)
* npm (Required by Gulp)
* PostgreSQL
* pthread on POSIX-compliant systems
* Sodium
* VMime
* Wt (Witty) Web-toolkit


## Database

Up to version 0.3.1, CppDB was the main database connectivity library which has support for PostgreSQL, MySQL and drivatives (e.g. MariaDB, Percona), and SQLite 3. I dropped support for CppDB due to the lack of an update in years. As a result, this project won't support MySQL and SQLite 3.

Initially, due to many tradeoffs, I decided to use a combination of PostgreSQL and Cassandra since the application design requires both SQL and NoSQL data sachems; and, on top of that the in-memory persistence database Redis for improved performance. After a few benchmarks and many good reasons I decided to go the PostgreSQL only path. Furthermore, in addition to its great performance PostgreSQL supports both SQL (RDBMS) and NoSQL (true documents using JSON and JSONB; and key/value using HStore, JSON, and JSONB) schemes.

Between the database accessibility libraries I chose libpqxx over SOCI, despite the fact that SOCI was the most similar to CppDB. The reasons for this are performance and abundance of the new features.

To initialize the database and grant a user inside PostgreSQL:

    $ sudo -u postgres psql -d template1
    template1=# CREATE ROLE blog_subscription_service LOGIN ENCRYPTED PASSWORD '${SECRET_PASSWORD}' NOINHERIT VALID UNTIL 'infinity';
    template1=# CREATE DATABASE blog_subscription_service_production WITH ENCODING='UTF8' OWNER blog_subscription_service;
    template1=# \q

To verify if the database was created successfully or not:

    $ sudo -u postgres -H psql -d blog_subscription_service_production
    blog_subscription_service_production=> \q

Or:

    $ sudo -u postgres pg_dump blog_subscription_service_production

After creating the database, you have to modify _${PROJECT_ROOT}/definitions.cmake_ in one of the following ways:

1. By modifying predefined key/values with a limited control over the connection to the database:

    ```
    SET ( PGSQL_HOST "localhost" CACHE STRING "" )
    SET ( PGSQL_PORT "5432" CACHE STRING "" )
    SET ( PGSQL_DATABASE "blog-subscription-service_production" CACHE STRING "" )
    SET ( PGSQL_USER "blog-subscription-service" CACHE STRING "" )
    SET ( PGSQL_PASSWORD "BE_SURE_TO_USE_A_STRONG_SECRET_PASSPHRASE_HERE" CACHE STRING "" )
    ```

2. Or, by specifying a connection string with more control over the details:

    ```
    # Either Keyword/Value Connection Strings or Connection URIs,
    # to set more values such as:
    # host, hostaddr, port, dbname, user, password, connect_timeout,
    # client_encoding, options, application_name, fallback_application_name,
    # keepalives, keepalives_idle, keepalives_interval, keepalives_count, tty,
    # sslmode, requiressl, sslcompression, sslcert, sslkey, sslrootcert, sslcrl,
    # requirepeer, krbsrvname, gsslib, service
    # https://www.postgresql.org/docs/9.6/static/libpq-connect.html#LIBPQ-CONNSTRING
    # NOTE: If this parameter is set, the rest of the PGSQL_* variables will be ignored.
    SET ( PGSQL_CONNECTION_STRING "" CACHE STRING "" )
    ```

e.g., keyword/value connection strings:

    SET ( PGSQL_CONNECTION_STRING "" CACHE STRING "host=localhost port=5432 dbname=blog-subscription-service_production user=blog-subscription-service passwprd=BE_SURE_TO_USE_A_STRONG_SECRET_PASSPHRASE_HERE connect_timeout=10 application_name=blog-subscription-service" )

or, connection URIs:

    SET ( PGSQL_CONNECTION_STRING "" CACHE STRING "postgresql://blog-subscription-service_production:BE_SURE_TO_USE_A_STRONG_SECRET_PASSPHRASE_HERE@localhost:5432/blog-subscription-service_production?connect_timeout=10&application_name=blog-subscription-service" )


## Supported Platforms

I developed this on a x64 FreeBSD 11.0-STABLE machine and tested it on another x64 FreeBSD 11.0-RELEASE VPS successfully. I also ported this to Funtoo Linux (a variant of Gentoo). Support for Microsoft Windows needs a bit of work, I tried my best for the most parts, but did not have the time to make the whole project work on Windows, too.


## Build

You can build either using Ninja or traditional build systems such as BSD Make or GNU Make. It also supports Qt Creator as an IDE. For building inside Qt Creator which supports CMake, Ninja, and Make, simply drag CMakeLists.txt - located in the root directory of the project - to Qt Creator's main window. Alternatively follow the following instructions if you want to build from command line:

To build using Ninja:

    $ cd blog-subscription-service
    $ mkdir build
    $ cd build
    $ sudo cmake -GNinja -DCMAKE_BUILD_TYPE=ClangNativeMaxSpeedRel -DAPP_ROOT_DIR=/srv/babaei.net/subscribe ../
    $ ninja
    $ sudo ninja install

CMAKE_BUILD_TYPE possible values

* None
* Debug
* Release
* RelWithDebInfo
* MinSizeRel
* GCCMinSizeRel
* GCCMaxSpeedRel
* GCCNativeMinSizeRel
* GCCNativeMaxSpeedRel
* ClangMinSizeRel
* ClangMaxSpeedRel
* ClangNativeMinSizeRel
* ClangNativeMaxSpeedRel

__Note__: For other available options please build using CMake GUI.

To build using either BSD Make or GNU Make:

    $ cd blog-subscription-service
    $ mkdir build
    $ cd build
    $ sudo cmake -GNinja -DCMAKE_BUILD_TYPE=ClangNativeMaxSpeedRel -DAPP_ROOT_DIR=/srv/babaei.net/subscribe ../

make -j<NUMBER_OF_CPU_CORES+1>, e.g.

    $ make -j9
    $ sudo make install

Or, if you are using GNU Make on *BSD operating systems:

    $ gmake -j9
    $ sudo gmake install

__Note__: In the _${PROJECT_ROOT}_ directory you can customize the build by modifying the following files:

* build-config.cmake
* definitions.cmake
* product-info.cmake

__Security Warning__: You must change the following default values inside _definitions.cmake_:

    # WARNING: YOU MUST CHANGE THESE VALUES !!
    # ASCII to HEX converter  :  http://www.dolcevie.com/js/converter.html
    # Random string generator :  https://strongpasswordgenerator.com/
    # ROOT_USERNAME: 4 to 16 chars
    # ROOT_INITIAL_EMAIL: ANY_VALID_EMAIL
    # ROOT_INITIAL_PASSWORD: 8 to 24 chars
    # CRYPTO_KEY: exactly 16, 24 or 32 chars
    # CRYPTO_IV: exactly 16 chars as Crypto++ only supports 128-bit IVs
    SET ( ROOT_USERNAME         "72:6f:6f:74" CACHE STRING "" ) # root
    SET ( ROOT_INITIAL_EMAIL    "6e:6f:2d:72:65:70:6c:79:40:62:61:62:61:65:69:2e:6e:65:74" CACHE STRING "" ) # no-reply@babaei.net
    SET ( ROOT_INITIAL_PASSWORD "41:35:34:38:33:37:2e:65:50:58:32:72:42:39:39:42:34:34:38:53:24:36:50:54" CACHE STRING "" ) # A54837.ePX2rB99B448S$6PT
    SET ( CRYPTO_KEY            "52:7c:62:5d:2b:2e:27:54:55:5d:7b:72:68:7e:2f:75:40:7a:32:7e:5f:3d:68:2b:3e:38:42:4a:37:7d:38:78" CACHE STRING "" ) # R|b]+.'TU]{rh~/u@z2~_=h+>8BJ7}8x
    SET ( CRYPTO_IV             "45:2b:25:5d:2d:3b:32:3f:41:79:21:29:66:39:73:3e" CACHE STRING "" ) # E+%]-;2?Ay!)f9s>

And perhaps these lines (although you can change them at run-time, later):

    SET (INITAL_EN_HOME_PAGE_URL "http://www.babaei.net/" CACHE STRING "" )
    SET (INITAL_FA_HOME_PAGE_URL "http://fa.babaei.net/" CACHE STRING "" )
    SET (INITAL_EN_HOME_PAGE_TITLE "The blog of Mamadou Babaei" CACHE STRING "" )
    SET (INITAL_FA_HOME_PAGE_TITLE "وبلاگ محمد صادق بابائی" CACHE STRING "" )


## Utilities

Aside from the core library and the service application itself, there are afew small utilities that you need in order to make everything works. The source code for these utilities are inside the _Utils_ directory.

* _geoip-updater_: Updates GeoIP database
* _spawn-fastcgi_: Spawns the FastCGI process of the application
* _spawn-wthttpd_: Spawns wthttpd-server version of the application (Recommended)

## spawn-wthttpd.db and Nginx example configuration

#### spawn-wthttpd Configuration:

_spawn-wthttpd_ configuration file should be placed inside _${APP_ROOT_DIRECTORY}/db_ directory. You may want to consult [wthttpd documentation](http://www.webtoolkit.eu/wt/doc/reference/html/InstallationUnix.html) for more info. I also recommend to take a look at [${APP_ROOT_DIRECTORY}/etc/wt_config.xml configuration documentation](http://www.webtoolkit.eu/wt/doc/reference/html/overview.html#configuration_sec).

    {
        "apps" :
        [
            {
                "app"                     :  "/srv/babaei.net/subscribe/www/subscribe.app",
                "workdir"                 :  "/srv/babaei.net/subscribe/www/",
                "threads"                 :  "-1",
                "servername"              :  "",
                "docroot"                 :  "/srv/babaei.net/subscribe/www/;/favicon.ico,/css,/fonts,/img,/js,/resources",
                "approot"                 :  "/srv/babaei.net/subscribe/www/",
                "errroot"                 :  "/srv/.nginx/err/",
                "accesslog"               :  "",
                "compression"             :  "no",
                "deploy-path"             :  "/",
                "session-id-prefix"       :  "",
                "pid-file"                :  "/srv/babaei.net/subscribe/tmp/subscribe.app",
                "config"                  :  "/srv/babaei.net/subscribe/etc/wt_config.xml",
                "max-memory-request-size" :  "8388608",
                "gdb"                     :  "no",
                "http-address"            :  "127.0.0.1",
                "http-port"               :  "10101",
                "https-address"           :  "",
                "https-port"              :  "",
                "ssl-certificate"         :  "",
                "ssl-private-key"         :  "",
                "ssl-tmp-dh"              :  "",
                "ssl-enable-v3"           :  "",
                "ssl-client-verification" :  "",
                "ssl-verify-depth"        :  "",
                "ssl-ca-certificates"     :  "",
                "ssl-cipherlist"          :  ""
            }
        ]
    }

#### Nginx Configuration

    http {
        map $http_upgrade $connection_upgrade {
            default upgrade;
            '' close;
        }

        upstream BlogSubscriptionService {
            server 127.0.0.1:10101;
        }

        server {
            listen        80;
            server_tokens off;
            server_name   subscribe.babaei.net;

            #error_log     /srv/babaei.net/log/blog-subscription-service_error_log;
            #access_log    /srv/babaei.net/log/blog-subscription-service_access_log;

            charset utf-8;
            merge_slashes on;

            location / {
                gzip off;

                proxy_read_timeout 300;
                proxy_connect_timeout 300;
                proxy_redirect off;

                proxy_http_version 1.1;
                proxy_set_header Upgrade $http_upgrade;
                proxy_set_header Connection $connection_upgrade;
                proxy_set_header Host $http_host;
                proxy_set_header X-Real-IP $remote_addr;
                proxy_set_header X-Forwarded-Ssl on;
                proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
                proxy_set_header X-Forwarded-Proto $scheme;
                proxy_set_header X-Frame-Options SAMEORIGIN;

                proxy_pass http://BlogSubscriptionService;
            }
        }
    }

#### Cron Jobs

Add the following [cron jobs](http://www.babaei.net/blog/2015/06/11/the-proper-way-of-adding-a-cron-job/) to spawn the application process at startup:

    # At Boot
    @reboot     /srv/babaei.net/subscribe/bin/spawn-wthttpd

Or, if you are not confident about _spawn-wthttpd_ (you think it may crash):

    # wthttpd auto-spawn & tracker
    # every 1 minute
    */1     *       *       *       *       /srv/babaei.net/subscribe/bin/spawn-wthttpd


## spawn-fastcgi.db and Nginx example configuration

### spawn-fastcgi.db Configuration

_sspawn-fastcgi.db_ configuration file should be placed inside _${APP_ROOT_DIRECTORY}/db_ directory. You may want to consult [Wt FastCGI documentation](http://www.webtoolkit.eu/wt/doc/reference/html/InstallationUnix.html) for more info. I also recommend to take a look at [${APP_ROOT_DIRECTORY}/etc/wt_config.xml configuration documentation](http://www.webtoolkit.eu/wt/doc/reference/html/overview.html#configuration_sec).

    {
        "apps" :
        [
            {
                "app"     :  "/srv/babaei.net/subscribe/www/subscribe.app",
                "config"  :  "/srv/babaei.net/subscribe/etc/wt_config.xml",
                "socket"  :  "/srv/babaei.net/subscribe/tmp/blog-subscription-service.socket",
                "workdir" :  "/srv/babaei.net/subscribe/www/"
            }
        ]
    }

#### Nginx Configuration

    http {
        server {
            listen        80;
            server_tokens off;
            server_name   subscribe.babaei.net;

            #error_log     /srv/babaei.net/log/blog-subscription-service_error_log;
            #access_log    /srv/babaei.net/log/blog-subscription-service_access_log;

            root           /srv/babaei.net/subscribe/www;
            index          index.html subscribe.app;
            fastcgi_index  subscribe.app;

            charset utf-8;
            merge_slashes on;

            location /subscribe.app {
                fastcgi_pass   unix:/srv/babaei.net/subscribe/tmp/blog-subscription-service.socket;

                fastcgi_param  QUERY_STRING       $query_string;
                fastcgi_param  REQUEST_METHOD     $request_method;
                fastcgi_param  CONTENT_TYPE       $content_type;
                fastcgi_param  CONTENT_LENGTH     $content_length;

                if ($document_uri ~ "^/subscribe.app/(.*)") {
                    set $apache_path_info /$1;
                }

                fastcgi_param  SCRIPT_NAME        /babaei.net/subscribe/www/subscribe.app;
                fastcgi_param  PATH_INFO          $apache_path_info;
                fastcgi_param  REQUEST_URI        $request_uri;
                fastcgi_param  DOCUMENT_URI       $document_uri;
                fastcgi_param  DOCUMENT_ROOT      $document_root;
                fastcgi_param  SERVER_PROTOCOL    $server_protocol;

                fastcgi_param  GATEWAY_INTERFACE  CGI/1.1;
                fastcgi_param  SERVER_SOFTWARE    nginx/$nginx_version;

                fastcgi_param  REMOTE_ADDR        $remote_addr;
                fastcgi_param  REMOTE_PORT        $remote_port;
                fastcgi_param  SERVER_ADDR        $server_addr;
                fastcgi_param  SERVER_PORT        $server_port;
                fastcgi_param  SERVER_NAME        $server_name;
            }
        }
    }

#### Cron Jobs

Add the following [cron jobs](http://www.babaei.net/blog/2015/06/11/the-proper-way-of-adding-a-cron-job/) to spawn the application process at startup:

    # At Boot
    @reboot     /srv/babaei.net/subscribe/bin/spawn-fastcgi

Or, if you are not confident about _spawn-fastcgi_ (you think it may crash):

    # FastCGI auto-spawn & tracker
    # every 1 minute
    */1     *       *       *       *       /srv/babaei.net/subscribe/bin/spawn-fastcgi


## Update GeoIP Database Automatically

Add the following [cron jobs](http://www.babaei.net/blog/2015/06/11/the-proper-way-of-adding-a-cron-job/):

    # GeoIP updater
    # every 7 days at 00:30am UTC
    30      00      1,8,15,22,28    *       *       /srv/babaei.net/subscribe/bin/geoupdater


## Sreenshots

![Admin Login](/Screenshots/001.png?raw=true "Admin Login")

![Admin Dashboard](/Screenshots/002.png?raw=true "Admin Dashboard")

![Admin Dashboard (FA)](/Screenshots/003.png?raw=true "Admin Dashboard (FA)")

![Newsletter Editor](/Screenshots/004.png?raw=true "Newsletter Editor")

![Subscribers List](/Screenshots/005.png?raw=true "Subscribers List")

![Contacts Editor (Mobile Responsive)](/Screenshots/006.png?raw=true "Contacts Editor (Mobile Responsive)")

![Contacts Editor (FA)](/Screenshots/007.png?raw=true "Contacts Editor (FA)")

![Settings (Mobile Responsive / FA)](/Screenshots/008.png?raw=true "Settings (Mobile Responsive / FA)")

![System Monitor](/Screenshots/009.png?raw=true "System Monitor")

![System Monitor (Mobile Responsive / Menu)](/Screenshots/010.png?raw=true "System Monitor (Mobile Responsive / Menu)")

![Subscribe Form (Mobile Responsive)](/Screenshots/011.png?raw=true "Subscribe Form (Mobile Responsive)")

![Subscribe Form (FA)](/Screenshots/012.png?raw=true "Subscribe Form (FA)")

![Contact Form](/Screenshots/013.png?raw=true "Contact Form")


## User Guide

[Visit this blog post for more information](https://www.babaei.net/blog/my-reddit-wallpaper-downloader-script).

## License

(The MIT License)

Copyright (c) 2016 - 2019 Mamadou Babaei

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the ‘Software’), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED ‘AS IS’, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
