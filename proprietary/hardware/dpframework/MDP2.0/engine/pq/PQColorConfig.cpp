#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1
#include <cutils/properties.h>
#include <cutils/log.h>
#include <PQCommon.h>
#include "PQServiceCommon.h"
#include "PQColorConfig.h"
#ifdef SUPPORT_PQ_WHITE_LIST
#include "PQWhiteList.h"
#endif
#include <vendor/mediatek/hardware/pq/2.0/IPictureQuality.h>

using vendor::mediatek::hardware::pq::V2_0::IPictureQuality;
using vendor::mediatek::hardware::pq::V2_0::dispPQIndexParams;
using vendor::mediatek::hardware::pq::V2_0::Result;

bool PQColorConfig::m_ColorTableInit = false;
DISPLAY_PQ_T PQColorConfig::m_ColorTable = {};

PQColorConfig::PQColorConfig()
    : m_ColorParameterInit(false)
{
    PQ_LOGI("[PQColorConfig] PQColorConfig()... ");

    //memset(&m_ColorTable, 0, sizeof(DISPLAY_PQ_T));
    memset(&m_ColorTableIndex, 0, sizeof(DISP_PQ_PARAM));
    memset(&m_ColorConfig, 0, sizeof(COLOR_CONFIG_T));
#ifdef CONFIG_FOR_SOURCE_PQ
    memset(&m_DispStatus, 0, sizeof(m_DispStatus) );
    m_ColorEXEnable = false;
#endif
    char value[PROPERTY_VALUE_MAX];
    property_get(PQ_COLOR_MODE_STR, value, PQ_COLOR_MODE_DEFAULT);
    m_ColorMode = atoi(value);

    initColorTable();

    m_PQParameterSN = 0;

    PQ_LOGI("[PQColorConfig] PQColorConfig(), m_ColorTableInit = %d ", m_ColorTableInit);
};

PQColorConfig::~PQColorConfig()
{
    PQ_LOGI("[PQColorConfig] ~PQColorConfig()... ");
};

bool PQColorConfig::initColorTable(void)
{
    DISPLAY_PQ_T     *mdp_color_ptr;

    if (m_ColorTableInit == false)
    {
        if (isMDPColorMode())
        {
            PQConfig* pPQConfig = PQConfig::getInstance();
            int32_t offset = 0;
            int32_t size = 0;
            int32_t isNoError = 0;
            /* get register value from ashmem */
            size = sizeof(DISPLAY_PQ_T) / sizeof(unsigned int);
            isNoError = pPQConfig->getAshmemArray(PROXY_COLOR_CUST, offset, &m_ColorTable, size);
            if (isNoError != 1)
            {
                PQ_LOGE("[PQColorConfig] can't load primary_pqindex from ashmem\n");
                return m_ColorTableInit;
            }
        }
    }

    m_ColorTableInit = true;

    return m_ColorTableInit;
}

bool PQColorConfig::isEnabled(int32_t scenario)
{
    PQConfig* pPQConfig = PQConfig::getInstance();

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        return false;
    }

    bool enable = true;

    enable &= isMDPColorMode();
    enable &= m_ColorTableInit;

    enable &= pPQConfig->getAshmemContext(CONTENT_COLOR_ENABLE);

    if (scenario == MEDIA_VIDEO || scenario == MEDIA_VIDEO_CODEC)
    {
        enable &= pPQConfig->getAshmemContext(VIDEO_CONTENT_COLOR_ENABLE);
    }
    else if (scenario == MEDIA_ISP_CAPTURE)
    {
        enable &= false;
    }

#ifdef SUPPORT_PQ_WHITE_LIST
    //check if WhiteListServer is ready on N0
    PQWhiteList &whiteList = PQWhiteList::getInstance();
    enable &= whiteList.isPQEnabledOnActive();
    PQ_LOGI("[PQColorConfig] whiteList.isPQEnabledOnActive() = %d \n", whiteList.isPQEnabledOnActive());
#endif

    PQ_LOGI("[PQColorConfig] PQColorConfig::isEnabled = %d \n", enable);
    PQ_LOGI("[PQColorConfig] m_ColorTableInit %d\n", m_ColorTableInit);
    PQ_LOGI("[PQColorConfig] getAshmemContext(CONTENT_COLOR_ENABLE) = %d \n", pPQConfig->getAshmemContext(CONTENT_COLOR_ENABLE));
    PQ_LOGI("[PQColorConfig] getAshmemContext(VIDEO_CONTENT_COLOR_ENABLE) = %d \n", pPQConfig->getAshmemContext(VIDEO_CONTENT_COLOR_ENABLE));

    return enable;
}

bool PQColorConfig::isMDPColorMode(void)
{
    PQ_LOGI("[PQColorConfig] m_ColorMode = %d \n",m_ColorMode);

    if (m_ColorMode == MDP_COLOR || m_ColorMode == DISP_MDP_COLOR)
    {

        return true;
    }
    else
    {
        return false;
    }
}

bool PQColorConfig::composeColorParameter(int32_t scenario)
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    bool updateFlag = false;
    uint32_t PQSerial;

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        PQ_LOGE("[PQDSConfig] Bypass composeColorParameter");
        return false;
    }

    PQSerial = pPQConfig->getAshmemContext(PQ_PARAM_SN);

    if (PQSerial != m_PQParameterSN)
    {
        updateFlag = true;
    }

    if (m_ColorParameterInit == false || updateFlag == true)
    {
        sp<IPictureQuality> service = IPictureQuality::getService();
        if (service == nullptr) {
            ALOGD("[PQColorConfig] failed to get HW service");
            return false;
        }

        android::hardware::Return<void> ret = service->getMappedColorIndex(scenario, 0,
            [&] (Result retval, const dispPQIndexParams &index) {
            if (retval == Result::OK) {
                m_ColorTableIndex.u4SatGain = index.u4SatGain;
                m_ColorTableIndex.u4PartialY = index.u4PartialY;
                memcpy(&(m_ColorTableIndex.u4HueAdj[0]), &(index.u4HueAdj[0]), sizeof(m_ColorTableIndex.u4HueAdj));
                memcpy(&(m_ColorTableIndex.u4SatAdj[0]), &(index.u4SatAdj[0]), sizeof(m_ColorTableIndex.u4SatAdj));
                m_ColorTableIndex.u4Contrast = index.u4Contrast;
                m_ColorTableIndex.u4Brightness = index.u4Brightness;
#ifdef COLOR_3_0
                m_ColorTableIndex.u4ColorLUT = index.u4ColorLUT;
#endif
            }
        });
        if (!ret.isOk()){
            PQ_LOGE("Transaction error in IPictureQuality::getMappedColorIndex");
        }

        if (m_ColorTableIndex.u4Brightness >= BRIGHTNESS_SIZE ||
            m_ColorTableIndex.u4Contrast >= CONTRAST_SIZE ||
            m_ColorTableIndex.u4SatGain >= GLOBAL_SAT_SIZE ||
            m_ColorTableIndex.u4HueAdj[PURP_TONE] >= COLOR_TUNING_INDEX ||
            m_ColorTableIndex.u4HueAdj[SKIN_TONE] >= COLOR_TUNING_INDEX ||
            m_ColorTableIndex.u4HueAdj[GRASS_TONE] >= COLOR_TUNING_INDEX||
            m_ColorTableIndex.u4HueAdj[SKY_TONE] >= COLOR_TUNING_INDEX ||
            m_ColorTableIndex.u4SatAdj[PURP_TONE] >= COLOR_TUNING_INDEX ||
            m_ColorTableIndex.u4SatAdj[SKIN_TONE] >= COLOR_TUNING_INDEX ||
            m_ColorTableIndex.u4SatAdj[GRASS_TONE] >= COLOR_TUNING_INDEX||
            m_ColorTableIndex.u4SatAdj[SKY_TONE] >= COLOR_TUNING_INDEX )
        {
            PQ_LOGE("[PQColorConfig] Tuning index range error !\n");
            return false;
        }

        PQ_LOGI("[PQColorConfig] composeColorParameter: u4SatGain =  %d, u4Contrast = %d, u4Brightness = %d\n",m_ColorTableIndex.u4SatGain, m_ColorTableIndex.u4Contrast, m_ColorTableIndex.u4Brightness);
        PQ_LOGI("[PQColorConfig] composeColorParameter: u4HueAdj[PURP_TONE] =  %d, u4HueAdj[SKIN_TONE] = %d, u4HueAdj[GRASS_TONE]  = %d, u4HueAdj[SKY_TONE] = %d\n",m_ColorTableIndex.u4HueAdj[PURP_TONE],m_ColorTableIndex.u4HueAdj[SKIN_TONE],m_ColorTableIndex.u4HueAdj[GRASS_TONE],m_ColorTableIndex.u4HueAdj[SKY_TONE]);
        PQ_LOGI("[PQColorConfig] composeColorParameter: u4SatAdj[PURP_TONE] =  %d, u4SatAdj[SKIN_TONE] = %d, u4SatAdj[GRASS_TONE]  = %d, u4SatAdj[SKY_TONE] = %d\n",m_ColorTableIndex.u4SatAdj[PURP_TONE],m_ColorTableIndex.u4SatAdj[SKIN_TONE],m_ColorTableIndex.u4SatAdj[GRASS_TONE],m_ColorTableIndex.u4SatAdj[SKY_TONE]);

        m_ColorConfig.GLOBAL_SAT = m_ColorTable.GLOBAL_SAT[m_ColorTableIndex.u4SatGain];
        m_ColorConfig.CONTRAST = m_ColorTable.CONTRAST[m_ColorTableIndex.u4Contrast];
        m_ColorConfig.BRIGHTNESS = m_ColorTable.BRIGHTNESS[m_ColorTableIndex.u4Brightness];
#ifdef COLOR_2_1
        m_ColorConfig.LSP_SAT_LIMIT = 1;
        m_ColorConfig.S_GAIN_BY_Y_EN = m_ColorTable.S_GAIN_BY_Y_EN;
        m_ColorConfig.LSP_EN = m_ColorTable.LSP_EN;
        m_ColorConfig.TWO_D_WINDOW = 0x40185E57;
#else
        m_ColorConfig.LSP_SAT_LIMIT = 0;
        m_ColorConfig.S_GAIN_BY_Y_EN = 0;
        m_ColorConfig.LSP_EN = 0;
        m_ColorConfig.TWO_D_WINDOW = 0x40106051;
#endif
        m_ColorConfig.BYPASS = 0;
        m_ColorConfig.DBG_CFG_MAIN = 0;

        if(scenario == MEDIA_VIDEO || scenario == MEDIA_VIDEO_CODEC)
        {
            m_ColorConfig.Y_LEV_ADJ = 0x0;
            m_ColorConfig.CBOOST_YOFFSET = 0x40;
        }
        else
        {
            m_ColorConfig.Y_LEV_ADJ = 0x40;
            m_ColorConfig.CBOOST_YOFFSET = 0x0;
        }
        m_ColorConfig.NEW_CBOOST_LMT_L = 0x80;
        m_ColorConfig.CBOOST_YCONST = 0x40;
#ifdef COLOR_2_1
        m_ColorConfig.LSP_1 = (m_ColorTable.LSP[3] << 0) | (m_ColorTable.LSP[2] << 7) | (m_ColorTable.LSP[1] << 14) | (m_ColorTable.LSP[0] << 22);
        m_ColorConfig.LSP_2 = (m_ColorTable.LSP[7] << 0) | (m_ColorTable.LSP[6] << 8) | (m_ColorTable.LSP[5] << 16) | (m_ColorTable.LSP[4] << 23);
#endif

        m_ColorConfig.COLOR_CM_CONTROL = 0x0 | (0x3 << 1) | (0x3 << 4) | (0x3 << 7);

        if (sizeof(m_ColorConfig.PARTIAL_Y) == sizeof(m_ColorTable.PARTIAL_Y[0]) &&
            sizeof(m_ColorConfig.PURP_TONE_S) == sizeof(m_ColorTable.PURP_TONE_S[0]) &&
            sizeof(m_ColorConfig.SKIN_TONE_S) == sizeof(m_ColorTable.SKIN_TONE_S[0]) &&
            sizeof(m_ColorConfig.GRASS_TONE_S) == sizeof(m_ColorTable.GRASS_TONE_S[0]) &&
            sizeof(m_ColorConfig.SKY_TONE_S) == sizeof(m_ColorTable.SKY_TONE_S[0]) &&
            sizeof(m_ColorConfig.PURP_TONE_H) == sizeof(m_ColorTable.PURP_TONE_H[0]) &&
            sizeof(m_ColorConfig.SKIN_TONE_H) == sizeof(m_ColorTable.SKIN_TONE_H[0]) &&
            sizeof(m_ColorConfig.GRASS_TONE_H) == sizeof(m_ColorTable.GRASS_TONE_H[0]) &&
            sizeof(m_ColorConfig.SKY_TONE_H) == sizeof(m_ColorTable.SKY_TONE_H[0])
#ifdef COLOR_2_1
            && sizeof(m_ColorConfig.S_GAIN_BY_Y) == sizeof(m_ColorTable.S_GAIN_BY_Y)
#endif
#ifdef COLOR_3_0
            && sizeof(m_ColorConfig.COLOR_3D) == sizeof(m_ColorTable.COLOR_3D[0])
#endif
            )
        {
            memcpy(&m_ColorConfig.PARTIAL_Y, &(m_ColorTable.PARTIAL_Y[m_ColorTableIndex.u4PartialY]), sizeof(m_ColorConfig.PARTIAL_Y));
            memcpy(&m_ColorConfig.PURP_TONE_S, &(m_ColorTable.PURP_TONE_S[m_ColorTableIndex.u4SatAdj[PURP_TONE]]), sizeof(m_ColorConfig.PURP_TONE_S));
            memcpy(&m_ColorConfig.SKIN_TONE_S, &(m_ColorTable.SKIN_TONE_S[m_ColorTableIndex.u4SatAdj[SKIN_TONE]]), sizeof(m_ColorConfig.SKIN_TONE_S));
            memcpy(&m_ColorConfig.GRASS_TONE_S, &(m_ColorTable.GRASS_TONE_S[m_ColorTableIndex.u4SatAdj[GRASS_TONE]]), sizeof(m_ColorConfig.GRASS_TONE_S));
            memcpy(&m_ColorConfig.SKY_TONE_S, &(m_ColorTable.SKY_TONE_S[m_ColorTableIndex.u4SatAdj[SKY_TONE]]), sizeof(m_ColorConfig.SKY_TONE_S));
            memcpy(&m_ColorConfig.PURP_TONE_H, &(m_ColorTable.PURP_TONE_H[m_ColorTableIndex.u4HueAdj[PURP_TONE]]), sizeof(m_ColorConfig.PURP_TONE_H));
            memcpy(&m_ColorConfig.SKIN_TONE_H, &(m_ColorTable.SKIN_TONE_H[m_ColorTableIndex.u4HueAdj[SKIN_TONE]]), sizeof(m_ColorConfig.SKIN_TONE_H));
            memcpy(&m_ColorConfig.GRASS_TONE_H, &(m_ColorTable.GRASS_TONE_H[m_ColorTableIndex.u4HueAdj[GRASS_TONE]]), sizeof(m_ColorConfig.GRASS_TONE_H));
            memcpy(&m_ColorConfig.SKY_TONE_H, &(m_ColorTable.SKY_TONE_H[m_ColorTableIndex.u4HueAdj[SKY_TONE]]), sizeof(m_ColorConfig.SKY_TONE_H));
#ifdef COLOR_2_1
            memcpy(&m_ColorConfig.S_GAIN_BY_Y, &(m_ColorTable.S_GAIN_BY_Y), sizeof(m_ColorConfig.S_GAIN_BY_Y));
#endif
#ifdef COLOR_3_0
            memcpy(&m_ColorConfig.COLOR_3D, &(m_ColorTable.COLOR_3D[m_ColorTableIndex.u4ColorLUT]), sizeof(m_ColorConfig.COLOR_3D));
#endif
        } else {
            PQ_LOGE("[PQColorConfig] composeColorRegisters: Parameter size does not match (%d, %d) (%d, %d) (%d, %d)",
                sizeof(m_ColorConfig.PARTIAL_Y), sizeof(m_ColorTable.PARTIAL_Y[0]),
                sizeof(m_ColorConfig.PURP_TONE_S), sizeof(m_ColorTable.PURP_TONE_S[0]),
                sizeof(m_ColorConfig.SKIN_TONE_S), sizeof(m_ColorTable.SKIN_TONE_S[0]));
            PQ_LOGE("[PQColorConfig] composeColorRegisters: (%d, %d) (%d, %d) (%d, %d) (%d, %d) (%d, %d) (%d, %d) ",
                sizeof(m_ColorConfig.GRASS_TONE_S), sizeof(m_ColorTable.GRASS_TONE_S[0]),
                sizeof(m_ColorConfig.SKY_TONE_S), sizeof(m_ColorTable.SKY_TONE_S[0]),
                sizeof(m_ColorConfig.PURP_TONE_H), sizeof(m_ColorTable.PURP_TONE_H[0]),
                sizeof(m_ColorConfig.SKIN_TONE_H), sizeof(m_ColorTable.SKIN_TONE_H[0]),
                sizeof(m_ColorConfig.GRASS_TONE_H), sizeof(m_ColorTable.GRASS_TONE_H[0]),
                sizeof(m_ColorConfig.SKY_TONE_H), sizeof(m_ColorTable.SKY_TONE_H[0]));
#ifdef COLOR_2_1
            PQ_LOGE("[PQColorConfig] composeColorRegisters: (%d, %d)",
                sizeof(m_ColorConfig.S_GAIN_BY_Y), sizeof(m_ColorTable.S_GAIN_BY_Y[0]));
#endif
#ifdef COLOR_3_0
            PQ_LOGE("[PQColorConfig] composeColorRegisters: (%d, %d)",
                sizeof(m_ColorConfig.COLOR_3D), sizeof(m_ColorTable.COLOR_3D[0]));
#endif
            return false;
        }

        m_PQParameterSN = PQSerial;
        m_ColorParameterInit = true;
    }

    return true;
}

bool PQColorConfig::getColorConfig(COLOR_CONFIG_T** colorConfig, int32_t scenario)
{
    bool status = true;

    status &= initColorTable();

    status &= composeColorParameter(scenario);

    m_ColorConfig.ENABLE = isEnabled(scenario);

    if (status == false){
        PQ_LOGI("[PQColorConfig] getColorConfig failed\n");
        m_ColorConfig.BYPASS = 1;
    }
    else if (m_ColorConfig.ENABLE == false)
    {
        PQ_LOGI("[PQColorAdaptor] m_ColorConfig.ENABLE == false\n");
        m_ColorConfig.BYPASS = 1;
    }
    else
    {
        m_ColorConfig.BYPASS = 0;
    }

    *colorConfig = &m_ColorConfig;

    return status;
}

#ifdef CONFIG_FOR_SOURCE_PQ

void PQColorConfig::setColorEXEnable(bool enable)
{
    m_ColorEXEnable = enable;
}
bool PQColorConfig::getColorEXEnable(void)
{
    return m_ColorEXEnable;
}
void PQColorConfig::getDisplayStatus(DISP_PQ_STATUS &status, bool bPerFrame)
{
//    AutoMutex lock(s_ALMutex);

    if (bPerFrame)
    {
        getDisplayStatusEx(m_DispStatus);
    }

    status = m_DispStatus;
}

void PQColorConfig::getDisplayStatusEx(DISP_PQ_STATUS &status)
{
    int           drvID = -1;
    int32_t       ret;

    drvID = open("/proc/mtk_mira", O_RDONLY, 0);
    if (-1 == drvID)
    {
        DPLOGE("[PQColorConfig] can't open display driver\n");
        assert(0);
        return;
    }

    ret = ioctl(drvID, DISP_IOCTL_PQ_GET_DISP_STATUS, &status);
    if (0 != ret)
    {
        DPLOGE("[PQColorConfig] get Display Status fail(%d) %s\n", ret, strerror(errno));
        close(drvID);
        return;
    }

    close(drvID);
}
#endif
