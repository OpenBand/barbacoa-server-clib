#include <server_clib/config.h>

#ifndef __USE_XOPEN // for strptime
#define __USE_XOPEN
#endif
#include <time.h>

extern char* strptime(const char* s, const char* format, struct tm* tm);

#include <stdlib.h>

static BOOL is_valid_context(const config_ctx_t* pctx)
{
    return pctx && pctx->pline && pctx->ptok && pctx->ptok->end > pctx->ptok->start;
}

static BOOL get_to(const config_ctx_t* pctx, char* buff, const size_t sz)
{
    if (!is_valid_context(pctx) || !buff || !sz)
        return false;

    size_t act_sz = (size_t)(pctx->ptok->end - pctx->ptok->start);
    if (act_sz >= sz)
        return false;

    const char* pval = pctx->pline + pctx->ptok->start;
    strncpy(buff, pval, act_sz);
    buff[act_sz] = 0;
    return true;
}

static BOOL get_key(const config_ctx_t* pctx, char* buff, const size_t sz)
{
    if (!is_valid_context(pctx))
        return false;

    if (pctx->ptok->type != JSMN_STRING && pctx->ptok->type != JSMN_PRIMITIVE)
        return false;

    return get_to(pctx, buff, sz);
}

static BOOL check_for_comment(const config_ctx_t* pctx)
{
    if (!is_valid_context(pctx))
        return false;

    if (pctx->ptok->type != JSMN_PRIMITIVE)
        return false;

    const char* pval = pctx->pline + pctx->ptok->start;
    return *pval == '#';
}

static BOOL parse_config_line(const char* pline, const size_t sz, config_get_value_ft callback)
{
    jsmn_parser parser;
    jsmn_init(&parser);

    jsmntok_t* pptok = NULL;
    int r = jsmn_parse(&parser, pline, sz, pptok, 0);
    if (r < 1)
        return true; // spaces only

    jsmn_init(&parser);
    pptok = alloca((size_t)r * sizeof(jsmntok_t));
    bzero(pptok, (size_t)r * sizeof(jsmntok_t));
    r = jsmn_parse(&parser, pline, sz, pptok, (unsigned int)r);
    if (r < 1)
        return false;

    jsmntok_t* ptok = pptok;

    config_ctx_t ctx;
    bzero(&ctx, sizeof(ctx));
    ctx.pline = pline;
    ctx.ptok = ptok;
    ctx.pptok_array = NULL;

    if (check_for_comment(&ctx))
        return true;

    size_t ci = 0;
    BOOL read_key = true;
    char key_buff[MAX_INPUT];

    for (; ci < (size_t)r; ++ci)
    {
        if ((read_key) && (ptok->type == JSMN_STRING || ptok->type == JSMN_PRIMITIVE))
        {
            if (!get_key(&ctx, key_buff, MAX_INPUT))
                return false;
            read_key = false;
        }
        else if (!read_key)
        {
            jsmntok_t* ptok_array_info = ptok;
            jsmntok_t* ptok_array = ptok_array_info;
            if (ptok->type == JSMN_ARRAY && ptok->size > 0)
            {
                ptok_array += 1;
                ctx.ptok = ptok_array_info;
                ctx.pptok_array = &ptok_array;
                ptok += ptok->size;
            }
            if (!callback(key_buff, &ctx))
                return false;
            read_key = true;
        }

        ptok += 1;
        ctx.ptok = ptok;
    }

    return true;
}

BOOL config_load(const char* path_to_config, config_get_value_ft callback)
{
    FILE* fp = fopen(path_to_config, "r");
    if (!fp)
    {
        return false;
    }

    BOOL result = true;
    char* pline = NULL;
    size_t sz = 0;
    ssize_t read = 0;
    while ((read = getline(&pline, &sz, fp)) > 0)
    {
        result = parse_config_line(pline, sz, callback);
        if (!result)
            break;
    }
    free(pline);
    fclose(fp);
    return result;
}

BOOL config_get_string(const config_ctx_t* pctx, char* buff, const size_t sz)
{
    if (!is_valid_context(pctx))
        return false;

    if (pctx->ptok->type != JSMN_STRING)
        return false;

    return get_to(pctx, buff, sz);
}
BOOL config_get_int(const config_ctx_t* pctx, int* val)
{
    if (!is_valid_context(pctx) || !val)
        return false;

    if (pctx->ptok->type != JSMN_PRIMITIVE)
        return false;

    const char* pval = pctx->pline + pctx->ptok->start;
    char s_char = (*pval);
    if ((s_char >= '0' && s_char <= '9') || s_char == '-')
    {
        char buff[MAX_INPUT];
        if (!get_to(pctx, buff, MAX_INPUT))
            return false;

        *val = atoi(buff);

        return true;
    }

    return false;
}
BOOL config_get_bool(const config_ctx_t* pctx, BOOL* val)
{
    if (!is_valid_context(pctx) || !val)
        return false;

    if (pctx->ptok->type != JSMN_PRIMITIVE)
        return false;

    const char* pval = pctx->pline + pctx->ptok->start;
    char s_char = (*pval);
    if (s_char == '0')
    {
        *val = false;
    }
    else if (s_char == '1')
    {
        *val = true;
    }
    else
    {
        static const char tree_tok[] = "true";
        static const char false_tok[] = "false";

        size_t sz = (size_t)(pctx->ptok->end - pctx->ptok->start);
        if (!strncmp(pval, tree_tok, sz))
        {
            *val = true;
        }
        else if (!strncmp(pval, false_tok, sz))
        {
            *val = false;
        }
        else
        {
            return false;
        }
    }

    return true;
}

BOOL config_get_string_array(
    const config_ctx_t* pctx, char** pbuff, size_t* psz, const size_t item_sz, const size_t max_sz)
{
    if (!is_valid_context(pctx) || !pbuff || !psz || !item_sz || !max_sz)
        return false;

    if (pctx->ptok->type != JSMN_ARRAY || pctx->ptok->size < 1 || !pctx->pptok_array)
        return false;

    size_t sz = (size_t)pctx->ptok->size;
    if (sz > max_sz)
        sz = max_sz;
    char* pval = (char*)malloc(sz * item_sz * sizeof(char));
    if (!pval)
        return false;
    bzero(pval, sz * item_sz * sizeof(char));

    char* pval_item = pval;
    jsmntok_t* ptok_array = (*pctx->pptok_array);
    for (size_t ci = 0; ci < sz; ++ci)
    {
        config_ctx_t item_context;
        item_context.pline = pctx->pline;
        item_context.ptok = ptok_array;
        item_context.pptok_array = NULL;
        if (!config_get_string(&item_context, pval_item, item_sz))
        {
            free(pval);
            return false;
        }
        pval_item += item_sz;
        ptok_array += 1;
    }

    (*psz) = sz;
    (*pbuff) = pval;

    return true;
}

BOOL config_get_int_array(const config_ctx_t* pctx, int** ppval, size_t* psz, const size_t max_sz)
{
    if (!is_valid_context(pctx) || !ppval || !psz || !max_sz)
        return false;

    if (pctx->ptok->type != JSMN_ARRAY || pctx->ptok->size < 1 || !pctx->pptok_array)
        return false;

    size_t sz = (size_t)pctx->ptok->size;
    if (sz > max_sz)
        sz = max_sz;
    int* pval = (int*)calloc(sz, sizeof(int));

    jsmntok_t* ptok_array = (*pctx->pptok_array);
    for (size_t ci = 0; ci < sz; ++ci)
    {
        config_ctx_t item_context;
        item_context.pline = pctx->pline;
        item_context.ptok = ptok_array;
        item_context.pptok_array = NULL;
        int val = 0;
        if (!config_get_int(&item_context, &val))
        {
            free(pval);
            return false;
        }
        pval[ci] = val;
        ptok_array += 1;
    }

    (*psz) = sz;
    (*ppval) = pval;

    return true;
}

BOOL config_get_bool_array(const config_ctx_t* pctx, int** ppval, size_t* psz, const size_t max_sz)
{
    if (!is_valid_context(pctx) || !ppval || !psz || !max_sz)
        return false;

    if (pctx->ptok->type != JSMN_ARRAY || pctx->ptok->size < 1 || !pctx->pptok_array)
        return false;

    size_t sz = (size_t)pctx->ptok->size;
    if (sz > max_sz)
        sz = max_sz;
    int* pval = (int*)calloc(sz, sizeof(int));

    jsmntok_t* ptok_array = (*pctx->pptok_array);
    for (size_t ci = 0; ci < sz; ++ci)
    {
        config_ctx_t item_context;
        item_context.pline = pctx->pline;
        item_context.ptok = ptok_array;
        item_context.pptok_array = NULL;
        BOOL val = false;
        if (!config_get_bool(&item_context, &val))
        {
            free(pval);
            return false;
        }
        pval[ci] = (int)val;
        ptok_array += 1;
    }

    (*psz) = sz;
    (*ppval) = pval;

    return true;
}

static BOOL config_get_time(const config_ctx_t* pctx, const char* format, const BOOL should_utc, time_t* val)
{
    char buff[MAX_INPUT];
    if (!config_get_string(pctx, buff, MAX_INPUT))
        return false;

    struct tm val_;
    if (!strptime(buff, format, &val_))
        return false;

    *val = mktime(&val_);
    if (should_utc)
        *val += val_.tm_gmtoff;
    return true;
}

BOOL config_get_local_time(const config_ctx_t* pctx, const char* format, time_t* val)
{
    return config_get_time(pctx, format, false, val);
}

BOOL config_get_utc_time(const config_ctx_t* pctx, const char* format, time_t* val)
{
    return config_get_time(pctx, format, true, val);
}
