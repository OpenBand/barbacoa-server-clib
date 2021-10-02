#pragma once

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

// Pack/unpack full buffer to ZIP format

BOOL srv_c_zip_pack_best_speed(const unsigned char* p_input,
                               const size_t input_sz,
                               unsigned char** p_output,
                               size_t* output_sz,
                               const BOOL allocate_buffer);
BOOL srv_c_zip_pack_best_size(const unsigned char* p_input,
                              const size_t input_sz,
                              unsigned char** p_output,
                              size_t* output_sz,
                              const BOOL allocate_buffer);
BOOL srv_c_zip_unpack(const unsigned char* p_input,
                      const size_t input_sz,
                      unsigned char** p_output,
                      size_t* output_sz,
                      const BOOL allocate_buffer);

#ifdef __cplusplus
}
#endif
