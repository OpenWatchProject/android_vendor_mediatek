#include <unistd.h>
#include <time.h>

long int getMs()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

void sleepMs(int ms)
{
    usleep(ms * 1000);
}

