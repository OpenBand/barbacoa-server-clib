#pragma once

#include "common.h"

#include <server_clib/jsmn.h>

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    const char* pline;
    jsmntok_t* ptok;
    jsmntok_t** pptok_array;
} srv_c_config_ctx_t;

typedef BOOL (*srv_c_config_get_value_ft)(const char* key, const srv_c_config_ctx_t* pctx);

BOOL srv_c_config_load(const char* path_to_config, srv_c_config_get_value_ft callback);

BOOL srv_c_config_get_string(const srv_c_config_ctx_t* pctx, char* buff, const size_t sz);
BOOL srv_c_config_get_int(const srv_c_config_ctx_t* pctx, int* val);
BOOL srv_c_config_get_bool(const srv_c_config_ctx_t* pctx, BOOL* val);

BOOL srv_c_config_get_string_array(
    const srv_c_config_ctx_t* pctx, char** buff, size_t* sz, const size_t item_sz, const size_t max_sz);
BOOL srv_c_config_get_int_array(const srv_c_config_ctx_t* pctx, int** val, size_t* sz, const size_t max_sz);
BOOL srv_c_config_get_bool_array(const srv_c_config_ctx_t* pctx, int** val, size_t* sz, const size_t max_sz);

// read config string as local time (format - for ex. '%d.%m.%Y %H:%M:%S')
BOOL srv_c_config_get_local_time(const srv_c_config_ctx_t* pctx, const char* format, time_t* val);
// read config string as UTC time
BOOL srv_c_config_get_utc_time(const srv_c_config_ctx_t* pctx, const char* format, time_t* val);

#ifdef __cplusplus
}
#endif
