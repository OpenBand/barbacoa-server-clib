#pragma once

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    size_t chunk_sz; // factor that determine buffer enlarge
    BOOL string_mode; // to concatenate big strings or raw data
    const char* pbuff;
    const char* pextra_buff;
    size_t sz;
    size_t written;
    size_t rest;
    char* pos;
} srv_c_rubber_ctx_t;

size_t srv_c_rubber_init_from_buff(
    srv_c_rubber_ctx_t* pctx, char* pbuff, const size_t buff_sz, const size_t chunk_sz, const BOOL string_mode);
size_t srv_c_rubber_init(srv_c_rubber_ctx_t* pctx, const size_t chunk_sz, const BOOL string_mode);
size_t srv_c_rubber_enlarge(srv_c_rubber_ctx_t* pctx, const size_t additional_space);
size_t srv_c_rubber_destroy(srv_c_rubber_ctx_t* pctx);

// get cursor data from context and move to next pos by 'written' value if it has been set
char* srv_c_rubber_pos(srv_c_rubber_ctx_t* pctx, int* pwritten);
size_t srv_c_rubber_rest(srv_c_rubber_ctx_t* pctx, int* pwritten);

// get cursor data from context
char* srv_c_rubber_get(const srv_c_rubber_ctx_t* pctx);
char* srv_c_rubber_get_pos(const srv_c_rubber_ctx_t* pctx);
size_t srv_c_rubber_get_rest(const srv_c_rubber_ctx_t* pctx);

#ifdef __cplusplus
}
#endif
