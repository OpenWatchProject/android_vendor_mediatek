#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1
#include <cutils/properties.h>
#include <cutils/log.h>
#include <fcntl.h>

#include <PQCommon.h>
#include "PQServiceCommon.h"
#include <PQDCAdaptor.h>
#include "PQTuningBuffer.h"

#include "ui/gralloc_extra.h"

#include "mdp_reg_tdshp.h"
#include <vendor/mediatek/hardware/pq/2.0/IPictureQuality.h>
#define ARR_LEN_4BYTE(arr) (sizeof(arr) / 4)
#define DC_CAL_REGS_THRES 2 //ms

#define DC_APPLY_CMD_TIMEOUT_MS       (100)
#define MILLI_TO_NANO(v) (static_cast<nsecs_t>(v) * static_cast<nsecs_t>(1000L * 1000L))

using vendor::mediatek::hardware::pq::V2_0::IPictureQuality;
using vendor::mediatek::hardware::pq::V2_0::Result;
using vendor::mediatek::hardware::pq::V2_0::globalPQIndex_t;

PQDCAdaptor* PQDCAdaptor::s_pInstance[] = {};
PQMutex   PQDCAdaptor::s_ALMutex;

static const ADLOutput defaultADLSetting =
{
    LumaCurve:{0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512},
    dync_cboost_yoffset:0,
    dync_bypass:0,
    new_boost_lmt_l:0x80,
#if DYN_CONTRAST_VERSION == 1
    dync_cboost_yoffset_sel:0x2,
    dync_cboost_yconst:0x20,
    tdshp_cboost_yoffset:0,
    tdshp_cboost_yoffset_sel:0x2,
    tdshp_cboost_yconst:0x10,
#endif
#if DYN_CONTRAST_VERSION == 2
    cboost_gain:0x80,
    contour_eds_slope:0x08,
    contour_eds_th:0x08,
    diffcount_slope:0x10,
    diffcount_th:0x0,
    noise_eds_slope:0x2F,
    noise_eds_th:0x00,
    signchange_slope:0x2F,
    signchange_th:0x1F,
    skin_conf_en:0,
    w1_skin_bright_wgt:0x10,
    w1_skin_dark_wgt:0x3F,
    dync_cboost_yoffset_sel:0x2,
    dync_cboost_yconst:0x20,
#endif
};

PQDCAdaptor* PQDCAdaptor::getInstance(uint32_t identifier)
{
    AutoMutex lock(s_ALMutex);

    if(NULL == s_pInstance[identifier])
    {
        s_pInstance[identifier] = new PQDCAdaptor(identifier);
        atexit(PQDCAdaptor::destroyInstance);
    }

    return s_pInstance[identifier];
}

void PQDCAdaptor::destroyInstance()
{
    AutoMutex lock(s_ALMutex);

    for (int identifier = 0; identifier < DC_ENGINE_MAX_NUM; identifier++){
        if (NULL != s_pInstance[identifier])
        {
            delete s_pInstance[identifier];
            s_pInstance[identifier] = NULL;
        }
    }
}

PQDCAdaptor::PQDCAdaptor(uint32_t identifier)
        : m_identifier(identifier),
          m_isSWregBufferInit(false),
          m_debugFlag(0),
          PQAlgorithmAdaptor(PROXY_DC_SWREG,
                             PROXY_DC_INPUT,
                             PROXY_DC_OUTPUT)
{
    PQ_LOGD("[PQDCAdaptor] PQDCAdaptor()... ");

    ADLInitParam initParam;
    ADLInitReg initReg;

    m_pADLFW = new CPQDCFW;
    getDCTable();
    memcpy(m_pADLFW->pNormalModeReg, m_pADLFW->pADLReg, sizeof(ADLReg));
};

PQDCAdaptor::~PQDCAdaptor()
{
    PQ_LOGD("[PQDCAdaptor] ~PQDCAdaptor()... ");

    delete m_pADLFW;
};

bool PQDCAdaptor::calRegs(PQSession* pPQSession, DpCommand &command, DpConfig &config, int32_t *pFrameConfigLabel, bool LumaCurveOnly)
{
    AutoMutex lock(s_ALMutex);

    PQTimeValue    begin;
    PQTimeValue    end;
    int32_t        diff;
    PQTimeValue    time_s;
    PQTimeValue    time_e;
    int32_t        time_d;
    PQ_TIMER_GET_CURRENT_TIME(begin);
    PQ_TIMER_GET_CURRENT_TIME(time_s);

    int32_t adl_enable;

    DpPqConfig* DpPqConfig;
    DpPqParam  PqParam;

    pPQSession->getDpPqConfig(&DpPqConfig);
    pPQSession->getPQParam(&PqParam);

    // check 2 enable paths
    if (DpPqConfig->enDC)
    {
        command.setTDShpPar(PqParam.u.video.id);
        command.setPQSessionID(config.pqSessionId);
    }
    else
    {
        command.setTDShpPar(config.parTDSHP >> 16);
        command.setPQSessionID(config.pqSessionId);
    }

    PQ_TIMER_GET_CURRENT_TIME(time_e);
    PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d);
    PQ_LOGI("[PQDCAdaptor] DC preceding procedure, time %d ms\n", time_d);


    PQ_TIMER_GET_CURRENT_TIME(time_s);

    DC_CONFIG_T dcConfig;
    pPQSession->getDCConfig(&dcConfig);
    adl_enable = dcConfig.ENABLE;
    m_debugFlag = dcConfig.debugFlag;
    PQ_LOGI("[PQDCAdaptor] adl_enable[%d], m_debugFlag = [%d]", adl_enable, m_debugFlag);

    PQ_TIMER_GET_CURRENT_TIME(time_e);
    PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d);
    PQ_LOGI("[PQDCAdaptor] getDCConfig, time %d ms\n", time_d);

    PQ_TIMER_GET_CURRENT_TIME(time_s);

    if ((config.parTDSHP & 0xFFFF0000) ||
         (DpPqConfig->enDC != 0))
    {
        if (adl_enable)
        {
            onCalcADL(pPQSession, command, config, ADL_ENABLE_FLAG_ON, pFrameConfigLabel, LumaCurveOnly);
        }
        else
        {
            onCalcADL(pPQSession, command, config, ADL_ENABLE_FLAG_ON_DISABLE, pFrameConfigLabel, LumaCurveOnly);
        }
        adl_enable = 1;
    }
    else
    {
        adl_enable = 0;
        onCalcADL(pPQSession, command, config, ADL_ENABLE_FLAG_OFF, pFrameConfigLabel, LumaCurveOnly);
    }

    PQ_TIMER_GET_CURRENT_TIME(time_e);
    PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d);
    PQ_LOGI("[PQDCAdaptor] onCalcADL, time %d ms\n", time_d);

    PQ_TIMER_GET_CURRENT_TIME(end);
    PQ_TIMER_GET_DURATION_IN_MS(begin, end, diff);
    if (diff > DC_CAL_REGS_THRES)
    {
        PQ_LOGD("[PQDCAdaptor] calRegs: adl_enable[%d] time %d ms\n", adl_enable, diff);
    }
    else
    {
        PQ_LOGI("[PQDCAdaptor] calRegs: adl_enable[%d] time %d ms\n", adl_enable, diff);
    }

    return adl_enable;
}

void PQDCAdaptor::tuningDCInput(ADLInput *input, int32_t scenario)
{
    PQTuningBuffer *p_buffer = m_inputBuffer;
    unsigned int *overwritten_buffer = p_buffer->getOverWrittenBuffer();
    unsigned int *reading_buffer = p_buffer->getReadingBuffer();
    size_t copy_size = sizeof(ADLInput);

    if (p_buffer->isValid() == false) {
        return;
    }

    if (copy_size > p_buffer->getModuleSize()) {
        copy_size = p_buffer->getModuleSize();
    }

    if (p_buffer->isOverwritten()) {
        if (p_buffer->isSync()) {
            p_buffer->pull();
        }
        memcpy(input, overwritten_buffer, copy_size);
    } else if (scenario == MEDIA_PICTURE) {
        p_buffer->resetReady();
        memcpy(reading_buffer, input, copy_size);
        p_buffer->push();
    } else if (p_buffer->toBePrepared()) {
        memcpy(reading_buffer, input, copy_size);
        p_buffer->push();
    }
}

bool PQDCAdaptor::tuningDCOutput(ADLOutput *output, int32_t scenario)
{
    PQTuningBuffer *p_buffer = m_outputBuffer;
    unsigned int *overwritten_buffer = p_buffer->getOverWrittenBuffer();
    unsigned int *reading_buffer = p_buffer->getReadingBuffer();
    size_t copy_size = sizeof(ADLOutput);

    if (p_buffer->isValid() == false) {
        return false;
    }

    if (copy_size > p_buffer->getModuleSize()) {
        copy_size = p_buffer->getModuleSize();
    }

    if (p_buffer->isOverwritten()) {
        if (p_buffer->isSync()) {
            p_buffer->pull();
        }
        memcpy(output, overwritten_buffer, copy_size);
    } else if (scenario == MEDIA_PICTURE) {
        p_buffer->resetReady();
        memcpy(reading_buffer, output, copy_size);
        p_buffer->push();
    } else if (p_buffer->toBePrepared()) {
        memcpy(reading_buffer, output, copy_size);
        p_buffer->push();
    }

    if (p_buffer->isBypassHWAccess()) {
        return true;
    }

    return false;
}

void PQDCAdaptor::tuningDCSWReg(ADLReg* SWReg, int32_t scenario)
{
    PQTuningBuffer *p_buffer = m_swRegBuffer;
    unsigned int *overwritten_buffer = p_buffer->getOverWrittenBuffer();
    unsigned int *reading_buffer = p_buffer->getReadingBuffer();
    size_t copy_size = sizeof(ADLReg);

    if (p_buffer->isValid() == false) {
        return;
    }

    if (copy_size > p_buffer->getModuleSize()) {
        copy_size = p_buffer->getModuleSize();
    }

    if (m_isSWregBufferInit == false)  //for PQDCTuningAPK
    {
        memcpy(reading_buffer, SWReg, copy_size);
        m_isSWregBufferInit = true;
    }

    if (p_buffer->isOverwritten()) {
        if (p_buffer->isSync()) {
            p_buffer->pull();
        }
        memcpy(SWReg, overwritten_buffer, copy_size);
    } else if (scenario == MEDIA_PICTURE) {
        p_buffer->resetReady();
        memcpy(reading_buffer, SWReg, copy_size);
        p_buffer->push();
    } else if (p_buffer->toBePrepared()) {
        memcpy(reading_buffer, SWReg, copy_size);
        p_buffer->push();
    }
}

bool PQDCAdaptor::getDCTable()
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;
    /* get register value from ashmem */
    for (int index = 0; index < PROXY_DC_CUST_MAX; index++)
    {
        offset += size;
        if (index == PROXY_DC_CUST_ADLREG)
        {
            size = ARR_LEN_4BYTE(ADLReg);
            isNoError = pPQConfig->getAshmemArray(PROXY_DC_CUST, offset, m_pADLFW->pADLReg, size);
        }
        else if (index == PROXY_DC_CUST_HDRREG)
        {
            size = ARR_LEN_4BYTE(ADLReg);
            isNoError = pPQConfig->getAshmemArray(PROXY_DC_CUST, offset, m_pADLFW->pHDRModeReg, size);
        }

        if (isNoError < 0)
        {
            break;
        }
    }
    return (isNoError == 1) ? 1 : 0;
}

void PQDCAdaptor::onCalcADL(PQSession* pPQSession, DpCommand &command, DpConfig &config, int32_t enableFlag, int32_t *pFrameConfigLabel, bool LumaCurveOnly)
{
    DP_TRACE_CALL();

    uint32_t *pHist = 0;
    uint32_t **ppHist = &pHist;
    uint32_t size;
    uint32_t adl_debug = 0;
    int32_t ret;
    uint16_t adl_id = config.parTDSHP >> 16;
    int32_t adl_LmtFullSwitch = 0;

    if ((config.inProfile == DP_PROFILE_JPEG) ||
        (config.inProfile == DP_PROFILE_FULL_BT601))
    {
        adl_LmtFullSwitch = 1;
    }

    PQ_LOGI("[PQDCAdaptor] onCalcADL(), adl_LmtFullSwitch[%d]", adl_LmtFullSwitch);

    DpPqConfig* DpPqConfig;
    DpPqParam  PqParam;

    pPQSession->getDpPqConfig(&DpPqConfig);
    pPQSession->getPQParam(&PqParam);

    // update it with PQ param
    if (DpPqConfig->enDC)
    {
        if (PqParam.scenario == MEDIA_PICTURE)
        {
            adl_id = ADL_ID_IMAGE;
            PQ_LOGD("[PQDCAdaptor] image DC detected.");
        }
        else
        {
            adl_id = PqParam.u.video.id;

        }
    }
    ADLOutput adl_output;
    memcpy(&adl_output, &defaultADLSetting, sizeof(ADLOutput));

    if (isADLEnable(enableFlag, PqParam.scenario) == false)
    {
        adl_output.dync_bypass = 1;
    }
    else
    {
        if (isADLVideoIDValid(adl_id) == false)
        {
            // not a valid video, write default curve.
        }
        else
        {
            if (PqParam.scenario == MEDIA_PICTURE)
            {
                onCalculateImage(pPQSession, adl_LmtFullSwitch, PqParam.u.image, &adl_output);
            }
            else
            {
                if (enableFlag == ADL_ENABLE_FLAG_ON)
                {
                    PQ_LOGI("[PQDCAdaptor] onCalculate(ADL_ENABLE_FLAG_ON)");
                    ret = onCalculate(pPQSession, adl_LmtFullSwitch, config.outWidth, config.outHeight, true, &adl_output);
                }
                else
                {
                    PQ_LOGI("[PQDCAdaptor] onCalculate(ADL_ENABLE_FLAG_ON_DISABLE)");

#if DYN_CONTRAST_VERSION == 2 && !defined(SUPPORT_VIDEO_DRE)
                    if (pPQSession->getDCHandle()->isHDRContent)
                    {
                        PQ_LOGI("[PQDCAdaptor] HDR video, enforce DC on");
                        ret = onCalculate(pPQSession, adl_LmtFullSwitch, config.outWidth, config.outHeight, true, &adl_output);
                    }
                    else
#endif
                    {
                        PQ_LOGI("[PQDCAdaptor] onCalculate will be bypassed");
                        ret = onCalculate(pPQSession, adl_LmtFullSwitch, config.outWidth, config.outHeight, false, &adl_output);
                    }
                }
            }
        }
    }

    int32_t scenario = pPQSession->getScenario();

    bool bypassHWAccess = tuningDCOutput(&adl_output, scenario);

    if (bypassHWAccess)
    {
        return;
    }

    if (LumaCurveOnly)
    {
        ADL_updateLumaCurve(command, adl_output, pFrameConfigLabel);
    }
    else
    {
        ADL_writeLumaCurve(command, adl_output, pFrameConfigLabel);
    }
    // reset LumaHist
    //ADL_resetLumaHist(command);
}

bool PQDCAdaptor::isADLVideoIDValid(int32_t adl_id)
{
    if (adl_id == 0)
    {
        PQ_LOGD("[PQDCAdaptor] video id = 0, use DC default curve!");
        return false;
    }

    return true;
}

bool PQDCAdaptor::isADLEnable(int32_t enableFlag, int32_t scenario)
{
    if (enableFlag == ADL_ENABLE_FLAG_OFF)
    {
        return false;
    }
    else if ((scenario == MEDIA_PICTURE) &&
             (enableFlag != ADL_ENABLE_FLAG_ON))
    {
        PQ_LOGD("[PQDCAdaptor] bypass image DC()... DC property not on");
        return false;
    }

    return true;
}

void PQDCAdaptor::ADL_writeLumaCurve(DpCommand &command, const ADLOutput &output, int32_t *pFrameConfigLabel)
{
    int32_t index = 0;

    MM_REG_WRITE(command, MDP_Y_FTN_1_0_MAIN, output.LumaCurve[0] | (output.LumaCurve[1]<<16), 0xFFFFFFFF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_Y_FTN_3_2_MAIN, output.LumaCurve[2] | (output.LumaCurve[3]<<16), 0xFFFFFFFF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_Y_FTN_5_4_MAIN, output.LumaCurve[4] | (output.LumaCurve[5]<<16), 0xFFFFFFFF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_Y_FTN_7_6_MAIN, output.LumaCurve[6] | (output.LumaCurve[7]<<16), 0xFFFFFFFF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_Y_FTN_9_8_MAIN, output.LumaCurve[8] | (output.LumaCurve[9]<<16), 0xFFFFFFFF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_Y_FTN_11_10_MAIN, output.LumaCurve[10] | (output.LumaCurve[11]<<16), 0xFFFFFFFF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_Y_FTN_13_12_MAIN, output.LumaCurve[12] | (output.LumaCurve[13]<<16), 0xFFFFFFFF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_Y_FTN_15_14_MAIN, output.LumaCurve[14] | (output.LumaCurve[15]<<16), 0xFFFFFFFF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_Y_FTN_17_16_MAIN, output.LumaCurve[16], 0xFFFFFFFF, &pFrameConfigLabel[index++]);

#if DYN_CONTRAST_VERSION == 0
    MM_REG_WRITE(command, MDP_C_BOOST_MAIN_2, output.dync_cboost_yoffset, 0x0000007F, &pFrameConfigLabel[index++]);
#elif DYN_CONTRAST_VERSION == 1
    MM_REG_WRITE(command, MDP_C_BOOST_MAIN_2, (output.dync_cboost_yconst << 24) |
        (output.dync_cboost_yoffset_sel << 16) | output.dync_cboost_yoffset, 0xFF03007F, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_TDSHP_C_BOOST_MAIN_2, (output.tdshp_cboost_yconst << 24) |
        (output.tdshp_cboost_yoffset_sel << 16) | output.tdshp_cboost_yoffset, 0xFF03007F, &pFrameConfigLabel[index++]);
#elif DYN_CONTRAST_VERSION == 2
    MM_REG_WRITE(command, MDP_C_BOOST_MAIN, output.cboost_gain, 0x000000FF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_C_BOOST_MAIN_2, (output.dync_cboost_yconst << 24) |
        (output.dync_cboost_yoffset_sel << 16) | output.dync_cboost_yoffset, 0xFF03007F, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_BITPLUS_01, output.contour_eds_slope  << 20 |
                                          output.contour_eds_th  << 11 |
                                          output.diffcount_slope << 5  |
                                          output.diffcount_th  << 0, 0x03FFFFFF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_BITPLUS_02, output.noise_eds_slope  << 20 |
                                          output.noise_eds_th  << 11 |
                                          output.signchange_slope << 5  |
                                          output.signchange_th  << 0, 0x03FFFFFF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_TDSHP_CFG, output.skin_conf_en << 11, 0x00000800, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_DC_SKIN_RANGE0, output.w1_skin_bright_wgt << 0 |
                                              output.w1_skin_dark_wgt << 6, 0x00000FFF, &pFrameConfigLabel[index++]);
#endif
    MM_REG_WRITE(command, MDP_C_BOOST_MAIN , output.new_boost_lmt_l << 16, 0x00FF0000, &pFrameConfigLabel[index++]);  // update MDP_C_BOOST_MAIN by MM/DE6
    MM_REG_WRITE(command, MDP_TDSHP_00 , output.dync_bypass << 28, 0x10000000, &pFrameConfigLabel[index++]); // TDS_ADAP_LUMA_BP = 0, enable

}

void PQDCAdaptor::ADL_updateLumaCurve(DpCommand &command, const ADLOutput &output, int32_t *pFrameConfigLabel)
{
    int32_t index = 0;

    MM_REG_WRITE(command, MDP_Y_FTN_1_0_MAIN, output.LumaCurve[0] | (output.LumaCurve[1]<<16), 0xFFFFFFFF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_Y_FTN_3_2_MAIN, output.LumaCurve[2] | (output.LumaCurve[3]<<16), 0xFFFFFFFF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_Y_FTN_5_4_MAIN, output.LumaCurve[4] | (output.LumaCurve[5]<<16), 0xFFFFFFFF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_Y_FTN_7_6_MAIN, output.LumaCurve[6] | (output.LumaCurve[7]<<16), 0xFFFFFFFF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_Y_FTN_9_8_MAIN, output.LumaCurve[8] | (output.LumaCurve[9]<<16), 0xFFFFFFFF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_Y_FTN_11_10_MAIN, output.LumaCurve[10] | (output.LumaCurve[11]<<16), 0xFFFFFFFF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_Y_FTN_13_12_MAIN, output.LumaCurve[12] | (output.LumaCurve[13]<<16), 0xFFFFFFFF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_Y_FTN_15_14_MAIN, output.LumaCurve[14] | (output.LumaCurve[15]<<16), 0xFFFFFFFF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_Y_FTN_17_16_MAIN, output.LumaCurve[16], 0xFFFFFFFF, NULL, pFrameConfigLabel[index++]);

#if DYN_CONTRAST_VERSION == 0
    MM_REG_WRITE(command, MDP_C_BOOST_MAIN_2, output.dync_cboost_yoffset, 0x0000007F, NULL, pFrameConfigLabel[index++]);
#elif DYN_CONTRAST_VERSION == 1
    MM_REG_WRITE(command, MDP_C_BOOST_MAIN_2, (output.dync_cboost_yconst << 24) |
        (output.dync_cboost_yoffset_sel << 16) | output.dync_cboost_yoffset, 0xFF03007F, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_TDSHP_C_BOOST_MAIN_2, (output.tdshp_cboost_yconst << 24) |
        (output.tdshp_cboost_yoffset_sel << 16) | output.tdshp_cboost_yoffset, 0xFF03007F, NULL, pFrameConfigLabel[index++]);
#elif DYN_CONTRAST_VERSION == 2
    MM_REG_WRITE(command, MDP_C_BOOST_MAIN, output.cboost_gain, 0x000000FF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_C_BOOST_MAIN_2, (output.dync_cboost_yconst << 24) |
        (output.dync_cboost_yoffset_sel << 16) | output.dync_cboost_yoffset, 0xFF03007F, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_BITPLUS_01, output.contour_eds_slope  << 20 |
                                          output.contour_eds_th  << 11 |
                                          output.diffcount_slope << 5  |
                                          output.diffcount_th  << 0, 0x03FFFFFF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_BITPLUS_02, output.noise_eds_slope  << 20 |
                                          output.noise_eds_th  << 11 |
                                          output.signchange_slope << 5  |
                                          output.signchange_th  << 0, 0x03FFFFFF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_TDSHP_CFG, output.skin_conf_en << 11, 0x00000800, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_DC_SKIN_RANGE0, output.w1_skin_bright_wgt << 0 |
                                              output.w1_skin_dark_wgt << 6, 0x00000FFF, NULL, pFrameConfigLabel[index++]);
#endif

    MM_REG_WRITE(command, MDP_C_BOOST_MAIN , output.new_boost_lmt_l << 16, 0x00FF0000, NULL, pFrameConfigLabel[index++]);  // update MDP_C_BOOST_MAIN by MM/DE6
    MM_REG_WRITE(command, MDP_TDSHP_00 , output.dync_bypass << 28, 0x10000000, NULL, pFrameConfigLabel[index++]); // TDS_ADAP_LUMA_BP = 0, enable

}

int32_t PQDCAdaptor::onCalculate(PQSession* pPQSession, int32_t LmtFullSwitch, int32_t width, int32_t height, bool enable, ADLOutput *output)
{
    int32_t i, adl_debug;
    dcHandle* currDC_p;
    uint64_t id = pPQSession->getID();

    currDC_p = pPQSession->getDCHandle();

    PQ_LOGI("[PQDCAdaptor] onCalculate(), enable[%d]\n", enable);

    if (enable == false)
    {
        return 0;
    }

    //Let the new video use previous FW.
    //ALGO will regard it as scene change so that it won't use the previous histogram.
    //checkAndResetUnusedADLFW(currDC_p, id);

    PQDCHistList* pWaitingHistList = currDC_p->pWaitingHistList;
    PQDCHistList* pDoneHistList    = currDC_p->pDoneHistList;
    DpCondition* pHistListCond = currDC_p->pHistListCond;

    PQTimeValue startTime, currTime;
    uint32_t waitTime = DC_APPLY_CMD_TIMEOUT_MS;
    int32_t diff;
    bool isAvailable;

    //waiting for refer frame done
    PQ_TIMER_GET_CURRENT_TIME(startTime);

    while(pWaitingHistList->size() >= PQ_REFER_STEP)
    {
        PQ_LOGI("[PQDCAdaptor] onCalculate(), id[%llx]  waiting[%d] done[%d] waiting...\n", id, pWaitingHistList->size(), pDoneHistList->size());
        do {
            pHistListCond->waitRelative(s_ALMutex, MILLI_TO_NANO(waitTime));

            PQ_TIMER_GET_CURRENT_TIME(currTime);
            PQ_TIMER_GET_DURATION_IN_MS(startTime, currTime, diff);

            if (diff >= DC_APPLY_CMD_TIMEOUT_MS)
            {
                break;
            }
            else
            {
                waitTime = DC_APPLY_CMD_TIMEOUT_MS - diff;
            }
        } while (currDC_p->isAvailable == false);

        //re-get handle after waiting
        currDC_p = pPQSession->getDCHandle();
        pWaitingHistList = currDC_p->pWaitingHistList;
        pDoneHistList    = currDC_p->pDoneHistList;
        pHistListCond    = currDC_p->pHistListCond;

        if (currDC_p->isAvailable == false)
        {
            PQDCHistList::iterator iter = currDC_p->pWaitingHistList->begin();

            currDC_p->pWaitingHistList->erase(iter);
            PQ_LOGE("[PQDCAdaptor] error: isAvailable = %d", currDC_p->isAvailable);
            break;
        }
    }

    PQ_LOGI("[PQDCAdaptor] onCalculate(), id[%llx]  waiting[%d] done[%d]....\n", id, pWaitingHistList->size(), pDoneHistList->size());

    uint32_t step = PQ_REFER_STEP - pWaitingHistList->size();

    //the first PQ_REFER_STEP frames, just return default curve
    if ((pDoneHistList->size() < step) || (currDC_p->isAvailable == false))
    {
        if (currDC_p->pADLFW == NULL)
        {
            currDC_p->pADLFW = new CPQDCFW;
            memcpy(currDC_p->pADLFW->pADLReg, m_pADLFW->pADLReg, sizeof(ADLReg));
            memcpy(currDC_p->pADLFW->pNormalModeReg, m_pADLFW->pNormalModeReg, sizeof(ADLReg));
            memcpy(currDC_p->pADLFW->pHDRModeReg, m_pADLFW->pHDRModeReg, sizeof(ADLReg));
            currDC_p->pADLFW->setDebugFlags(m_debugFlag);
        }

        memcpy(output, &defaultADLSetting, sizeof(ADLOutput));

        PQ_LOGI("[PQDCAdaptor] first frame!  owner[%llx]", id);
    }
    else
    {
        PQDCHistList::iterator iter = pDoneHistList->begin();
        std::advance(iter, pDoneHistList->size() - step);
        ADLInput ADLInput;

        for (i = 0; i < LUMA_HIST_NUM_WEIGHT; i++)
        {
            // rounding before div 16
            ADLInput.LumaHist[i] = (iter->hist[i] + 8) >> 4;
        }

        ADLInput.LumaSum = iter->hist[DC_LUMA_HISTOGRAM_NUM - 2];
        ADLInput.ColorHist = iter->hist[DC_LUMA_HISTOGRAM_NUM - 1];

#if DYN_CONTRAST_VERSION == 2
        for (i = DC_LUMA_HISTOGRAM_NUM; i < TOTAL_HISTOGRAM_NUM; i++)
        {
            // rounding before div 16
            ADLInput.ContourHist[i - DC_LUMA_HISTOGRAM_NUM] = (iter->hist[i] + 8) >> 4;
        }
        ADLInput.HDR_Mode = currDC_p->isHDRContent;
#endif

        iter->ref--;
        if(iter->ref == 0)
        {
            pDoneHistList->erase(iter);
        }

        ADLInput.ImgArea = width * height;   // resize before TDSHP, so we use outW/H
        if (pPQSession->globalPQParam.globalPQSupport != 0 && pPQSession->globalPQParam.globalPQType != GLOBAL_PQ_VIDEO)
        {
            ADLInput.LmtFullSwitch = 1;
            ADLInput.VideoImgSwitch = 1;  // ui
        }
        else
        {
            ADLInput.LmtFullSwitch = LmtFullSwitch;
            ADLInput.VideoImgSwitch = 0;  // video
        }
        ADLInput.dync_cboost_yoffset = 0;
#if DYN_CONTRAST_VERSION == 1
        ADLInput.tdshp_cboost_yoffset = 0;
#endif
        currDC_p->pADLFW->pADLReg->bALOnOff = 1;

        if (pPQSession->globalPQParam.globalPQSupport != 0)
        {
#if DYN_CONTRAST_VERSION == 0
            DpPqParam PqParam;
            pPQSession->getPQParam(&PqParam);
            if (PqParam.u.video.grallocExtraHandle == NULL)
            {
                PQ_LOGE("[PQDCAdaptor] onCalculate(), grallocExtraHandle == NULL");
            }
            else
            {
                gralloc_extra_ion_sf_info_t sf_info;
                int32_t err = gralloc_extra_query(PqParam.u.video.grallocExtraHandle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);
                if (err != 0)
                {
                    PQ_LOGE("[PQDCAdaptor] gralloc_extra_query failed");
                }
                else if (sf_info.status2 & GRALLOC_EXTRA_BIT2_UI_PQ_ALGO_RESET_ON)
                {
                    PQ_LOGD("reset DC algo");
                    ADLInput.CurveResetEn = 1;
                }
            }
#endif

            DISP_PQ_DC_PARAM dcparam;
            if (pPQSession->globalPQParam.globalPQindexInit == 0)
            {
                sp<IPictureQuality> service = IPictureQuality::getService();
                if (service == nullptr) {
                    PQ_LOGD("[onCalculate] failed to get HW service");
                }
                else
                {
                    android::hardware::Return<void> ret = service->getGlobalPQIndex(
                        [&] (Result retval, globalPQIndex_t index) {
                        if (retval == Result::OK) {
                            memcpy(&pPQSession->globalPQParam.globalPQindex, &index, sizeof(GLOBAL_PQ_INDEX_T));
                        }
                    });
                    if (!ret.isOk()){
                        PQ_LOGE("Transaction error in IPictureQuality::getGlobalPQIndex");
                    }
                }

                pPQSession->globalPQParam.globalPQindexInit = 1;
            }
            if (pPQSession->globalPQParam.globalPQType == GLOBAL_PQ_VIDEO)
                memcpy(&dcparam, pPQSession->globalPQParam.globalPQindex.dcindex.entry[(pPQSession->globalPQParam.globalPQStrength & 0xff00) >> 8], sizeof(DISP_PQ_DC_PARAM));
            else
                memcpy(&dcparam, pPQSession->globalPQParam.globalPQindex.dcindex.entry[(pPQSession->globalPQParam.globalPQStrength & 0xff000000) >> 24], sizeof(DISP_PQ_DC_PARAM));

            currDC_p->pADLFW->pADLReg->bBSOnOff         = dcparam.param[BlackEffectEnable];
            currDC_p->pADLFW->pADLReg->bWSOnOff         = dcparam.param[WhiteEffectEnable];
            currDC_p->pADLFW->pADLReg->bStrongBSOffset  = dcparam.param[StrongBlackEffect];
            currDC_p->pADLFW->pADLReg->bStrongWSOffset  = dcparam.param[StrongWhiteEffect];
            currDC_p->pADLFW->pADLReg->bBSAdaptiveLmt   = dcparam.param[AdaptiveBlackEffect];
            currDC_p->pADLFW->pADLReg->bWSAdaptiveLmt   = dcparam.param[AdaptiveWhiteEffect];
            currDC_p->pADLFW->pADLReg->bForceScenceChangeEn = dcparam.param[ScenceChangeOnceEn];
            currDC_p->pADLFW->pADLReg->bForceScenceChange2En= dcparam.param[ScenceChangeControlEn];
            currDC_p->pADLFW->pADLReg->bForceScenceChange2  = dcparam.param[ScenceChangeControl];
            currDC_p->pADLFW->pADLReg->bMaxDiffThreshold    = dcparam.param[ScenceChangeTh1];
            currDC_p->pADLFW->pADLReg->bTotalDiffThreshold  = dcparam.param[ScenceChangeTh2];
            currDC_p->pADLFW->pADLReg->bAPLDiffThreshold    = dcparam.param[ScenceChangeTh3];
            currDC_p->pADLFW->pADLReg->bADLWeight1      = dcparam.param[ContentSmooth1];
            currDC_p->pADLFW->pADLReg->bADLWeight2      = dcparam.param[ContentSmooth2];
            currDC_p->pADLFW->pADLReg->bADLWeight3      = dcparam.param[ContentSmooth3];
            currDC_p->pADLFW->pADLReg->bMIDDCGain       = dcparam.param[MiddleRegionGain1];
            currDC_p->pADLFW->pADLReg->bMIDACGain       = dcparam.param[MiddleRegionGain2];
            currDC_p->pADLFW->pADLReg->bBSDCGain        = dcparam.param[BlackRegionGain1];
            currDC_p->pADLFW->pADLReg->bBSACGain        = dcparam.param[BlackRegionGain2];
            currDC_p->pADLFW->pADLReg->bBSLevel         = dcparam.param[BlackRegionRange];
            currDC_p->pADLFW->pADLReg->bBSRatio         = dcparam.param[BlackEffectLevel];
            currDC_p->pADLFW->pADLReg->bBSOffsetThL     = dcparam.param[BlackEffectParam1];
            currDC_p->pADLFW->pADLReg->bBSOffsetThH     = dcparam.param[BlackEffectParam2];
            currDC_p->pADLFW->pADLReg->bBSOffsetL       = dcparam.param[BlackEffectParam3];
            currDC_p->pADLFW->pADLReg->bBSOffsetH       = dcparam.param[BlackEffectParam4];
            currDC_p->pADLFW->pADLReg->bWSDCGain        = dcparam.param[WhiteRegionGain1];
            currDC_p->pADLFW->pADLReg->bWSACGain        = dcparam.param[WhiteRegionGain2];
            currDC_p->pADLFW->pADLReg->bWSLevel         = dcparam.param[WhiteRegionRange];
            currDC_p->pADLFW->pADLReg->bWSRatio         = dcparam.param[WhiteEffectLevel];
            currDC_p->pADLFW->pADLReg->bWSOffsetThL     = dcparam.param[WhiteEffectParam1];
            currDC_p->pADLFW->pADLReg->bWSOffsetThH     = dcparam.param[WhiteEffectParam2];
            currDC_p->pADLFW->pADLReg->bWSOffsetL       = dcparam.param[WhiteEffectParam3];
            currDC_p->pADLFW->pADLReg->bWSOffsetH       = dcparam.param[WhiteEffectParam4];
            currDC_p->pADLFW->pADLReg->bYGammaIdx1      = dcparam.param[ContrastAdjust1];
            currDC_p->pADLFW->pADLReg->bYGammaIdx2      = dcparam.param[ContrastAdjust2];
            currDC_p->pADLFW->pADLReg->bIIRStrength     = dcparam.param[DCChangeSpeedLevel];
            currDC_p->pADLFW->pADLReg->bSkinBlendmethod = dcparam.param[ProtectRegionEffect];
            currDC_p->pADLFW->pADLReg->bIIRStrengthSkin = dcparam.param[DCChangeSpeedLevel2];
            currDC_p->pADLFW->pADLReg->bSkinWgtSlope    = dcparam.param[ProtectRegionWeight];
            currDC_p->pADLFW->pADLReg->bALOnOff         = dcparam.param[DCEnable];
            /* only for global pq*/
#if DYN_CONTRAST_VERSION == 0
            currDC_p->pADLFW->pADLReg->DarkSceneTh      = dcparam.param[DarkSceneTh];
            currDC_p->pADLFW->pADLReg->DarkSceneSlope   = dcparam.param[DarkSceneSlope];
            currDC_p->pADLFW->pADLReg->DarkDCGain       = dcparam.param[DarkDCGain];
            currDC_p->pADLFW->pADLReg->DarkACGain       = dcparam.param[DarkACGain];
            currDC_p->pADLFW->pADLReg->BinomialTh       = dcparam.param[BinomialTh];
            currDC_p->pADLFW->pADLReg->BinomialSlope    = dcparam.param[BinomialSlope];
            currDC_p->pADLFW->pADLReg->BinomialDCGain   = dcparam.param[BinomialDCGain];
            currDC_p->pADLFW->pADLReg->BinomialACGain   = dcparam.param[BinomialACGain];

            currDC_p->pADLFW->pADLReg->BinomialTarRange = dcparam.param[BinomialTarRange];
            currDC_p->pADLFW->pADLReg->bGlobalPQEn      = dcparam.param[bGlobalPQEn];
            currDC_p->pADLFW->pADLReg->bHistAvoidFlatBgEn   = dcparam.param[bHistAvoidFlatBgEn];
            currDC_p->pADLFW->pADLReg->bIIRCurveDiffSumTh   = dcparam.param[bIIRCurveDiffSumTh];
            currDC_p->pADLFW->pADLReg->bIIRCurveDiffMaxTh   = dcparam.param[bIIRCurveDiffMaxTh];
#endif
        }

        int32_t scenario = pPQSession->getScenario();

        tuningDCInput(&ADLInput, scenario);

        tuningDCSWReg(currDC_p->pADLFW->pADLReg, scenario);
        currDC_p->pADLFW->onCalculate(ADLInput, output);

        if (currDC_p->pADLFW->pADLReg->bALOnOff == 0)
        {
            memcpy(output, &defaultADLSetting, sizeof(ADLOutput));
        }

#if DYN_CONTRAST_VERSION == 0
        if (pPQSession->globalPQParam.globalPQSupport != 0)
        {
            if (output->steady_state_flag == 0)
            {
                sp<IPictureQuality> service = IPictureQuality::getService();
                if (service == nullptr)
                {
                    PQ_LOGD("[onCalculate] failed to get HW service");
                }
                else
                {
                    android::hardware::Return<Result> ret = service->setGlobalPQStableStatus(output->steady_state_flag);
                    if (!ret.isOk() || ret != Result::OK)
                    {
                        PQ_LOGE("setGlobalPQStableStatus() failed!");
                    }
                }
            }
        }
#endif
    }

    //push one node into waiting list
    PQDCHist hist;
    hist.ref = 1;//reference 1 by 1
    pWaitingHistList->push_back(hist);
    currDC_p->isAvailable = false;

    // update time of curr instance
    PQ_TIMER_GET_CURRENT_TIME(currDC_p->workTime);

    return 0;
}

// for Image
int32_t PQDCAdaptor::onCalculateImage(PQSession* pPQSession, int32_t LmtFullSwitch, const DpImageParam &param, ADLOutput *output)
{
    int32_t i, adl_debug;
    dcHandle* currDC_p;

    currDC_p = pPQSession->getDCHandle();

    //if (tuningFlag & TDSHP_FLAG_DC_TUNING)
    //{
    //    updateDCFWparam(currDC_p->pADLFW);
    //}

    ADLInput input;

    memcpy(&input.LumaHist[0], &param.info[0], sizeof(uint32_t)*17);
    input.LumaSum = param.info[17];
    input.ColorHist = param.info[18];
    input.ImgArea = param.info[19];
    input.LmtFullSwitch = LmtFullSwitch;
    input.VideoImgSwitch = 1;   // image
    input.dync_cboost_yoffset = 0;
#if DYN_CONTRAST_VERSION == 1
    input.tdshp_cboost_yoffset = 0;
#endif
    PQ_LOGD("[PQDCAdaptor] onCalculateImage()... info00[%d], info01[%d], info02[%d], info03[%d], info04[%d]", param.info[0], param.info[1], param.info[2], param.info[3], param.info[4]);
    PQ_LOGD("[PQDCAdaptor] onCalculateImage()... info05[%d], info06[%d], info07[%d], info08[%d], info09[%d]", param.info[5], param.info[6], param.info[7], param.info[8], param.info[9]);
    PQ_LOGD("[PQDCAdaptor] onCalculateImage()... info10[%d], info11[%d], info12[%d], info13[%d], info14[%d]", param.info[10], param.info[11], param.info[12], param.info[13], param.info[14]);
    PQ_LOGD("[PQDCAdaptor] onCalculateImage()... info15[%d], info16[%d], info17[%d], info18[%d], info19[%d]", param.info[15], param.info[16], param.info[17], param.info[18], param.info[19]);
    PQ_LOGD("[PQDCAdaptor] onCalculateImage()... LmtFullSwitch[%d], VideoImgSwitch[%d]", input.LmtFullSwitch, input.VideoImgSwitch);

    int32_t scenario = pPQSession->getScenario();

    tuningDCInput(&input, scenario);
    tuningDCSWReg(currDC_p->pADLFW->pADLReg, scenario);
    currDC_p->pADLFW->onCalculate(input, output);
    PQ_LOGD("[PQDCAdaptor] onCalculateImage()... out00[%d], out01[%d], out02[%d], out03[%d], out04[%d]", output->LumaCurve[0], output->LumaCurve[1], output->LumaCurve[2], output->LumaCurve[3], output->LumaCurve[4]);
    PQ_LOGD("[PQDCAdaptor] onCalculateImage()... out05[%d], out06[%d], out07[%d], out08[%d], out09[%d]", output->LumaCurve[5], output->LumaCurve[6], output->LumaCurve[7], output->LumaCurve[8], output->LumaCurve[9]);
    PQ_LOGD("[PQDCAdaptor] onCalculateImage()... out10[%d], out11[%d], out12[%d], out13[%d], out14[%d]", output->LumaCurve[10], output->LumaCurve[11], output->LumaCurve[12], output->LumaCurve[13], output->LumaCurve[14]);
    PQ_LOGD("[PQDCAdaptor] onCalculateImage()... out15[%d], out16[%d], dync_cboost_y[%d]", output->LumaCurve[15], output->LumaCurve[16], output->dync_cboost_yoffset);
#if DYN_CONTRAST_VERSION == 1
    PQ_LOGD("[PQDCAdaptor] onCalculateImage()... tdshp_cboost_y[%d]", output->tdshp_cboost_yoffset);
#endif

    return 0;
}

void PQDCAdaptor::checkAndResetUnusedADLFW(PQSession* pPQSession, dcHandle* currDC_p, uint64_t id)
{
    PQTimeValue currTime;
    PQ_TIMER_GET_CURRENT_TIME(currTime);
    int32_t diff;

    PQ_TIMER_GET_DURATION_IN_MS(currDC_p->workTime, currTime, diff);

    /*if DC object is unused for a while, treat it as different video content and reset
      DC related members, or the continuity of histogram may be wrong. */
    if (diff >= PQDC_NEW_CLIP_TIME_INTERVAL)
    {
        if (pPQSession->globalPQParam.globalPQSupport == 0)
        {
            /*reset DC FW object, it will be newed later in first frame condition*/
            delete currDC_p->pADLFW;
            currDC_p->pADLFW = NULL;

            currDC_p->pHistListCond->signal();

            /*reset DC lists, it wll trigger first frame condition later*/
            delete currDC_p->pHistListCond;
            delete currDC_p->pWaitingHistList;
            delete currDC_p->pDoneHistList;

            currDC_p->pWaitingHistList = new PQDCHistList;
            currDC_p->pDoneHistList = new PQDCHistList;
            currDC_p->pHistListCond = new DpCondition();

            PQ_LOGD("[PQDCAdaptor] checkAndResetUnusedADLFW(), id[%llx], time diff[%d]", id, diff);
        }
    }
}
