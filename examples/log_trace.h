#pragma once

#include <server_clib/macro.h>

#include <string.h>
#include <unistd.h>

#define LOG(...)                  \
    fprintf(stdout, __VA_ARGS__); \
    fprintf(stdout, "%s", "\n");  \
    fflush(stdout)

static void log_s_(const char* text, int out_fd)
{
    char buff[1024] = { 0 };
    size_t sz = sizeof(buff);
    memset(buff, 0, sz);
    int ln = SRV_C_MIN(sz - 1, strnlen(text, sz));
    strncpy(buff, text, ln);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
    write(out_fd, buff, strnlen(buff, sz)); //signal safe
#pragma GCC diagnostic pop
}

#define LOG_THREAD(mtx, ...)  \
    pthread_mutex_lock(&mtx); \
                              \
    LOG(__VA_ARGS__);         \
    pthread_mutex_unlock(&mtx)

#define LOG_SIG_(T) \
    log_s_(T, 2)

#define LOG_SIG(T) \
    LOG_SIG_(T);   \
    LOG_SIG_("\n")

#define LOG_SIG_NUMBER(signo)                        \
    int32_t s__                                      \
        = 0x3030 | (signo / 10) | (signo % 10) << 8; \
    LOG_SIG((char*)&s__)
