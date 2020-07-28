#ifndef __DP_ENGINE_MUTEX_H__
#define __DP_ENGINE_MUTEX_H__

#include "DpDataType.h"
#include "DpEngineType.h"

#ifndef ISP_MAX_OUTPUT_PORT_NUM
#define ISP_MAX_OUTPUT_PORT_NUM 4
#endif //ISP_MAX_OUTPUT_PORT_NUM

class DpPathBase;
class DpCommand;

class DpEngineMutex
{
public:
    DpEngineMutex(DpPathBase *path);

    ~DpEngineMutex();

    bool require(DpCommand&);

    bool release(DpCommand&);

private:
    DpPathBase *m_pPath;
    int32_t    m_mutexID;
    uint32_t   m_mutexMod;
    uint32_t   m_mutexMod2;
    int64_t    m_engineSOF[tTotal];
};

#endif  // __DP_ENGINE_MUTEX_H__
