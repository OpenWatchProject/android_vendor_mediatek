#ifndef __DP_MMU_HANDLER_H__
#define __DP_MMU_HANDLER_H__

#include "DpMemory.h"
#include "DpMutex.h"

#if defined(MTK_M4U_SUPPORT)
class MTKM4UDrv;
#else
#include <ion/ion.h>
#endif //MTK_M4U_SUPPORT

class DpMmuHandler: public DpMemory
{
public:
    DpMmuHandler(void     *pBase,
                 uint32_t size);

    DpMmuHandler(uint32_t MVABase,
                 uint32_t size);

    DpMmuHandler(void     *pVABase,
                 uint32_t MVABase,
                 uint32_t size);

    virtual ~DpMmuHandler();

    static void destroyInstance();

    virtual uint32_t queryFeature(void);

    virtual uint32_t mapHWAddress(DpEngineType type,
                                  uint32_t     plane);

    virtual void* mapSWAddress();

    virtual bool flushMemory(FLUSH_TYPE_ENUM type,
                             int32_t         offset = 0,
                             int32_t         size   = 0);

private:
#if defined(MTK_M4U_SUPPORT)
    static MTKM4UDrv *s_pM4uDrv;
#else
    static int32_t    s_IONHandle;
#endif //MTK_M4U_SUPPORT
    static DpMutex   s_instMutex;

    bool             m_selfAlloc;
    bool             m_selfMapMVA;
    uint32_t         m_memorySize;
    void             *m_memoryVA;
    uint32_t         m_memoryPA;
    DpEngineType     m_engineType;
    //int32_t          m_outputPort;
    uint32_t         m_curPlane;
    //int32_t          m_coherent;
};

#endif  // __DP_MMU_HANDLER_H__
