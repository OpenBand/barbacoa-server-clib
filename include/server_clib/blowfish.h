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
} blowfish_ctx_t;

BOOL blowfish_init(blowfish_ctx_t* ctx, uint8_t* key, int32_t keyLen);
BOOL blowfish_encrypt_chunk(blowfish_ctx_t* ctx, uint32_t* xl, uint32_t* xr);
BOOL blowfish_decrypt_chunk(blowfish_ctx_t* ctx, uint32_t* xl, uint32_t* xr);
BOOL blowfish_destroy(blowfish_ctx_t* pctx);

size_t blowfish_get_min_chunk_length(void);
size_t blowfish_get_stream_output_length(size_t input_long);

// return processed input bytes or -1
long blowfish_stream_encrypt(blowfish_ctx_t* ctx,
                             const unsigned char* p_input,
                             const size_t input_sz,
                             unsigned char* p_output,
                             const size_t output_sz);
long blowfish_stream_decrypt(blowfish_ctx_t* ctx,
                             const unsigned char* p_input,
                             const size_t input_sz,
                             unsigned char* p_output,
                             const size_t output_sz);

#ifdef __cplusplus
}
#endif
