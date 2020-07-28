#ifndef __SPI_DEBUG_H__
#define __SPI_DEBUG_H__

#include <stdint.h>
#include "drCommon.h"

#include "DrApi/DrApiLogging.h"

#define SPI_LOG(string, args...)                                   \
    if(1)                                                           \
    {                                                               \
        drDbgPrintf("[SPI][tz]"string, ##args);                    \
    }
#define SPI_MSG(string, args...)                                   \
    if(1)                                \
    {                                                               \
        drDbgPrintf("[SPI][tz]"string, ##args);                    \
    }

#define SPI_ERR(string, args...)                                   \
    if(1)                                                           \
    {                                                               \
        drDbgPrintf("[SPI][tz][ERR]"string, ##args);               \
    }
//--------------------------------------------------------------------------

#endif  // __SPI_DEBUG_H__

