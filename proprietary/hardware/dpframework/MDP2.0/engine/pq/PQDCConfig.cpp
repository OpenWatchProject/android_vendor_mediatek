#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1
#include <cutils/properties.h>
#include <cutils/log.h>
#include <PQCommon.h>
#include "PQServiceCommon.h"
#include "PQDCConfig.h"
#ifdef SUPPORT_PQ_WHITE_LIST
#include "PQWhiteList.h"
#endif

PQDCConfig::PQDCConfig()
{
    PQ_LOGI("[PQDCConfig] PQDCConfig()... ");

};

PQDCConfig::~PQDCConfig()
{
    PQ_LOGI("[PQDCConfig] ~PQDCConfig()... ");
};

bool PQDCConfig::isEnabled(GlobalPQParam &globalPQParam)
{
    PQConfig* pPQConfig = PQConfig::getInstance();

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return false;
    }

    bool enable;

    if (globalPQParam.globalPQSupport != 0)
    {
        enable = (globalPQParam.globalPQSwitch >> 1) & 0x1;
    }
    else
    {
        enable =  (pPQConfig->getAshmemContext(DC_ENABLE));
    }

#ifdef SUPPORT_PQ_WHITE_LIST
    //check if WhiteListServer is ready on N0
    PQWhiteList &whiteList = PQWhiteList::getInstance();
    enable &= whiteList.isPQEnabledOnActive();
    PQ_LOGI("[PQDCConfig] whiteList.isPQEnabledOnActive() = %d \n", whiteList.isPQEnabledOnActive());
#endif

    PQ_LOGI("[PQDCConfig] PQDCConfig::isEnabled = %d \n", enable);
    PQ_LOGI("[PQDCConfig] getAshmemContext(DC_ENABLE) = %d \n", pPQConfig->getAshmemContext(DC_ENABLE));

    return enable;
}

bool PQDCConfig::getDCConfig(DC_CONFIG_T* DCConfig, GlobalPQParam &globalPQParam)
{
    bool status = true;

    DCConfig->ENABLE = isEnabled(globalPQParam);
    DCConfig->debugFlag = getDebugFlag();

    return status;
}

uint32_t PQDCConfig::getDebugFlag(void)
{
    PQConfig* pPQConfig = PQConfig::getInstance();

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return 0;
    }

    return pPQConfig->getAshmemContext(DC_DEBUG_FLAG);
}
