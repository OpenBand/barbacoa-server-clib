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
} config_ctx_t;

typedef BOOL (*config_get_value_ft)(const char* key, const config_ctx_t* pctx);

BOOL config_load(const char* path_to_config, config_get_value_ft callback);

BOOL config_get_string(const config_ctx_t* pctx, char* buff, const size_t sz);
BOOL config_get_int(const config_ctx_t* pctx, int* val);
BOOL config_get_bool(const config_ctx_t* pctx, BOOL* val);

BOOL config_get_string_array(
    const config_ctx_t* pctx, char** buff, size_t* sz, const size_t item_sz, const size_t max_sz);
BOOL config_get_int_array(const config_ctx_t* pctx, int** val, size_t* sz, const size_t max_sz);
BOOL config_get_bool_array(const config_ctx_t* pctx, int** val, size_t* sz, const size_t max_sz);

// read config string as local time (format - for ex. '%d.%m.%Y %H:%M:%S')
BOOL config_get_local_time(const config_ctx_t* pctx, const char* format, time_t* val);
// read config string as UTC time
BOOL config_get_utc_time(const config_ctx_t* pctx, const char* format, time_t* val);

#ifdef __cplusplus
}
#endif
