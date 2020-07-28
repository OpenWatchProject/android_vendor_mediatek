#include "DpTileUtil.h"

int32_t tileCalculateLCM(int32_t a, int32_t b)
{
    int32_t m;
    int32_t n;

    /* fast return */
    if (1 == a)
    {
        return b;
    }
    if (1 == b)
    {
        return a;
    }

    m = a;
    n = b;

    while (m != n)
    {
        if (m > n)
        {
            m -= n;
        }
        else
        {
            n -= m;
        }
    }

    if (1 == m)
    {
        return a * b;
    }
    else
    {
        return a * b / m;
    }
}
