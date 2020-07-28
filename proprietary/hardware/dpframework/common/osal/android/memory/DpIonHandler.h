#ifndef __DP_ION_HANDLER_H__
#define __DP_ION_HANDLER_H__

#include <ion/ion.h>
#include "DpMemory.h"
#include "DpMutex.h"

class DpIonHandler: public DpMemory
{
public:
    DpIonHandler(void     *pBase,
                 uint32_t size);

    DpIonHandler(int32_t  FD,
                 uint32_t size,
                 bool     cache,
                 int32_t  *pInternalFD);

    virtual ~DpIonHandler();

    static void destroyInstance();

    virtual uint32_t queryFeature(void);

    virtual uint32_t mapHWAddress(DpEngineType type,
                                  uint32_t     plane);

    virtual void* mapSWAddress();

    virtual bool flushMemory(FLUSH_TYPE_ENUM type,
                             int32_t         offset = 0,
                             int32_t         size   = 0);

private:
    static int32_t    s_IONHandle;
    static DpMutex    s_instMutex;

    bool              m_cacheAble;
    bool              m_selfAlloc;
    bool              m_selfMapMVA;
    ion_user_handle_t m_allocHandle;
    int32_t           m_shareFD;
    uint32_t          m_memorySize;
    void              *m_memoryVA;
    uint32_t          m_memoryPA;
    DpEngineType      m_engineType;
    //int32_t           m_outputPort;
    uint32_t          m_curPlane;

    // Allocate buffer internal
    bool alloc_internal(uint32_t size);

    // Import specified buffer
    bool import_internal(int32_t FD);

    // Free the allocated buffer
    void free_internal();

    int32_t cacheFlushbyRange();

    int32_t cacheInvalidbyRange();
};

#endif  // __DP_ION_HANDLER_H__
