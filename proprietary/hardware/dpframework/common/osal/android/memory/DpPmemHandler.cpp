#include "DpPmemHandler.h"
#include "DpLogger.h"

#include <sys/types.h>

//bobuld workaround pdk build error needing to review
//#define HAVE_PMEM

#ifdef HAVE_PMEM
#include <cutils/pmem.h>
#endif

DpPmemHandler::DpPmemHandler(void     *pVA,
                             uint32_t  pMVA,
                             uint32_t size)
    : m_pOrig(pVA),
      m_pPmemVA(0),
      m_pmemPA(0),
#ifdef HAVE_PMEM
      m_pmemFD(-1),
#endif
      m_pmemSize(0)
{
    bool status;

    DPLOGI("DpPmemHandler: Create PMEM handler begin\n");
    
    if(0 != pMVA)
    {
        m_isSelfAlloc = false;
        m_pmemPA = pMVA;
        m_pPmemVA = pVA;   
        m_pmemSize = size;
    }else
    {    
        m_isSelfAlloc = true;
        status = alloc_internal(size);
        assert(true == status);
    }
    DPLOGI("DpPmemHandler: Create PMEM handler end\n");
}


DpPmemHandler::~DpPmemHandler()
{
    DPLOGI("DpPmemHandler: Destroy PMEM handler begin\n");
    if(m_isSelfAlloc)
    {
        free_internal();
    }

    DPLOGI("DpPmemHandler: Destroy PMEM handler end\n");
}


uint32_t DpPmemHandler::queryFeature(void)
{
    return FEATURE_FLUSH_BUFFER_RANGE;
}


uint32_t DpPmemHandler::mapHWAddress(DpEngineType,
                                     uint32_t)
{
    return m_pmemPA;
}


void* DpPmemHandler::mapSWAddress()
{
    return m_pPmemVA;
}

bool DpPmemHandler::alloc_internal(uint32_t size)
{
#ifdef HAVE_PMEM
    DPLOGI("DpPmemHandler - Allocate pmem size %d\n", size);
    if(m_pPmemVA != 0) 
    {
        DPLOGE(" Already allocate memory, please call free if want to allocate again");
        return false;
    }

    m_pPmemVA = pmem_alloc_sync(size , &m_pmemFD);
    if(NULL == m_pPmemVA)
    {
        DPLOGE("Can not allocate pmem memory\n");
        return false;
    }
    m_pmemSize = size;
    m_pmemPA   = (uint32_t)pmem_get_phys(m_pmemFD);

    return true;
#else
    DP_UNUSED(size);
    return false;
#endif
}


void DpPmemHandler::free_internal()
{
#ifdef HAVE_PMEM
    if(m_pPmemVA != 0)
    {
        pmem_free(m_pPmemVA , m_pmemSize, m_pmemFD);
    }
#endif
    m_pPmemVA = 0;
    m_pmemPA  = 0;
}


bool DpPmemHandler::flushMemory(FLUSH_TYPE_ENUM type,
                                int32_t         offset,
                                int32_t         size)
{
    if ((0 != m_pOrig) && (0 != m_pPmemVA))
    {
        if (0 == size)
        {
            size = m_pmemSize;
        }

        if (m_isSelfAlloc)
        {
            switch (type)
            {
                case FLUSH_BEFORE_HW_READ_WHOLE_CACHE:
                case FLUSH_BEFORE_HW_READ_BUFFER_RANGE:
                    DPLOGI("DpPmemHandler: flush read buffer\n");
                    DPLOGI(" copy %p to %p with size %d\n", (uint8_t*)m_pOrig + offset, (uint8_t*)m_pPmemVA + offset, size);   
                    memcpy((uint8_t*)m_pPmemVA + offset, (uint8_t*)m_pOrig + offset, size);
                    break;
                case FLUSH_AFTER_HW_WRITE_BUFFER_RANGE:
                    DPLOGI("DpPmemHandler: flush write buffer\n");
                    DPLOGI(" copy %p to %p with size %d\n", (uint8_t*)m_pPmemVA + offset, (uint8_t*)m_pOrig + offset, size);
                    memcpy((uint8_t*)m_pOrig + offset, (uint8_t*)m_pPmemVA + offset, size);
                    break;
                default:
                    return false;
            }
        }
    }

    return true;
}
