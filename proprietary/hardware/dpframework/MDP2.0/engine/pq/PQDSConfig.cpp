#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1
#include <cutils/properties.h>
#include <cutils/log.h>
#include <PQCommon.h>
#include "PQServiceCommon.h"
#include "PQDSConfig.h"
#include "PQTimer.h"
#ifdef SUPPORT_PQ_WHITE_LIST
#include "PQWhiteList.h"
#endif
#include <vendor/mediatek/hardware/pq/2.0/IPictureQuality.h>

using vendor::mediatek::hardware::pq::V2_0::IPictureQuality;
using vendor::mediatek::hardware::pq::V2_0::dispPQIndexParams;
using vendor::mediatek::hardware::pq::V2_0::Result;

bool PQDSConfig::m_TDSHPTableInit = false;
DISPLAY_TDSHP_T PQDSConfig::m_TDSHPTable = {};

PQDSConfig::PQDSConfig()
        : m_DSParameterInit(false)
{
    PQ_LOGI("[PQDSConfig] PQDSConfig()... ");

    memset(&m_TDSHPTableIndex, 0, sizeof(DISP_PQ_PARAM));
    memset(&m_DSConfig, 0, sizeof(DS_CONFIG_T));
    initTDSHPTable();

    m_PQParameterSN = 0;
};

PQDSConfig::~PQDSConfig()
{
    PQ_LOGI("[PQDSConfig] ~PQDSConfig()... ");
};


bool PQDSConfig::initTDSHPTable(void)
{
    DISPLAY_TDSHP_T  *tdshp_table_ptr;
    if (m_TDSHPTableInit == false)
    {
        PQConfig* pPQConfig = PQConfig::getInstance();
        int32_t offset = 0;
        int32_t size = 0;
        int32_t isNoError = 0;
        /* get register value from ashmem */
        size = sizeof(DISPLAY_TDSHP_T) / sizeof(unsigned int);
        isNoError = pPQConfig->getAshmemArray(PROXY_TDSHP_CUST, offset, &m_TDSHPTable, size);
        if (isNoError != 1)
        {
            PQ_LOGE("[PQDSConfig] can't load TDSHP Table from ashmem\n");
            return m_TDSHPTableInit;
        }
    }

    m_TDSHPTableInit = true;

    return m_TDSHPTableInit;
}

bool PQDSConfig::composeDSParameter(int32_t scenario, GlobalPQParam &globalPQParam)
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    bool updateFlag = false;
    uint32_t PQSerial;

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        PQ_LOGE("[PQDSConfig] Bypass composeDSParameter");
        return false;
    }

    PQSerial = pPQConfig->getAshmemContext(PQ_PARAM_SN);

    if (PQSerial != m_PQParameterSN)
    {
        updateFlag = true;
    }

    if (m_DSParameterInit == false || updateFlag == true || globalPQParam.globalPQSupport != 0)
    {
        sp<IPictureQuality> service = IPictureQuality::getService();
        if (service == nullptr) {
            ALOGD("[PQDSConfig] failed to get HW service");
            return false;
        }

        android::hardware::Return<void> ret = service->getMappedColorIndex(scenario, 0,
            [&] (Result retval, const dispPQIndexParams &index) {
            if (retval == Result::OK) {
                m_TDSHPTableIndex.u4SHPGain = index.u4SHPGain;
            }
        });
        if (!ret.isOk()){
            PQ_LOGE("Transaction error in IPictureQuality::getMappedColorIndex");
        }

        PQ_LOGI("[PQDSConfig] u4SHPGain =  %d\n", m_TDSHPTableIndex.u4SHPGain);

        if (globalPQParam.globalPQSupport != 0)
        {
            if (globalPQParam.globalPQType == GLOBAL_PQ_VIDEO)
                m_TDSHPTableIndex.u4SHPGain = globalPQParam.globalPQStrength & 0xff;
            else
                m_TDSHPTableIndex.u4SHPGain = (globalPQParam.globalPQStrength & 0xff0000) >> 16;
        }

        if( m_TDSHPTableIndex.u4SHPGain>= THSHP_TUNING_INDEX)
        {
            PQ_LOGE("[PQDSConfig] Tuning index range error !\n");
            return false;
        }

        PQ_LOGI("[PQDSConfig] composeDSParameter: u4SHPGain =  %d\n", m_TDSHPTableIndex.u4SHPGain);

        if (sizeof(m_DSConfig.TDSHP) == sizeof(m_TDSHPTable.entry[0]))
        {
            memcpy(m_DSConfig.TDSHP, &(m_TDSHPTable.entry[m_TDSHPTableIndex.u4SHPGain]), sizeof(m_DSConfig.TDSHP));
        }
        else
        {
            PQ_LOGE("[PQDSConfig] composeDSParameter: Parameter size does not match (%d, %d) ",
            sizeof(m_DSConfig.TDSHP), sizeof(m_TDSHPTable.entry[0]));

            return false;
        }

        m_PQParameterSN = PQSerial;
        m_DSParameterInit = true;
    }

    return true;
}

int32_t PQDSConfig::isEnabled(GlobalPQParam &globalPQParam)
{
    PQConfig* pPQConfig = PQConfig::getInstance();

    PQTimeValue    time_s;
    PQTimeValue    time_e;
    int32_t        time_d;

    PQ_TIMER_GET_CURRENT_TIME(time_s);


    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return false;
    }

    PQ_TIMER_GET_CURRENT_TIME(time_e);
    PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d);
    PQ_LOGI("PQDSConfig::isEnabled check1  time %d ms\n", time_d);

    PQ_TIMER_GET_CURRENT_TIME(time_s);

    int32_t enable;

    if (globalPQParam.globalPQSupport != 0)
    {
        enable = globalPQParam.globalPQSwitch & 0x1;
    }
    else
    {
        enable = pPQConfig->getAshmemContext(SHP_ENABLE);
    }
    enable = m_TDSHPTableInit ? enable : 0;

#ifdef SUPPORT_PQ_WHITE_LIST
    //check if WhiteListServer is ready on N0
    PQWhiteList &whiteList = PQWhiteList::getInstance();
    enable = whiteList.isPQEnabledOnActive() ? enable : 0;
    PQ_LOGI("[PQDSConfig] whiteList.isPQEnabledOnActive() = %d \n", whiteList.isPQEnabledOnActive());
#endif

    PQ_LOGI("[PQDSConfig] PQDSConfig::isEnabled = %d \n", enable);
    PQ_LOGI("[PQDSConfig] m_TDSHPTableInit %d\n", m_TDSHPTableInit);
    PQ_LOGI("[PQDSConfig] getAshmemContext(SHP_ENABLE) = %d \n", pPQConfig->getAshmemContext(SHP_ENABLE));

    PQ_TIMER_GET_CURRENT_TIME(time_e);
    PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d);
    PQ_LOGI("PQDSConfig::isEnabled check2  time %d ms\n", time_d);

    return enable;
}

int32_t PQDSConfig::isDShpEnabled(void)
{
    int32_t enable;
    PQConfig* pPQConfig = PQConfig::getInstance();

    enable = pPQConfig->getAshmemContext(DSHP_ENABLE);

    PQ_LOGI("[PQDSConfig] PQDSConfig::isDSHPEnabled = %d \n",enable);

    return enable;
}

int32_t PQDSConfig::isISOShpEnabled(void)
{
    int32_t enable;
    PQConfig* pPQConfig = PQConfig::getInstance();

    enable = pPQConfig->getAshmemContext(ISO_SHP_ENABLE);

    PQ_LOGI("[PQDSConfig] PQDSConfig::isISOShpEnabled = %d \n",enable);

    return enable;
}

int32_t PQDSConfig::isUREnabled(void)
{
    int32_t enable;
    PQConfig* pPQConfig = PQConfig::getInstance();

    enable = pPQConfig->getAshmemContext(UR_ENABLE);

    PQ_LOGI("[PQDSConfig] PQDSConfig::isUREnabled = %d \n",enable);

    return enable;
}

bool PQDSConfig::getDSConfig(DS_CONFIG_T** DSConfig, int32_t scenario, GlobalPQParam &globalPQParam)
{
    bool status = true;

    PQTimeValue    time_s;
    PQTimeValue    time_e;
    int32_t        time_d;

    PQ_TIMER_GET_CURRENT_TIME(time_s);

    status &= initTDSHPTable();
    PQ_TIMER_GET_CURRENT_TIME(time_e);
    PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d);
    PQ_LOGI("[PQDSConfig] initTDSHPTable, time %d ms\n", time_d);

    PQ_LOGI("[PQDSConfig] getDSConfig: m_DSParameterInit =  %d\n", m_DSParameterInit);

    PQ_TIMER_GET_CURRENT_TIME(time_s);

    status &= composeDSParameter(scenario, globalPQParam);

    PQ_TIMER_GET_CURRENT_TIME(time_e);
    PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d);
    PQ_LOGI("[PQDSConfig] composeDSParameter, time %d ms\n", time_d);

    PQ_TIMER_GET_CURRENT_TIME(time_s);
    m_DSConfig.ENABLE = isEnabled(globalPQParam);
    m_DSConfig.DS_ENABLE = isDShpEnabled();
    m_DSConfig.ISO_ENABLE = isISOShpEnabled();
    m_DSConfig.UR_ENABLE = isUREnabled();

    PQ_TIMER_GET_CURRENT_TIME(time_e);
    PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d);
    PQ_LOGI("[PQDSConfig] check enable config, time %d ms\n", time_d);

    m_DSConfig.ispTuningFlag = getIspTuningFlag();

    *DSConfig = &m_DSConfig;

    return status;
}

uint32_t PQDSConfig::getIspTuningFlag(void)
{
    PQConfig* pPQConfig = PQConfig::getInstance();

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return 0;
    }

    return pPQConfig->getAshmemContext(MDP_CZ_ISPTUNING_FLAG);
}
