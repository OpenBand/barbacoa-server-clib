#include <server_clib/zip_stream.h>
#include <server_clib/macro.h>

#include <zlib.h>

#define windowBits 15
#define GZIP_ENCODING 16

static BOOL deflate_init(z_stream* strm)
{
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
    strm->opaque = Z_NULL;
    return Z_OK
        == deflateInit2(strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, windowBits | GZIP_ENCODING, 8, Z_DEFAULT_STRATEGY);
}

static BOOL inflate_init(z_stream* strm)
{
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
    strm->opaque = Z_NULL;
    return Z_OK == inflateInit2(strm, windowBits | GZIP_ENCODING);
}

static BOOL init_context(zip_stream_ctx_t* pctx, BOOL (*zip_init_f)(z_stream*))
{
    if (!pctx || !zip_init_f)
        return false;

    bzero(pctx->z_stream, sizeof(pctx->z_stream));

    z_stream* strm = NULL;
    if (sizeof(pctx->z_stream) >= sizeof(z_stream))
    {
        strm = (z_stream*)pctx->z_stream;
        pctx->pz_stream = NULL;
    }
    else
    {
        strm = (z_stream*)malloc(sizeof(z_stream));
        if (!strm)
            return false;
        pctx->pz_stream = strm;
    }

    if (!zip_init_f(strm))
    {
        if (pctx->pz_stream)
        {
            free(pctx->pz_stream);
            pctx->pz_stream = NULL;
        }
        return false;
    }

    return true;
}

BOOL zip_stream_pack_init(zip_stream_ctx_t* pctx)
{
    return init_context(pctx, deflate_init);
}
BOOL zip_stream_unpack_init(zip_stream_ctx_t* pctx)
{
    return init_context(pctx, inflate_init);
}

static z_stream* get_z_stream(zip_stream_ctx_t* pctx)
{
    if (!pctx)
        return NULL;

    if (pctx->pz_stream)
        return (z_stream*)pctx->pz_stream;

    return (z_stream*)pctx->z_stream;
}

static BOOL destroy_context(zip_stream_ctx_t* pctx, int (*zip_end_f)(z_stream*))
{
    if (!pctx || !zip_end_f)
        return false;

    z_stream* strm = get_z_stream(pctx);
    if (!strm)
        return false;

    int result_ = zip_end_f(strm);
    BOOL result = Z_OK == result_;

    if (pctx->pz_stream)
    {
        free(pctx->pz_stream);
        pctx->pz_stream = NULL;
    }
    else
    {
        bzero(pctx->z_stream, sizeof(pctx->z_stream));
    }

    return result;
}

BOOL zip_stream_pack_destroy(zip_stream_ctx_t* pctx)
{
    return destroy_context(pctx, deflateEnd);
}
BOOL zip_stream_unpack_destroy(zip_stream_ctx_t* pctx)
{
    return destroy_context(pctx, inflateEnd);
}

static long process_context(zip_stream_ctx_t* pctx,
                            const unsigned char* p_input,
                            const size_t input_sz,
                            unsigned char* p_output,
                            const size_t output_sz,
                            int (*zip_process_f)(z_stream*, int),
                            int (*zip_reset_f)(z_stream*),
                            const BOOL flash,
                            const BOOL finish)
{
    if (!pctx || !p_output || !output_sz || !output_sz || !zip_process_f || !zip_reset_f)
        return -1;

    z_stream* strm = get_z_stream(pctx);
    if (!strm)
        return -1;

    int mode = (flash) ? Z_FULL_FLUSH : Z_PARTIAL_FLUSH;
    if (finish)
        mode = Z_FINISH;

    if (p_input && input_sz)
    {
        strm->avail_in = (uInt)input_sz;
        strm->next_in = (z_const Bytef *)p_input;
    }
    strm->avail_out = (uInt)output_sz;
    strm->next_out = p_output;

    int result = zip_process_f(strm, mode);
    if (result < 0)
    {
        if (Z_BUF_ERROR == result)
            return 0;
        else
        {
            zip_reset_f(strm);
            return -1;
        }
    }

    return output_sz - strm->avail_out;
}

long zip_stream_start_pack_chunk(zip_stream_ctx_t* pctx,
                                 const unsigned char* p_input,
                                 const size_t input_sz,
                                 unsigned char* p_output,
                                 const size_t output_sz)
{
    return process_context(pctx, p_input, input_sz, p_output, output_sz, deflate, deflateReset, false, false);
}
long zip_stream_pack_chunk(zip_stream_ctx_t* pctx, unsigned char* p_output, const size_t output_sz)
{
    return process_context(pctx, NULL, 0, p_output, output_sz, deflate, deflateReset, false, false);
}
long zip_stream_finish_pack_chunk(zip_stream_ctx_t* pctx, unsigned char* p_output, const size_t output_sz)
{
    return process_context(pctx, NULL, 0, p_output, output_sz, deflate, deflateReset, true, false);
}
long zip_stream_finish_pack(zip_stream_ctx_t* pctx, unsigned char* p_output, const size_t output_sz)
{
    return process_context(pctx, NULL, 0, p_output, output_sz, deflate, deflateReset, false, true);
}

long zip_stream_start_unpack_chuck(zip_stream_ctx_t* pctx,
                                   const unsigned char* p_input,
                                   const size_t input_sz,
                                   unsigned char* p_output,
                                   const size_t output_sz)
{
    return process_context(pctx, p_input, input_sz, p_output, output_sz, inflate, inflateReset, false, false);
}
long zip_stream_unpack_chuck(zip_stream_ctx_t* pctx, unsigned char* p_output, const size_t output_sz)
{
    return process_context(pctx, NULL, 0, p_output, output_sz, inflate, inflateReset, false, false);
}
