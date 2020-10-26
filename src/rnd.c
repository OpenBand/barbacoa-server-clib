#include <server_clib/rnd.h>

#include <time.h>

uint64_t create_pseudo_random(const uint64_t seed, const uint64_t offset)
{
    /// High performance random generator
    /// http://xorshift.di.unimi.it/
    uint64_t r = seed + offset * 2685821657736338717ULL;
    r ^= (r >> 12);
    r ^= (r << 25);
    r ^= (r >> 27);
    r *= 2685821657736338717ULL;
    return r;
}

uint32_t create_pseudo_random_from_time(const uint32_t offset)
{
    time_t now = time(NULL);

    return (uint32_t)create_pseudo_random((uint64_t)now, (uint64_t)offset);
}
