#include <server_clib/rubber.h>

size_t srv_c_rubber_init_from_buff(
    srv_c_rubber_ctx_t* pctx, char* pbuff, const size_t buff_sz, const size_t chunk_sz, const BOOL string_mode)
{
    if (!pctx)
        return 0;

    if (buff_sz && chunk_sz > buff_sz)
        return 0;

    bzero(pctx, sizeof(srv_c_rubber_ctx_t));

    if (chunk_sz)
        pctx->chunk_sz = chunk_sz;
    else
        pctx->chunk_sz = MAX_INPUT;
    pctx->string_mode = string_mode;

    if (buff_sz)
        pctx->sz = buff_sz;
    else
        pctx->sz = PIPE_BUF;

    if (pctx->chunk_sz > pctx->sz)
        pctx->chunk_sz = pctx->sz;

    if (pbuff)
    {
        pctx->pbuff = pbuff;
        pctx->pextra_buff = NULL;
    }
    else
    {
        pctx->pbuff = malloc(pctx->sz);
        if (!pctx->pbuff)
            return 0;
        pctx->pextra_buff = pctx->pbuff;
    }

    pctx->written = 0;
    pctx->rest = pctx->sz;
    pctx->pos = (char*)(pctx->pbuff);

    return pctx->sz;
}

size_t srv_c_rubber_init(srv_c_rubber_ctx_t* pctx, const size_t chunk_sz, const BOOL string_mode)
{
    return srv_c_rubber_init_from_buff(pctx, NULL, 0, chunk_sz, string_mode);
}

size_t srv_c_rubber_enlarge(srv_c_rubber_ctx_t* pctx, const size_t additional_space)
{
    if (!additional_space)
        return pctx->sz;

    size_t old_sz = pctx->sz;
    size_t new_sz = old_sz + additional_space;
    if (!pctx->pextra_buff)
    {
        pctx->pextra_buff = malloc(new_sz);
        if (!pctx->pextra_buff)
            return 0;
        memcpy((void*)(pctx->pextra_buff), pctx->pbuff, old_sz);
    }
    else
    {
        const char* pbefore = pctx->pextra_buff;
        pctx->pextra_buff = realloc((void*)pbefore, new_sz);
        if (!pctx->pextra_buff)
        {
            pctx->pextra_buff = pbefore;
            return 0; // context should be destroyed!
        }
    }

    pctx->sz = new_sz;
    pctx->pos = (char*)(pctx->pextra_buff) + pctx->written;
    pctx->rest = additional_space;

    return pctx->sz;
}

size_t srv_c_rubber_destroy(srv_c_rubber_ctx_t* pctx)
{
    if (pctx->pextra_buff)
    {
        free((void*)(pctx->pextra_buff));
    }

    size_t r = pctx->sz;

    bzero(pctx, sizeof(srv_c_rubber_ctx_t));

    return r;
}

static BOOL rubber_next_(srv_c_rubber_ctx_t* pctx, const int written)
{
    if (!pctx || written <= 0)
        return false;

    size_t written_ = (size_t)written;

    if (pctx->rest <= written_)
        written_ = (pctx->string_mode) ? pctx->rest - 1 : pctx->rest; // remove previous '\0' char

    pctx->pos += written_;
    pctx->rest -= written_;
    pctx->written += written_;

    if (pctx->rest < pctx->chunk_sz / 5)
    {
        srv_c_rubber_enlarge(pctx, (!pctx->pextra_buff) ? (pctx->sz) : (pctx->chunk_sz));

        char* delim_pos = pctx->pos;
        delim_pos--;

        const char DELIM = '\n';

        if (pctx->string_mode && *delim_pos != DELIM)
        {
            memcpy(pctx->pos, &DELIM, 1);
            rubber_next_(pctx, 1);
        }
    }

    return true;
}

char* srv_c_rubber_pos(srv_c_rubber_ctx_t* pctx, int* pwritten)
{
    if (!pctx)
        return NULL;

    if (!pwritten)
        return pctx->pos;

    if (*pwritten > 0)
    {
        if (!rubber_next_(pctx, *pwritten))
            return 0;

        *pwritten = 0;
    }

    return pctx->pos;
}

size_t srv_c_rubber_rest(srv_c_rubber_ctx_t* pctx, int* pwritten)
{
    if (!pctx)
        return 0;

    if (!pwritten)
        return pctx->rest;

    if (*pwritten > 0)
    {
        if (!rubber_next_(pctx, *pwritten))
            return 0;

        *pwritten = 0;
    }

    return pctx->rest;
}

char* srv_c_rubber_get(const srv_c_rubber_ctx_t* pctx)
{
    if (!pctx)
        return NULL;

    return (pctx->pextra_buff) ? (char*)(pctx->pextra_buff) : (char*)(pctx->pbuff);
}

char* srv_c_rubber_get_pos(const srv_c_rubber_ctx_t* pctx)
{
    if (!pctx)
        return NULL;

    return pctx->pos;
}

size_t srv_c_rubber_get_rest(const srv_c_rubber_ctx_t* pctx)
{
    if (!pctx)
        return 0;

    return pctx->rest;
}
