#define LOG_TAG "PQ"
#include "PQAshmem.h"
#include <hidlmemory/mapping.h>

using vendor::mediatek::hardware::pq::V2_0::Result;
using ::android::hardware::hidl_memory;

PQAshmem* PQAshmem::s_pInstance = NULL;
PQMutex   PQAshmem::s_ALMutex;

PQAshmem* PQAshmem::getInstance()
{
    AutoMutex lock(s_ALMutex);

    if(NULL == s_pInstance)
    {
        s_pInstance = new PQAshmem();
        atexit(PQAshmem::destroyInstance);
    }

    return s_pInstance;
}

void PQAshmem::destroyInstance()
{
    AutoMutex lock(s_ALMutex);

    if (NULL != s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = NULL;
    }
}

PQAshmem::PQAshmem()
    :m_is_ashmem_init(false),
    m_ashmem_base(NULL)
{
    m_ashmem_max_offset = static_cast<uint32_t>(PQAshmemProxy::getAshmemMaxOffset());
}

PQAshmem::~PQAshmem()
{
}

bool PQAshmem::initAshmem(sp<IPictureQuality> service)
{
    AutoMutex lock(s_ALMutex);

    do {
        if (m_is_ashmem_init == true) {
            break;
        }

        android::hardware::Return<void> ret = service->getAshmem(
            [&] (Result retval, const hidl_memory& memory) {
            if (retval == Result::OK) {
                m_mapAshmem = mapMemory(memory);
                if (m_mapAshmem != NULL) {
                    m_ashmem_base = static_cast<unsigned int*>(static_cast<void*>(m_mapAshmem->getPointer()));
                    m_is_ashmem_init = true;
                }
            }
        });
        if (!ret.isOk()){
            PQ_LOGE("Transaction error in IPictureQuality::getAshmem");
        }
    } while (0);

    return m_is_ashmem_init;
}

unsigned int *PQAshmem::getAshmemBase(void)
{
    if (m_is_ashmem_init == true)
    {
        return m_ashmem_base;
    }
    else
    {
        return NULL;
    }
}

int32_t PQAshmem::setAshmemValueByOffset(uint32_t offset, int32_t value)
{
    AutoMutex lock(s_ALMutex);

    if (m_is_ashmem_init == false) {
        PQ_LOGD("[PQAshmem] setAshmemValueByOffset : Ashmem is not ready\n");
        return -1;
    }

    if (offset > m_ashmem_max_offset) {
        PQ_LOGD("[PQAshmem] setAshmemValueByOffset : offset (0x%08x) is invalid\n", (int)(offset*sizeof(uint32_t)));
        return -1;
    }

    m_mapAshmem->update();
    *(m_ashmem_base + offset) = value;
    m_mapAshmem->commit();
    PQ_LOGI("[PQAshmem] setAshmemValueByOffset : set offset 0x%08x = 0x%08x\n", (int)(offset*sizeof(uint32_t)), value);

    return 0;
}

int32_t PQAshmem::getAshmemValueByOffset(uint32_t offset, int32_t *value)
{
    AutoMutex lock(s_ALMutex);

    if (m_is_ashmem_init == false) {
        PQ_LOGD("[PQAshmem] getAshmemValueByOffset : Ashmem is not ready\n");
        return -1;
    }

    if (offset > m_ashmem_max_offset) {
        PQ_LOGD("[PQAshmem] getAshmemValueByOffset : offset (0x%08x) is invalid\n", (int)(offset*sizeof(uint32_t)));
        return -1;
    }

    // get field value
    m_mapAshmem->read();
    *value = *(m_ashmem_base + offset);
    m_mapAshmem->commit();
    PQ_LOGI("[PQAshmem] getAshmemValueByOffset : get offset 0x%08x = 0x%08x\n", (int)(offset*sizeof(uint32_t)), *value);

    return 0;
}

int32_t PQAshmem::setAshmemValueByAddr(unsigned int *addr, int32_t value)
{
    AutoMutex lock(s_ALMutex);

    if (m_is_ashmem_init == false) {
        PQ_LOGD("[PQAshmem] setAshmemValueByAddr : Ashmem is not ready\n");
        return -1;
    }

    if (addr > (m_ashmem_base + m_ashmem_max_offset) || addr < m_ashmem_base) {
        PQ_LOGD("[PQAshmem] setAshmemValueByAddr : addr (%p) is invalid; m_ashmem_base(%p); m_ashmem_max_offset(%d)\n", addr, m_ashmem_base, m_ashmem_max_offset);
        return -1;
    }

    m_mapAshmem->update();
    *(addr) = value;
    m_mapAshmem->commit();
    PQ_LOGI("[PQAshmem] setAshmemValueByAddr : set addr %p = 0x%08x\n", addr, value);

    return 0;
}

int32_t PQAshmem::getAshmemValueByAddr(unsigned int *addr, int32_t *value)
{
    AutoMutex lock(s_ALMutex);

    if (m_is_ashmem_init == false) {
        PQ_LOGD("[PQAshmem] getAshmemValueByAddr : Ashmem is not ready\n");
        return -1;
    }

    if (addr > (m_ashmem_base + m_ashmem_max_offset) || addr < m_ashmem_base) {
        PQ_LOGD("[PQAshmem] getAshmemValueByAddr : addr (%p) is invalid;m_ashmem_base(%p); m_ashmem_max_offset(%d)\n", addr, m_ashmem_base, m_ashmem_max_offset);
        return -1;
    }

    // get field value
    m_mapAshmem->read();
    *value = *(addr);
    m_mapAshmem->commit();
    PQ_LOGI("[PQAshmem] getAshmemValueByAddr : get addr %p = 0x%08x\n", addr, *value);

    return 0;
}
