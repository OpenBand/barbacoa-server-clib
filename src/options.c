#include <server_clib/options.h>
#include <server_clib/macro.h>
#include <server_clib/rubber.h>

#include <getopt.h>

#include <unistd.h>
#include <libgen.h>
#include <stdbool.h>

static const char* get_long_name_copy_short(const char short_opt)
{
    static char buff[MAX_INPUT];
    int pos;
    pos = (int)short_opt;
    pos -= (int)'a';
    pos *= 2;
    buff[pos] = short_opt;
    buff[pos + 1] = 0;
    return &buff[pos];
}

static const char* get_program_name()
{
    static char buffer[PATH_MAX];
    bzero(buffer, sizeof(buffer));
    SRV_C_CALL(readlink("/proc/self/exe", buffer, sizeof(buffer)) > 0);
    return basename(buffer);
}

static const char* get_argument_info(const options_ctx_t* pctx, struct option* pos_long_option)
{
    if (!pctx || !pos_long_option)
        return "";

    if (pos_long_option->has_arg == required_argument)
        return (pctx->required_argument_description[0]) ? pctx->required_argument_description
                                                        : "(argument is required)";
    else if (pos_long_option->has_arg == optional_argument)
        return (pctx->optional_argument_description[0]) ? pctx->optional_argument_description
                                                        : "(argument is optional)";

    return "";
}

static BOOL get_info(const options_ctx_t* pctx, rubber_ctx_t* prbuff, struct option* plong_options)
{
    if (!pctx)
        return false;

    if (!prbuff || !prbuff->sz)
        return false;

    const char* DELIM = "~";

    int wtn = 0;
    wtn = snprintf(rubber_pos(prbuff, &wtn), rubber_rest(prbuff, &wtn),
                   "[%s] Version %d.%d.%d."
#if !defined(SRV_C_OPT_MUTE_DATE)
                   " Build %s."
#endif
                   "\n",
                   get_program_name(), (int)pctx->version_major, (int)pctx->version_minor, (int)pctx->version_patch
#if !defined(SRV_C_OPT_MUTE_DATE)
                   ,
                   __DATE__
#endif
    );
    if (wtn < 1)
        return false;

    wtn = snprintf(rubber_pos(prbuff, &wtn), rubber_rest(prbuff, &wtn), "%s\n", DELIM);
    if (wtn < 1)
        return false;

    if (pctx->description[0])
    {
        wtn = snprintf(rubber_pos(prbuff, &wtn), rubber_rest(prbuff, &wtn), "%s\n", pctx->description);
        if (wtn < 1)
            return false;

        wtn = snprintf(rubber_pos(prbuff, &wtn), rubber_rest(prbuff, &wtn), "%s\n", DELIM);
        if (wtn < 1)
            return false;
    }
    struct option* pos_long_options = plong_options;
    while (pos_long_options->name)
    {
        char short_opt[1];
        *short_opt = (char)pos_long_options->val;

        const char* pdesc = NULL;
        if (pctx->get_option_description)
        {
            pdesc = pctx->get_option_description((char)pos_long_options->val);
        }
        switch (short_opt[0])
        {
        case REQUIRED_ARGUMENT_SYMBOL:
        case OPTIONAL_ARGUMENT_SYMBOL:
            wtn = snprintf(rubber_pos(prbuff, &wtn), rubber_rest(prbuff, &wtn),
                           "%s"
                           "\t[%s]\n",
                           get_program_name(), (pdesc) ? pdesc : get_argument_info(pctx, pos_long_options));
            break;
        default:
            if (strlen(pos_long_options->name) == 1 && !strncmp(short_opt, pos_long_options->name, sizeof(short_opt)))
            {
                wtn = snprintf(rubber_pos(prbuff, &wtn), rubber_rest(prbuff, &wtn),
                               "-%s"
                               "\t%s\n",
                               short_opt, (pdesc) ? pdesc : get_argument_info(pctx, pos_long_options));
            }
            else
            {
                wtn = snprintf(rubber_pos(prbuff, &wtn), rubber_rest(prbuff, &wtn),
                               "-%s, --%s"
                               "\t%s\n",
                               short_opt, pos_long_options->name,
                               (pdesc) ? pdesc : get_argument_info(pctx, pos_long_options));
            }
        }
        if (wtn < 1)
            return false;

        pos_long_options++;
    }
    return true;
}

void options_init(options_ctx_t* pctx, const char* description, const char* short_options_names_only)
{
    bzero(pctx, sizeof(options_ctx_t));

    if (description)
        strncpy(pctx->description, description, sizeof(pctx->description));
    if (short_options_names_only)
        strncpy(pctx->short_options_names_only, short_options_names_only, sizeof(pctx->short_options_names_only));

    pctx->show_info = true;
}

void options_init_stealth(options_ctx_t* pctx, const char* short_options_names_only)
{
    bzero(pctx, sizeof(options_ctx_t));

    if (short_options_names_only)
        strncpy(pctx->short_options_names_only, short_options_names_only, sizeof(pctx->short_options_names_only));

    pctx->show_info = false;
}

int options_parse(const options_ctx_t* pctx,
                  int argc,
                  char* argv[],
                  BOOL (*pset_option)(const char short_opt, const char* val))
{
    if (!pctx)
        return -1;

    options_get_option_details_ft pget_option_long_name = pctx->get_option_long_name;
    if (!pget_option_long_name)
        pget_option_long_name = get_long_name_copy_short;

    size_t sz = (pctx->short_options_names_only[0]) ? (strlen(pctx->short_options_names_only)) : 0;
    char short_options[sz * 2 + 2];
    bzero(short_options, sizeof(short_options));
    struct option long_options[sz + 2];
    bzero(long_options, sizeof(long_options));

    // prepare input for getopt
    {
        const char* pos_short_options_names_only
            = (pctx->short_options_names_only[0]) ? pctx->short_options_names_only : NULL;
        char* pos_short_options = short_options;
        struct option* pos_long_options = long_options;
        while (pos_short_options_names_only && *pos_short_options_names_only)
        {
            (*pos_short_options) = (*pos_short_options_names_only);
            (*pos_long_options).name = pget_option_long_name(*pos_short_options_names_only);
            (*pos_long_options).val = (*pos_short_options_names_only);
            if (pos_short_options_names_only[1] == REQUIRED_ARGUMENT_SYMBOL)
            {
                pos_short_options++;
                (*pos_short_options) = ':';
                (*pos_long_options).has_arg = required_argument;
            }
            else if (pos_short_options_names_only[1] == OPTIONAL_ARGUMENT_SYMBOL)
            {
                pos_short_options++;
                (*pos_short_options) = ':';
                (*pos_long_options).has_arg = optional_argument;
            }
            else
            {
                switch (pos_short_options_names_only[0])
                {
                case REQUIRED_ARGUMENT_SYMBOL:
                    (*pos_long_options).has_arg = required_argument;
                    break;
                case OPTIONAL_ARGUMENT_SYMBOL:
                    (*pos_long_options).has_arg = optional_argument;
                    break;
                default:
                    (*pos_long_options).has_arg = no_argument;
                }
            }
            pos_short_options++;
            pos_short_options_names_only++;
            if (*pos_short_options_names_only == REQUIRED_ARGUMENT_SYMBOL
                || *pos_short_options_names_only == OPTIONAL_ARGUMENT_SYMBOL)
                pos_short_options_names_only++;
            pos_long_options++;
        }

        (*pos_short_options) = 'h';
        (*pos_long_options).name = "help";
        (*pos_long_options).val = (*pos_short_options);
        (*pos_long_options).has_arg = no_argument;
    }

    optind = 1;

    // process input
    int next_option = 0;
    do
    {
        bool flg_usage = false;
        bool flg_error = false;
        next_option = getopt_long(argc, argv, short_options, long_options, NULL);
        switch (next_option)
        {
        case -1:
            break;
        case 'h':
            flg_usage = true;
            break;
        default:
        {
            int pos_long_options = 0;
            char* pos_short_options = short_options;
            while (*pos_short_options)
            {
                if (*pos_short_options == next_option)
                {
                    if (pos_short_options[1] == ':')
                    {
                        pos_short_options++;
                        if (!optarg && long_options[pos_long_options].has_arg == required_argument)
                        {
                            flg_error = true;
                            break;
                        }
                    }
                    if (pset_option && !pset_option((char)next_option, optarg))
                    {
                        flg_error = true;
                    }
                    break;
                }
                pos_short_options++;
                pos_long_options++;
            }

            flg_error = flg_error || (*pos_short_options) == 0;
            if (pctx->show_info)
                flg_usage = flg_error;
        }
        }
        if (flg_usage)
        {
            rubber_ctx_t ctx;
            char info[PIPE_BUF];

            if (!rubber_init_from_buff(&ctx, info, sizeof info, 0, true))
                return -1;

            SRV_C_CALL(get_info(pctx, &ctx, long_options));
            if (flg_error)
            {
                fprintf(stderr, "%s\n", rubber_get(&ctx));
            }
            else
            {
                printf("%s\n", rubber_get(&ctx));
            }

            rubber_destroy(&ctx);
        }

        if (flg_error || flg_usage)
            return (flg_error) ? -1 : 0;
    } while (next_option != -1);

    return optind;
}

const char* options_parse_stealth_single_option(int argc, char* argv[], BOOL required)
{
    return options_parse_single_option(argc, argv, required, NULL);
}

const char* options_parse_single_option(int argc, char* argv[], BOOL required, const options_ctx_t* pctx)
{
    if (argc < 2)
        return NULL;

    options_ctx_t ctx;

    char options[2] = { 0 };
    options[0] = (required) ? REQUIRED_ARGUMENT_SYMBOL : OPTIONAL_ARGUMENT_SYMBOL;

    options_init_stealth(&ctx, options);
    if (pctx)
    {
        ctx.show_info = pctx->show_info;
        ctx.version_major = pctx->version_major;
        ctx.version_minor = pctx->version_minor;
        ctx.version_patch = pctx->version_patch;
        strncpy(ctx.description, pctx->description, MAX_INPUT);
        strncpy(ctx.required_argument_description, pctx->required_argument_description, MAX_INPUT);
        strncpy(ctx.optional_argument_description, pctx->optional_argument_description, MAX_INPUT);
    }

    int r = options_parse(&ctx, argc, argv, NULL);
    if (r > 0)
        return argv[1];
    return NULL;
}
