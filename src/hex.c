#include <server_clib/hex.h>

long srv_c_hex_stream_to_hex(const unsigned char* p_input, const size_t input_sz, char* buff, const size_t buff_sz)
{
    if (!p_input || !input_sz || !buff || !buff_sz)
        return -1;

    const char hex_abc[] = "0123456789abcdef";

    char* out_pos = buff;
    char* out_end = out_pos + buff_sz;

    size_t i = 0;
    for (; i < input_sz && out_pos != out_end && ++out_pos != out_end; ++i, ++out_pos)
    {
        *out_pos-- = hex_abc[(p_input[i] & 0x0f)];
        *out_pos++ = hex_abc[(p_input[i] >> 4)];
    }

    if (out_pos != out_end)
        *out_pos = 0;

    return (long)i;
}

static unsigned char ch_from_hex(const char c)
{
    if (c >= '0' && c <= '9')
        return (unsigned char)c - '0';
    if (c >= 'a' && c <= 'f')
        return (unsigned char)c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return (unsigned char)c - 'A' + 10;

    return 0;
}

long srv_c_hex_stream_from_hex(const char* buff, const size_t buff_sz, unsigned char* p_output, const size_t output_sz)
{
    if (!p_output || !buff_sz || !buff || !output_sz)
        return -1;

    const char* i = buff;
    const char* i_end = i + buff_sz;
    unsigned char* out_pos = p_output;
    unsigned char* out_end = out_pos + output_sz;
    while (i != i_end && out_end != out_pos)
    {
        unsigned char decoded = (unsigned char)(ch_from_hex(*i) << 4);
        ++i;
        if (i != i_end)
        {
            decoded |= ch_from_hex(*i);
            ++i;

            *out_pos = decoded;
            ++out_pos;
        }
        else
        {
            --i;
            break;
        }
    }

    return i - buff;
}
