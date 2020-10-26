/*
 * Copyright (c) 2019 Andrew Masilevich (a.masilevich@gmail.com)
 *
 * The MIT License
 *
 */

#pragma once

#include "macro.h"
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
typedef int BOOL;
#ifndef FALSE
#define FALSE 0
#endif //! FALSE
#ifndef TRUE
#define TRUE 1
#endif //! TRUE
#ifndef NULL
#define NULL 0
#endif //! NULL
#else // C++
typedef bool BOOL;
#ifndef FALSE
#define FALSE false
#endif //! FALSE
#ifndef TRUE
#define TRUE true
#endif //! TRUE
#endif // C
