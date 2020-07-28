#ifndef __PQTUNINGBUFFER_H__
#define __PQTUNINGBUFFER_H__

#include <stdio.h>
#include "PQAshmemProxy.h"

class PQTuningBuffer
{
public:
    PQTuningBuffer(ProxyTuningBuffer tuning_mode);
    ~PQTuningBuffer();

    bool isValid(void);
    bool isBypassHWAccess(void);
    bool isOverwritten(void);
    bool isSync(void);
    bool toBePrepared(void);
    void push(void);
    void resetReady(void);
    void pull(void);

    unsigned int *getOverWrittenBuffer(void) { return m_overwritten_addr; }
    unsigned int *getReadingBuffer(void) { return m_base_addr; }
    unsigned int getModuleSize(void) { return m_module_size; }
protected:
    unsigned int *queryAshmemBufferBase(void);

protected:
    bool m_is_ashmem_init;
    unsigned int *m_base_addr;
    unsigned int *m_overwritten_addr;
    int m_tunning_mode;
    int m_max_offset;
    unsigned int m_module_size;
};
#endif //__PQTUNINGBUFFER_H__