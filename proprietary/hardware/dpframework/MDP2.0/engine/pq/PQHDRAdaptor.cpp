#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1
#include <cutils/log.h>
#include <fcntl.h>

#include <PQCommon.h>
#include <PQHDRAdaptor.h>
#include "PQTuningBuffer.h"

#include "mdp_reg_hdr.h"

#include "cust_color.h"

#if defined(HDR_MT6799)
#define HDR_GAIN_TABLE_EVEN_WRITE (12)
#define HDR_GAIN_TABLE_ODD_WRITE  (13)
#define HDR_GAIN_TABLE_UPDATE     (8)
#elif defined(HDR_MT6779)
#define HDR_GAIN_TABLE_EVEN_WRITE (14)
#define HDR_GAIN_TABLE_ODD_WRITE  (15)
#define HDR_GAIN_TABLE_UPDATE     (11)
#endif

#define HDR_DRIVER_DEBUG_MODE (1 << 0)

#define HDR_APPLY_CMD_TIMEOUT_MS       (100)
#define HDR_CAL_REGS_THRES 20

#define ARR_LEN_4BYTE(arr) (sizeof(arr) / 4)

#define MILLI_TO_NANO(v) (static_cast<nsecs_t>(v) * static_cast<nsecs_t>(1000L * 1000L))

PQHDRAdaptor* PQHDRAdaptor::s_pInstance[] = {};
PQMutex   PQHDRAdaptor::s_ALMutex;

PQHDRAdaptor* PQHDRAdaptor::getInstance(uint32_t identifier)
{
    AutoMutex lock(s_ALMutex);

    if(NULL == s_pInstance[identifier])
    {
        s_pInstance[identifier] = new PQHDRAdaptor(identifier);
        atexit(PQHDRAdaptor::destroyInstance);
    }

    return s_pInstance[identifier];
}

void PQHDRAdaptor::destroyInstance()
{
    AutoMutex lock(s_ALMutex);

    for (int identifier = 0; identifier < HDR_ENGINE_MAX_NUM; identifier++){
        if (NULL != s_pInstance[identifier])
        {
            delete s_pInstance[identifier];
            s_pInstance[identifier] = NULL;
        }
    }
}

PQHDRAdaptor::PQHDRAdaptor(uint32_t identifier)
        : m_identifier(identifier),
          m_lastWidth(0),
          m_lastHeight(0),
          PQAlgorithmAdaptor(PROXY_HDR_SWREG,
                             PROXY_HDR_INPUT,
                             PROXY_HDR_OUTPUT)
{
    PQ_LOGD("[PQHDRAdaptor] PQHDRAdaptor()... ");

    m_pHDRFW = new CPQHDRFW;
    getHDRTable();

    memset(&m_pHDRConfig, 0x0, sizeof(HDR_CONFIG_T));
    m_pHDRFW->setDebugFlag(m_pHDRConfig.debugFlag);

    memcpy(&m_initHDRFWReg, m_pHDRFW->pHDRFWReg, sizeof(HDRFWReg));
    memset(&m_lastHDRInfo, 0, sizeof(m_lastHDRInfo));
    memset(&m_lastOutput, 0, sizeof(m_lastOutput));
};

PQHDRAdaptor::~PQHDRAdaptor()
{
    PQ_LOGD("[PQHDRAdaptor] ~PQHDRAdaptor()... ");

    delete m_pHDRFW;
};

void PQHDRAdaptor::tuningHDRInput(DHDRINPUT *input, int32_t scenario)
{
    PQTuningBuffer *p_buffer = m_inputBuffer;
    unsigned int *overwritten_buffer = p_buffer->getOverWrittenBuffer();
    unsigned int *reading_buffer = p_buffer->getReadingBuffer();
    size_t copy_size = sizeof(DHDRINPUT);

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

bool PQHDRAdaptor::tuningHDROutput(DHDROUTPUT *output, int32_t scenario)
{
    PQTuningBuffer *p_buffer = m_outputBuffer;
    unsigned int *overwritten_buffer = p_buffer->getOverWrittenBuffer();
    unsigned int *reading_buffer = p_buffer->getReadingBuffer();
    size_t copy_size = sizeof(DHDROUTPUT);

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

void PQHDRAdaptor::tuningHDRSWReg(HDRFWReg *SWReg, int32_t scenario)
{
    PQTuningBuffer *p_buffer = m_swRegBuffer;
    unsigned int *overwritten_buffer = p_buffer->getOverWrittenBuffer();
    unsigned int *reading_buffer = p_buffer->getReadingBuffer();
    size_t copy_size = sizeof(HDRFWReg);

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

bool PQHDRAdaptor::getHDRTable()
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;

    // Panel nits & gamut default settings
    m_internalDispPanelSpec.panel_nits = PQ_HDR_DEFAULT_PANEL_NITS;
    m_internalDispPanelSpec.gamut = REC709;

    /* get register value from ashmem */
    for (int index = 0; index < PROXY_HDR_CUST_MAX; index++)
    {
        offset += size;
        if (index == PROXY_HDR_CUST_REG)
        {
            size = ARR_LEN_4BYTE(HDRFWReg);
            isNoError = pPQConfig->getAshmemArray(PROXY_HDR_CUST, offset, m_pHDRFW->pHDRFWReg, size);
        }
        else if (index == PROXY_HDR_CUST_PANELSPEC)
        {
            size = ARR_LEN_4BYTE(PANEL_SPEC);
            isNoError = pPQConfig->getAshmemArray(PROXY_HDR_CUST, offset, &m_internalDispPanelSpec, size);
        }

        if (isNoError < 0)
        {
            break;
        }
    }

    PQ_LOGI("[PQHDRAdaptor] panel_nits[%d], gamut[%d]\n",
        m_internalDispPanelSpec.panel_nits, m_internalDispPanelSpec.gamut);

    return (isNoError == 1) ? 1 : 0;
}

bool PQHDRAdaptor::setPQID(uint64_t id)
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;
    /* get register value from ashmem */
    for (int index = 0; index < PROXY_HDR_CUST_MAX; index++)
    {
        offset += size;
        if (index == PROXY_HDR_CUST_REG)
        {
            size = ARR_LEN_4BYTE(HDRFWReg);
        }
        else if (index == PROXY_HDR_CUST_PANELSPEC)
        {
            size = ARR_LEN_4BYTE(PANEL_SPEC);
        }
        else if (index == PROXY_HDR_PQ_ID)
        {
            size = ARR_LEN_4BYTE(uint64_t);
            isNoError = pPQConfig->setAshmemArray(PROXY_HDR_CUST, offset, &id, size);
        }

        if (isNoError < 0)
        {
            break;
        }
    }
    return (isNoError == 1) ? 1 : 0;
}

uint64_t PQHDRAdaptor::getPQID()
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;
    uint64_t id = 0;
    /* get register value from ashmem */
    for (int index = 0; index < PROXY_HDR_CUST_MAX; index++)
    {
        offset += size;
        if (index == PROXY_HDR_CUST_REG)
        {
            size = ARR_LEN_4BYTE(HDRFWReg);
        }
        else if (index == PROXY_HDR_CUST_PANELSPEC)
        {
            size = ARR_LEN_4BYTE(PANEL_SPEC);
        }
        else if (index == PROXY_HDR_PQ_ID)
        {
            size = ARR_LEN_4BYTE(uint64_t);
            isNoError = pPQConfig->getAshmemArray(PROXY_HDR_CUST, offset, &id, size);
            //PQ_LOGI("[PQHDRAdaptor] getPQID[%llx]", id);
        }
    }
    if (isNoError == 1)
    {
        return id;
    }
    else
    {
        return 0;
    }
}

bool PQHDRAdaptor::getHDROutput(DHDROUTPUT *outParam, int32_t flag)
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;
    DHDROUTPUT temp_Output;
    DHDROUTPUT zero_Output;
    memset(&zero_Output, 0x0, sizeof(DHDROUTPUT));
    /* get register value from ashmem */
    for (int index = 0; index < PROXY_HDR_CUST_MAX; index++)
    {
        offset += size;
        if (index == PROXY_HDR_CUST_REG)
        {
            size = ARR_LEN_4BYTE(HDRFWReg);
        }
        else if (index == PROXY_HDR_CUST_PANELSPEC)
        {
            size = ARR_LEN_4BYTE(PANEL_SPEC);
        }
        else if (index == PROXY_HDR_PQ_ID)
        {
            size = ARR_LEN_4BYTE(uint64_t);
        }
        else if (index == PROXY_HDR_OUTPUT_BUFFER)
        {
            size = ARR_LEN_4BYTE(DHDROUTPUT);
            isNoError = pPQConfig->getAshmemArray(PROXY_HDR_CUST, offset, &temp_Output, size);
            /* copy register to outParam */
            if ((isNoError == 1) && (memcmp(&temp_Output, &zero_Output, sizeof(DHDROUTPUT)) != 0))
            {
                if (flag == 0)
                {
                    memcpy(outParam, &temp_Output, sizeof(DHDROUTPUT));
                }
                else if (flag == 1)
                {
                    /* only copy HDRFWFeedbackMemberVariable */
                    memcpy(&outParam->HDRFW_FBMV, &temp_Output.HDRFW_FBMV, sizeof(HDRFWFeedbackMemberVariable));
                }
            }
        }

        if (isNoError < 0)
        {
            break;
        }
    }
    return (isNoError == 1) ? 1 : 0;
}

bool PQHDRAdaptor::setHDROutput(DHDROUTPUT *outParam)
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;
    /* set register value from ashmem */
    for (int index = 0; index < PROXY_HDR_CUST_MAX; index++)
    {
        offset += size;
        if (index == PROXY_HDR_CUST_REG)
        {
            size = ARR_LEN_4BYTE(HDRFWReg);
        }
        else if (index == PROXY_HDR_CUST_PANELSPEC)
        {
            size = ARR_LEN_4BYTE(PANEL_SPEC);
        }
        else if (index == PROXY_HDR_PQ_ID)
        {
            size = ARR_LEN_4BYTE(uint64_t);
        }
        else if (index == PROXY_HDR_OUTPUT_BUFFER)
        {
            size = ARR_LEN_4BYTE(DHDROUTPUT);
            isNoError = pPQConfig->setAshmemArray(PROXY_HDR_CUST, offset, outParam, size);
        }

        if (isNoError < 0)
        {
            break;
        }
    }
    return (isNoError == 1) ? 1 : 0;
}

bool PQHDRAdaptor::calRegs(PQSession* pPQSession, DpCommand &command, DpConfig &config,
    int32_t *pFrameConfigLabel, const bool curveOnly)
{
    PQTimeValue    begin;
    PQTimeValue    end;
    int32_t        diff;
    PQTimeValue    time_s;
    PQTimeValue    time_e;
    int32_t        time_d[2];
    PQ_TIMER_GET_CURRENT_TIME(begin);
    DP_VDEC_DRV_COLORDESC_T HDRInfo;
    memset(&HDRInfo, 0, sizeof(DP_VDEC_DRV_COLORDESC_T));

    DpPqConfig* PQConfig;
    pPQSession->getDpPqConfig(&PQConfig);

    DpPqParam  PqParam;
    pPQSession->getPQParam(&PqParam);

    DHDRINPUT inParam;
    memset(&inParam, 0, sizeof(DHDRINPUT));

    DHDROUTPUT outParam;
    memset(&outParam, 0, sizeof(DHDROUTPUT));

    int32_t scenario = PqParam.scenario;

    bool hdrEnable = false;
    bool ccorrEnable = false;
    if (PQConfig->enHDR)
    {
        hdrEnable |= pPQSession->getHDRInfo(&PqParam, &HDRInfo);
    }
#if defined(HDR_MT6779)
    ccorrEnable |= PQConfig->enCcorr;
#endif
    bool isDebugMode = false;
    PQ_LOGI("[PQHDRAdaptor] hdrEnable[%d], scenario[%d]\n", hdrEnable, scenario);

    {
        AutoMutex lock(s_ALMutex);

        command.setPQSessionID(config.pqSessionId);

        pPQSession->getHDRConfig(&m_pHDRConfig);

        if (hdrEnable == true)
        {
            if (m_pHDRConfig.ENABLE == 0 || m_pHDRConfig.ENABLE == 1)
                hdrEnable = m_pHDRConfig.ENABLE;
        }
#if defined(HDR_MT6779)
        if (ccorrEnable == true)
        {
            if (m_pHDRConfig.CCORR_ENABLE == 0 || m_pHDRConfig.CCORR_ENABLE == 1)
                ccorrEnable = m_pHDRConfig.CCORR_ENABLE;
        }
#endif

        m_pHDRFW->setDebugFlag(m_pHDRConfig.debugFlag);

        if ((m_pHDRConfig.driverDebugFlag & HDR_DRIVER_DEBUG_MODE) != 0)
        {
            isDebugMode = true;
        }

        PQ_LOGI("[PQHDRAdaptor] hdrEnable[%d], ccorrEnable[%d], debugFlag[%d], driverdebugFlag[%d]",
            hdrEnable, ccorrEnable, m_pHDRConfig.debugFlag, m_pHDRConfig.driverDebugFlag);

        if ((hdrEnable == true || ccorrEnable == true) &&
            (scenario == MEDIA_VIDEO || scenario == MEDIA_VIDEO_CODEC) &&
            isDebugMode == false)
        {
            PQ_TIMER_GET_CURRENT_TIME(time_s);
            initHDRInitParamIn(&HDRInfo, &inParam, PqParam.u.video.isHDR2SDR,
                m_pHDRConfig.externalPanelNits, config);
#if defined(HDR_MT6779)
            initCcorrInitParamIn(&inParam, ccorrEnable, PqParam);
#endif
            if (PQConfig->enCcorr)
            {
                if (m_pHDRFW->pHDRFWReg->gamut_matrix_en == 1)
                {
                    /* set ccorr matrix from custom file */
                    inParam.panel_spec.gamut = Others;
                }
                m_pHDRFW->onInitPlatform(&inParam, &outParam);
                memcpy(&m_lastOutput, &outParam, sizeof(DHDROUTPUT));
                onCalculateCore(scenario, &inParam, &outParam, config);
            }
            else if (PQConfig->enHDR)
            {
                onCalculate(pPQSession, &inParam, &outParam, config, &HDRInfo);
            }
            PQ_TIMER_GET_CURRENT_TIME(time_e);
            PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d[0]);

            if (isSEIInfoExist(&HDRInfo) || memcmp(&HDRInfo, &m_lastHDRInfo, sizeof(uint32_t) * 3) != 0)
            {
                memcpy(&m_lastHDRInfo,&HDRInfo,sizeof(m_lastHDRInfo));
            }
        }
    }

    uint32_t hdr_hw_relay = 0;
    uint32_t hdr_hw_en = 1;

    if (isDebugMode == false && ((hdrEnable == false && ccorrEnable == false) || outParam.hdr_en == 0
#if defined(HDR_MT6779)
        || outParam.hdr_relay == 1
#endif
    ))
    {
        PQ_LOGI("[PQHDRAdaptor] hdrEnable[%d], ccorrEnable[%d], outParam.hdr_en[%d], outParam.hdr_relay[%d]",
            hdrEnable, ccorrEnable, outParam.hdr_en, outParam.hdr_relay);
        hdr_hw_relay = 1;
#if defined(HDR_MT6799)
        hdr_hw_en = 0;
#endif
    }

    if (curveOnly == false)
    {
#if defined(HDR_MT6799)
        // Set MDP_HDR enable
        MM_REG_WRITE(command, MDP_HDR_BYPASS, hdr_hw_relay, 0x1);
        MM_REG_WRITE(command, MDP_HDR_TOP, hdr_hw_en, 0x1);
#elif defined(HDR_MT6779)
        // Set MDP_HDR relay mode
        MM_REG_WRITE(command, MDP_HDR_RELAY, hdr_hw_relay, 0x1);
#endif
    }

    bool bypassHWAccess = tuningHDROutput(&outParam, scenario);

    PQ_LOGI("[PQHDRAdaptor] hdr_hw_relay[%d], hdrEnable[%d], hdr_en[%d], m_pHDRConfig.debugFlag[%d], bypassHWAccess[%d]",
        hdr_hw_relay, hdrEnable, outParam.hdr_en, isDebugMode, bypassHWAccess);
    PQ_TIMER_GET_CURRENT_TIME(time_s);
    if (hdr_hw_relay == 0 && isDebugMode == false && bypassHWAccess == false)
    {
        if (curveOnly == false && PQConfig->enCcorr == 0)
        {
            onConfigHDR(command, &outParam, pFrameConfigLabel);
        }
        else if (curveOnly == true && PQConfig->enCcorr == 0)
        {
            onUpdateHDR(command, &outParam, pFrameConfigLabel);
        }
        else if (curveOnly == false && PQConfig->enCcorr == 1)
        {
            onConfigCcorr(command, &outParam);
        }
    }
    PQ_TIMER_GET_CURRENT_TIME(time_e);
    PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d[1]);
    PQ_TIMER_GET_CURRENT_TIME(end);
    PQ_TIMER_GET_DURATION_IN_MS(begin, end, diff);
    if (diff > HDR_CAL_REGS_THRES)
    {
        PQ_LOGD("[PQHDRAdaptor] calRegs, time %d, %d, %d ms\n", diff, time_d[0], time_d[1]);
    }
    else
    {
        PQ_LOGI("[PQHDRAdaptor] calRegs, time %d, %d, %d ms\n", diff, time_d[0], time_d[1]);
    }

    return (bool)hdr_hw_relay;
}

void PQHDRAdaptor::onConfigCcorr(DpCommand &command, DHDROUTPUT *outParam)
{
#if defined(HDR_MT6779)
    MM_REG_WRITE(command, MDP_HDR_TOP, outParam->input_gamma << 16
                                 | outParam->output_gamma << 24, 0x7070000);
    /* tone_map_en = 0 */
    MM_REG_WRITE(command, MDP_HDR_TONE_MDP_TOP, 0, MDP_HDR_TONE_MDP_TOP_MASK);
    /* reg_luma_gain_en = 0 */
    MM_REG_WRITE(command, MDP_HDR_A_LUMINANCE, 0, 0x00000001);
    /* reg_hist_en = 0 */
    MM_REG_WRITE(command, MDP_HDR_HIST_ADDR, 0, 0x100);

    MM_REG_WRITE(command, MDP_HDR_3x3_COEF_0, outParam->gamut_matrix_en << 0, 0x00000001);
    MM_REG_WRITE(command, MDP_HDR_3x3_COEF_1, outParam->c01 << 16
                                        | outParam->c00 << 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_HDR_3x3_COEF_2, outParam->c10 << 16
                                        | outParam->c02 << 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_HDR_3x3_COEF_3, outParam->c12 << 16
                                        | outParam->c11 << 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_HDR_3x3_COEF_4, outParam->c21 << 16
                                        | outParam->c20 << 0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_HDR_3x3_COEF_5, outParam->c22 << 0, 0x0000FFFF);

    MM_REG_WRITE(command, MDP_HDR_Y2R_09, outParam->y2r_input_format << 2, 0x4);
    MM_REG_WRITE(command, MDP_HDR_R2Y_09, outParam->r2y_output_format << 2, 0x4);
#endif
}

void PQHDRAdaptor::onConfigHDR(DpCommand &command, DHDROUTPUT *outParam, int32_t *pFrameConfigLabel)
{
    int32_t index = 0;

    /* set hist_addr to 0 at frame start and set reg_hist_en = 1 */
    MM_REG_WRITE(command, MDP_HDR_HIST_ADDR, 1 << 13 | 1 << 8 | 0 << 0, 0x0000213F, &pFrameConfigLabel[index++]);
#if defined(HDR_MT6799)
    MM_REG_WRITE(command, MDP_HDR_TOP, outParam->bt2020_in << 1
                                 | outParam->sdr_gamma << 12
                                 | outParam->BBC_gamma << 16
                                 | outParam->bt2020_const_luma << 17, 0x00031002, &pFrameConfigLabel[index++]);
#elif defined(HDR_MT6779)
    MM_REG_WRITE(command, MDP_HDR_TOP, outParam->input_gamma << 16
                                 | outParam->output_gamma << 24, 0x7070000, &pFrameConfigLabel[index++]);
#endif
    // y is config by frame, x is config by tile
    MM_REG_WRITE(command, MDP_HDR_HIST_CTRL_0,  outParam->hist_begin_y << 16, 0x1FFF0000, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_HIST_CTRL_1,  outParam->hist_end_y << 16, 0x1FFF0000, &pFrameConfigLabel[index++]);

    MM_REG_WRITE(command, MDP_HDR_3x3_COEF_0, outParam->gamut_matrix_en << 0, 0x00000001, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_3x3_COEF_1, outParam->c01 << 16
                                        | outParam->c00 << 0, 0xFFFFFFFF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_3x3_COEF_2, outParam->c10 << 16
                                        | outParam->c02 << 0, 0xFFFFFFFF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_3x3_COEF_3, outParam->c12 << 16
                                        | outParam->c11 << 0, 0xFFFFFFFF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_3x3_COEF_4, outParam->c21 << 16
                                        | outParam->c20 << 0, 0xFFFFFFFF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_3x3_COEF_5, outParam->c22 << 0, 0x0000FFFF, &pFrameConfigLabel[index++]);
#if defined(HDR_MT6799)
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P12, outParam->reg_p1 << 0
                                          | outParam->reg_p2 << 16, 0xFFFFFFFF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P34, outParam->reg_p3 << 0
                                          | outParam->reg_p4 << 16, 0xFFFFFFFF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P5,  outParam->reg_p5 << 0, 0x0000FFFF, &pFrameConfigLabel[index++]);

    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S0, outParam->reg_slope0 << 0, 0x0007FFFF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S1, outParam->reg_slope1 << 0, 0x0007FFFF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S2, outParam->reg_slope2 << 0, 0x0007FFFF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S3, outParam->reg_slope3 << 0, 0x0007FFFF, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S4, outParam->reg_slope4 << 0, 0x0007FFFF, &pFrameConfigLabel[index++]);
#elif defined(HDR_MT6779)
    MM_REG_WRITE(command, MDP_HDR_TONE_MDP_TOP, 1, MDP_HDR_TONE_MDP_TOP_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P01, outParam->reg_p1 << 0
                                          | outParam->reg_p2 << 16, MDP_HDR_TONE_MAP_P01_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P02, outParam->reg_p3 << 0
                                          | outParam->reg_p4 << 16, MDP_HDR_TONE_MAP_P02_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P03, outParam->reg_p5 << 0
                                          | outParam->reg_p6 << 16, MDP_HDR_TONE_MAP_P03_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P04, outParam->reg_p7 << 0
                                          | outParam->reg_p8 << 16, MDP_HDR_TONE_MAP_P04_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P05, outParam->reg_p9 << 0
                                          | outParam->reg_p10 << 16, MDP_HDR_TONE_MAP_P05_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P06, outParam->reg_p11 << 0
                                          | outParam->reg_p12 << 16, MDP_HDR_TONE_MAP_P06_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P07, outParam->reg_p13 << 0
                                          | outParam->reg_p14 << 16, MDP_HDR_TONE_MAP_P07_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P08, outParam->reg_p15 << 0
                                          | outParam->reg_p16 << 16, MDP_HDR_TONE_MAP_P08_MASK, &pFrameConfigLabel[index++]);

    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S00, outParam->reg_slope0 << 0, MDP_HDR_TONE_MAP_S00_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S01, outParam->reg_slope1 << 0, MDP_HDR_TONE_MAP_S01_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S02, outParam->reg_slope2 << 0, MDP_HDR_TONE_MAP_S02_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S03, outParam->reg_slope3 << 0, MDP_HDR_TONE_MAP_S03_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S04, outParam->reg_slope4 << 0, MDP_HDR_TONE_MAP_S04_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S05, outParam->reg_slope5 << 0, MDP_HDR_TONE_MAP_S05_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S06, outParam->reg_slope6 << 0, MDP_HDR_TONE_MAP_S06_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S07, outParam->reg_slope7 << 0, MDP_HDR_TONE_MAP_S07_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S08, outParam->reg_slope8 << 0, MDP_HDR_TONE_MAP_S08_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S09, outParam->reg_slope9 << 0, MDP_HDR_TONE_MAP_S09_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S10, outParam->reg_slope10 << 0, MDP_HDR_TONE_MAP_S10_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S11, outParam->reg_slope11 << 0, MDP_HDR_TONE_MAP_S11_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S12, outParam->reg_slope12 << 0, MDP_HDR_TONE_MAP_S12_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S13, outParam->reg_slope13 << 0, MDP_HDR_TONE_MAP_S13_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S14, outParam->reg_slope14 << 0, MDP_HDR_TONE_MAP_S14_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S15, outParam->reg_slope15 << 0, MDP_HDR_TONE_MAP_S15_MASK, &pFrameConfigLabel[index++]);
#endif
    MM_REG_WRITE(command, MDP_HDR_B_CHANNEL_NR, outParam->reg_filter_no << 1
                                          | outParam->reg_nr_strength << 4, 0x000000F2, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_A_LUMINANCE,  outParam->reg_luma_gain_en << 0
                                          | outParam->reg_maxRGB_weight << 4, 0x000000F1, &pFrameConfigLabel[index++]);

    MM_REG_WRITE(command, MDP_HDR_GAIN_TABLE_0, 0 << HDR_GAIN_TABLE_ODD_WRITE | 1 << HDR_GAIN_TABLE_EVEN_WRITE | 0 << 0,
                                          1 << HDR_GAIN_TABLE_ODD_WRITE | 1 << HDR_GAIN_TABLE_EVEN_WRITE | 0x7F << 0, &pFrameConfigLabel[index++]); //write even, set gain_table_addr = 0 at frame start

    for (int gainTableIndex = 0; gainTableIndex < GAIN_TABLE_DATA_MAX; gainTableIndex += 2)
    {
        MM_REG_WRITE(command, MDP_HDR_GAIN_TABLE_1, outParam->GainCurve[gainTableIndex + 1] << 16
                                              | outParam->GainCurve[gainTableIndex] << 0, 0xFFFFFFFF, &pFrameConfigLabel[index++]);
    }

    MM_REG_WRITE(command, MDP_HDR_GAIN_TABLE_0, 1 << HDR_GAIN_TABLE_ODD_WRITE | 0 << HDR_GAIN_TABLE_EVEN_WRITE | 0 << 0,
                                         1 << HDR_GAIN_TABLE_ODD_WRITE | 1 << HDR_GAIN_TABLE_EVEN_WRITE | 0x7F << 0, &pFrameConfigLabel[index++]); //write odd, set gain_table_addr = 0 at frame start

    for (int gainTableIndex = 0; gainTableIndex < GAIN_TABLE_DATA_MAX; gainTableIndex += 2)
    {
        MM_REG_WRITE(command, MDP_HDR_GAIN_TABLE_1, outParam->GainCurve[gainTableIndex + 1] << 16
                                              | outParam->GainCurve[gainTableIndex] << 0, 0xFFFFFFFF, &pFrameConfigLabel[index++]);
    }

    MM_REG_WRITE(command, MDP_HDR_GAIN_TABLE_0, 1 << HDR_GAIN_TABLE_UPDATE, 1 << HDR_GAIN_TABLE_UPDATE, &pFrameConfigLabel[index++]); //set gain_table_update

#if defined(HDR_MT6779) //y2r & r2y
    MM_REG_WRITE(command, MDP_HDR_Y2R_00, outParam->y2r_c00 << 0
                        | outParam->y2r_c01 << 16, MDP_HDR_Y2R_00_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_Y2R_01, outParam->y2r_c02 << 0
                        | outParam->y2r_c10 << 16, MDP_HDR_Y2R_01_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_Y2R_02, outParam->y2r_c11 << 0
                        | outParam->y2r_c12 << 16, MDP_HDR_Y2R_02_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_Y2R_03, outParam->y2r_c20 << 0
                        | outParam->y2r_c21 << 16, MDP_HDR_Y2R_03_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_Y2R_04, outParam->y2r_c22 << 0, MDP_HDR_Y2R_04_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_Y2R_05, outParam->y2r_pre_add_0 << 0
                        | outParam->y2r_pre_add_1 << 16, MDP_HDR_Y2R_05_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_Y2R_06, outParam->y2r_pre_add_2 << 0, MDP_HDR_Y2R_06_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_Y2R_07, outParam->y2r_post_add_0 << 0
                        | outParam->y2r_post_add_1 << 16, MDP_HDR_Y2R_07_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_Y2R_08, outParam->y2r_post_add_2 << 0, MDP_HDR_Y2R_08_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_Y2R_09, outParam->y2r_input_format << 2, 0x3 << 2, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_R2Y_00, outParam->r2y_c00 << 0
                        | outParam->r2y_c01 << 16, MDP_HDR_R2Y_00_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_R2Y_01, outParam->r2y_c02 << 0
                        | outParam->r2y_c10 << 16, MDP_HDR_R2Y_01_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_R2Y_02, outParam->r2y_c11 << 0
                        | outParam->r2y_c12 << 16, MDP_HDR_R2Y_02_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_R2Y_03, outParam->r2y_c20 << 0
                        | outParam->r2y_c21 << 16, MDP_HDR_R2Y_03_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_R2Y_04, outParam->r2y_c22 << 0, MDP_HDR_R2Y_04_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_R2Y_05, outParam->r2y_pre_add_0 << 0
                        | outParam->r2y_pre_add_1 << 16, MDP_HDR_R2Y_05_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_R2Y_06, outParam->r2y_pre_add_2 << 0, MDP_HDR_R2Y_06_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_R2Y_07, outParam->r2y_post_add_0 << 0
                        | outParam->r2y_post_add_1 << 16, MDP_HDR_R2Y_07_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_R2Y_08, outParam->r2y_post_add_2 << 0, MDP_HDR_R2Y_08_MASK, &pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_R2Y_09, outParam->r2y_output_format << 2, 0x3 << 2, &pFrameConfigLabel[index++]);
#endif
}

void PQHDRAdaptor::onUpdateHDR(DpCommand &command, DHDROUTPUT *outParam, int32_t *pFrameConfigLabel)
{
    int32_t index = 0;

    /* set hist_addr to 0 at frame start and set reg_hist_en = 1 */
    MM_REG_WRITE(command, MDP_HDR_HIST_ADDR, 1 << 13 | 1 << 8 | 0 << 0, 0x0000213F, NULL, pFrameConfigLabel[index++]);
#if defined(HDR_MT6799)
    MM_REG_WRITE(command, MDP_HDR_TOP, outParam->bt2020_in << 1
                                 | outParam->sdr_gamma << 12
                                 | outParam->BBC_gamma << 16
                                 | outParam->bt2020_const_luma << 17, 0x00031002, NULL, pFrameConfigLabel[index++]);
#elif defined(HDR_MT6779)
    MM_REG_WRITE(command, MDP_HDR_TOP, outParam->input_gamma << 16
                                 | outParam->output_gamma << 24, 0x7070000, NULL, pFrameConfigLabel[index++]);
#endif
    // y is config by frame, x is config by tile
    MM_REG_WRITE(command, MDP_HDR_HIST_CTRL_0,  outParam->hist_begin_y << 16, 0x1FFF0000, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_HIST_CTRL_1,  outParam->hist_end_y << 16, 0x1FFF0000, NULL, pFrameConfigLabel[index++]);

    MM_REG_WRITE(command, MDP_HDR_3x3_COEF_0, outParam->gamut_matrix_en << 0, 0x00000001, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_3x3_COEF_1, outParam->c01 << 16
                                        | outParam->c00 << 0, 0xFFFFFFFF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_3x3_COEF_2, outParam->c10 << 16
                                        | outParam->c02 << 0, 0xFFFFFFFF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_3x3_COEF_3, outParam->c12 << 16
                                        | outParam->c11 << 0, 0xFFFFFFFF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_3x3_COEF_4, outParam->c21 << 16
                                        | outParam->c20 << 0, 0xFFFFFFFF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_3x3_COEF_5, outParam->c22 << 0, 0x0000FFFF, NULL, pFrameConfigLabel[index++]);
#if defined(HDR_MT6799)
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P12, outParam->reg_p1 << 0
                                          | outParam->reg_p2 << 16, 0xFFFFFFFF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P34, outParam->reg_p3 << 0
                                          | outParam->reg_p4 << 16, 0xFFFFFFFF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P5,  outParam->reg_p5 << 0, 0x0000FFFF, NULL, pFrameConfigLabel[index++]);

    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S0, outParam->reg_slope0 << 0, 0x0007FFFF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S1, outParam->reg_slope1 << 0, 0x0007FFFF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S2, outParam->reg_slope2 << 0, 0x0007FFFF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S3, outParam->reg_slope3 << 0, 0x0007FFFF, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S4, outParam->reg_slope4 << 0, 0x0007FFFF, NULL, pFrameConfigLabel[index++]);
#elif defined(HDR_MT6779)
    MM_REG_WRITE(command, MDP_HDR_TONE_MDP_TOP, 1, MDP_HDR_TONE_MDP_TOP_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P01, outParam->reg_p1 << 0
                                          | outParam->reg_p2 << 16, MDP_HDR_TONE_MAP_P01_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P02, outParam->reg_p3 << 0
                                          | outParam->reg_p4 << 16, MDP_HDR_TONE_MAP_P02_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P03, outParam->reg_p5 << 0
                                          | outParam->reg_p6 << 16, MDP_HDR_TONE_MAP_P03_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P04, outParam->reg_p7 << 0
                                          | outParam->reg_p8 << 16, MDP_HDR_TONE_MAP_P04_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P05, outParam->reg_p9 << 0
                                          | outParam->reg_p10 << 16, MDP_HDR_TONE_MAP_P05_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P06, outParam->reg_p11 << 0
                                          | outParam->reg_p12 << 16, MDP_HDR_TONE_MAP_P06_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P07, outParam->reg_p13 << 0
                                          | outParam->reg_p14 << 16, MDP_HDR_TONE_MAP_P07_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_P08, outParam->reg_p15 << 0
                                          | outParam->reg_p16 << 16, MDP_HDR_TONE_MAP_P08_MASK, NULL, pFrameConfigLabel[index++]);

    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S00, outParam->reg_slope0 << 0, MDP_HDR_TONE_MAP_S00_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S01, outParam->reg_slope1 << 0, MDP_HDR_TONE_MAP_S01_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S02, outParam->reg_slope2 << 0, MDP_HDR_TONE_MAP_S02_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S03, outParam->reg_slope3 << 0, MDP_HDR_TONE_MAP_S03_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S04, outParam->reg_slope4 << 0, MDP_HDR_TONE_MAP_S04_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S05, outParam->reg_slope5 << 0, MDP_HDR_TONE_MAP_S05_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S06, outParam->reg_slope6 << 0, MDP_HDR_TONE_MAP_S06_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S07, outParam->reg_slope7 << 0, MDP_HDR_TONE_MAP_S07_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S08, outParam->reg_slope8 << 0, MDP_HDR_TONE_MAP_S08_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S09, outParam->reg_slope9 << 0, MDP_HDR_TONE_MAP_S09_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S10, outParam->reg_slope10 << 0, MDP_HDR_TONE_MAP_S10_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S11, outParam->reg_slope11 << 0, MDP_HDR_TONE_MAP_S11_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S12, outParam->reg_slope12 << 0, MDP_HDR_TONE_MAP_S12_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S13, outParam->reg_slope13 << 0, MDP_HDR_TONE_MAP_S13_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S14, outParam->reg_slope14 << 0, MDP_HDR_TONE_MAP_S14_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_TONE_MAP_S15, outParam->reg_slope15 << 0, MDP_HDR_TONE_MAP_S15_MASK, NULL, pFrameConfigLabel[index++]);
#endif
    MM_REG_WRITE(command, MDP_HDR_B_CHANNEL_NR, outParam->reg_filter_no << 1
                                          | outParam->reg_nr_strength << 4, 0x000000F2, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_A_LUMINANCE,  outParam->reg_luma_gain_en << 0
                                          | outParam->reg_maxRGB_weight << 4, 0x000000F1, NULL, pFrameConfigLabel[index++]);

    MM_REG_WRITE(command, MDP_HDR_GAIN_TABLE_0, 0 << HDR_GAIN_TABLE_ODD_WRITE | 1 << HDR_GAIN_TABLE_EVEN_WRITE | 0 << 0,
                                          1 << HDR_GAIN_TABLE_ODD_WRITE | 1 << HDR_GAIN_TABLE_EVEN_WRITE | 0x7F << 0, NULL, pFrameConfigLabel[index++]); //write even, set gain_table_addr = 0 at frame start

    for (int gainTableIndex = 0; gainTableIndex < GAIN_TABLE_DATA_MAX; gainTableIndex += 2)
    {
        MM_REG_WRITE(command, MDP_HDR_GAIN_TABLE_1, outParam->GainCurve[gainTableIndex + 1] << 16
                                              | outParam->GainCurve[gainTableIndex] << 0, 0xFFFFFFFF, NULL, pFrameConfigLabel[index++]);
    }

    MM_REG_WRITE(command, MDP_HDR_GAIN_TABLE_0, 1 << HDR_GAIN_TABLE_ODD_WRITE | 0 << HDR_GAIN_TABLE_EVEN_WRITE | 0 << 0,
                                         1 << HDR_GAIN_TABLE_ODD_WRITE | 1 << HDR_GAIN_TABLE_EVEN_WRITE | 0x7F << 0, NULL, pFrameConfigLabel[index++]); //write odd, set gain_table_addr = 0 at frame start

    for (int gainTableIndex = 0; gainTableIndex < GAIN_TABLE_DATA_MAX; gainTableIndex += 2)
    {
        MM_REG_WRITE(command, MDP_HDR_GAIN_TABLE_1, outParam->GainCurve[gainTableIndex + 1] << 16
                                              | outParam->GainCurve[gainTableIndex] << 0, 0xFFFFFFFF, NULL, pFrameConfigLabel[index++]);
    }

    MM_REG_WRITE(command, MDP_HDR_GAIN_TABLE_0, 1 << HDR_GAIN_TABLE_UPDATE, 1 << HDR_GAIN_TABLE_UPDATE, NULL, pFrameConfigLabel[index++]); //set gain_table_update

#if defined(HDR_MT6779) //y2r & r2y
    MM_REG_WRITE(command, MDP_HDR_Y2R_00, outParam->y2r_c00 << 0
                        | outParam->y2r_c01 << 16, MDP_HDR_Y2R_00_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_Y2R_01, outParam->y2r_c02 << 0
                        | outParam->y2r_c10 << 16, MDP_HDR_Y2R_01_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_Y2R_02, outParam->y2r_c11 << 0
                        | outParam->y2r_c12 << 16, MDP_HDR_Y2R_02_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_Y2R_03, outParam->y2r_c20 << 0
                        | outParam->y2r_c21 << 16, MDP_HDR_Y2R_03_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_Y2R_04, outParam->y2r_c22 << 0, MDP_HDR_Y2R_04_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_Y2R_05, outParam->y2r_pre_add_0 << 0
                        | outParam->y2r_pre_add_1 << 16, MDP_HDR_Y2R_05_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_Y2R_06, outParam->y2r_pre_add_2 << 0, MDP_HDR_Y2R_06_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_Y2R_07, outParam->y2r_post_add_0 << 0
                        | outParam->y2r_post_add_1 << 16, MDP_HDR_Y2R_07_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_Y2R_08, outParam->y2r_post_add_2 << 0, MDP_HDR_Y2R_08_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_Y2R_09, outParam->y2r_input_format << 2, 0x3 << 2, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_R2Y_00, outParam->r2y_c00 << 0
                        | outParam->r2y_c01 << 16, MDP_HDR_R2Y_00_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_R2Y_01, outParam->r2y_c02 << 0
                        | outParam->r2y_c10 << 16, MDP_HDR_R2Y_01_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_R2Y_02, outParam->r2y_c11 << 0
                        | outParam->r2y_c12 << 16, MDP_HDR_R2Y_02_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_R2Y_03, outParam->r2y_c20 << 0
                        | outParam->r2y_c21 << 16, MDP_HDR_R2Y_03_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_R2Y_04, outParam->r2y_c22 << 0, MDP_HDR_R2Y_04_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_R2Y_05, outParam->r2y_pre_add_0 << 0
                        | outParam->r2y_pre_add_1 << 16, MDP_HDR_R2Y_05_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_R2Y_06, outParam->r2y_pre_add_2 << 0, MDP_HDR_R2Y_06_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_R2Y_07, outParam->r2y_post_add_0 << 0
                        | outParam->r2y_post_add_1 << 16, MDP_HDR_R2Y_07_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_R2Y_08, outParam->r2y_post_add_2 << 0, MDP_HDR_R2Y_08_MASK, NULL, pFrameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_HDR_R2Y_09, outParam->r2y_output_format << 2, 0x3 << 2, NULL, pFrameConfigLabel[index++]);
#endif
}

bool PQHDRAdaptor::isHDRinfoChanged(DP_VDEC_DRV_COLORDESC_T *HDRInfo)
{

    PQ_LOGI("[PQHDRAdaptor] m_lastHDRInfo.u4ColorPrimaries = %d", m_lastHDRInfo.u4ColorPrimaries);
    PQ_LOGI("[PQHDRAdaptor] m_lastHDRInfo.u4TransformCharacter = %d", m_lastHDRInfo.u4TransformCharacter);
    PQ_LOGI("[PQHDRAdaptor] m_lastHDRInfo.u4MatrixCoeffs = %d", m_lastHDRInfo.u4MatrixCoeffs);

    for (int index = 0; index < 3; index++)
    {
        PQ_LOGI("[PQHDRAdaptor] m_lastHDRInfo.u4DisplayPrimariesX[%d] = %d", index, m_lastHDRInfo.u4DisplayPrimariesX[index]);
        PQ_LOGI("[PQHDRAdaptor] m_lastHDRInfo.u4DisplayPrimariesY[%d] = %d", index, m_lastHDRInfo.u4DisplayPrimariesY[index]);
    }

    PQ_LOGI("[PQHDRAdaptor] m_lastHDRInfo.u4WhitePointX = %d", m_lastHDRInfo.u4WhitePointX);
    PQ_LOGI("[PQHDRAdaptor] m_lastHDRInfo.u4WhitePointY = %d", m_lastHDRInfo.u4WhitePointY);
    PQ_LOGI("[PQHDRAdaptor] m_lastHDRInfo.u4MaxDisplayMasteringLuminance = %d", m_lastHDRInfo.u4MaxDisplayMasteringLuminance);
    PQ_LOGI("[PQHDRAdaptor] m_lastHDRInfo.u4MinDisplayMasteringLuminance = %d", m_lastHDRInfo.u4MinDisplayMasteringLuminance);
    PQ_LOGI("[PQHDRAdaptor] m_lastHDRInfo.u4MaxContentLightLevel = %d", m_lastHDRInfo.u4MaxContentLightLevel);
    PQ_LOGI("[PQHDRAdaptor] m_lastHDRInfo.u4MaxPicAverageLightLevel = %d", m_lastHDRInfo.u4MaxPicAverageLightLevel);


    PQ_LOGI("[PQHDRAdaptor] HDRInfo->u4ColorPrimaries = %d", HDRInfo->u4ColorPrimaries);
    PQ_LOGI("[PQHDRAdaptor] HDRInfo->u4TransformCharacter = %d", HDRInfo->u4TransformCharacter);
    PQ_LOGI("[PQHDRAdaptor] HDRInfo->u4MatrixCoeffs = %d", HDRInfo->u4MatrixCoeffs);

    for (int index = 0; index < 3; index++)
    {
        PQ_LOGI("[PQHDRAdaptor] HDRInfo->u4DisplayPrimariesX[%d] = %d", index, HDRInfo->u4DisplayPrimariesX[index]);
        PQ_LOGI("[PQHDRAdaptor] HDRInfo->u4DisplayPrimariesY[%d] = %d", index, HDRInfo->u4DisplayPrimariesY[index]);
    }

    PQ_LOGI("[PQHDRAdaptor] HDRInfo->u4WhitePointX = %d", HDRInfo->u4WhitePointX);
    PQ_LOGI("[PQHDRAdaptor] HDRInfo->u4WhitePointY = %d", HDRInfo->u4WhitePointY);
    PQ_LOGI("[PQHDRAdaptor] HDRInfo->u4MaxDisplayMasteringLuminance = %d", HDRInfo->u4MaxDisplayMasteringLuminance);
    PQ_LOGI("[PQHDRAdaptor] HDRInfo->u4MinDisplayMasteringLuminance = %d", HDRInfo->u4MinDisplayMasteringLuminance);
    PQ_LOGI("[PQHDRAdaptor] HDRInfo->u4MaxContentLightLevel = %d", HDRInfo->u4MaxContentLightLevel);
    PQ_LOGI("[PQHDRAdaptor] HDRInfo->u4MaxPicAverageLightLevel = %d", HDRInfo->u4MaxPicAverageLightLevel);

    if (memcmp(HDRInfo, &m_lastHDRInfo, sizeof(uint32_t) * 3) != 0)
    {
        return true;
    }
    else if (memcmp(&(HDRInfo->u4DisplayPrimariesX[0]), &(m_lastHDRInfo.u4DisplayPrimariesX[0]), sizeof(uint32_t) * 12) != 0)
    {
        return isSEIInfoExist(HDRInfo);
    }

    return false;
}

bool PQHDRAdaptor::isSEIInfoExist(DP_VDEC_DRV_COLORDESC_T *HDRInfo)
{
    if (HDRInfo->u4DisplayPrimariesX[0] == 0 &&
        HDRInfo->u4DisplayPrimariesX[1] == 0 &&
        HDRInfo->u4DisplayPrimariesX[2] == 0 &&
        HDRInfo->u4DisplayPrimariesY[0] == 0 &&
        HDRInfo->u4DisplayPrimariesY[1] == 0 &&
        HDRInfo->u4DisplayPrimariesY[2] == 0 &&
        HDRInfo->u4WhitePointX == 0 &&
        HDRInfo->u4WhitePointY == 0 &&
        HDRInfo->u4MaxDisplayMasteringLuminance == 0 &&
        HDRInfo->u4MinDisplayMasteringLuminance == 0 &&
        HDRInfo->u4MaxContentLightLevel == 0 &&
        HDRInfo->u4MaxPicAverageLightLevel == 0)
        return false;
    else
        return true;
}

int32_t PQHDRAdaptor::onCalculateCore(const int32_t scenario, DHDRINPUT *input, DHDROUTPUT *output, DpConfig &config)
{
    initHDRFWinput(input, config);

    tuningHDRInput(input, scenario);

    tuningHDRSWReg(m_pHDRFW->pHDRFWReg , scenario);

    memcpy(output, &m_lastOutput, sizeof(DHDROUTPUT));
    m_pHDRFW->onCalculate(input, output);
    memcpy(&m_lastOutput, output, sizeof(DHDROUTPUT));

    PQ_LOGI("[PQHDRAdaptor] onCalculateCore complete");

    return 0;
}

int32_t PQHDRAdaptor::onCalculate(PQSession* pPQSession, DHDRINPUT *input, DHDROUTPUT *output, DpConfig &config
                                ,DP_VDEC_DRV_COLORDESC_T *HDRInfo)
{
    PQTimeValue    begin;
    PQTimeValue    end;
    int32_t        diff;
    PQTimeValue    time_s;
    PQTimeValue    time_e;
    int32_t        time_d[2];
    HDRHandle* currHDR_p;
    uint64_t id = pPQSession->getID();

    currHDR_p = pPQSession->getHDRHandle();
    checkAndResetUnusedHDRFW(currHDR_p, id);

    PQHDRRegInfoList* pWaitingHistList = currHDR_p->pWaitingHistList;
    PQHDRRegInfoList* pDoneHistList    = currHDR_p->pDoneHistList;
    DpCondition* pHistListCond = currHDR_p->pHistListCond;

    //waiting for refer frame done
    PQ_TIMER_GET_CURRENT_TIME(begin);
    PQ_TIMER_GET_CURRENT_TIME(time_s);
    while (pWaitingHistList->size() >= PQ_REFER_STEP)
    {
        PQ_LOGI("[PQHDRAdaptor] onCalculate(), id[%llx]  waiting[%d] done[%d] waiting...\n", id, pWaitingHistList->size(), pDoneHistList->size());

        PQTimeValue startTime, currTime;
        uint32_t waitTime = HDR_APPLY_CMD_TIMEOUT_MS;
        int32_t diff;
        bool isAvailable;

        PQ_TIMER_GET_CURRENT_TIME(startTime);
        do {
            pHistListCond->waitRelative(s_ALMutex, MILLI_TO_NANO(waitTime));

            PQ_TIMER_GET_CURRENT_TIME(currTime);
            PQ_TIMER_GET_DURATION_IN_MS(startTime, currTime, diff);

            if (diff >= HDR_APPLY_CMD_TIMEOUT_MS)
            {
                break;
            }
            else
            {
                waitTime = HDR_APPLY_CMD_TIMEOUT_MS - diff;
            }
        } while (currHDR_p->isAvailable == false);

        //re-get handle after waiting
        currHDR_p = pPQSession->getHDRHandle();
        pWaitingHistList = currHDR_p->pWaitingHistList;
        pDoneHistList    = currHDR_p->pDoneHistList;
        pHistListCond    = currHDR_p->pHistListCond;

        if (currHDR_p->isAvailable == false)
        {
            PQHDRRegInfoList::iterator iter = currHDR_p->pWaitingHistList->begin();

            currHDR_p->pWaitingHistList->erase(iter);
            PQ_LOGE("[PQHDRAdaptor] error: isAvailable = %d", currHDR_p->isAvailable);
            break;
        }
    }
    PQ_TIMER_GET_CURRENT_TIME(time_e);
    PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d[0]);
    PQ_LOGI("[PQHDRAdaptor] onCalculate(), id[%llx]  waiting[%d] done[%d]....\n", id, pWaitingHistList->size(), pDoneHistList->size());

    DpPqParam param;
    pPQSession->getPQParam(&param);
    uint32_t step = PQ_REFER_STEP - pWaitingHistList->size();

    //the first PQ_REFER_STEP frames or video thumbnail, just return default hdr setting
    if ((pDoneHistList->size() < step) || (currHDR_p->isAvailable == false) ||
        (param.scenario == MEDIA_VIDEO_CODEC && param.enable == false))
    {
        if ((param.scenario == MEDIA_VIDEO_CODEC && param.enable == false) &&
                    param.u.video.isThumbnail)
        {
            m_pHDRFW->onInitCommon();
            m_pHDRFW->onInitPlatform(input, output);
            setHDROutput(output);
        }
        if (!isHDRinfoChanged(HDRInfo)) //different video but same hdr info
        {
            memcpy(output, &(m_lastOutput), sizeof(DHDROUTPUT));
            getHDROutput(output, 0);
            PQ_LOGI("[PQHDRAdaptor] different video but same hdr info");
        }
        else
        {
            delete m_pHDRFW;

            m_pHDRFW = new CPQHDRFW;
            m_pHDRFW->setDebugFlag(m_pHDRConfig.debugFlag);
            memcpy(m_pHDRFW->pHDRFWReg, &m_initHDRFWReg, sizeof(HDRFWReg));

            tuningHDRInput(input, param.scenario);
            m_pHDRFW->onInitPlatform(input, output);
            getHDROutput(output, 0);
            PQ_LOGI("[PQHDRAdaptor] first frame!  owner[%llx]", id);
        }

        setPQID(id);
        memcpy(&m_lastOutput, output, sizeof(DHDROUTPUT));
        memcpy(&input->HDRFW_FBMV, &m_lastOutput.HDRFW_FBMV, sizeof(HDRFWFeedbackMemberVariable));

    }
    else
    {
        PQ_TIMER_GET_CURRENT_TIME(time_s);
        uint64_t pre_id = getPQID();
        PQHDRRegInfoList::iterator iter = pDoneHistList->begin();
        std::advance(iter, pDoneHistList->size() - step);

        memcpy(&(input->RGBmaxHistogram_1), &(iter->hist), HDR_TOTAL_HISTOGRAM_NUM * sizeof(uint32_t));
        input->iHWReg.UPpos = ((iter->LetterBoxPos & UPposMask) >> UPposShift);
        input->iHWReg.DNpos = ((iter->LetterBoxPos & DNposMask) >> DNposShift);

        iter->ref--;

        if(iter->ref == 0)
        {
            pDoneHistList->erase(iter);
        }

        if (pre_id != 0 && pre_id != id)
        {
            getHDROutput(output, 1);
            memcpy(&input->HDRFW_FBMV, &output->HDRFW_FBMV, sizeof(HDRFWFeedbackMemberVariable));
            setPQID(id);
        }
        else
        {
            memcpy(&input->HDRFW_FBMV, &m_lastOutput.HDRFW_FBMV, sizeof(HDRFWFeedbackMemberVariable));
        }

        onCalculateCore(param.scenario, input, output, config);
        setHDROutput(output);

        PQ_TIMER_GET_CURRENT_TIME(time_e);
        PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d[1]);

    }
    //push one node into waiting list
    PQHDRRegInfo HDRRegInfo;
    HDRRegInfo.ref = 1;//reference 1 by 1
    pWaitingHistList->push_back(HDRRegInfo);
    currHDR_p->isAvailable = false;

    // update time of curr instance
    PQ_TIMER_GET_CURRENT_TIME(currHDR_p->workTime);
    PQ_TIMER_GET_CURRENT_TIME(end);
    PQ_TIMER_GET_DURATION_IN_MS(begin, end, diff);
    if (diff > HDR_CAL_REGS_THRES)
    {
        PQ_LOGD("[PQHDRAdaptor] onCalculate, time %d, %d, %d ms\n", diff, time_d[0], time_d[1]);
    }
    else
    {
        PQ_LOGI("[PQHDRAdaptor] onCalculate, time %d, %d, %d ms\n", diff, time_d[0], time_d[1]);
    }

    return 0;
}

void PQHDRAdaptor::checkAndResetUnusedHDRFW(HDRHandle* currHDR_p, uint64_t id)
{
    PQTimeValue currTime;
    PQ_TIMER_GET_CURRENT_TIME(currTime);
    int32_t diff;

    PQ_TIMER_GET_DURATION_IN_MS(currHDR_p->workTime, currTime, diff);

    /*if HDR object is unused for a while, treat it as different video content and reset
      HDR related members, or the continuity of histogram may be wrong. */
    if (diff >= HDR_NEW_CLIP_TIME_INTERVAL)
    {
        /*reset HDR FW object, it will be newed later in first frame condition*/
        //delete currHDR_p->pHDRFW;
        //currHDR_p->pHDRFW = NULL;

        currHDR_p->pHistListCond->signal();

        /*reset DC lists, it wll trigger first frame condition later*/
        delete currHDR_p->pWaitingHistList;
        delete currHDR_p->pDoneHistList;
        delete currHDR_p->pHistListCond;

        currHDR_p->pWaitingHistList = new PQHDRRegInfoList;
        currHDR_p->pDoneHistList = new PQHDRRegInfoList;
        currHDR_p->pHistListCond = new DpCondition();

        PQ_LOGI("[PQHDRAdaptor] checkAndResetUnusedHDRFW(), id[%llx], time diff[%d]", id, diff);
    }
}

void PQHDRAdaptor::initHDRFWinput(DHDRINPUT *input, DpConfig &config)
{
#if defined(HDR_MT6799)
    input->iHWReg.sdr_gamma = m_lastOutput.sdr_gamma;
    input->iHWReg.BBC_gamma = m_lastOutput.BBC_gamma;
#elif defined(HDR_MT6779)
    input->iHWReg.input_gamma = m_lastOutput.input_gamma;
#endif
    input->iHWReg.reg_hist_en = m_lastOutput.reg_hist_en;
    input->iHWReg.lbox_det_en = 1;

    input->cwidth = config.inCropWidth;
    input->cheight = config.inCropHeight;
    input->resolution_change = 0;

    PQ_LOGI("[PQHDRAdaptor] input->cwidth = %d, input->cheight = %d", input->cwidth, input->cheight);
    PQ_LOGI("[PQHDRAdaptor] m_lastWidth = %d, m_lastHeight = %d", m_lastWidth, m_lastHeight);

    if (m_lastWidth != config.inCropWidth || m_lastHeight != config.inCropHeight)
    {
        input->resolution_change = 1;
        m_lastWidth = config.inCropWidth;
        m_lastHeight = config.inCropHeight;
    }
}

void PQHDRAdaptor::initHDRInitParamIn(DP_VDEC_DRV_COLORDESC_T *HDRInfo, DHDRINPUT *input,
                            bool isHDR2SDR, uint32_t externalPanelNits, DpConfig &config)
{
    memcpy(&(input->HDR2SDR_STMDInfo), HDRInfo, sizeof(DP_VDEC_DRV_COLORDESC_T));

    if (isHDR2SDR)
    {
        /*external display*/
        input->panel_spec.panel_nits = externalPanelNits;
        input->panel_spec.gamut = REC709; /*always set REC709 for external display*/
    }
    else
    {
        /*internal display*/
        input->panel_spec.panel_nits = m_internalDispPanelSpec.panel_nits;
        input->panel_spec.gamut = m_internalDispPanelSpec.gamut;
    }

    m_pHDRFW->pHDRFWReg->panel_spec.panel_nits = input->panel_spec.panel_nits;

    PQ_LOGI("[PQHDRAdaptor] isHDR2SDR = %d, panel_nits = %d, panel_spec.gamut = %d", isHDR2SDR, input->panel_spec.panel_nits, input->panel_spec.gamut);
    PQ_LOGI("[PQHDRAdaptor] externalPanelNits = %d, m_internalDispPanelSpec.panel_nits = %d", externalPanelNits, m_internalDispPanelSpec.panel_nits);
}

void PQHDRAdaptor::initCcorrInitParamIn(DHDRINPUT *input, bool ccorr_en, DpPqParam &PqParam)
{
    PQ_LOGI("[PQHDRAdaptor] ccorr_en = %d", ccorr_en);
    input->ccorr_en = ccorr_en;

    if (ccorr_en == false)
        return;

    PQ_LOGI("[PQHDRAdaptor] input u4ColorPrimaries = %d, PqParam.srcGamut = %d ",
        input->HDR2SDR_STMDInfo.u4ColorPrimaries, PqParam.srcGamut);
    /* Forcely change u4ColorPrimaries when ccorr_en is true*/
    input->HDR2SDR_STMDInfo.u4ColorPrimaries = PqParam.srcGamut;
}
