#include "DpRandGen.h"

DP_STATUS_ENUM utilRNGGenRand(uint8_t    *pBuffer,
                              uint32_t   bufSize)
{
    uint32_t index;
    uint32_t count;
    uint32_t remain;
    uint32_t random;

    UTIL_RNG_MWC_INITIALIZE();

    count  = bufSize / 4;
    remain = bufSize % 4;
    for (index = 0; index < count; index++)
    {
        UTIL_RNG_MWC_GENERATE(random);

        if (0x0 == (((uint32_t)pBuffer) & 0x03))
        {
            *((uint32_t*)pBuffer) = random;
        }
        else
        {
            pBuffer[0] = (uint8_t)((random >>  0) & 0x0FF);
            pBuffer[1] = (uint8_t)((random >>  8) & 0x0FF);
            pBuffer[2] = (uint8_t)((random >> 16) & 0x0FF);
            pBuffer[3] = (uint8_t)((random >> 24) & 0x0FF);
        }
        pBuffer += 4;
    }

    UTIL_RNG_MWC_GENERATE(random);
    for (index = 0; index < remain; index++)
    {
        *pBuffer = (uint8_t)(random & 0x0FF);
        pBuffer += 1;
        random >>= 8;
    }

    return DP_STATUS_RETURN_SUCCESS;
}
