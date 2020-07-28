#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1
#include <cutils/log.h>
#include <PQCommon.h>
#include "PQServiceCommon.h"
#include "PQHFGConfig.h"
#ifdef SUPPORT_PQ_WHITE_LIST
#include "PQWhiteList.h"
#endif

PQHFGConfig::PQHFGConfig()
{
    PQ_LOGI("[PQHFGConfig] PQHFGConfig()... ");

}

PQHFGConfig::~PQHFGConfig()
{
    PQ_LOGI("[PQHFGConfig] ~PQHFGConfig()... ");
}

int32_t PQHFGConfig::isEnabled()
{
    int32_t enable;
    PQConfig* pPQConfig = PQConfig::getInstance();

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return false;
    }

    enable =  (pPQConfig->getAshmemContext(HFG_ENABLE));

#ifdef SUPPORT_PQ_WHITE_LIST
    PQWhiteList &whiteList = PQWhiteList::getInstance();
    enable = whiteList.isPQEnabledOnActive() ? enable : 0;
#endif

    PQ_LOGI("[PQHFGConfig] PQHFGConfig::isEnabled = %d \n", enable);
    return enable;
}

bool PQHFGConfig::getHFGConfig(HFG_CONFIG_T* HFGConfig)
{
    bool status = true;

    HFGConfig->ENABLE = isEnabled();
    HFGConfig->ispTuningFlag = getIspTuningFlag();

    return status;
}

uint32_t PQHFGConfig::getIspTuningFlag(void)
{
    PQConfig* pPQConfig = PQConfig::getInstance();

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return 0;
    }

    return pPQConfig->getAshmemContext(MDP_HFG_ISPTUNING_FLAG);
}
