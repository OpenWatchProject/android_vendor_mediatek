#ifndef __DP_RAND_GEN_H__
#define __DP_RAND_GEN_H__

#include "DpDataType.h"

#define UTIL_RNG_MWC_INITIALIZE()                   \
    uint32_t m_z = rand();                          \
    uint32_t m_w = m_z;                             \


#define UTIL_RNG_MWC_GENERATE(rnd)                  \
do {                                                \
   m_z = 36969 * (m_z & 65535) + (m_z >> 16);       \
   m_w = 18000 * (m_w & 65535) + (m_w >> 16);       \
   rnd = ((m_z << 16) + m_w);                       \
} while(0)                                          \


DP_STATUS_ENUM utilRNGGenRand(uint8_t    *pBuffer,
                              uint32_t   bufSize);

#endif  // __DP_RAND_GEN_H__
