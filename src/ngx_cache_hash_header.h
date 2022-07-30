//
// Created by liuminjian on 2022/7/30.
//

#ifndef CACHE_HASH_HEADER_NGINX_MODULE_NGX_cache_hash_HEADER_H
#define CACHE_HASH_HEADER_NGINX_MODULE_NGX_cache_hash_HEADER_H

#include <ngx_core.h>
#include <ngx_http.h>

static ngx_int_t ngx_cache_hash_header_post_config(ngx_conf_t *cf);
static ngx_int_t ngx_cache_hash_header_handler(ngx_http_request_t *r);
static ngx_int_t ngx_http_add_header_helper(ngx_http_request_t *r);
static void * ngx_cache_hash_header_create_loc_conf(ngx_conf_t *cf);
void ngx_cache_hash_header_create_key(ngx_http_request_t *r, ngx_table_elt_t *ch, ngx_table_elt_t *crc32h);
void ngx_cache_hash_header_init(ngx_http_request_t *r, ngx_table_elt_t *h);

typedef struct {
    ngx_flag_t                       enable;
} ngx_cache_hash_header_loc_conf_t;

#endif //CACHE_HASH_HEADER_NGINX_MODULE_NGX_cache_hash_HEADER_H
