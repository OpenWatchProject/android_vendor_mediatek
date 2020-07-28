#ifndef __PQCONFIG_H__
#define __PQCONFIG_H__

#include "PQMutex.h"
#include "cust_color.h"
#include "cust_tdshp.h"
#ifdef SUPPORT_CCORR
#include "cust_ccorr.h"
#endif
#include "DpDataType.h"
#include "PQLogger.h"
#include "PQAshmem.h"

enum PQServiceStatus
{
    PQSERVICE_READY = 0,
    PQSERVICE_NOT_READY,
    BOOT_MODE_UNSUPPORT_PQSERVICE,
    PQSERVICE_NOT_ENABLE
};

enum BOOT_MODE_ENUM
{
    NORMAL_BOOT = 0,
    META_MODE = 1,
    RECOVERY_BOOT = 2,
    SW_REBOOT = 3,
    FACTORY_MODE = 4,
    ADVMETA_BOOT = 5,
    ALARM_BOOT = 7
};

#define GLOBAL_PQ_VIDEO 0
#define GLOBAL_PQ_OTHER 1
typedef struct GlobalPQParam
{
    uint32_t globalPQSupport;
    int32_t globalPQSwitch;
    uint32_t globalPQStrength;
    GLOBAL_PQ_INDEX_T globalPQindex;
    uint32_t globalPQindexInit;
    uint32_t globalPQType;
} GlobalPQParam;

class PQConfig
{
public:
    PQConfig();
    ~PQConfig();
    static PQConfig*    getInstance();
    static void         destroyInstance();

    bool                isEnabled();
    int32_t             getAshmemContext(uint32_t offset);
    PQServiceStatus     getPQServiceStatus(void);
    template <typename T>
    int32_t getAshmemArray(ProxyCustBuffer proxyNum, int32_t offset, T* value, int32_t size)
    {
        PQAshmem* pPQAshmem = PQAshmem::getInstance();
        int32_t ret = 0;

        ret = pPQAshmem->getAshmemArrayByOffset(proxyNum, offset, value, size);
        if (ret == 0)
        {
            return 1;
        }
        else
        {
            PQ_LOGE("[PQConfig] getAshmemArray failed\n");
            return ret;
        }
    }
    template <typename T>
    int32_t setAshmemArray(ProxyCustBuffer proxyNum, int32_t offset, T* value, int32_t size)
    {
        PQAshmem* pPQAshmem = PQAshmem::getInstance();
        int32_t ret = 0;

        ret = pPQAshmem->setAshmemArrayByOffset(proxyNum, offset, value, size);
        if (ret == 0)
        {
            return 1;
        }
        else
        {
            PQ_LOGE("[PQConfig] setAshmemArray failed\n");
            return ret;
        }
    }

private:
    void                checkBootMode(void);
    PQServiceStatus     m_PQServiceStatus;
    PQServiceStatus     updatePQServiceStatus(void);

private:
    static PQConfig     *s_pInstance;
    static PQMutex      s_ALMutex;
};
#endif //__PQCONFIG_H__
