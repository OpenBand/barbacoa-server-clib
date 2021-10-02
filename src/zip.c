#include <server_clib/zip.h>
#include <server_clib/macro.h>

#include <zlib.h>

static BOOL zip_pack(const unsigned char* p_input,
                     const size_t input_sz,
                     unsigned char** pp_output,
                     size_t* buff_sz,
                     const int level,
                     const BOOL allocate_buffer)
{
    if (!p_input || !input_sz || !pp_output || !buff_sz)
        return false;

    if (!allocate_buffer && !*pp_output)
        return false;

    uLong sz_zip = compressBound(input_sz);
    if (!sz_zip)
        return false;

    unsigned char* p_output = NULL;

    if (allocate_buffer)
    {
        p_output = (unsigned char*)malloc(sz_zip);
        if (!p_output)
            return false;
    }
    else
    {
        if (sz_zip > *buff_sz)
            return false;

        p_output = *pp_output;
    }

    uLong sz_zip_predicted = sz_zip;

    int result = compress2((Bytef*)p_output, &sz_zip, (const Bytef*)p_input, input_sz, level);

    if (Z_OK != result)
    {
        if (allocate_buffer)
            free(p_output);
        return false;
    }

    if (allocate_buffer && sz_zip_predicted != sz_zip)
    {
        unsigned char* pbefore = p_output;
        p_output = (unsigned char*)realloc(p_output, sz_zip);
        if (!p_output)
        {
            free(pbefore);
            return false;
        }
    }

    (*buff_sz) = (size_t)sz_zip;
    if (allocate_buffer)
        (*pp_output) = p_output;
    return p_output != NULL;
}

BOOL srv_c_zip_pack_best_speed(const unsigned char* p_input,
                         const size_t input_sz,
                         unsigned char** p_output,
                         size_t* output_sz,
                         const BOOL allocate_buffer)
{
    return zip_pack(p_input, input_sz, p_output, output_sz, Z_BEST_SPEED, allocate_buffer);
}

BOOL srv_c_zip_pack_best_size(const unsigned char* p_input,
                        const size_t input_sz,
                        unsigned char** p_output,
                        size_t* output_sz,
                        const BOOL allocate_buffer)
{
    return zip_pack(p_input, input_sz, p_output, output_sz, Z_BEST_COMPRESSION, allocate_buffer);
}

BOOL srv_c_zip_unpack(const unsigned char* p_input,
                const size_t input_sz,
                unsigned char** pp_output,
                size_t* buff_sz,
                const BOOL allocate_buffer)
{
    if (!p_input || !input_sz || !pp_output || !buff_sz)
        return false;

    if (!allocate_buffer && !*pp_output)
        return false;

    uLong sz_zip_predicted = (uLong)SRV_C_MAX(input_sz, *buff_sz);

    unsigned char* p_output = NULL;

    if (allocate_buffer)
    {
        p_output = (unsigned char*)malloc(sz_zip_predicted);
        if (!p_output)
            return false;
    }
    else
    {
        p_output = *pp_output;
    }

    uLong sz_zip = sz_zip_predicted;
    int result = uncompress((Bytef*)p_output, &sz_zip, (const Bytef*)p_input, input_sz);

    if (Z_OK != result)
    {
        if (allocate_buffer)
            free(p_output);
        return false;
    }

    if (allocate_buffer && sz_zip_predicted != sz_zip)
    {
        unsigned char* pbefore = p_output;
        p_output = (unsigned char*)realloc(p_output, sz_zip);
        if (!p_output)
        {
            free(pbefore);
            return false;
        }
    }

    (*buff_sz) = (size_t)sz_zip;
    if (allocate_buffer)
        (*pp_output) = p_output;
    return p_output != NULL;
}
