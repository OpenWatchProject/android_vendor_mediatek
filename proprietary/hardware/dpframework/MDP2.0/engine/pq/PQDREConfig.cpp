#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1
#include <cutils/log.h>
#include "PQServiceCommon.h"
#include "PQDREConfig.h"
#ifdef SUPPORT_PQ_WHITE_LIST
#include "PQWhiteList.h"
#endif

PQDREConfig::PQDREConfig()
{
    PQ_LOGI("[PQDREConfig] PQDREConfig()... ");
}

PQDREConfig::~PQDREConfig()
{
    PQ_LOGI("[PQDREConfig] ~PQDREConfig()... ");
}

bool PQDREConfig::getDREConfig(DRE_CONFIG_T* DREConfig)
{
    bool status = true;

    DREConfig->ENABLE = isEnabled();
    DREConfig->VP_ENABLE = isVPEnabled();
    DREConfig->HDR_VP_ENABLE = isHDRVPEnabled();
    DREConfig->debugFlag = getDebugFlag();
    DREConfig->adaptiveCaltmFlag = getAdaptiveCaltmFlag();
    DREConfig->driverDebugFlag = getDriverDebugFlag();
    DREConfig->driverBLKFlag = getDriverBlockFlag();
    DREConfig->demoWinX = getDemoWindowX();
    DREConfig->ispTuningFlag = getIspTuningFlag();

    return status;
}

uint32_t PQDREConfig::isEnabled(void)
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    uint32_t enable = 0;
    bool whiteList_enable = true;

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return enable;
    }

#ifdef SUPPORT_PQ_WHITE_LIST
    PQWhiteList &whiteList = PQWhiteList::getInstance();
    whiteList_enable = whiteList.isPQEnabledOnActive();
    PQ_LOGI("[PQDREConfig] whiteList.isPQEnabledOnActive() = %d \n", whiteList_enable);
#endif

    if (whiteList_enable == true)
    {
        enable = pPQConfig->getAshmemContext(MDP_DRE_ENABLE);
    }

    PQ_LOGI("[PQDREConfig] PQDREConfig::isEnabled = %d \n", enable);

    return enable;
}

uint32_t PQDREConfig::isVPEnabled(void)
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    uint32_t enable = 0;
    bool whiteList_enable = true;

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return enable;
    }

#ifdef SUPPORT_PQ_WHITE_LIST
    PQWhiteList &whiteList = PQWhiteList::getInstance();
    whiteList_enable = whiteList.isPQEnabledOnActive();
    PQ_LOGI("[PQDREConfig] whiteList.isPQEnabledOnActive() = %d \n", whiteList_enable);
#endif

    if (whiteList_enable == true)
    {
        enable = pPQConfig->getAshmemContext(MDP_DRE_VP_ENABLE);
    }

    PQ_LOGI("[PQDREConfig] PQDREConfig::isVPEnabled = %d \n", enable);

    return enable;
}

uint32_t PQDREConfig::isHDRVPEnabled(void)
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    uint32_t enable = 0;
    bool whiteList_enable = true;

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return enable;
    }

#ifdef SUPPORT_PQ_WHITE_LIST
    PQWhiteList &whiteList = PQWhiteList::getInstance();
    whiteList_enable = whiteList.isPQEnabledOnActive();
    PQ_LOGI("[PQDREConfig] whiteList.isPQEnabledOnActive() = %d \n", whiteList_enable);
#endif

    if (whiteList_enable == true)
    {
        enable = pPQConfig->getAshmemContext(MDP_DRE_HDR_VP_ENABLE);
    }

    PQ_LOGI("[PQDREConfig] PQDREConfig::isHDRVPEnabled = %d \n", enable);

    return enable;
}

uint32_t PQDREConfig::getDebugFlag(void)
{
    PQConfig* pPQConfig = PQConfig::getInstance();

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return 0;
    }

    return pPQConfig->getAshmemContext(MDP_DRE_DEBUG_FLAG);
}

uint32_t PQDREConfig::getAdaptiveCaltmFlag(void)
{
    PQConfig* pPQConfig = PQConfig::getInstance();

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return 0;
    }

    return pPQConfig->getAshmemContext(ADAPTIVE_CALTM_DEBUG_FLAG);
}

uint32_t PQDREConfig::getDriverDebugFlag(void)
{
    PQConfig* pPQConfig = PQConfig::getInstance();

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return 0;
    }

    return pPQConfig->getAshmemContext(MDP_DREDRIVER_DEBUG_FLAG);
}

uint32_t PQDREConfig::getDemoWindowX(void)
{
    PQConfig* pPQConfig = PQConfig::getInstance();

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return 0;
    }

    return pPQConfig->getAshmemContext(MDP_DRE_DEMOWIN_X_FLAG);
}

uint32_t PQDREConfig::getDriverBlockFlag(void)
{
    PQConfig* pPQConfig = PQConfig::getInstance();

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return 0;
    }

    return pPQConfig->getAshmemContext(MDP_DREDRIVER_BLK_FLAG);
}

uint32_t PQDREConfig::getIspTuningFlag(void)
{
    PQConfig* pPQConfig = PQConfig::getInstance();

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return 0;
    }

    return pPQConfig->getAshmemContext(MDP_DRE_ISPTUNING_FLAG);
}

