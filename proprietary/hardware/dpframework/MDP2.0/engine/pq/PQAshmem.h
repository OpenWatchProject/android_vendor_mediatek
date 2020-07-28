#ifndef __PQ_ASHMEM_H__
#define __PQ_ASHMEM_H__

#include "PQMutex.h"
#include "PQAshmemProxy.h"
#include "PQLogger.h"
#include <cutils/log.h>
#include <android/hidl/memory/1.0/IMemory.h>
#include <vendor/mediatek/hardware/pq/2.0/IPictureQuality.h>

using vendor::mediatek::hardware::pq::V2_0::IPictureQuality;
using namespace android;
using ::android::hidl::memory::V1_0::IMemory;
using ::android::sp;

class PQAshmem
{
public:
    PQAshmem();
    ~PQAshmem();
    static PQAshmem*    getInstance();
    static void         destroyInstance();

    int32_t             setAshmemValueByOffset(uint32_t offset, int32_t value);
    int32_t             getAshmemValueByOffset(uint32_t offset, int32_t *value);
    int32_t             setAshmemValueByAddr(unsigned int *addr, int32_t value);
    int32_t             getAshmemValueByAddr(unsigned int *addr, int32_t *value);
    template <typename T>
    int32_t getAshmemArrayByOffset(ProxyCustBuffer proxyNum, int32_t offset, T* value, int32_t size)
    {
        AutoMutex lock(s_ALMutex);

        int s_ashmem_max_offset = PQAshmemProxy::getAshmemMaxOffset();

        if (m_is_ashmem_init == false) {
            PQ_LOGD("[PQAshmem] getAshmemArrayByOffset : Ashmem is not ready\n");
            return -1;
        }

        if (offset < 0 || offset >= MODULE_CUST_MAX_OFFSET) {
            PQ_LOGD("[PQAshmem] getAshmemArrayByOffset : offset (0x%08x) is invalid\n", (int)(offset*sizeof(int32_t)));
            return -1;
        }

        if (size < 0 || (offset + size) >= MODULE_CUST_MAX_OFFSET) {
            PQ_LOGD("[PQAshmem] getAshmemArrayByOffset : size (0x%08x) is invalid\n", (int)(size*sizeof(int32_t)));
            return -1;
        }

        offset += (s_ashmem_max_offset + proxyNum * MODULE_CUST_MAX_OFFSET);
        m_mapAshmem->read();
        memcpy(value, m_ashmem_base + offset, (int)(size * sizeof(unsigned int)));
        m_mapAshmem->commit();
        PQ_LOGI("[PQConfig] getAshmemArray : get array proxyNum:%d offset:0x%08x size:%d\n",
            proxyNum, (int)(offset*sizeof(unsigned int)), size);
        return 0;
    }
    template <typename T>
    int32_t setAshmemArrayByOffset(ProxyCustBuffer proxyNum, int32_t offset, T* value, int32_t size)
    {
        AutoMutex lock(s_ALMutex);

        int s_ashmem_max_offset = PQAshmemProxy::getAshmemMaxOffset();

        if (m_is_ashmem_init == false) {
            PQ_LOGD("[PQAshmem] setAshmemValueByOffset : Ashmem is not ready\n");
            return -1;
        }

        if (offset < 0 || offset >= MODULE_CUST_MAX_OFFSET) {
            PQ_LOGD("[PQAshmem] setAshmemArrayByOffset : offset (0x%08x) is invalid\n", (int)(offset*sizeof(int32_t)));
            return -1;
        }

        if (size < 0 || (offset + size) >= MODULE_CUST_MAX_OFFSET) {
            PQ_LOGD("[PQAshmem] setAshmemArrayByOffset : size (0x%08x) is invalid\n", (int)(size*sizeof(int32_t)));
            return -1;
        }

        offset += (s_ashmem_max_offset + proxyNum * MODULE_CUST_MAX_OFFSET);
        m_mapAshmem->update();
        memcpy(m_ashmem_base + offset, value, (int)(size * sizeof(unsigned int)));
        m_mapAshmem->commit();
        PQ_LOGI("[PQConfig] getAshmemArray : set array proxyNum:%d offset:0x%08x size:%d\n",
            proxyNum, (int)(offset*sizeof(unsigned int)), size);

        return 0;
    }
    unsigned int        *getAshmemBase(void);
    bool                initAshmem(sp<IPictureQuality> service);

private:
    static PQAshmem     *s_pInstance;
    static PQMutex      s_ALMutex;
    uint32_t            m_ashmem_max_offset;
    bool                m_is_ashmem_init;
    unsigned int        *m_ashmem_base;
    sp<IMemory>         m_mapAshmem;
};
#endif

