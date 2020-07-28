#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1
#include <cutils/properties.h>
#include <cutils/log.h>
#include <PQCommon.h>
#include "PQServiceCommon.h"
#include "PQHDRConfig.h"
#ifdef SUPPORT_PQ_WHITE_LIST
#include "PQWhiteList.h"
#endif

PQHDRConfig::PQHDRConfig()
{
    PQ_LOGI("[PQHDRConfig] PQHDRConfig()... ");
    char value[PROPERTY_VALUE_MAX];
    property_get(PQ_HDR_VIDEO_SUPPORT_STR, value, PQ_HDR_VIDEO_SUPPORT_DEFAULT);
    m_HDRSupport = atoi(value);
};

PQHDRConfig::~PQHDRConfig()
{
    PQ_LOGI("[PQHDRConfig] ~PQHDRConfig()... ");
};

uint32_t PQHDRConfig::isEnabled(void)
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
    PQ_LOGI("[PQHDRConfig] whiteList.isPQEnabledOnActive() = %d \n", whiteList_enable);
#endif

    if (whiteList_enable == true)
    {
        enable = pPQConfig->getAshmemContext(HDR_VIDEO_ENABLE);
    }

    PQ_LOGI("[PQHDRConfig] PQHDRConfig::isEnabled = %d \n", enable);

    return enable;
}

uint32_t PQHDRConfig::isCcorrEnabled(void)
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
    PQ_LOGI("[PQHDRConfig] whiteList.isPQEnabledOnActive() = %d \n", whiteList_enable);
#endif

    if (whiteList_enable == true)
    {
        enable = pPQConfig->getAshmemContext(MDP_CCORR_DEBUG_FLAG);
    }

    PQ_LOGI("[PQHDRConfig] PQHDRConfig::isCcorrEnabled = %d \n", enable);

    return enable;
}

uint32_t PQHDRConfig::getExternalPanelNits(void)
{
    uint32_t externlPanelNits = EXTERNAL_PANEL_NITS_DEFAULT;
    PQConfig* pPQConfig = PQConfig::getInstance();

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return externlPanelNits;
    }

    externlPanelNits = pPQConfig->getAshmemContext(EXTERNAL_PANEL_NITS);

    PQ_LOGI("[PQHDRConfig] ExternalPanelNits = %d \n", externlPanelNits);

    return externlPanelNits;
}

bool PQHDRConfig::getHDRConfig(HDR_CONFIG_T* HDRConfig)
{
    bool status = true;

    HDRConfig->ENABLE = isEnabled();
    HDRConfig->CCORR_ENABLE = isCcorrEnabled();
    HDRConfig->debugFlag = getDebugFlag();
    HDRConfig->externalPanelNits = getExternalPanelNits();
    HDRConfig->driverDebugFlag = getDriverDebugFlag();

    return status;
}

uint32_t PQHDRConfig::getDebugFlag(void)
{
    PQConfig* pPQConfig = PQConfig::getInstance();

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return 0;
    }

    return pPQConfig->getAshmemContext(HDR_DEBUG_FLAG);
}

uint32_t PQHDRConfig::getDriverDebugFlag(void)
{
    PQConfig* pPQConfig = PQConfig::getInstance();

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return 0;
    }

    return pPQConfig->getAshmemContext(HDR_DRIVER_DEBUG_FLAG);
}
