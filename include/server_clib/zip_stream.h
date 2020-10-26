#pragma once

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

// in should be greater or equal sizeof(struct z_stream_s)
// to prevent malloc at pz_stream
#define ZIP_STREAM_PREDICTED_ZLIB_CTX_SIZE 190

typedef struct
{
    unsigned char z_stream[ZIP_STREAM_PREDICTED_ZLIB_CTX_SIZE];
    void* pz_stream;
} zip_stream_ctx_t;

// Pack/unpack stream buffer to GZIP format

BOOL zip_stream_pack_init(zip_stream_ctx_t* pctx);
BOOL zip_stream_unpack_init(zip_stream_ctx_t* pctx);
BOOL zip_stream_pack_destroy(zip_stream_ctx_t* pctx);
BOOL zip_stream_unpack_destroy(zip_stream_ctx_t* pctx);

// return processed input bytes or -1
long zip_stream_start_pack_chunk(zip_stream_ctx_t* pctx,
                                 const unsigned char* p_input,
                                 const size_t input_sz,
                                 unsigned char* p_output,
                                 const size_t output_sz);
long zip_stream_pack_chunk(zip_stream_ctx_t* pctx, unsigned char* p_output, const size_t output_sz);
long zip_stream_finish_pack_chunk(zip_stream_ctx_t* pctx, unsigned char* p_output, const size_t output_sz);
long zip_stream_finish_pack(zip_stream_ctx_t* pctx, unsigned char* p_output, const size_t output_sz);

long zip_stream_start_unpack_chuck(zip_stream_ctx_t* pctx,
                                   const unsigned char* p_input,
                                   const size_t input_sz,
                                   unsigned char* p_output,
                                   const size_t output_sz);
long zip_stream_unpack_chuck(zip_stream_ctx_t* pctx, unsigned char* p_output, const size_t output_sz);

#ifdef __cplusplus
}
#endif
