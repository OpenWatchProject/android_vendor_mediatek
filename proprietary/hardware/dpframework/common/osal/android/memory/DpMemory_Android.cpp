#include "DpMemory.h"
#include "DpMmuHandler.h"
#include "DpIonHandler.h"
#include "DpPmemHandler.h"
#include "DpLogger.h"
#include "DpProfiler.h"

#define DPFRAMEWORK_M4U_ENABLE 1

DpMemory* DpMemory::Factory(DP_MEMORY_ENUM memory,
                            void           *pBase,
                            uint32_t       size)
{
    if (DP_MEMORY_PHY == memory)
    {
        DPLOGI("DpMemory: try creating PMEM memory handler\n");
        return new DpPmemHandler(pBase, 0, size);
    }
    else
    {
#if DPFRAMEWORK_M4U_ENABLE
    #if defined(MTK_M4U_SUPPORT)
        DPLOGI("DpMemory: try creating MMU memory handler\n");
        return new DpMmuHandler(pBase, size);
    #else
        DPLOGI("DpMemory: try creating ION memory handler\n");
        return new DpIonHandler(pBase, size);
    #endif //MTK_M4U_SUPPORT
#else
        DPLOGI("DpMemory: try creating PMEM memory handler\n");
        return new DpPmemHandler(pBase, 0, size);
#endif
    }

    return 0;
}


DpMemory* DpMemory::Factory(DP_MEMORY_ENUM memory,
                            int32_t        FD,
                            uint32_t       size,
                            bool           cache,
                            int32_t        *pInternalFD)
{
    if (DP_MEMORY_ION == memory)
    {
#if DPFRAMEWORK_M4U_ENABLE
        DPLOGI("DpMemoryProxy: try creating ION memory handler\n");
        return new DpIonHandler(FD, size, cache, pInternalFD);
#else

        DPLOGI("DpMemoryProxy: try creating PMEM memory handler\n");
        return new DpPmemHandler(NULL, 0, size);
#endif
    }

    return 0;
}


// Pre-allocated memory(include VA and MVA) use Mmu handler
DpMemory* DpMemory::Factory(DP_MEMORY_ENUM memory,
                            void           *pVABase,
                            uint32_t       MVABase,
                            uint32_t       size)
{
#if DPFRAMEWORK_M4U_ENABLE
    if ((DP_MEMORY_MVA == memory) && (0 != MVABase))
    {
        if (NULL != pVABase)
        {
            DPLOGI("Try creating pre-allocated MMU memory handler\n");
            return new DpMmuHandler(pVABase, MVABase, size);
        }
        else
        {
            DPLOGI("Try creating pre-allocated MMU memory handler (MVA only)\n");
            return new DpMmuHandler(MVABase, size);
        }
    }
#else
    return new DpPmemHandler(pVABase, MVABase, size);
#endif

    return 0;
}
