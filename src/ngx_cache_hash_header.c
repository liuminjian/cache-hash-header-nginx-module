//
// Created by liuminjian on 2022/7/30.
//
#include <ngx_config.h>
#include "ngx_cache_hash_header.h"
#include <ngx_md5.h>
#include <ngx_core.h>
#include <nginx.h>
#include <stdlib.h>
#include <stdio.h>

static ngx_command_t  ngx_cache_hash_header_commands[] = {

        {ngx_string("cache_hash_header"),
         NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
         ngx_conf_set_flag_slot,
         NGX_HTTP_LOC_CONF_OFFSET,
         offsetof(ngx_cache_hash_header_loc_conf_t, enable),
         NULL},
};

static ngx_http_module_t  ngx_cache_hash_header_module_ctx = {
        NULL,                                   /* preconfiguration */
        ngx_cache_hash_header_post_config,      /* postconfiguration */

        NULL, /* create main configuration */
        NULL,                                   /* init main configuration */

        NULL,                                   /* create server configuration */
        NULL,                                   /* merge server configuration */

        ngx_cache_hash_header_create_loc_conf,  /* create location configuration */
        NULL    /* merge location configuration */
};

ngx_module_t  ngx_cache_hash_header_module = {
        NGX_MODULE_V1,
        &ngx_cache_hash_header_module_ctx,   /* module context */
        ngx_cache_hash_header_commands,      /* module directives */
        NGX_HTTP_MODULE,                       /* module type */
        NULL,                                  /* init master */
        NULL,                                  /* init module */
        NULL,                                  /* init process */
        NULL,                                  /* init thread */
        NULL,                                  /* exit thread */
        NULL,                                  /* exit process */
        NULL,                                  /* exit master */
        NGX_MODULE_V1_PADDING
};

static ngx_int_t
ngx_cache_hash_header_post_config(ngx_conf_t *cf)
{
    ngx_http_handler_pt             *h;
    ngx_http_core_main_conf_t       *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
    // 在nginx的NGX_HTTP_REWRITE_PHASE阶段挂载ngx_cache_hash_header_handler
    h = ngx_array_push(&cmcf->phases[NGX_HTTP_REWRITE_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }
    *h = ngx_cache_hash_header_handler;
    return NGX_OK;
}

static ngx_int_t
ngx_cache_hash_header_handler(ngx_http_request_t *r)
{
    ngx_int_t                            rc;
    ngx_uint_t                           i;

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "headers more rewrite handler, uri \"%V\"", &r->uri);

    ngx_cache_hash_header_loc_conf_t       *chcf;

    chcf = ngx_http_get_module_loc_conf(r, ngx_cache_hash_header_module);

    if (chcf->enable <= 0) {
        return NGX_DECLINED;
    }

    rc = ngx_http_add_header_helper(r);

    if (rc != NGX_OK) {
        return rc;
    }

    return NGX_DECLINED;
}

static ngx_int_t
ngx_http_add_header_helper(ngx_http_request_t *r)
{
    ngx_table_elt_t             *ch;
    ngx_table_elt_t             *crc32h;
    // 增加两个header Cache-Hash 和 CRC32
    ch = ngx_list_push(&r->headers_in.headers);
    if (ch == NULL) {
        return NGX_ERROR;
    }
    crc32h = ngx_list_push(&r->headers_in.headers);
    if (crc32h == NULL) {
        return NGX_ERROR;
    }
    // 初始化hash header
    ngx_cache_hash_header_init(r, ch);
    ngx_cache_hash_header_init(r, crc32h);
    // 设置header的key
    ngx_str_set(&ch->key, "Cache-Hash");
    ngx_str_set(&crc32h->key, "CRC32");
    // 设置header的value
    ngx_cache_hash_header_create_key(r, ch, crc32h);
    return NGX_OK;
}

static void *
ngx_cache_hash_header_create_loc_conf(ngx_conf_t *cf)
{
    ngx_cache_hash_header_loc_conf_t    *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_cache_hash_header_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }
    conf->enable = NGX_CONF_UNSET;
    return conf;
}

void
ngx_cache_hash_header_create_key(ngx_http_request_t *r, ngx_table_elt_t *ch, ngx_table_elt_t *crc32h)
{
    u_char             key[NGX_HTTP_CACHE_KEY_LEN];
    ngx_md5_t          md5;
    uint32_t           crc32;
    u_char             *p;
    u_char             crc32p[11];
    // 获取cache key对应的hash值
    ngx_crc32_init(crc32);
    ngx_md5_init(&md5);
    ngx_crc32_update(&crc32, r->uri.data, r->uri.len);
    ngx_md5_update(&md5, r->uri.data, r->uri.len);
    ngx_crc32_final(crc32);
    ngx_md5_final(key, &md5);
    p = ngx_pnalloc(r->pool, 2 * NGX_HTTP_CACHE_KEY_LEN + 1);
    ngx_hex_dump(p, key, NGX_HTTP_CACHE_KEY_LEN);
    ch->value.data = p;
    ch->value.len = 2 * NGX_HTTP_CACHE_KEY_LEN;
    ch->lowcase_key = ngx_pnalloc(r->pool, ch->key.len);
    ngx_strlow(ch->lowcase_key, ch->key.data, ch->key.len);
    // 设置crc32的header
    ngx_sprintf(crc32p,"%i", crc32);
    p = ngx_pnalloc(r->pool, ngx_strlen(crc32p) + 1);
    ngx_cpystrn(p, crc32p, ngx_strlen(crc32p) + 1);
    crc32h->value.data = p;
    crc32h->value.len = ngx_strlen(crc32p);
    crc32h->lowcase_key = ngx_pnalloc(r->pool, crc32h->key.len);
    ngx_strlow(crc32h->lowcase_key, crc32h->key.data, crc32h->key.len);
}

void
ngx_cache_hash_header_init(ngx_http_request_t *r, ngx_table_elt_t *h)
{
    h->hash = r->header_hash;
    h->key.len = r->header_name_end - r->header_name_start;
    h->key.data = r->header_name_start;
    h->key.data[h->key.len] = '\0';
    h->value.len = r->header_end - r->header_start;
    h->value.data = r->header_start;
    h->value.data[h->value.len] = '\0';
}