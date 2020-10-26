#pragma once

#include <stdlib.h>

enum exit_code
{
    exit_code_ok = EXIT_SUCCESS,
    exit_code_error = EXIT_FAILURE,
    exit_code_bad_argument,
    exit_code_bad_resource,
    exit_code_bad_privileges,
    exit_code_never,
};
