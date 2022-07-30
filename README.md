# cache-hash-header-nginx-module

**a nginx module for pass proxy cache key**

*This module is not distributed with the Nginx source.* See [the installation instructions](#installation).

Installation
============

Grab the nginx source code from [nginx.org](http://nginx.org/), for example,
the version 1.17.8 (see [nginx compatibility](#compatibility)), and then build the source with this module:

```bash

 wget 'http://nginx.org/download/nginx-1.17.8.tar.gz'
 tar -xzvf nginx-1.17.8.tar.gz
 cd nginx-1.17.8/

 # Here we assume you would install you nginx under /opt/nginx/.
 ./configure --prefix=/opt/nginx \
     --add-module=/path/to/cache-hash-header-nginx-module

 make
 make install
```

Description
===========

This module allows you to add proxy_cache hash header.

```nginx
 location /hello {
     cache-hash-header on;
 }
 
 GET /hello HTTP/1.0
 Host: 127.0.0.1:8082
 CustomHead: CustomHead
 Connection: close
 User-Agent: curl/7.77.0
 Accept: */*
 Cache-Hash: 3ed10a7d12d348c9187f3b6c9dc5c604
 CRC32: 3070904283
```