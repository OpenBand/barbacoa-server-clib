#pragma once

#include "common.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t create_pseudo_random(const uint64_t seed, const uint64_t offset);

uint32_t create_pseudo_random_from_time(const uint32_t offset);

#ifdef __cplusplus
}
#endif
