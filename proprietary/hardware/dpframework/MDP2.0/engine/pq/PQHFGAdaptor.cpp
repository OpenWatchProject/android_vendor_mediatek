#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1
#include <cutils/properties.h>
#include <cutils/log.h>
#include <fcntl.h>

#include <PQCommon.h>
#include <PQHFGAdaptor.h>
#include "PQTuningBuffer.h"
#include "PQIspTuning.h"
#include "mdp_reg_tdshp.h"

#include <cutils/properties.h>
#define ISP_DEBUG "persist.vendor.sys.isp.pqhfg.debug"
#define ISP_HFG_DEBUG "persist.vendor.sys.isp.adaptivehfg.debug"
#define ARR_LEN_4BYTE(arr) (sizeof(arr) / 4)

PQHFGAdaptor* PQHFGAdaptor::s_pInstance[] = {};
PQMutex   PQHFGAdaptor::s_ALMutex;

PQHFGAdaptor* PQHFGAdaptor::getInstance(uint32_t identifier)
{
    AutoMutex lock(s_ALMutex);

    if(NULL == s_pInstance[identifier])
    {
        s_pInstance[identifier] = new PQHFGAdaptor(identifier);
        atexit(PQHFGAdaptor::destroyInstance);
    }

    return s_pInstance[identifier];
}

void PQHFGAdaptor::destroyInstance()
{
    AutoMutex lock(s_ALMutex);

    for (int identifier = 0; identifier < HFG_ENGINE_MAX_NUM; identifier++){
        if (NULL != s_pInstance[identifier])
        {
            delete s_pInstance[identifier];
            s_pInstance[identifier] = NULL;
        }
    }
}

PQHFGAdaptor::PQHFGAdaptor(uint32_t identifier)
        : PQAlgorithmAdaptor(PROXY_HFG_SWREG,
                             PROXY_HFG_INPUT,
                             PROXY_HFG_OUTPUT),
          m_pAdaptiveHFGFW(NULL),
          m_identifier(identifier)
{
    PQ_LOGD("[PQHFGAdaptor] PQHFGAdaptor()... ");

    CPQDSFW *InputPQDSFW = new CPQDSFW;
    m_pAdaptiveHFGFW = new TAdaptiveHFG(InputPQDSFW);
    char c_isPqDebug[PROPERTY_VALUE_MAX];
    property_get(ISP_DEBUG, c_isPqDebug, "0");
    m_ispPqDebug = atoi(c_isPqDebug);
    property_get(ISP_HFG_DEBUG, c_isPqDebug, "0");
    m_pAdaptiveHFGFW->AdaptiveHFGReg->DebugFlag= atoi(c_isPqDebug);
    memset(&m_HFGConfig, 0x0, sizeof(HFG_CONFIG_T));
};

PQHFGAdaptor::~PQHFGAdaptor()
{
    PQ_LOGD("[PQHFGAdaptor] ~PQHFGAdaptor()... ");

    delete m_pAdaptiveHFGFW;
};

bool PQHFGAdaptor::calRegs(PQSession* pPQSession, DpCommand &command, DpConfig &config)
{
    uint32_t hfg_enable = 0;
    HFGInput_frame inParam;
    HFGOutput_frame outParam;
    unsigned int ispTuningFlag = 0;
    memset(&inParam, 0, sizeof(inParam));
    memset(&outParam, 0, sizeof(outParam));

    AutoMutex lock(s_ALMutex);

    pPQSession->getHFGConfig(&m_HFGConfig);

    DpPqConfig* PQConfig;
    pPQSession->getDpPqConfig(&PQConfig);

    DpPqParam PQParam;
    pPQSession->getPQParam(&PQParam);
    /* check enable flag from setPQparameter */
    if (PQConfig->enHFG == 0 || PQConfig->enHFG == 1)
        hfg_enable = PQConfig->enHFG;

    /* check debug system property*/
    if (m_HFGConfig.ENABLE == 0 || m_HFGConfig.ENABLE == 1)
        hfg_enable = m_HFGConfig.ENABLE;

    PQ_LOGI("[PQHFGAdaptor] getHFGConfig enable[%d]", hfg_enable);

    /* set input/output size */
    inParam.MDP_input_width= config.inWidth;
    inParam.MDP_input_height = config.inHeight;
    inParam.MDP_output_width = config.outWidth;
    inParam.MDP_output_height = config.outHeight;
    inParam.MDP_crop_x = config.inXOffset;
    inParam.MDP_crop_y = config.inYOffset;

    PQ_LOGI("[PQHFGAdaptor] MDP_input_width[%d], MDP_input_height[%d]", inParam.MDP_input_width, inParam.MDP_input_height);
    PQ_LOGI("[PQHFGAdaptor] MDP_output_width[%d], MDP_output_height[%d]", inParam.MDP_output_width, inParam.MDP_output_height);
    PQ_LOGI("[PQHFGAdaptor] MDP_crop_x[%d], MDP_crop_y[%d]", inParam.MDP_crop_x, inParam.MDP_crop_y);

    tuningHFGInput(&inParam, PQParam.scenario);

    if (m_HFGConfig.ispTuningFlag != 0)
    {
        PQIspTuning *pPQIspTuning = PQIspTuning::getInstance();

        if (pPQIspTuning != NULL)
        {
            if (pPQIspTuning->loadIspTuningFile() == true)
            {
                pPQIspTuning->getHFGTuningValue(&inParam);
                ispTuningFlag = 1;
            }
        }
    }

    /* call ALGO FW */
    if (PQParam.scenario == MEDIA_ISP_PREVIEW || PQParam.scenario == MEDIA_ISP_CAPTURE)
    {
        m_pAdaptiveHFGFW->onCalculateFrame(PQParam.u.isp.iso,
            PQParam.u.isp.dpHFGParam.upperISO,
            PQParam.u.isp.dpHFGParam.lowerISO,
            PQParam.u.isp.dpHFGParam.p_upperSetting,
            PQParam.u.isp.dpHFGParam.p_lowerSetting,
            PQParam.u.isp.dpHFGParam.p_slkParam,
            &inParam, &outParam, ispTuningFlag);
    }

    bool bypassHWAccess = tuningHFGOutput(&outParam, PQParam.scenario);

    if (PQParam.scenario == MEDIA_ISP_PREVIEW || PQParam.scenario == MEDIA_ISP_CAPTURE)
    {
#ifdef SUPPORT_DUMP_IN_FW
        dumpFwRegInFw(&PQParam);
#else
        dumpFWReg(&PQParam, &inParam, &outParam, m_pAdaptiveHFGFW->AdaptiveHFG_dump, m_pAdaptiveHFGFW->AdaptiveHFG_exif);
#endif
    }

    if(bypassHWAccess)
    {
        return hfg_enable;
    }

    MM_REG_WRITE(command, MDP_HFG_CTRL      , (hfg_enable&0x1) << 8 | (hfg_enable&0x1), 1<<8 | 1);

    if (hfg_enable)
    {
        MM_REG_WRITE(command, MDP_HFG_RAN_0     , (outParam.HFG_NOISE_SEED0&0xFFFF) << 16
                                                 | (outParam.HFG_NOISE_GSEED&0xFFFF)
                                                , MDP_HFG_RAN_0_MASK);
        MM_REG_WRITE(command, MDP_HFG_RAN_1     , (outParam.HFG_NOISE_SEED2&0xFFFF) << 16
                                                 | (outParam.HFG_NOISE_SEED1&0xFFFF)
                                                , MDP_HFG_RAN_1_MASK);
        MM_REG_WRITE(command, MDP_HFG_RAN_3     , (outParam.HFG_IMAGE_WD&0x1FFFFFFF)
                                                , MDP_HFG_RAN_3_MASK);
        MM_REG_WRITE(command, MDP_HFC_CON_0     , (outParam.HFC_NOISE_STD&0xFF)
                                                , MDP_HFC_CON_0_MASK);
        MM_REG_WRITE(command, MDP_HFC_LUMA_0    , (outParam.HFC_LUMA_CPX3&0xFF) << 16
                                                 | (outParam.HFC_LUMA_CPX2&0xFF) << 8
                                                 | (outParam.HFC_LUMA_CPX1&0xFF)
                                                , MDP_HFC_LUMA_0_MASK);
        MM_REG_WRITE(command, MDP_HFC_LUMA_1    , (outParam.HFC_LUMA_CPY3&0x3F) << 24
                                                 | (outParam.HFC_LUMA_CPY2&0x3F) << 16
                                                 | (outParam.HFC_LUMA_CPY1&0x3F) << 8
                                                 | (outParam.HFC_LUMA_CPY0&0x3F)
                                                , MDP_HFC_LUMA_1_MASK);
        MM_REG_WRITE(command, MDP_HFC_LUMA_2    , (outParam.HFC_LUMA_SP3&0x1F) << 24
                                                 | (outParam.HFC_LUMA_SP2&0x1F) << 16
                                                 |(outParam.HFC_LUMA_SP1&0x1F) << 8
                                                 | (outParam.HFC_LUMA_SP0&0x1F)
                                                , MDP_HFC_LUMA_2_MASK);
        MM_REG_WRITE(command, MDP_HFC_SL2_0     , (outParam.HFC_SL2_LINK_EN&0x1) << 24
                                                 | (outParam.HFC_SL2_CPX3&0xFF) << 16
                                                 |(outParam.HFC_SL2_CPX2&0xFF) << 8
                                                 | (outParam.HFC_SL2_CPX1&0xFF)
                                                , MDP_HFC_SL2_0_MASK);
        MM_REG_WRITE(command, MDP_HFC_SL2_1     , (outParam.HFC_SL2_CPY3&0x3F) << 24
                                                 | (outParam.HFC_SL2_CPY2&0x3F) << 16
                                                 |(outParam.HFC_SL2_CPY1&0x3F) << 8
                                                 | (outParam.HFC_SL2_CPY0&0x3F)
                                                , MDP_HFC_SL2_1_MASK);
        MM_REG_WRITE(command, MDP_HFC_SL2_2     , (outParam.HFC_SL2_SP3&0x1F) << 24
                                                 | (outParam.HFC_SL2_SP2&0x1F) << 16
                                                 |(outParam.HFC_SL2_SP1&0x1F) << 8
                                                 | (outParam.HFC_SL2_SP0&0x1F)
                                                , MDP_HFC_SL2_2_MASK);
        MM_REG_WRITE(command, MDP_SL2_CEN       , (outParam.SL2_CENTR_Y&0x3FFF) << 16
                                                 | (outParam.SL2_CENTR_X&0x3FFF)
                                                , MDP_SL2_CEN_MASK);
        MM_REG_WRITE(command, MDP_SL2_RR_CON0   , (outParam.SL2_R_1&0x3FFF) << 16
                                                 | (outParam.SL2_R_0&0x3FFF)
                                                , MDP_SL2_RR_CON0_MASK);
        MM_REG_WRITE(command, MDP_SL2_RR_CON1   , (outParam.SL2_GAIN_1&0xFF) << 24
                                                 | (outParam.SL2_GAIN_0&0xFF) << 16
                                                 | (outParam.SL2_R_2&0x3FFF)
                                                , MDP_SL2_RR_CON1_MASK);
        MM_REG_WRITE(command, MDP_SL2_GAIN      , (outParam.SL2_SET_ZERO&0x1) << 24
                                                 | (outParam.SL2_GAIN_4&0xFF) << 16
                                                 | (outParam.SL2_GAIN_3&0xFF) << 8
                                                 | (outParam.SL2_GAIN_2&0xFF)
                                                , MDP_SL2_GAIN_MASK);
        MM_REG_WRITE(command, MDP_SL2_RZ        , (outParam.SL2_VRZ_COMP&0x3FFF) << 16
                                                 | (outParam.SL2_HRZ_COMP&0x3FFF)
                                                , MDP_SL2_RZ_MASK);
        MM_REG_WRITE(command, MDP_SL2_SLP_CON0  , (outParam.SL2_SLP_1&0x1FFFFFFF)
                                                , MDP_SL2_SLP_CON0_MASK);
        MM_REG_WRITE(command, MDP_SL2_SLP_CON1  , (outParam.SL2_SLP_2&0x1FFFFFFF)
                                                , MDP_SL2_SLP_CON1_MASK);
        MM_REG_WRITE(command, MDP_SL2_SLP_CON2  , (outParam.SL2_SLP_3&0x1FFFFFFF)
                                                , MDP_SL2_SLP_CON2_MASK);
        MM_REG_WRITE(command, MDP_SL2_SLP_CON3  , (outParam.SL2_SLP_4&0x1FFFFFFF)
                                                , MDP_SL2_SLP_CON3_MASK);
    }

    return hfg_enable;
}

bool PQHFGAdaptor::calTileRegs(PQSession* pPQSession, DpCommand &command,
    const uint32_t tileWidth, const uint32_t tileHeight,
    const uint32_t tileXOffset, const uint32_t tileYOffset)
{
    AutoMutex lock(s_ALMutex);

    uint32_t hfg_enable = 0;
    HFGInput_tile inTileParam;
    HFGOutput_tile outTileParam;
    memset(&inTileParam, 0, sizeof(HFGInput_tile));
    memset(&outTileParam, 0, sizeof(HFGOutput_tile));

    inTileParam.tile_width = tileWidth;
    inTileParam.tile_height = tileHeight;
    inTileParam.tile_x_ofst = tileXOffset;
    inTileParam.tile_y_ofst = tileYOffset;

    pPQSession->getHFGConfig(&m_HFGConfig);

    DpPqConfig* PQConfig;
    pPQSession->getDpPqConfig(&PQConfig);

    /* check enable flag from setPQparameter */
    if (PQConfig->enHFG == 0 || PQConfig->enHFG == 1)
        hfg_enable = PQConfig->enHFG;

    /* check debug system property*/
    if (m_HFGConfig.ENABLE == 0 || m_HFGConfig.ENABLE == 1)
        hfg_enable = m_HFGConfig.ENABLE;

    if (hfg_enable)
    {
        /* call ALGO FW */
        m_pAdaptiveHFGFW->onCalculateTile(&inTileParam, &outTileParam);

        /* write register */
        MM_REG_WRITE(command, MDP_HFG_RAN_2     , (outTileParam.HFG_Y_OFST&0xFFFF) << 16
                                                 | (outTileParam.HFG_X_OFST&0xFFFF)
                                                , MDP_HFG_RAN_2_MASK);
        MM_REG_WRITE(command, MDP_HFG_RAN_4     , (outTileParam.HFG_TILE_HT&0xFFFF) << 16
                                                 | (outTileParam.HFG_TILE_WD&0xFFFF)
                                                , MDP_HFG_RAN_4_MASK);
        MM_REG_WRITE(command, MDP_HFG_CROP_X    , (outTileParam.HFG_CROP_XEND&0xFFFF) << 16
                                                 | (outTileParam.HFG_CROP_XSTART&0xFFFF)
                                                , MDP_HFG_CROP_X_MASK);
        MM_REG_WRITE(command, MDP_HFG_CROP_Y    , (outTileParam.HFG_CROP_YEND&0xFFFF) << 16
                                                 | (outTileParam.HFG_CROP_YSTART&0xFFFF)
                                                , MDP_HFG_CROP_Y_MASK);
        MM_REG_WRITE(command, MDP_SL2_XOFF      , (outTileParam.SL2_X_OFST&0x1FFFFFFF)
                                                , MDP_SL2_XOFF_MASK);
        MM_REG_WRITE(command, MDP_SL2_YOFF      , (outTileParam.SL2_Y_OFST&0x1FFFFFFF)
                                                , MDP_SL2_YOFF_MASK);
        /* write SL2_SIZE*/
        MM_REG_WRITE(command, MDP_SL2_SIZE      , (outTileParam.SL2_TPIPE_HT&0xFFFF) << 16
                                                 | (outTileParam.SL2_TPIPE_WD&0xFFFF)
                                                , MDP_SL2_SIZE_MASK);
    }

    /* dump tile register for debug */
    if ((m_ispPqDebug & HFG_DUMP_TILE) != 0)
    {
        debugDumpTileReg(&inTileParam, &outTileParam);
    }
    return true;
}

void PQHFGAdaptor::debugDumpTileReg(const HFGInput_tile *inTileParam, const HFGOutput_tile *outTileParam)
{
    PQ_LOGD("[PQHFGAdaptor] INPUT: tile_width[%d], tile_height[%d]", inTileParam->tile_width, inTileParam->tile_height);
    PQ_LOGD("[PQHFGAdaptor] INPUT: tile_x_ofst[%d], tile_y_ofst[%d]", inTileParam->tile_x_ofst, inTileParam->tile_y_ofst);

    PQ_LOGD("[PQHFGAdaptor] OUTPUT: HFG_X_OFST[%d], HFG_Y_OFST[%d]", outTileParam->HFG_X_OFST, outTileParam->HFG_Y_OFST);
    PQ_LOGD("[PQHFGAdaptor] OUTPUT: HFG_TILE_WD[%d], HFG_TILE_HT[%d]", outTileParam->HFG_TILE_WD, outTileParam->HFG_TILE_HT);
    PQ_LOGD("[PQHFGAdaptor] OUTPUT: HFG_CROP_XSTART[%d], HFG_CROP_XEND[%d]", outTileParam->HFG_CROP_XSTART, outTileParam->HFG_CROP_XEND);
    PQ_LOGD("[PQHFGAdaptor] OUTPUT: HFG_CROP_YSTART[%d], HFG_CROP_YEND[%d]", outTileParam->HFG_CROP_YSTART, outTileParam->HFG_CROP_YEND);
    PQ_LOGD("[PQHFGAdaptor] OUTPUT: SL2_X_OFST[%d], SL2_Y_OFST[%d]", outTileParam->SL2_X_OFST, outTileParam->SL2_Y_OFST);
    PQ_LOGD("[PQHFGAdaptor] OUTPUT: SL2_TPIPE_HT[%d], SL2_TPIPE_WD[%d]", outTileParam->SL2_TPIPE_HT, outTileParam->SL2_TPIPE_WD);
}

void PQHFGAdaptor::tuningHFGInput(HFGInput_frame *input, int32_t scenario)
{
    PQTuningBuffer *p_buffer = m_inputBuffer;
    unsigned int *overwritten_buffer = p_buffer->getOverWrittenBuffer();
    unsigned int *reading_buffer = p_buffer->getReadingBuffer();
    size_t copy_size = sizeof(HFGInput_frame);

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

bool PQHFGAdaptor::tuningHFGOutput(HFGOutput_frame *output, int32_t scenario)
{
    PQTuningBuffer *p_buffer = m_outputBuffer;
    unsigned int *overwritten_buffer = p_buffer->getOverWrittenBuffer();
    unsigned int *reading_buffer = p_buffer->getReadingBuffer();
    size_t copy_size = sizeof(HFGOutput_frame);

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

void PQHFGAdaptor::dumpFWReg(DpPqParam *PQParam, HFGInput_frame *input, HFGOutput_frame *output,
    TAdaptiveHFG_Dump *adaptiveHFG_Dump, TAdaptiveHFG_Exif *adaptiveHFG_Exif)
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
            PQ_LOGI("[PQHFGAdaptor] Dump Exif for ISP_CAPTURE");
        else
            return;
    }
    else
    {
        PQ_LOGI("[PQHFGAdaptor] Unknown scenario = [%d]", PQParam->scenario);
        return;
    }

    uint32_t bufferSize = 256 + 3*(sizeof(HFGInput_frame) + sizeof(HFGOutput_frame) + sizeof(TAdaptiveHFG_Dump) + sizeof(TAdaptiveHFG_Exif));
    char *buffer = new char[bufferSize];
    int  cnt = 0;
    char *pRegDump = NULL;

    cnt += sprintf(buffer + cnt, "HFGInput_frame:\n");

    pRegDump = (char*)input;
    for (int i = 0; i < (int)sizeof(HFGInput_frame); i++)
    {
        cnt += sprintf(buffer + cnt, "%02X\n", pRegDump[i]);
    }

    cnt += sprintf(buffer + cnt, "HFGOutput_frame:\n");

    pRegDump = (char*)output;
    for (int i = 0; i < (int)sizeof(HFGOutput_frame); i++)
    {
        cnt += sprintf(buffer + cnt, "%02X\n", pRegDump[i]);
    }

    cnt += sprintf(buffer + cnt, "TAdaptiveHFG_Dump:\n");

    pRegDump = (char*)adaptiveHFG_Dump;
    for (int i = 0; i < (int)sizeof(TAdaptiveHFG_Dump); i++)
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

        PQ_LOGD("[PQHFGAdaptor]Dump register to %s\n", name);
    }
    else
    {
        PQ_LOGD("[PQHFGAdaptor]Open %s failed, \n", name);
    }

EXIF:
    if (PQParam->u.isp.p_mdpSetting != NULL && PQParam->u.isp.p_mdpSetting->buffer != NULL)
    {
        char* p_mdpsetting_buf = (char*)PQParam->u.isp.p_mdpSetting->buffer;
        uint32_t *mdpsetting_offset = &(PQParam->u.isp.p_mdpSetting->offset);
        uint32_t mdpsetting_max_size = PQParam->u.isp.p_mdpSetting->size;

        cnt = 0;
        cnt += sprintf(buffer + cnt, "HFGInput_frame:\n");

        pRegDump = (char*)input;
        for (int i = 0; i < (int)sizeof(HFGInput_frame); i++)
        {
            cnt += sprintf(buffer + cnt, "%02X\n", pRegDump[i]);
        }

        cnt += sprintf(buffer + cnt, "HFGOutput_frame:\n");

        pRegDump = (char*)output;
        for (int i = 0; i < (int)sizeof(HFGOutput_frame); i++)
        {
            cnt += sprintf(buffer + cnt, "%02X\n", pRegDump[i]);
        }

        cnt += sprintf(buffer + cnt, "TAdaptiveHFG_Exif:\n");

        pRegDump = (char*)adaptiveHFG_Exif;
        for (int i = 0; i < (int)sizeof(TAdaptiveHFG_Exif); i++)
        {
            cnt += sprintf(buffer + cnt, "%02X\n", pRegDump[i]);
        }

        if (*mdpsetting_offset + cnt > mdpsetting_max_size)
        {
            PQ_LOGD("[PQHFGAdaptor] mdpSetting buffer overflow\n");
            delete [] buffer;

            return;
        }

        memcpy(p_mdpsetting_buf + *mdpsetting_offset, buffer, cnt);
        *mdpsetting_offset += cnt;
    }

    delete [] buffer;
}
#ifdef SUPPORT_DUMP_IN_FW
void PQHFGAdaptor::dumpFwRegInFw(DpPqParam *PQParam)
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
            PQ_LOGI("[PQHFGAdaptor] Dump Exif for ISP_CAPTURE");
        else
            return;
    }
    else
    {
        PQ_LOGI("[PQHFGAdaptor] Unknown scenario = [%d]", PQParam->scenario);
        return;
    }

    uint32_t bufferSize_mdp = 10*(sizeof(HFGInput_frame) + sizeof(HFGOutput_frame) + sizeof(TAdaptiveHFG_Dump));
    uint32_t bufferSize_exif = 10*(sizeof(HFGInput_frame) + sizeof(HFGOutput_frame) + sizeof(TAdaptiveHFG_Exif));
    char *buffer_mdp = NULL;
    char *buffer_exif = new char[bufferSize_exif];
    int cnt_mdp = 0;
    int cnt_exif = 0;

    if (!enableDumpRegister && !forceEnableDump)
        goto EXIF;

    buffer_mdp = new char[bufferSize_mdp];
    cnt_mdp += sprintf(buffer_mdp + cnt_mdp, "____mdp_section_start____\n");
    m_pAdaptiveHFGFW->DumpMDP(buffer_mdp, cnt_mdp, bufferSize_mdp);
    cnt_mdp += sprintf(buffer_mdp + cnt_mdp, "____mdp_section_end____\n");
    PQ_LOGI("[PQHFGAdaptor] DumpMDP, cnt_mdp[%d], bufferSize_mdp[%d]\n", cnt_mdp, bufferSize_mdp);

    FILE *pFile;

    pFile = fopen(name, "ab");

    if (NULL != pFile)
    {
        fwrite(buffer_mdp, cnt_mdp, 1, pFile);

        fclose(pFile);

        PQ_LOGD("[PQHFGAdaptor]Dump register to %s, cnt[%d], bufferSize[%d]\n", name, cnt_mdp, bufferSize_mdp);
    }
    else
    {
        PQ_LOGD("[PQHFGAdaptor]Open %s failed, \n", name);
    }

    if (buffer_mdp != NULL)
    {
        delete [] buffer_mdp;
    }

EXIF:
    if (PQParam->u.isp.p_mdpSetting != NULL && PQParam->u.isp.p_mdpSetting->buffer != NULL)
    {
        char* p_mdpsetting_buf = (char*)PQParam->u.isp.p_mdpSetting->buffer;
        uint32_t *mdpsetting_offset = &(PQParam->u.isp.p_mdpSetting->offset);
        uint32_t mdpsetting_max_size = PQParam->u.isp.p_mdpSetting->size;

        cnt_exif += sprintf(buffer_exif + cnt_exif, "____mdp_section_start____\n");
        m_pAdaptiveHFGFW->DumpExif(buffer_exif, cnt_exif, bufferSize_exif);
        cnt_exif += sprintf(buffer_exif + cnt_exif, "____mdp_section_end____\n");
        PQ_LOGI("[PQHFGAdaptor] DumpEXIF, cnt_exif[%d], bufferSize_exif[%d]\n", cnt_exif, bufferSize_exif);

        if (*mdpsetting_offset + cnt_exif > mdpsetting_max_size)
        {
            PQ_LOGD("[PQHFGAdaptor] mdpSetting buffer overflow, offset[%d], cnt[%d], mdpsetting_max_size[%d]\n",
                *mdpsetting_offset, cnt_exif, mdpsetting_max_size);
            delete [] buffer_exif;

            return;
        }

        memcpy(p_mdpsetting_buf + *mdpsetting_offset, buffer_exif, cnt_exif);
        *mdpsetting_offset += cnt_exif;
    }

    delete [] buffer_exif;
}
#endif