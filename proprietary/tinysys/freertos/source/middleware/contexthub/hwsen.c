#include "hwsen.h"

struct sensorDriverConvert map[] = {
    { { 1, 1, 1}, {0, 1, 2} },
    { { -1, 1, 1}, {1, 0, 2} },
    { { -1, -1, 1}, {0, 1, 2} },
    { { 1, -1, 1}, {1, 0, 2} },

    { { -1, 1, -1}, {0, 1, 2} },
    { { 1, 1, -1}, {1, 0, 2} },
    { { 1, -1, -1}, {0, 1, 2} },
    { { -1, -1, -1}, {1, 0, 2} },
};
/*----------------------------------------------------------------------------*/
int sensorDriverGetConvert(int32_t direction, struct sensorDriverConvert *cvt)
{
    struct sensorDriverConvert *src;
    if (!cvt)
        return -1;
    else if (direction >= sizeof(map) / sizeof(map[0]))
        return -1;

    //*cvt = map[direction];
    src = &map[direction];
    memcpy(cvt, src, sizeof(struct sensorDriverConvert));
    return 0;
}
