#pragma once

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

// return processed input bytes or -1
long hex_stream_to_hex(const unsigned char* p_input, const size_t input_sz, char* buff, const size_t buff_sz);
long hex_stream_from_hex(const char* buff, const size_t buff_sz, unsigned char* p_output, const size_t output_sz);

#ifdef __cplusplus
}
#endif
