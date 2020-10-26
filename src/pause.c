#include <server_clib/pause.h>

#include <sys/time.h>
#include <unistd.h>

static __suseconds_t get_milliseconds(struct timeval* ptv)
{
    __suseconds_t ret = (__suseconds_t)1000000 * ptv->tv_sec;
    ret += ptv->tv_usec;
    return ret / 1000;
}

void wpause(int milliseconds)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    __suseconds_t start = get_milliseconds(&tv);
    int left = milliseconds;
    while (left > 0)
    {
        usleep(left * 1000);
        gettimeofday(&tv, NULL);
        left -= (int)(get_milliseconds(&tv) - start);
    }
}
