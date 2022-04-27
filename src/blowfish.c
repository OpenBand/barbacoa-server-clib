#include <server_clib/blowfish.h>

#include "blowfish_tables.h"

#include <arpa/inet.h>

#define ROUND_N 16

static uint32_t F(srv_c_blowfish_ctx_t* ctx, uint32_t x)
{
    uint16_t a, b, c, d;
    uint32_t y;

    d = (uint16_t)(x & 0xFF);
    x >>= 8;
    c = (uint16_t)(x & 0xFF);
    x >>= 8;
    b = (uint16_t)(x & 0xFF);
    x >>= 8;
    a = (uint16_t)(x & 0xFF);
    y = ctx->S[0][a] + ctx->S[1][b];
    y = y ^ ctx->S[2][c];
    y = y + ctx->S[3][d];

    return y;
}

BOOL srv_c_blowfish_init(srv_c_blowfish_ctx_t* ctx, uint8_t* key, int32_t keyLen)
{
    if (!ctx || !key || !keyLen)
        return false;

    int32_t i, j, k;
    uint32_t data, datal, datar;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 256; j++)
            ctx->S[i][j] = ORIG_S[i][j];
    }

    j = 0;
    for (i = 0; i < ROUND_N + 2; ++i)
    {
        data = 0x00000000;
        for (k = 0; k < 4; ++k)
        {
            data = (data << 8) | key[j];
            j = j + 1;
            if (j >= keyLen)
                j = 0;
        }
        ctx->P[i] = ORIG_P[i] ^ data;
    }

    datal = 0x00000000;
    datar = 0x00000000;

    for (i = 0; i < ROUND_N + 2; i += 2)
    {
        srv_c_blowfish_encrypt_chunk(ctx, &datal, &datar);
        ctx->P[i] = datal;
        ctx->P[i + 1] = datar;
    }

    for (i = 0; i < 4; ++i)
    {
        for (j = 0; j < 256; j += 2)
        {
            srv_c_blowfish_encrypt_chunk(ctx, &datal, &datar);
            ctx->S[i][j] = datal;
            ctx->S[i][j + 1] = datar;
        }
    }

    return true;
}

BOOL srv_c_blowfish_encrypt_chunk(srv_c_blowfish_ctx_t* ctx, uint32_t* xl, uint32_t* xr)
{
    if (!ctx || !xl || !xr)
        return false;

    uint32_t Xl;
    uint32_t Xr;
    uint32_t temp;
    int16_t i;

    Xl = *xl;
    Xr = *xr;

    for (i = 0; i < ROUND_N; ++i)
    {
        Xl = Xl ^ ctx->P[i];
        Xr = F(ctx, Xl) ^ Xr;

        temp = Xl;
        Xl = Xr;
        Xr = temp;
    }

    temp = Xl;
    Xl = Xr;
    Xr = temp;

    Xr = Xr ^ ctx->P[ROUND_N];
    Xl = Xl ^ ctx->P[ROUND_N + 1];

    *xl = Xl;
    *xr = Xr;

    return true;
}

BOOL srv_c_blowfish_decrypt_chunk(srv_c_blowfish_ctx_t* ctx, uint32_t* xl, uint32_t* xr)
{
    if (!ctx || !xl || !xr)
        return false;

    uint32_t Xl;
    uint32_t Xr;
    uint32_t temp;
    int16_t i;

    Xl = *xl;
    Xr = *xr;

    for (i = ROUND_N + 1; i > 1; --i)
    {
        Xl = Xl ^ ctx->P[i];
        Xr = F(ctx, Xl) ^ Xr;

        /* Exchange Xl and Xr */
        temp = Xl;
        Xl = Xr;
        Xr = temp;
    }

    /* Exchange Xl and Xr */
    temp = Xl;
    Xl = Xr;
    Xr = temp;

    Xr = Xr ^ ctx->P[1];
    Xl = Xl ^ ctx->P[0];

    *xl = Xl;
    *xr = Xr;

    return true;
}

BOOL srv_c_blowfish_destroy(srv_c_blowfish_ctx_t* pctx)
{
    if (!pctx)
        return false;
    bzero(pctx->P, sizeof(pctx->P));
    bzero(pctx->S, sizeof(pctx->S));
    return true;
}

size_t srv_c_blowfish_get_min_chunk_length(void)
{
    return 8;
}

size_t srv_c_blowfish_get_stream_output_length(size_t input_long)
{
    long input_long_ = (long)input_long;
    long tmp = input_long_ % 8;
    if (tmp != 0)
    {
        tmp = input_long_ + 8 - tmp;
        return (size_t)tmp;
    }
    else
        return (size_t)input_long_;
}

static long blowfish_stream_process(srv_c_blowfish_ctx_t* ctx,
                                    const unsigned char* p_input,
                                    const size_t input_sz,
                                    unsigned char* p_output,
                                    const size_t output_sz,
                                    BOOL (*blowfish_process_f)(srv_c_blowfish_ctx_t*, uint32_t*, uint32_t*),
                                    BOOL ecrypt)
{
    if (!ctx || !p_input || !input_sz || !p_output || !output_sz)
        return -1;

    const unsigned char* p_in_pos = p_input;
    size_t in_rest = input_sz;
    size_t out_rest = output_sz;
    size_t in_rest_fin = 0;

    const unsigned char* p_in_end = p_input + input_sz;
    unsigned char* p_out_pos = p_output;
    unsigned char* p_out_end = p_output + output_sz;

    unsigned char pint_rest[8];

    while (p_in_pos != p_in_end && p_out_pos != p_out_end)
    {
        if (!in_rest || out_rest < 8)
            break;

        if (in_rest < 8)
        {
            bzero(pint_rest, sizeof(pint_rest));
            memcpy(pint_rest, p_in_pos, in_rest);
            p_in_pos = pint_rest;
            in_rest_fin = 8 - in_rest;
            in_rest = 8;
        }

        uint32_t xl = 0;
        uint32_t xr = 0;
        memcpy(&xl, (uint32_t*)p_in_pos, sizeof(xl));
        p_in_pos += 4;
        memcpy(&xr, (uint32_t*)p_in_pos, sizeof(xr));
        p_in_pos += 4;

        in_rest -= 8;

        if (!ecrypt)
        {
            xl = ntohl(xl);
            xr = ntohl(xr);
        }

        // TODO: check for different site of blowfish_process_f calling on BE-LE / LE-BE systems
        // echo -n I | od -to2 | head -n1 | cut -f2 -d" " | cut -c6 -> 1 for Little Endian, -> 0 for Big Endian
        // lscpu | grep Endian -> *
        // Now blowfish_process_f is called on host's Byte Order
        SRV_C_ASSERT(blowfish_process_f(ctx, &xl, &xr));

        if (ecrypt)
        {
            xl = htonl(xl);
            xr = htonl(xr);
        }

        memcpy(p_out_pos, &xl, sizeof(xl));
        p_out_pos += 4;
        memcpy(p_out_pos, &xr, sizeof(xr));
        p_out_pos += 4;

        out_rest -= 8;
    }

    return input_sz - in_rest + in_rest_fin;
}

long srv_c_blowfish_stream_encrypt(srv_c_blowfish_ctx_t* ctx,
                             const unsigned char* p_input,
                             const size_t input_sz,
                             unsigned char* p_output,
                             const size_t output_sz)
{
    return blowfish_stream_process(ctx, p_input, input_sz, p_output, output_sz, srv_c_blowfish_encrypt_chunk, true);
}

long srv_c_blowfish_stream_decrypt(srv_c_blowfish_ctx_t* ctx,
                             const unsigned char* p_input,
                             const size_t input_sz,
                             unsigned char* p_output,
                             const size_t output_sz)
{
    return blowfish_stream_process(ctx, p_input, input_sz, p_output, output_sz, srv_c_blowfish_decrypt_chunk, false);
}
