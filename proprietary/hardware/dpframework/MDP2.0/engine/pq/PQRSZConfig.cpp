#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1
#include <cutils/properties.h>
#include <cutils/log.h>
#include <PQCommon.h>
#include "PQServiceCommon.h"
#include "PQRSZConfig.h"
#ifdef SUPPORT_PQ_WHITE_LIST
#include "PQWhiteList.h"
#endif
PQRSZConfig::PQRSZConfig()
{
    PQ_LOGI("[PQRSZConfig] PQRSZConfig()... ");

};

PQRSZConfig::~PQRSZConfig()
{
    PQ_LOGI("[PQRSZConfig] ~PQRSZConfig()... ");
};

int32_t PQRSZConfig::isUREnabled(void)
{
    int32_t enable;
    PQConfig* pPQConfig = PQConfig::getInstance();

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return false;
    }

    enable =  (pPQConfig->getAshmemContext(UR_ENABLE));

#ifdef SUPPORT_PQ_WHITE_LIST
    PQWhiteList &whiteList = PQWhiteList::getInstance();
    enable = whiteList.isPQEnabledOnActive() ? enable : 0;
#endif

    PQ_LOGI("[PQRSZConfig] PQRSZConfig::isUREnabled = %d \n", enable);

    return enable;
}


bool PQRSZConfig::getRSZConfig(RSZ_CONFIG_T* RSZConfig)
{
    bool status = true;

    RSZConfig->UR_ENABLE = isUREnabled();
    RSZConfig->ispTuningFlag = getIspTuningFlag();

    return status;
}

uint32_t PQRSZConfig::getIspTuningFlag(void)
{
    PQConfig* pPQConfig = PQConfig::getInstance();

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return 0;
    }

    return pPQConfig->getAshmemContext(MDP_CZ_ISPTUNING_FLAG);
}