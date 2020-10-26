#pragma once

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif
// it is interruption resistant usleep
void wpause(int milliseconds);
#ifdef __cplusplus
}
#endif
