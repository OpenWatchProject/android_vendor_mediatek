#ifndef __DP_PMEM_HANDLER_H__
#define __DP_PMEM_HANDLER_H__

#include "DpMemory.h"

class DpPmemHandler: public DpMemory
{
public:
    DpPmemHandler(void     *pVA,
                  uint32_t  pMVA,
                  uint32_t size);

    virtual ~DpPmemHandler();
   
    virtual uint32_t queryFeature(void);

    virtual uint32_t mapHWAddress(DpEngineType type,
                                  uint32_t     plane);

    virtual void* mapSWAddress();

    virtual bool flushMemory(FLUSH_TYPE_ENUM type,
                             int32_t         offset = 0,
                             int32_t         size   = 0);

private:
    void        *m_pOrig;
    void        *m_pPmemVA;
    uint32_t    m_pmemPA;
#ifdef HAVE_PMEM
    int32_t     m_pmemFD;
#endif
    uint32_t    m_pmemSize;
    bool        m_isSelfAlloc;

    // Allocate buffer internal
    bool alloc_internal(uint32_t size);

    // Free the allocated buffer
    void free_internal();
};

#endif  // __DP_PMEM_HANDLER_H__
