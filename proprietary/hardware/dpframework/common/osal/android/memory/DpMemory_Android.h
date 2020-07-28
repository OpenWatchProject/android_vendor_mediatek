#ifndef __DP_MEMORY_ANDROID_H__
#define __DP_MEMORY_ANDROID_H__

#include "DpDataType.h"

class DpMemory
{
public:
    enum FEATURE_ENUM
    {
        FEATURE_FLUSH_WHOLE_CACHE  = 0x01,
        FEATURE_FLUSH_BUFFER_RANGE = 0x02
    };

    enum FLUSH_TYPE_ENUM
    {
        FLUSH_BEFORE_HW_READ_WHOLE_CACHE,
        FLUSH_BEFORE_HW_READ_BUFFER_RANGE,
        FLUSH_AFTER_HW_WRITE_BUFFER_RANGE
    };

    static DpMemory* Factory(DP_MEMORY_ENUM memory,
                             void           *pBase,
                             uint32_t        size);

    static DpMemory* Factory(DP_MEMORY_ENUM memory,
                             int32_t        FD,
                             uint32_t       size,
                             bool           cache = true,
                             int32_t        *pInternalFD = NULL);

    static DpMemory* Factory(DP_MEMORY_ENUM memory,
                             void           *pVABase,
                             uint32_t       MVABase,
                             uint32_t       size);

    DpMemory()
    {
    }

    virtual ~DpMemory()
    {
    }

    virtual uint32_t queryFeature(void) = 0;

    virtual uint32_t mapHWAddress(DpEngineType type,
                                  uint32_t     layer) = 0;

    virtual void* mapSWAddress() = 0;

    virtual bool flushMemory(FLUSH_TYPE_ENUM type,
                             int32_t         offset = 0,
                             int32_t         size   = 0) = 0;
};

#endif  // __DP_MEMORY_ANDROID_H__
