#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1
#include <cutils/properties.h>
#include <cutils/log.h>
#include <fcntl.h>

#include <PQCommon.h>
#include <PQDSAdaptor.h>
#include "PQTuningBuffer.h"
#include "mdp_reg_tdshp.h"
#include "ui/gralloc_extra.h"
#include "graphics_mtk_defs.h"
#include "gralloc_mtk_defs.h"
#include "PQIspTuning.h"

#include <cutils/properties.h>
#define ISP_DEBUG "persist.vendor.sys.isp.pqds.debug"
#define ARR_LEN_4BYTE(arr) (sizeof(arr) / 4)
#define DS_CAL_REGS_THRES 2 //ms


PQDSAdaptor* PQDSAdaptor::s_pInstance[] = {};
PQMutex   PQDSAdaptor::s_ALMutex;

PQDSAdaptor* PQDSAdaptor::getInstance(uint32_t identifier)
{
    AutoMutex lock(s_ALMutex);

    if(NULL == s_pInstance[identifier])
    {
        s_pInstance[identifier] = new PQDSAdaptor(identifier);
        atexit(PQDSAdaptor::destroyInstance);
    }

    return s_pInstance[identifier];
}

void PQDSAdaptor::destroyInstance()
{
    AutoMutex lock(s_ALMutex);

    for (int identifier = 0; identifier < DS_ENGINE_MAX_NUM; identifier++){
        if (NULL != s_pInstance[identifier])
        {
            delete s_pInstance[identifier];
            s_pInstance[identifier] = NULL;
        }
    }
}

PQDSAdaptor::PQDSAdaptor(uint32_t identifier)
        : m_identifier(identifier),
          m_pDSFW(NULL),
          PQAlgorithmAdaptor(PROXY_DS_SWREG,
                             PROXY_DS_INPUT,
                             PROXY_DS_OUTPUT)
{
    PQ_LOGD("[PQDSAdaptor] PQDSAdaptor()... ");

    m_pDSFW = new CPQDSFW;
    getDSTable();
    memcpy(m_pDSFW->pDSRegNormal, m_pDSFW->pDSReg, sizeof(DSReg));

    char c_isPqDebug[PROPERTY_VALUE_MAX];
    property_get(ISP_DEBUG, c_isPqDebug, "0");
    m_ispPqDebug = atoi(c_isPqDebug);
    m_dsConfig = NULL;
};

PQDSAdaptor::~PQDSAdaptor()
{
    PQ_LOGD("[PQDSAdaptor] ~PQDSAdaptor()... ");

    delete m_pDSFW;
};

bool PQDSAdaptor::calRegs(PQSession* pPQSession, DpCommand &command, DpConfig &config)
{
    AutoMutex lock(s_ALMutex);

    PQTimeValue    begin;
    PQTimeValue    end;
    int32_t        diff;
    PQTimeValue    time_s;
    PQTimeValue    time_e;
    int32_t        time_d;
    PQ_TIMER_GET_CURRENT_TIME(begin);

    int32_t shp_enable = 0;

    PQ_TIMER_GET_CURRENT_TIME(time_s);

    pPQSession->getDSConfig(&m_dsConfig);

    DpPqConfig* PQConfig;
    pPQSession->getDpPqConfig(&PQConfig);

    PQ_TIMER_GET_CURRENT_TIME(time_e);
    PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d);

    PQ_LOGI("[PQDSAdaptor] getDSConfig, time %d ms\n", time_d);

    shp_enable = m_dsConfig->ENABLE;

    PQ_LOGI("[PQDSAdaptor] calRegs parTDSHP = 0x%08x, shp_enable[%d]\n", config.parTDSHP, shp_enable);
    PQ_LOGI("[PQDSAdaptor] calRegs PQConfig.enSharp[%d]\n", PQConfig->enSharp);

    PQ_TIMER_GET_CURRENT_TIME(time_s);
    if (shp_enable == 0)
    {
        // force disable
    }
    else
    {
        if (0 == (config.parTDSHP & 0x0000FFFF) &&
            0 == PQConfig->enSharp)
        {
            shp_enable = 0;
        }
        else
        {
            shp_enable = 1;
        }
    }
    onConfigTDSHP(pPQSession, command, config, shp_enable);

    PQ_TIMER_GET_CURRENT_TIME(time_e);
    PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d);

    PQ_LOGI("[PQDSAdaptor] onConfigTDSHP, time %d ms\n", time_d);

    PQ_TIMER_GET_CURRENT_TIME(end);
    PQ_TIMER_GET_DURATION_IN_MS(begin, end, diff);
    if (diff > DS_CAL_REGS_THRES)
    {
        PQ_LOGD("[PQDSAdaptor] calRegs, shp_enable[%d] time %d ms\n", shp_enable, diff);
    }
    else
    {
        PQ_LOGI("[PQDSAdaptor] calRegs, shp_enable[%d] time %d ms\n", shp_enable, diff);
    }

    return shp_enable;
}

void PQDSAdaptor::onConfigTDSHP(PQSession* pPQSession, DpCommand &command, DpConfig &config, bool enable)
{
    DP_TRACE_CALL();
    PQTimeValue    begin;
    PQTimeValue    end;
    int32_t        diff;

    PQ_TIMER_GET_CURRENT_TIME(begin);

    DpPqParam PQParam;
    pPQSession->getPQParam(&PQParam);

    if (config.scenario >= STREAM_ISP_VR && PQParam.scenario == MEDIA_UNKNOWN)
    {
        // setPQParam for old ISP PQ interface
        PQParam.enable = true;
        PQParam.scenario = MEDIA_ISP_PREVIEW;
        pPQSession->setPQparam(&PQParam);
    }

    onCalcTDSHP(pPQSession, command, config, enable);

    PQ_TIMER_GET_CURRENT_TIME(end);
    PQ_TIMER_GET_DURATION_IN_MS(begin, end, diff);
    PQ_LOGI("[PQDSAdaptor] onConfigTDSHP, time %d ms\n", diff);
    return;
}

void PQDSAdaptor::onCalculate(const DSInput *input, DSOutput *output)
{
    m_pDSFW->onCalculate(input, output);
}

void PQDSAdaptor::tuningDSInput(DSInput *input, int32_t scenario)
{
    PQTuningBuffer *p_buffer = m_inputBuffer;
    unsigned int *overwritten_buffer = p_buffer->getOverWrittenBuffer();
    unsigned int *reading_buffer = p_buffer->getReadingBuffer();
    size_t copy_size = sizeof(DSInput);

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

bool PQDSAdaptor::tuningDSOutput(DSOutput *output, int32_t scenario)
{
    PQTuningBuffer *p_buffer = m_outputBuffer;
    unsigned int *overwritten_buffer = p_buffer->getOverWrittenBuffer();
    unsigned int *reading_buffer = p_buffer->getReadingBuffer();
    size_t copy_size = sizeof(DSOutput);

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

void PQDSAdaptor::tuningDSSWReg(int32_t scenario)
{
    PQTuningBuffer *p_buffer = m_swRegBuffer;
    unsigned int *overwritten_buffer = p_buffer->getOverWrittenBuffer();
    unsigned int *reading_buffer = p_buffer->getReadingBuffer();
    size_t copy_size = sizeof(DSReg);

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
        memcpy(m_pDSFW->pDSReg, overwritten_buffer, copy_size);
    } else if (scenario == MEDIA_PICTURE) {
        p_buffer->resetReady();
        memcpy(reading_buffer, m_pDSFW->pDSReg, copy_size);
        p_buffer->push();
    } else if (p_buffer->toBePrepared()) {
        memcpy(reading_buffer, m_pDSFW->pDSReg, copy_size);
        p_buffer->push();
    }
}

bool PQDSAdaptor::getDSTable()
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;
    /* get register value from ashmem */
    for (int index = 0; index < PROXY_DS_CUST_MAX; index++)
    {
        offset += size;
        if (index == PROXY_DS_CUST_REG)
        {
            size = ARR_LEN_4BYTE(DSReg);
            isNoError = pPQConfig->getAshmemArray(PROXY_DS_CUST, offset, m_pDSFW->pDSReg, size);
        }
        else if (index == PROXY_DS_CUST_CZSWREG)
        {
            size = ARR_LEN_4BYTE(m_pDSFW->iDSRegEntry);
            isNoError = pPQConfig->getAshmemArray(PROXY_DS_CUST, offset, &m_pDSFW->iDSRegEntry, size);
        }
        else if (index == PROXY_DS_CUST_CZHWREG)
        {
            size = ARR_LEN_4BYTE(m_pDSFW->iDSHWRegEntry);
            isNoError = pPQConfig->getAshmemArray(PROXY_DS_CUST, offset, &m_pDSFW->iDSHWRegEntry, size);
        }
        else if (index == PROXY_DS_CUST_CZLEVEL)
        {
            size = ARR_LEN_4BYTE(m_pDSFW->iTdshpLevel);
            isNoError = pPQConfig->getAshmemArray(PROXY_DS_CUST, offset, &m_pDSFW->iTdshpLevel, size);
        }

        if (isNoError < 0)
        {
            break;
        }
    }
    return (isNoError == 1) ? 1 : 0;
}

void PQDSAdaptor::onCalcTDSHP(PQSession* pPQSession, DpCommand &command, DpConfig &config, bool enable)
{
    DP_TRACE_CALL();
    DSInput inParam;
    DSOutput outParam;

    PQTimeValue    begin;
    PQTimeValue    end;
    int32_t        diff;
    PQTimeValue    time_s;
    PQTimeValue    time_e;
    int32_t        time_d[6];

    PQ_TIMER_GET_CURRENT_TIME(begin);

    PQ_TIMER_GET_CURRENT_TIME(time_s);

    DpPqParam PQParam;
    pPQSession->getPQParam(&PQParam);

    initDSFWparam(m_dsConfig, config, &inParam, enable, &PQParam);

    int32_t scenario = pPQSession->getScenario();

    PQ_TIMER_GET_CURRENT_TIME(time_e);
    PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d[0]);

    PQ_TIMER_GET_CURRENT_TIME(time_s);

    ConfigDSParamByScenario(&inParam, &PQParam);

    PQConfig* pPQConfig = PQConfig::getInstance();
    DpPqConfig* DpPqConfig;
    pPQSession->getDpPqConfig(&DpPqConfig);

    PQ_TIMER_GET_CURRENT_TIME(time_e);
    PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d[1]);
    PQ_TIMER_GET_CURRENT_TIME(time_s);

#ifdef SUPPORT_CLEARZOOM
#ifdef SUPPORT_NVRAM_TUNING
    if (PQParam.u.isp.clearZoomParam.p_customSetting == NULL)
        PQ_LOGI("[PQDSAdaptor] ClearZoom NVRam setting Bypass...\n");
    else
        PQ_LOGI("[PQDSAdaptor] ClearZoom NVRam setting enabled \n");

    m_pDSFW->onLoadTable(&inParam, PQParam.u.isp.clearZoomParam.p_customSetting);
#else
    m_pDSFW->onLoadTable(&inParam);
#endif /* SUPPORT_NVRAM_TUNING */
#endif /* SUPPORT_CLEARZOOM */

    /* check enable flag from setPQparameter */
    if (DpPqConfig->enUR == 0 || DpPqConfig->enUR == 1)
        m_pDSFW->pDSReg->iUltraRes_en = DpPqConfig->enUR;

    /* check debug system property*/
    if (m_dsConfig->ENABLE == 0 || m_dsConfig->ENABLE == 1)
        inParam.iHWReg.tdshp_en = m_dsConfig->ENABLE;
    if (m_dsConfig->DS_ENABLE == 0 || m_dsConfig->DS_ENABLE == 1)
        m_pDSFW->pDSReg->DS_en = m_dsConfig->DS_ENABLE;
    if (m_dsConfig->ISO_ENABLE == 0 || m_dsConfig->ISO_ENABLE == 1)
        m_pDSFW->pDSReg->iISO_en = m_dsConfig->ISO_ENABLE;

    /* check UR force-enable flag */
    if (m_dsConfig->UR_ENABLE == 0 || m_dsConfig->UR_ENABLE == 1)
        m_pDSFW->pDSReg->iUltraRes_en = m_dsConfig->UR_ENABLE;
#ifndef SUPPORT_VIDEO_UR
    else if (scenario != MEDIA_ISP_PREVIEW && scenario != MEDIA_ISP_CAPTURE)
    {
        /* disable UR due to feature phaseout. */
        m_pDSFW->pDSReg->iUltraRes_en = 0;
    }
#endif

    tuningDSInput(&inParam, scenario);
    tuningDSSWReg(scenario);

    if (m_dsConfig->ispTuningFlag != 0)
    {
        PQIspTuning *pPQIspTuning = PQIspTuning::getInstance();

        if (pPQIspTuning != NULL)
        {
            if (pPQIspTuning->loadIspTuningFile() == true)
            {
                pPQIspTuning->getDSTuningValue(m_pDSFW->pDSReg, &inParam);
            }
        }
    }
    PQ_TIMER_GET_CURRENT_TIME(time_e);
    PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d[2]);
    PQ_TIMER_GET_CURRENT_TIME(time_s);

#ifdef SUPPORT_GAME_DRE
    uint32_t DSInfo[GAME_SW_REG_END - TDSHP_GAIN_HIGH];
    if ((PQParam.u.video.videoScenario == INFO_GAME) && (pPQSession->getDSReg(&PQParam, DSInfo)))
    {
        m_pDSFW->pDSReg->iUltraRes_en = 0;
        inParam.iHWReg.tdshp_gain_mid = DSInfo[0];
        inParam.iHWReg.tdshp_gain_high = DSInfo[1];
        inParam.iHWReg.tdshp_gain = DSInfo[2];
    }
#endif

    onCalculate(&inParam, &outParam);

    PQ_TIMER_GET_CURRENT_TIME(time_e);
    PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d[3]);
    PQ_TIMER_GET_CURRENT_TIME(time_s);

    bool bypassHWAccess = tuningDSOutput(&outParam, scenario);

    if (scenario == MEDIA_ISP_PREVIEW || scenario == MEDIA_ISP_CAPTURE)
    {
#ifdef SUPPORT_DUMP_IN_FW
        dumpFwRegInFw(&PQParam, &inParam, &outParam, m_pDSFW->pDSReg);
#else
        dumpFWReg(&PQParam, &inParam, &outParam, m_pDSFW->pDSReg);
#endif
    }

    PQ_TIMER_GET_CURRENT_TIME(time_e);
    PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d[4]);

    PQ_LOGI("[PQDSAdaptor] input sw[%d], sh[%d], dw[%d], dh[%d], gainH[%d], gainM[%d]\n", inParam.SrcWidth, inParam.SrcHeight, inParam.DstWidth, inParam.DstHeight, inParam.iHWReg.tdshp_gain_high, inParam.iHWReg.tdshp_gain_mid);
    PQ_LOGI("[PQDSAdaptor] output gainH[%d], gainM[%d]\n", outParam.iHWReg.tdshp_gain_high, outParam.iHWReg.tdshp_gain_mid);

    if (bypassHWAccess)
    {
        return;
    }

    PQ_TIMER_GET_CURRENT_TIME(time_s);

    MM_REG_WRITE(command, MDP_TDSHP_00     , outParam.iHWReg.tdshp_softcoring_gain << 0      //ISO adaptive
                                             | outParam.iHWReg.tdshp_gain_high << 8
                                             | outParam.iHWReg.tdshp_gain_mid << 16
                                             | outParam.iHWReg.tdshp_ink_sel << 24
                                             | outParam.iHWReg.tdshp_bypass_high << 29
                                             | outParam.iHWReg.tdshp_bypass_mid << 30
                                             | outParam.iHWReg.tdshp_en << 31
                                            , 0xEFFFFFFF);  // do not re-write ADAP_LUMA_BP

    MM_REG_WRITE(command, MDP_TDSHP_01     , outParam.iHWReg.tdshp_limit_ratio << 0
                                             | outParam.iHWReg.tdshp_gain << 4
                                             | outParam.iHWReg.tdshp_coring_zero << 16    //ISO adaptive
                                             | outParam.iHWReg.tdshp_coring_thr << 24     //ISO adaptive
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_TDSHP_02     , outParam.iHWReg.tdshp_coring_value << 8      //ISO adaptive
                                             | outParam.iHWReg.tdshp_bound << 16
                                             | outParam.iHWReg.tdshp_limit << 24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_TDSHP_03     , outParam.iHWReg.tdshp_sat_proc << 0
                                             | outParam.iHWReg.tdshp_ac_lpf_coe << 8
                                             | outParam.iHWReg.tdshp_clip_thr << 16
                                             | outParam.iHWReg.tdshp_clip_ratio << 24
                                             | outParam.iHWReg.tdshp_clip_en << 31
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_TDSHP_05     , outParam.iHWReg.tdshp_ylev_p048 << 0
                                             | outParam.iHWReg.tdshp_ylev_p032 << 8
                                             | outParam.iHWReg.tdshp_ylev_p016 << 16
                                             | outParam.iHWReg.tdshp_ylev_p000 << 24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_TDSHP_06     , outParam.iHWReg.tdshp_ylev_p112 << 0
                                             | outParam.iHWReg.tdshp_ylev_p096 << 8
                                             | outParam.iHWReg.tdshp_ylev_p080 << 16
                                             | outParam.iHWReg.tdshp_ylev_p064 << 24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_TDSHP_07     , outParam.iHWReg.tdshp_ylev_p176 << 0
                                             | outParam.iHWReg.tdshp_ylev_p160 << 8
                                             | outParam.iHWReg.tdshp_ylev_p144 << 16
                                             | outParam.iHWReg.tdshp_ylev_p128 << 24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_TDSHP_08     , outParam.iHWReg.tdshp_ylev_p240 << 0
                                             | outParam.iHWReg.tdshp_ylev_p224 << 8
                                             | outParam.iHWReg.tdshp_ylev_p208 << 16
                                             | outParam.iHWReg.tdshp_ylev_p192 << 24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_TDSHP_09     , outParam.iHWReg.tdshp_ylev_en << 14
                                             | outParam.iHWReg.tdshp_ylev_alpha << 16
                                             | outParam.iHWReg.tdshp_ylev_256 << 24
                                            , 0xFFFFFFFF);

    // PBC1
    MM_REG_WRITE(command, MDP_PBC_00       , outParam.iHWReg.pbc1_radius_r<< 0
                                             | outParam.iHWReg.pbc1_theta_r << 6
                                             | outParam.iHWReg.pbc1_rslope_1 << 12
                                             | outParam.iHWReg.pbc1_gain << 22
                                             | outParam.iHWReg.pbc1_lpf_en << 30
                                             | outParam.iHWReg.pbc1_en << 31
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_PBC_01       , outParam.iHWReg.pbc1_lpf_gain << 0
                                             | outParam.iHWReg.pbc1_tslope << 6
                                             | outParam.iHWReg.pbc1_radius_c << 16
                                             | outParam.iHWReg.pbc1_theta_c << 24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_PBC_02       , outParam.iHWReg.pbc1_edge_slope << 0
                                             | outParam.iHWReg.pbc1_edge_thr << 8
                                             | outParam.iHWReg.pbc1_edge_en << 14
                                             | outParam.iHWReg.pbc1_conf_gain << 16
                                             | outParam.iHWReg.pbc1_rslope << 22
                                            , 0xFFFFFFFF);
    // PBC2
    MM_REG_WRITE(command, MDP_PBC_03       , outParam.iHWReg.pbc2_radius_r<< 0
                                             | outParam.iHWReg.pbc2_theta_r << 6
                                             | outParam.iHWReg.pbc2_rslope_1 << 12
                                             | outParam.iHWReg.pbc2_gain << 22
                                             | outParam.iHWReg.pbc2_lpf_en << 30
                                             | outParam.iHWReg.pbc2_en << 31
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_PBC_04       , outParam.iHWReg.pbc2_lpf_gain << 0
                                             | outParam.iHWReg.pbc2_tslope << 6
                                             | outParam.iHWReg.pbc2_radius_c << 16
                                             | outParam.iHWReg.pbc2_theta_c << 24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_PBC_05       , outParam.iHWReg.pbc2_edge_slope << 0
                                             | outParam.iHWReg.pbc2_edge_thr << 8
                                             | outParam.iHWReg.pbc2_edge_en << 14
                                             | outParam.iHWReg.pbc2_conf_gain << 16
                                             | outParam.iHWReg.pbc2_rslope << 22
                                            , 0xFFFFFFFF);
    // PBC3
    MM_REG_WRITE(command, MDP_PBC_06       , outParam.iHWReg.pbc3_radius_r<< 0
                                             | outParam.iHWReg.pbc3_theta_r << 6
                                             | outParam.iHWReg.pbc3_rslope_1 << 12
                                             | outParam.iHWReg.pbc3_gain << 22
                                             | outParam.iHWReg.pbc3_lpf_en << 30
                                             | outParam.iHWReg.pbc3_en << 31
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_PBC_07       , outParam.iHWReg.pbc3_lpf_gain << 0
                                             | outParam.iHWReg.pbc3_tslope << 6
                                             | outParam.iHWReg.pbc3_radius_c << 16
                                             | outParam.iHWReg.pbc3_theta_c << 24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_PBC_08       , outParam.iHWReg.pbc3_edge_slope << 0
                                             | outParam.iHWReg.pbc3_edge_thr << 8
                                             | outParam.iHWReg.pbc3_edge_en << 14
                                             | outParam.iHWReg.pbc3_conf_gain << 16
                                             | outParam.iHWReg.pbc3_rslope << 22
                                            , 0xFFFFFFFF);
#ifdef TDSHP_1_1
    MM_REG_WRITE(command, MDP_DC_TWO_D_W1, 0x40106051, 0xFFFFFFFF); //color hist
    MM_REG_WRITE(command, MDP_TDSHP_CFG, 0x00000080, 0x00000080);  // always enable color_hist_en
#endif

#if DYN_CONTRAST_VERSION == 2
    MM_REG_WRITE(command, MDP_TDSHP_CFG, 0x00000400, 0x00000400);  // enable contour_hist_en
#endif

#ifdef TDSHP_2_0
    MM_REG_WRITE(command, MDP_TDSHP_10      , outParam.iHWReg.tdshp_mid_softlimit_ratio << 0
                                             | outParam.iHWReg.tdshp_mid_coring_zero << 16
                                             | outParam.iHWReg.tdshp_mid_coring_thr << 24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_TDSHP_11      , outParam.iHWReg.tdshp_mid_softcoring_gain << 0
                                             | outParam.iHWReg.tdshp_mid_coring_value << 8
                                             | outParam.iHWReg.tdshp_mid_bound << 16
                                             | outParam.iHWReg.tdshp_mid_limit << 24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_TDSHP_12      , outParam.iHWReg.tdshp_high_softlimit_ratio << 0
                                             | outParam.iHWReg.tdshp_high_coring_zero << 16
                                             | outParam.iHWReg.tdshp_high_coring_thr << 24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_TDSHP_13      , outParam.iHWReg.tdshp_high_softcoring_gain << 0
                                             | outParam.iHWReg.tdshp_high_coring_value << 8
                                             | outParam.iHWReg.tdshp_high_bound << 16
                                             | outParam.iHWReg.tdshp_high_limit << 24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_EDF_GAIN_00   , outParam.iHWReg.edf_clip_ratio_inc << 0
                                             | outParam.iHWReg.edf_edge_gain << 8
                                             | outParam.iHWReg.edf_detail_gain << 16
                                             | outParam.iHWReg.edf_flat_gain << 24
                                             | outParam.iHWReg.edf_gain_en << 31
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_EDF_GAIN_01   , outParam.iHWReg.edf_edge_th << 0
                                             | outParam.iHWReg.edf_detail_fall_th << 9
                                             | outParam.iHWReg.edf_detail_rise_th << 18
                                             | outParam.iHWReg.edf_flat_th << 25
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_EDF_GAIN_02   , outParam.iHWReg.edf_edge_slope << 0
                                             | outParam.iHWReg.edf_detail_fall_slope << 8
                                             | outParam.iHWReg.edf_detail_rise_slope << 16
                                             | outParam.iHWReg.edf_flat_slope << 24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_EDF_GAIN_03   , outParam.iHWReg.edf_edge_mono_slope << 0
                                             | outParam.iHWReg.edf_edge_mono_th << 8
                                             | outParam.iHWReg.edf_edge_mag_slope << 16
                                             | outParam.iHWReg.edf_edge_mag_th << 24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_EDF_GAIN_04   , outParam.iHWReg.edf_edge_trend_flat_mag << 8
                                             | outParam.iHWReg.edf_edge_trend_slope << 16
                                             | outParam.iHWReg.edf_edge_trend_th << 24
                                            , 0xFFFFFFFF);

    MM_REG_WRITE(command, MDP_EDF_GAIN_05   , outParam.iHWReg.edf_bld_wgt_mag << 0
                                             | outParam.iHWReg.edf_bld_wgt_mono << 8
                                             | outParam.iHWReg.edf_bld_wgt_trend << 16
                                            , 0xFFFFFFFF);

#endif // TDSHP_2_0

#ifdef TDSHP_3_0
    MM_REG_WRITE(command, MDP_POST_YLEV_00  , outParam.iHWReg.tdshp_post_ylev_p048 << 0
                                             | outParam.iHWReg.tdshp_post_ylev_p032 << 8
                                             | outParam.iHWReg.tdshp_post_ylev_p016 << 16
                                             | outParam.iHWReg.tdshp_post_ylev_p000 << 24
                                            , 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_POST_YLEV_01  , outParam.iHWReg.tdshp_post_ylev_p112 << 0
                                             | outParam.iHWReg.tdshp_post_ylev_p096 << 8
                                             | outParam.iHWReg.tdshp_post_ylev_p080 << 16
                                             | outParam.iHWReg.tdshp_post_ylev_p064 << 24
                                            , 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_POST_YLEV_02  , outParam.iHWReg.tdshp_post_ylev_p176 << 0
                                             | outParam.iHWReg.tdshp_post_ylev_p160 << 8
                                             | outParam.iHWReg.tdshp_post_ylev_p144 << 16
                                             | outParam.iHWReg.tdshp_post_ylev_p128 << 24
                                            , 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_POST_YLEV_03  , outParam.iHWReg.tdshp_post_ylev_p240 << 0
                                             | outParam.iHWReg.tdshp_post_ylev_p224 << 8
                                             | outParam.iHWReg.tdshp_post_ylev_p208 << 16
                                             | outParam.iHWReg.tdshp_post_ylev_p192 << 24
                                            , 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_POST_YLEV_04  , outParam.iHWReg.tdshp_post_ylev_en << 14
                                             | outParam.iHWReg.tdshp_post_ylev_alpha << 16
                                             | outParam.iHWReg.tdshp_post_ylev_256 << 24
                                            , 0xFFFFFFFF);
#endif // TDSHP_3_0
    PQ_TIMER_GET_CURRENT_TIME(time_e);
    PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d[5]);

    PQ_TIMER_GET_CURRENT_TIME(end);
    PQ_TIMER_GET_DURATION_IN_MS(begin, end, diff);
    if (diff > DS_CAL_REGS_THRES)
    {
        PQ_LOGD("[PQDSAdaptor] onCalcTDSHP, time %d, %d, %d, %d, %d, %d, %d ms\n",
            diff, time_d[0], time_d[1], time_d[2], time_d[3], time_d[4], time_d[5]);
    }
    else
    {
        PQ_LOGI("[PQDSAdaptor] onCalcTDSHP, time %d, %d, %d, %d, %d, %d, %d ms\n",
            diff, time_d[0], time_d[1], time_d[2], time_d[3], time_d[4], time_d[5]);
    }
}

void PQDSAdaptor::ConfigDSParamByScenario(DSInput *inParam, DpPqParam *PQParam)
{
    if (PQParam->scenario == MEDIA_ISP_PREVIEW)
    {
        inParam->inISOSpeed = PQParam->u.isp.iso ;
        inParam->VideoImgSwitch = 0;// 0:isp, 1:else

        PQ_LOGI("[PQDSAdaptor] ISP preview scenario: inParam.inISOSpeed = %d\n", inParam->inISOSpeed);
    }
    else if (PQParam->scenario == MEDIA_ISP_CAPTURE)
    {
        inParam->inISOSpeed = PQParam->u.isp.iso ;
        inParam->VideoImgSwitch = 1;// 0:isp, 1:else

        PQ_LOGI("[PQDSAdaptor] ISP capture scenario: inParam.inISOSpeed = %d\n", inParam->inISOSpeed);
    }
    else if (PQParam->scenario == MEDIA_PICTURE)
    {
        inParam->inISOSpeed = PQParam->u.image.iso;
        inParam->VideoImgSwitch = 1;// 0:isp, 1:else

        PQ_LOGI("[PQDSAdaptor] Image scenario: inParam.inISOSpeed = %d\n", inParam->inISOSpeed);
    }
    else
    {
        inParam->inISOSpeed = 0;
        inParam->VideoImgSwitch = 1;// 0:isp, 1:else

        PQ_LOGI("[PQDSAdaptor] no-iso info scenario: inParam.inISOSpeed = %d\n", inParam->inISOSpeed);
    }
}

bool PQDSAdaptor::getVideoSourceSize(DSInput *inParam, DpPqParam *PQParam)
{
    if (PQParam->scenario != MEDIA_VIDEO)
    {
        return false;
    }

    if (PQParam->u.video.grallocExtraHandle == NULL)
    {
        PQ_LOGE("[PQDSAdaptor] getVideoSourceSize(), grallocExtraHandle == NULL");
        return false;
    }

    ge_video_info_t info;
    memset(&info, 0, sizeof(ge_video_info_t));

    int32_t err = gralloc_extra_query(PQParam->u.video.grallocExtraHandle, GRALLOC_EXTRA_GET_VIDEO_INFO, &info);

    if (err != 0)
    {
        PQ_LOGE("[PQDSAdaptor] getVideoSourceSize(), GRALLOC_EXTRA_GET_VIDEO_INFO failed, err = %d", err);
        return false;
    }

    PQ_LOGI("[PQDSAdaptor] original width = [%d], height = [%d]", info.width, info.height);

    if (info.width == 0 || info.height == 0)
    {
        return false;
    }
    else
    {
        inParam->SrcWidth = info.width;
        inParam->SrcHeight = info.height;
    }

    return true;
}

void PQDSAdaptor::initDSFWparam(DS_CONFIG_T *dsConfig, DpConfig &config, DSInput *inParam, bool enable,
                                DpPqParam *PQParam)
{
    /*Query source video size for MJC case */
    if (!getVideoSourceSize(inParam, PQParam))
    {
        inParam->SrcWidth = config.inCropWidth;
        inParam->SrcHeight = config.inCropHeight;
    }

    inParam->DstWidth = config.outWidth;
    inParam->DstHeight = config.outHeight;

    PQ_LOGI("[PQDSAdaptor] inParam->SrcWidth[%d], inParam->SrcHeight[%d]\n", inParam->SrcWidth, inParam->SrcHeight);
    PQ_LOGI("[PQDSAdaptor] inParam->DstWidth[%d], inParam->DstHeight[%d]\n", inParam->DstWidth, inParam->DstHeight);

    memcpy(&(inParam->iHWReg), &(dsConfig->TDSHP), sizeof(inParam->iHWReg));
#ifdef TDSHP_2_0
    {
        inParam->iHWReg.tdshp_cboost_lmt_u = 0xFF;
        inParam->iHWReg.tdshp_cboost_lmt_l = 0x80;
        inParam->iHWReg.tdshp_cboost_en = 1;
        inParam->iHWReg.tdshp_cboost_gain = 0x80;
        inParam->iHWReg.tdshp_cboost_yconst = 0x8;
        inParam->iHWReg.tdshp_cboost_yoffset_sel = 0;
        inParam->iHWReg.tdshp_cboost_yoffset = 0;
    }
#endif
    if (enable)
    {
        /*do nothing, inParam->iHWReg.tdshp_en is custom setting*/
    }
    else
    {
        inParam->iHWReg.tdshp_en = false;
    }

    if (PQParam->scenario == MEDIA_ISP_PREVIEW || PQParam->scenario == MEDIA_ISP_CAPTURE)
    {
        inParam->iIsIspScenario = PQParam->u.isp.isIspScenario;
        inParam->iIspScenario = PQParam->u.isp.ispScenario;
    }
    else
    {
        inParam->iIsIspScenario = 0;
        inParam->iIspScenario = 0;
    }

    inParam->DebugFlagsDS = m_ispPqDebug;

    if (m_ispPqDebug != 0)
    {
        PQ_LOGD("[PQDSAdaptor] inParam->iIsIspScenario = [%d], inParam->iIspScenario = [%d]", inParam->iIsIspScenario, inParam->iIspScenario);
    }
}

void PQDSAdaptor::dumpFWReg(DpPqParam *PQParam, DSInput *input, DSOutput *output, DSReg *swReg)
{
    DP_TRACE_CALL();
    uint32_t enableDumpRegister;
    bool forceEnableDump = PQParam->u.isp.enableDump;
    char name[128] = {0};

    enableDumpRegister = DpDriver::getInstance()->getEnableDumpRegister();
    if (!enableDumpRegister && !forceEnableDump &&
        (PQParam->u.isp.p_mdpSetting == NULL || PQParam->u.isp.p_mdpSetting->buffer == NULL))
        return;

    if (PQParam->scenario == MEDIA_ISP_PREVIEW || PQParam->scenario == MEDIA_ISP_CAPTURE)
    {
        if (forceEnableDump ||
            (((enableDumpRegister == DUMP_ISP_PRV && PQParam->scenario == MEDIA_ISP_PREVIEW) ||
            (enableDumpRegister == DUMP_ISP_CAP && PQParam->scenario == MEDIA_ISP_CAPTURE) ||
            (enableDumpRegister == DUMP_ISP_PRV_CAP)) &&
            PQParam->u.isp.timestamp != 0xFFFFFFFF))
            sprintf(name, "/data/vendor/camera_dump/%09d-%04d-%04d-MDP-%s-%d-%s.mdp",
                PQParam->u.isp.timestamp,
                PQParam->u.isp.requestNo,
                PQParam->u.isp.frameNo,
                (PQParam->scenario == MEDIA_ISP_PREVIEW) ? "Prv" : "CAP",
                PQParam->u.isp.lensId,
                PQParam->u.isp.userString);
        else if (PQParam->scenario == MEDIA_ISP_CAPTURE)
            PQ_LOGI("[PQDSAdaptor] Dump Exif for ISP_CAPTURE");
        else
            return;
    }
    else
    {
        PQ_LOGI("[PQDSAdaptor] Unknown scenario = [%d]", PQParam->scenario);
        return;
    }

    char *buffer = new char[2048*4]();
    int  cnt = 0;
    char *pRegDump = NULL;

    cnt += sprintf(buffer + cnt, "DSInput:\n");

    pRegDump = (char*)input;
    for (int i = 0; i < sizeof(DSInput); i++)
    {
        cnt += sprintf(buffer + cnt, "%02X\n", pRegDump[i]);
    }

    cnt += sprintf(buffer + cnt, "DSOutput:\n");

    pRegDump = (char*)output;
    for (int i = 0; i < sizeof(DSOutput); i++)
    {
        cnt += sprintf(buffer + cnt, "%02X\n", pRegDump[i]);
    }

    cnt += sprintf(buffer + cnt, "DSReg:\n");

    pRegDump = (char*)swReg;
    for (int i = 0; i < sizeof(DSReg); i++)
    {
        cnt += sprintf(buffer + cnt, "%02X\n", pRegDump[i]);
    }

    if (!(PQParam->enable & PQ_DRE_EN))
        cnt += sprintf(buffer + cnt, "FW Reg dump end...\n");

    if (!enableDumpRegister && !forceEnableDump)
        goto EXIF;

    FILE *pFile;

    pFile = fopen(name, "ab");

    if (NULL != pFile)
    {
        fwrite(buffer, cnt, 1, pFile);

        fclose(pFile);

        PQ_LOGD("[PQDSAdaptor]Dump register to %s\n", name);
    }
    else
    {
        PQ_LOGD("[PQDSAdaptor]Open %s failed, \n", name);
    }

EXIF:
    if (PQParam->u.isp.p_mdpSetting != NULL && PQParam->u.isp.p_mdpSetting->buffer != NULL)
    {
        char* p_mdpsetting_buf = (char*)PQParam->u.isp.p_mdpSetting->buffer;
        uint32_t *mdpsetting_offset = &(PQParam->u.isp.p_mdpSetting->offset);
        uint32_t mdpsetting_max_size = PQParam->u.isp.p_mdpSetting->size;

        if (*mdpsetting_offset + cnt > mdpsetting_max_size)
        {
            PQ_LOGD("[PQDSAdaptor] mdpSetting buffer overflow\n");
            delete [] buffer;

            return;
        }

        memcpy(p_mdpsetting_buf + *mdpsetting_offset, buffer, cnt);
        *mdpsetting_offset += cnt;
    }

    delete [] buffer;
}
#ifdef SUPPORT_DUMP_IN_FW
void PQDSAdaptor::dumpFwRegInFw(DpPqParam *PQParam, DSInput *input, DSOutput *output, DSReg *swReg)
{
    DP_TRACE_CALL();
    uint32_t enableDumpRegister;
    bool forceEnableDump = PQParam->u.isp.enableDump;
    char name[128] = {0};

    enableDumpRegister = DpDriver::getInstance()->getEnableDumpRegister();
    if (!enableDumpRegister && !forceEnableDump &&
        (PQParam->u.isp.p_mdpSetting == NULL || PQParam->u.isp.p_mdpSetting->buffer == NULL))
        return;

    if (PQParam->scenario == MEDIA_ISP_PREVIEW || PQParam->scenario == MEDIA_ISP_CAPTURE)
    {
        if (forceEnableDump ||
            (((enableDumpRegister == DUMP_ISP_PRV && PQParam->scenario == MEDIA_ISP_PREVIEW) ||
            (enableDumpRegister == DUMP_ISP_CAP && PQParam->scenario == MEDIA_ISP_CAPTURE) ||
            (enableDumpRegister == DUMP_ISP_PRV_CAP)) &&
            PQParam->u.isp.timestamp != 0xFFFFFFFF))
            sprintf(name, "/data/vendor/camera_dump/%09d-%04d-%04d-MDP-%s-%d-%s.mdp",
                PQParam->u.isp.timestamp,
                PQParam->u.isp.requestNo,
                PQParam->u.isp.frameNo,
                (PQParam->scenario == MEDIA_ISP_PREVIEW) ? "Prv" : "CAP",
                PQParam->u.isp.lensId,
                PQParam->u.isp.userString);
        else if (PQParam->scenario == MEDIA_ISP_CAPTURE)
            PQ_LOGI("[PQDSAdaptor] Dump Exif for ISP_CAPTURE");
        else
            return;
    }
    else
    {
        PQ_LOGI("[PQDSAdaptor] Unknown scenario = [%d]", PQParam->scenario);
        return;
    }

    uint32_t bufferSize_mdp = 12*(sizeof(DSInput) + sizeof(DSOutput) + sizeof(DSReg));
    uint32_t bufferSize_exif = 10*(sizeof(DSInput) + sizeof(DSReg));
    char *buffer_mdp = NULL;
    char *buffer_exif = new char[bufferSize_exif];
    int cnt_mdp = 0;
    int cnt_exif = 0;

    if (enableDumpRegister || forceEnableDump)
    {
        buffer_mdp = new char[bufferSize_mdp];
    }
    else
    {
        goto EXIF;
    }

    cnt_mdp += sprintf(buffer_mdp + cnt_mdp, "____mdp_section_start____\n");
    m_pDSFW->DumpReg(input, buffer_mdp, cnt_mdp, bufferSize_mdp, sizeof(DSInput)/sizeof(uint32_t));
    m_pDSFW->DumpReg(output, buffer_mdp, cnt_mdp, bufferSize_mdp, sizeof(DSOutput)/sizeof(uint32_t));
    m_pDSFW->DumpReg(swReg, buffer_mdp, cnt_mdp, bufferSize_mdp, sizeof(DSReg)/sizeof(uint32_t));
    cnt_mdp += sprintf(buffer_mdp + cnt_mdp, "____mdp_section_end____\n");
    PQ_LOGI("[PQDSAdaptor] DumpReg: cnt_mdp[%d] bufferSize_mdp[%d]", cnt_mdp, bufferSize_mdp);

    FILE *pFile;

    pFile = fopen(name, "ab");

    if (NULL != pFile)
    {
        fwrite(buffer_mdp, cnt_mdp, 1, pFile);

        fclose(pFile);

        PQ_LOGD("[PQDSAdaptor]Dump register to %s, cnt[%d], bufferSize[%d]\n", name, cnt_mdp, bufferSize_mdp);
    }
    else
    {
        PQ_LOGD("[PQDSAdaptor]Open %s failed, \n", name);
    }

    if (buffer_mdp != NULL)
    {
        delete []buffer_mdp;
    }

EXIF:
    if (PQParam->u.isp.p_mdpSetting != NULL && PQParam->u.isp.p_mdpSetting->buffer != NULL)
    {
        char* p_mdpsetting_buf = (char*)PQParam->u.isp.p_mdpSetting->buffer;
        uint32_t *mdpsetting_offset = &(PQParam->u.isp.p_mdpSetting->offset);
        uint32_t mdpsetting_max_size = PQParam->u.isp.p_mdpSetting->size;

        cnt_exif += sprintf(buffer_exif + cnt_exif, "____mdp_section_start____\n");
        m_pDSFW->DumpReg(input, buffer_exif, cnt_exif, bufferSize_exif, sizeof(DSInput)/sizeof(uint32_t));
        cnt_exif += sprintf(buffer_exif + cnt_exif, "____mdp_section_end____\n");
        PQ_LOGI("[PQDSAdaptor] DumpReg: cnt_exif[%d] bufferSize_exif[%d]", cnt_exif, bufferSize_exif);

        if (*mdpsetting_offset + cnt_exif > mdpsetting_max_size)
        {
            PQ_LOGD("[PQDSAdaptor] mdpSetting buffer overflow\n");
            delete [] buffer_exif;

            return;
        }

        memcpy(p_mdpsetting_buf + *mdpsetting_offset, buffer_exif, cnt_exif);
        *mdpsetting_offset += cnt_exif;
    }

    delete [] buffer_exif;
}
#endif