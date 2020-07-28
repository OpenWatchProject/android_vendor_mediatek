// Log interface, do NOT modify
#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1
#include <cutils/log.h>
#define PQ_LOGD(fmt, arg...) ALOGD(fmt, ##arg)

#include "PQTuningBuffer.h"
#include "PQAshmem.h"
#include "PQConfig.h"

PQTuningBuffer::PQTuningBuffer(ProxyTuningBuffer tuning_mode)
    : m_is_ashmem_init(false),
    m_base_addr(NULL),
    m_tunning_mode(tuning_mode)
{
    m_max_offset = PQAshmemProxy::getModuleMaxOffset();
    m_module_size = PQAshmemProxy::getModuleSize();

    m_base_addr = queryAshmemBufferBase();
    m_overwritten_addr = new unsigned int[m_max_offset];
}

PQTuningBuffer::~PQTuningBuffer()
{
    delete[] m_overwritten_addr;
}

unsigned int *PQTuningBuffer::queryAshmemBufferBase(void)
{
    PQAshmem* pPQAshmem = PQAshmem::getInstance();
    unsigned int *p_ashmem_base = NULL;
    unsigned int *p_buffer_base = NULL;

    p_ashmem_base = pPQAshmem->getAshmemBase();
    if (p_ashmem_base != NULL)
    {
        p_buffer_base = p_ashmem_base + m_tunning_mode * m_max_offset;
    } else {
        PQ_LOGE("[PQTuningBuffer] queryAshmemBufferBase fail!!!\n");
    }

    return p_buffer_base;
}

bool PQTuningBuffer::isValid(void)
{
    PQConfig* pPQConfig = PQConfig::getInstance();

    do {
        if (m_is_ashmem_init == true) {
            break;
        }

        if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
        {
            PQ_LOGE("[PQTuningBuffer] PQ Service is not ready\n");
            break;
        }

        m_base_addr = queryAshmemBufferBase();
        if (m_base_addr == NULL) {
            PQ_LOGE("[PQTuningBuffer] ashmem is not valid\n");
            break;
        }

        m_is_ashmem_init = true;
    } while (0);

    return m_is_ashmem_init;
}

bool PQTuningBuffer::isBypassHWAccess(void)
{
    PQAshmem* pPQAshmem = PQAshmem::getInstance();
    int32_t tuning_offset = m_max_offset - (PROXY_TUNING_FLAG+1);
    int32_t getValue;

    pPQAshmem->getAshmemValueByAddr(m_base_addr + tuning_offset, &getValue);

    if (getValue != PROXY_TUNING_BYPASSHWACCESS) {
        return false;
    }

    return true;
}

bool PQTuningBuffer::isOverwritten(void)
{
    PQAshmem* pPQAshmem = PQAshmem::getInstance();
    int32_t tuning_offset = m_max_offset - (PROXY_TUNING_FLAG+1);
    int32_t getValue;

    pPQAshmem->getAshmemValueByAddr(m_base_addr + tuning_offset, &getValue);

    if (getValue != PROXY_TUNING_OVERWRITTEN) {
        return false;
    }

    return true;
}

bool PQTuningBuffer::isSync(void)
{
    PQAshmem* pPQAshmem = PQAshmem::getInstance();
    int32_t sync_offset = m_max_offset - (PROXY_SYNC_FLAG+1);
    int32_t getValue;

    pPQAshmem->getAshmemValueByAddr(m_base_addr + sync_offset, &getValue);

    if (getValue != 1) {
        return false;
    }

    return true;
}

bool PQTuningBuffer::toBePrepared(void)
{
    PQAshmem* pPQAshmem = PQAshmem::getInstance();
    int32_t tuning_offset = m_max_offset - (PROXY_TUNING_FLAG+1);
    int32_t prepare_offset = m_max_offset - (PROXY_PREPARE_FLAG+1);
    int32_t getValue;

    pPQAshmem->getAshmemValueByAddr(m_base_addr + tuning_offset, &getValue);

    if (getValue != PROXY_TUNING_READING) {
        return false;
    }

    pPQAshmem->getAshmemValueByAddr(m_base_addr + prepare_offset, &getValue);

    if (getValue != 1) {
        return false;
    }

    return true;
}

void PQTuningBuffer::push(void)
{
    PQAshmem* pPQAshmem = PQAshmem::getInstance();
    int32_t prepare_offset = m_max_offset - (PROXY_PREPARE_FLAG+1);
    int32_t ready_offset = m_max_offset - (PROXY_READY_FLAG+1);

    pPQAshmem->setAshmemValueByAddr(m_base_addr + prepare_offset, 0);
    pPQAshmem->setAshmemValueByAddr(m_base_addr + ready_offset, 1);

    memcpy(m_overwritten_addr, m_base_addr, m_module_size);
}

void PQTuningBuffer::resetReady(void)
{
    PQAshmem* pPQAshmem = PQAshmem::getInstance();
    int32_t ready_offset = m_max_offset - (PROXY_READY_FLAG+1);

    pPQAshmem->setAshmemValueByAddr(m_base_addr + ready_offset, 0);
}

void PQTuningBuffer::pull(void)
{
    PQAshmem* pPQAshmem = PQAshmem::getInstance();
    int32_t sync_offset = m_max_offset - (PROXY_SYNC_FLAG+1);
    int32_t ok_offset = m_max_offset - (PROXY_SYNC_OK_FLAG+1);

    memcpy(m_overwritten_addr, m_base_addr, m_module_size);

    pPQAshmem->setAshmemValueByAddr(m_base_addr + sync_offset, 0);
    pPQAshmem->setAshmemValueByAddr(m_base_addr + ok_offset, 1);
}
