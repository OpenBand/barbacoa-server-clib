#pragma once

#include "common.h"

#define SRV_C_REQUIRED_ARGUMENT_SYMBOL '!'
#define SRV_C_OPTIONAL_ARGUMENT_SYMBOL ':'

// short_options_names_only example: dc!p:t
//> -d -c ~/app.conf -p 1000
//> -c ~/app.conf --p
//> --p=2000 -t

#ifdef __cplusplus
extern "C" {
#endif

typedef const char* (*srv_c_options_get_option_details_ft)(const char short_opt);

typedef struct
{
    // Switch mode to show or hide usage info (help) if option is wrong. By default usage mode is ON (show info)
    BOOL show_info;

    size_t version_major;
    size_t version_minor;
    size_t version_patch;
    char description[MAX_INPUT];

    char short_options_names_only[MAX_INPUT];

    char required_argument_description[MAX_INPUT];
    char optional_argument_description[MAX_INPUT];

    srv_c_options_get_option_details_ft get_option_description;
    srv_c_options_get_option_details_ft get_option_long_name;
} srv_c_options_ctx_t;

void srv_c_options_init(srv_c_options_ctx_t* pctx, const char* description, const char* short_options_names_only);
void srv_c_options_init_stealth(srv_c_options_ctx_t* pctx, const char* short_options_names_only);

// it returns index of the next unparsed args or -1 for error or 0 if all args are parsed
int srv_c_options_parse(const srv_c_options_ctx_t* pctx,
                        int argc,
                        char* argv[],
                        BOOL (*pset_option)(const char short_opt, const char* val));

// return first option string or 0
const char* srv_c_options_parse_stealth_single_option(int argc, char* argv[], BOOL required);

const char* srv_c_options_parse_single_option(int argc, char* argv[], BOOL required, const srv_c_options_ctx_t* pctx);

#ifdef __cplusplus
}
#endif
