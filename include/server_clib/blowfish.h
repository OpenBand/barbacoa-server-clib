#pragma once

#include "common.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint32_t P[16 + 2];
    uint32_t S[4][256];
} srv_c_blowfish_ctx_t;

BOOL srv_c_blowfish_init(srv_c_blowfish_ctx_t* ctx, uint8_t* key, int32_t keyLen);
BOOL srv_c_blowfish_encrypt_chunk(srv_c_blowfish_ctx_t* ctx, uint32_t* xl, uint32_t* xr);
BOOL srv_c_blowfish_decrypt_chunk(srv_c_blowfish_ctx_t* ctx, uint32_t* xl, uint32_t* xr);
BOOL srv_c_blowfish_destroy(srv_c_blowfish_ctx_t* pctx);

size_t srv_c_blowfish_get_min_chunk_length(void);
size_t srv_c_blowfish_get_stream_output_length(size_t input_long);

// return processed input bytes or -1
long srv_c_blowfish_stream_encrypt(srv_c_blowfish_ctx_t* ctx,
                                   const unsigned char* p_input,
                                   const size_t input_sz,
                                   unsigned char* p_output,
                                   const size_t output_sz);
long srv_c_blowfish_stream_decrypt(srv_c_blowfish_ctx_t* ctx,
                                   const unsigned char* p_input,
                                   const size_t input_sz,
                                   unsigned char* p_output,
                                   const size_t output_sz);

#ifdef __cplusplus
}
#endif
