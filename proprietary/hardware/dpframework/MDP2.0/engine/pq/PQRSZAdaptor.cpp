#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1
#include <cutils/properties.h>
#include <cutils/log.h>
#include <fcntl.h>

#include <PQCommon.h>
#include <PQRSZAdaptor.h>
#include "PQTuningBuffer.h"
#include "mdp_reg_rsz.h"
#include "PQIspTuning.h"

#define ARR_LEN_4BYTE(arr) (sizeof(arr) / 4)

PQRSZAdaptor* PQRSZAdaptor::s_pInstance[] = {};
PQMutex   PQRSZAdaptor::s_ALMutex;

PQRSZAdaptor* PQRSZAdaptor::getInstance(uint32_t identifier)
{
    AutoMutex lock(s_ALMutex);

    if(NULL == s_pInstance[identifier])
    {
        s_pInstance[identifier] = new PQRSZAdaptor(identifier);
        atexit(PQRSZAdaptor::destroyInstance);
    }

    return s_pInstance[identifier];
}

void PQRSZAdaptor::destroyInstance()
{
    AutoMutex lock(s_ALMutex);

    for (int identifier = 0; identifier < RSZ_ENGINE_MAX_NUM; identifier++){
        if (NULL != s_pInstance[identifier])
        {
            delete s_pInstance[identifier];
            s_pInstance[identifier] = NULL;
        }
    }
}

PQRSZAdaptor::PQRSZAdaptor(uint32_t identifier)
        : PQAlgorithmAdaptor(PROXY_RSZ_SWREG,
                             PROXY_RSZ_INPUT,
                             PROXY_RSZ_OUTPUT),
          m_pRszFW(NULL),
          m_identifier(identifier)
{
    PQ_LOGD("[PQRSZAdaptor] PQRSZAdaptor()... ");

    m_pRszFW = new CPQRszFW;
    getRSZTable();
    memcpy(&m_pRszFW->m_NormalrszReg, &m_pRszFW->m_rszReg, sizeof(RszReg));
    memcpy(&m_pRszFW->m_NormalrszRegHW, &m_pRszFW->m_rszRegHW, sizeof(RszRegClearZoom));
};

PQRSZAdaptor::~PQRSZAdaptor()
{
    PQ_LOGD("[PQRSZAdaptor] ~PQRSZAdaptor()... ");
    delete m_pRszFW;
};

void PQRSZAdaptor::tuningRSZInput(RszInput *input, int32_t scenario)
{
    PQTuningBuffer *p_buffer = m_inputBuffer;
    unsigned int *overwritten_buffer = p_buffer->getOverWrittenBuffer();
    unsigned int *reading_buffer = p_buffer->getReadingBuffer();
    size_t copy_size = sizeof(RszInput);

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

bool PQRSZAdaptor::tuningRSZOutput(RszOutput *output, int32_t scenario)
{
    PQTuningBuffer *p_buffer = m_outputBuffer;
    unsigned int *overwritten_buffer = p_buffer->getOverWrittenBuffer();
    unsigned int *reading_buffer = p_buffer->getReadingBuffer();
    size_t copy_size = sizeof(RszOutput);

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

void PQRSZAdaptor::tuningRSZSWReg(int32_t scenario)
{
    PQTuningBuffer *p_buffer = m_swRegBuffer;
    unsigned int *overwritten_buffer = p_buffer->getOverWrittenBuffer();
    unsigned int *reading_buffer = p_buffer->getReadingBuffer();
    size_t copy_size = sizeof(m_pRszFW->m_rszReg);

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
        memcpy(&(m_pRszFW->m_rszReg), overwritten_buffer, copy_size);
    } else if (scenario == MEDIA_PICTURE) {
        p_buffer->resetReady();
        memcpy(reading_buffer, &(m_pRszFW->m_rszReg), copy_size);
        p_buffer->push();
    } else if (p_buffer->toBePrepared()) {
        memcpy(reading_buffer, &(m_pRszFW->m_rszReg), copy_size);
        p_buffer->push();
    }
}

bool PQRSZAdaptor::getRSZTable()
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;
    /* get register value from ashmem */
    for (int index = 0; index < PROXY_RSZ_CUST_MAX; index++)
    {
        offset += size;
        if (index == PROXY_RSZ_CUST_SWREG)
        {
            size = ARR_LEN_4BYTE(m_pRszFW->m_rszReg);
            isNoError = pPQConfig->getAshmemArray(PROXY_RSZ_CUST, offset, &m_pRszFW->m_rszReg, size);
        }
        else if (index == PROXY_RSZ_CUST_HWREG)
        {
            size = ARR_LEN_4BYTE(m_pRszFW->m_rszRegHW);
            isNoError = pPQConfig->getAshmemArray(PROXY_RSZ_CUST, offset, &m_pRszFW->m_rszRegHW, size);
        }
        else if (index == PROXY_RSZ_CUST_CZSWREG)
        {
            size = ARR_LEN_4BYTE(m_pRszFW->RszEntrySWReg);
            isNoError = pPQConfig->getAshmemArray(PROXY_RSZ_CUST, offset, &m_pRszFW->RszEntrySWReg, size);
        }
        else if (index == PROXY_RSZ_CUST_CZHWREG)
        {
            size = ARR_LEN_4BYTE(m_pRszFW->RszEntryHWReg);
            isNoError = pPQConfig->getAshmemArray(PROXY_RSZ_CUST, offset, &m_pRszFW->RszEntryHWReg, size);
        }
        else if (index == PROXY_RSZ_CUST_CZLEVEL)
        {
            size = ARR_LEN_4BYTE(m_pRszFW->RszLevel);
            isNoError = pPQConfig->getAshmemArray(PROXY_RSZ_CUST, offset, &m_pRszFW->RszLevel, size);
        }

        if (isNoError < 0)
        {
            break;
        }
    }
    return (isNoError == 1) ? 1 : 0;
}

bool PQRSZAdaptor::calRegs(PQSession* pPQSession, DpCommand &command,
     RszInput* inParam, RszOutput* outParam)
{
    AutoMutex lock(s_ALMutex);

    RSZ_CONFIG_T RSZConfig;
    int32_t scenario = MEDIA_UNKNOWN;
    DpPqParam PQParam;

    if (pPQSession != NULL)
    {
        DpPqConfig* DpPqConfig;
        pPQSession->getDpPqConfig(&DpPqConfig);
        pPQSession->getPQParam(&PQParam);
        pPQSession->getRSZConfig(&RSZConfig);
        scenario = pPQSession->getScenario();

#ifdef SUPPORT_CLEARZOOM
#ifdef SUPPORT_NVRAM_TUNING
        if (PQParam.u.isp.clearZoomParam.p_customSetting == NULL)
            PQ_LOGI("[PQRSZAdaptor] ClearZoom NVRam setting Bypass...\n");
        else
            PQ_LOGI("[PQRSZAdaptor] ClearZoom NVRam setting enabled \n");

        m_pRszFW->onLoadTable(inParam, PQParam.u.isp.clearZoomParam.p_customSetting);
#else
        m_pRszFW->onLoadTable(inParam);
#endif /* SUPPORT_NVRAM_TUNING */
#endif /* SUPPORT_CLEARZOOM */

        /* check enable flag from setPQparameter */
        if (DpPqConfig->enUR == 0 || DpPqConfig->enUR == 1)
            m_pRszFW->m_rszReg.ultraResEnable = DpPqConfig->enUR;

        /* check UR force-enable flag */
        if (RSZConfig.UR_ENABLE == 0 || RSZConfig.UR_ENABLE == 1)
            m_pRszFW->m_rszReg.ultraResEnable = RSZConfig.UR_ENABLE;
#ifndef SUPPORT_VIDEO_UR
        else if (scenario != MEDIA_ISP_PREVIEW && scenario != MEDIA_ISP_CAPTURE)
        {
            /* disable UR due to feature phaseout. */
            m_pRszFW->m_rszReg.ultraResEnable = 0;
        }
#endif

        if (DpPqConfig->enReFocus == 1)
        {
            m_pRszFW->m_rszReg.isp_RFUpTable = PQParam.u.isp.vsdofParam.defaultUpTable;
            m_pRszFW->m_rszReg.isp_RFDownTable = PQParam.u.isp.vsdofParam.defaultDownTable;
            m_pRszFW->m_rszReg.isp_RF_IBSE_gain = PQParam.u.isp.vsdofParam.IBSEGain;
            m_pRszFW->m_rszReg.isp_RF_switchRatio6Tap6nTap = PQParam.u.isp.vsdofParam.switchRatio6Tap6nTap;
            m_pRszFW->m_rszReg.isp_RF_switchRatio6nTapAcc = PQParam.u.isp.vsdofParam.switchRatio6nTapAcc;
        }
    }
    else
    {
#ifdef SUPPORT_CLEARZOOM
        m_pRszFW->onLoadTable(inParam);
#endif
    }

    if (scenario == MEDIA_ISP_PREVIEW ||
        scenario == MEDIA_ISP_CAPTURE ||
        scenario == MEDIA_VIDEO)
    {
        tuningRSZInput(inParam, scenario);
        tuningRSZSWReg(scenario);
    }


    if (pPQSession != NULL && RSZConfig.ispTuningFlag != 0)
    {
        PQIspTuning *pPQIspTuning = PQIspTuning::getInstance();

        if (pPQIspTuning != NULL)
        {
            if (pPQIspTuning->loadIspTuningFile() == true)
            {
                pPQIspTuning->getRszTuningValue(&m_pRszFW->m_rszReg, inParam);
                m_pRszFW->TuningReg2HWReg(inParam);
            }
        }
    }

    m_pRszFW->onCalculate(inParam, outParam);

    bool bypassHWAccess = false;

    if (scenario == MEDIA_ISP_PREVIEW ||
        scenario == MEDIA_ISP_CAPTURE ||
        scenario == MEDIA_VIDEO)
    {
        bypassHWAccess = tuningRSZOutput(outParam, scenario);
    }

    if (scenario == MEDIA_ISP_PREVIEW || scenario == MEDIA_ISP_CAPTURE)
    {
#ifdef SUPPORT_DUMP_IN_FW
        dumpFwRegInFw(&PQParam, inParam, outParam, &m_pRszFW->m_rszReg);
#else
        dumpFWReg(&PQParam, inParam, outParam, &m_pRszFW->m_rszReg);
#endif
    }

    if (bypassHWAccess)
    {
        return false;
    }

    PQ_LOGI("[PQRSZAdaptor] calRegs: m_identifier[%d]\n", m_identifier);
#ifdef RSZ_MT6799
    // Enable hor and ver always
    outParam->horEnable = 1;
    outParam->verEnable = 1;

    // Scaling size is 1, need to bound input
    if (inParam->cropWidth == inParam->dstWidth)
    {
        outParam->verticalFirst = 1;
    }

    // Retrieve RszOutput parameter
    uint32_t PRZ_control_1 = 0;
    uint32_t PRZ_control_2 = 0;
    uint32_t RSZ_horizontal_coeff_step = 0;
    uint32_t RSZ_vertical_coeff_step = 0;
    uint32_t RSZ_ibse_softclip = 0;
    uint32_t RSZ_tap_adapt = 0;
    uint32_t PRZ_ibse_gaincontrol_1 = 0;
    uint32_t PRZ_ibse_gaincontrol_2 = 0;
    uint32_t PRZ_ibse_ylevel_1 = 0;
    uint32_t PRZ_ibse_ylevel_2 = 0;
    uint32_t PRZ_ibse_ylevel_3 = 0;
    uint32_t PRZ_ibse_ylevel_4 = 0;
    uint32_t PRZ_ibse_ylevel_5 = 0;

    PRZ_control_1 |= outParam->horEnable << 0;//0
    PRZ_control_1 |= outParam->verEnable << 1;//1
    PRZ_control_1 |= outParam->verticalFirst << 4;//4
    PRZ_control_1 |= outParam->horAlgo << 5;//5-6
    PRZ_control_1 |= outParam->verAlgo << 7;//7-8
    PRZ_control_1 |= outParam->horTruncBit << 10;//10-12
    PRZ_control_1 |= outParam->verTruncBit << 13;//13-15
    PRZ_control_1 |= outParam->horTable << 16;//16-20
    PRZ_control_1 |= outParam->verTable << 21;//21-25

    PRZ_control_2 |= outParam->verCubicTruncEn << 27;//27
    PRZ_control_2 |= outParam->verLumaCubicTruncBit << 24;//24- 26
    PRZ_control_2 |= outParam->verChromaCubicTruncBit << 21;//21- 23
    PRZ_control_2 |= outParam->horCubicTruncEn << 20;//20
    PRZ_control_2 |= outParam->horLumaCubicTruncBit << 17;//17- 19
    PRZ_control_2 |= outParam->horChromaCubicTruncBit << 14;//14- 16
    PRZ_control_2 |= inParam->powerSavingMode << 9;//9
    PRZ_control_2 |= outParam->tapAdaptEnable << 7;//7
    PRZ_control_2 |= m_pRszFW->m_rszReg.IBSEEnable << 4;//4

    RSZ_horizontal_coeff_step = outParam->coeffStepX;
    RSZ_vertical_coeff_step = outParam->coeffStepY;

    RSZ_ibse_softclip |= outParam->IBSE_clip_ratio << 15;//15-19
    RSZ_ibse_softclip |= outParam->IBSE_clip_thr << 7;//7-14
    RSZ_ibse_softclip |= outParam->IBSEGainMid << 2;//2-6

    RSZ_tap_adapt |= outParam->TapAdaptSlope << 0;//0-3

    PRZ_ibse_gaincontrol_1 |= outParam->IBSE_gaincontrol_gain << 0;//0-7
    PRZ_ibse_gaincontrol_1 |= outParam->iHWReg.IBSE_gaincontrol_coring_value << 8;//8-15
    PRZ_ibse_gaincontrol_1 |= outParam->iHWReg.IBSE_gaincontrol_coring_thr << 16;//16-23
    PRZ_ibse_gaincontrol_1 |= outParam->iHWReg.IBSE_gaincontrol_coring_zero << 24;//24-31

    PRZ_ibse_gaincontrol_2 |= outParam->iHWReg.IBSE_gaincontrol_softlimit_ratio << 0;//0-7
    PRZ_ibse_gaincontrol_2 |= outParam->iHWReg.IBSE_gaincontrol_bound << 8;//8-15
    PRZ_ibse_gaincontrol_2 |= outParam->iHWReg.IBSE_gaincontrol_limit << 16;//16-23
    PRZ_ibse_gaincontrol_2 |= outParam->iHWReg.IBSE_gaincontrol_softcoring_gain << 24;//24-31

    PRZ_ibse_ylevel_1 |= outParam->iHWReg.IBSE_ylevel_p48 << 0;//0-7
    PRZ_ibse_ylevel_1 |= outParam->iHWReg.IBSE_ylevel_p32 << 8;//8-15
    PRZ_ibse_ylevel_1 |= outParam->iHWReg.IBSE_ylevel_p16 << 16;//16-23
    PRZ_ibse_ylevel_1 |= outParam->iHWReg.IBSE_ylevel_p0 << 24;//24-31

    PRZ_ibse_ylevel_2 |= outParam->iHWReg.IBSE_ylevel_p112 << 0;//0-7
    PRZ_ibse_ylevel_2 |= outParam->iHWReg.IBSE_ylevel_p96 << 8;//8-15
    PRZ_ibse_ylevel_2 |= outParam->iHWReg.IBSE_ylevel_p80 << 16;//16-23
    PRZ_ibse_ylevel_2 |= outParam->iHWReg.IBSE_ylevel_p64 << 24;//24-31

    PRZ_ibse_ylevel_3 |= outParam->iHWReg.IBSE_ylevel_p176 << 0;//0-7
    PRZ_ibse_ylevel_3 |= outParam->iHWReg.IBSE_ylevel_p160 << 8;//8-15
    PRZ_ibse_ylevel_3 |= outParam->iHWReg.IBSE_ylevel_p144 << 16;//16-23
    PRZ_ibse_ylevel_3 |= outParam->iHWReg.IBSE_ylevel_p128 << 24;//24-31

    PRZ_ibse_ylevel_4 |= outParam->iHWReg.IBSE_ylevel_p240 << 0;//0-7
    PRZ_ibse_ylevel_4 |= outParam->iHWReg.IBSE_ylevel_p224 << 8;//8-15
    PRZ_ibse_ylevel_4 |= outParam->iHWReg.IBSE_ylevel_p208 << 16;//16-23
    PRZ_ibse_ylevel_4 |= outParam->iHWReg.IBSE_ylevel_p192 << 24;//24-31

    PRZ_ibse_ylevel_5 |= outParam->iHWReg.IBSE_ylevel_alpha << 0;//0-5
    PRZ_ibse_ylevel_5 |= outParam->iHWReg.IBSE_ylevel_p256 << 8;//8-15

    /*
    RSZ_demo_in_hmask |= outParam->demoInHStart << 16;
    RSZ_demo_in_hmask |= outParam->demoInHEnd << 0;
    RSZ_demo_in_vmask |= outParam->demoInVStart << 16;
    RSZ_demo_in_vmask |= outParam->demoInVEnd << 0;
    */

    // Write command
    MM_REG_WRITE(command, PRZ_IBSE_SOFTCLIP,            RSZ_ibse_softclip,          0x000FFFFC);
    MM_REG_WRITE(command, PRZ_CONTROL_1,                PRZ_control_1,              0x03FFFDF3);
    MM_REG_WRITE(command, PRZ_CONTROL_2,                PRZ_control_2,              0x0FFFC290);
    MM_REG_WRITE(command, PRZ_HORIZONTAL_COEFF_STEP,    RSZ_horizontal_coeff_step,  0x007FFFFF);
    MM_REG_WRITE(command, PRZ_VERTICAL_COEFF_STEP,      RSZ_vertical_coeff_step,    0x007FFFFF);
    MM_REG_WRITE(command, PRZ_TAP_ADAPT,                RSZ_tap_adapt,              0x0000000F);

    MM_REG_WRITE(command, PRZ_IBSE_GAINCONTROL_1,       PRZ_ibse_gaincontrol_1,     0xFFFFFFFF);
    MM_REG_WRITE(command, PRZ_IBSE_GAINCONTROL_2,       PRZ_ibse_gaincontrol_2,     0xFFFFFFFF);

    MM_REG_WRITE(command, PRZ_IBSE_YLEVEL_1,            PRZ_ibse_ylevel_1,          0xFFFFFFFF);
    MM_REG_WRITE(command, PRZ_IBSE_YLEVEL_2,            PRZ_ibse_ylevel_2,          0xFFFFFFFF);
    MM_REG_WRITE(command, PRZ_IBSE_YLEVEL_3,            PRZ_ibse_ylevel_3,          0xFFFFFFFF);
    MM_REG_WRITE(command, PRZ_IBSE_YLEVEL_4,            PRZ_ibse_ylevel_4,          0xFFFFFFFF);
    MM_REG_WRITE(command, PRZ_IBSE_YLEVEL_5,            PRZ_ibse_ylevel_5,          0x0000FF3F);
#elif defined(RSZ_MT6759)
    // Enable hor and ver always
    outParam->horEnable = 1;
    outParam->verEnable = 1;

    // Retrieve RszOutput parameter
    uint32_t PRZ_control_1 = 0;
    uint32_t RSZ_horizontal_coeff_step = 0;
    uint32_t RSZ_vertical_coeff_step = 0;

    PRZ_control_1 |= outParam->horEnable << 0;//0
    PRZ_control_1 |= outParam->verEnable << 1;//1
    PRZ_control_1 |= outParam->verticalFirst << 4;//4
    PRZ_control_1 |= outParam->horAlgo << 5;//5-6
    PRZ_control_1 |= outParam->verAlgo << 7;//7-8
    PRZ_control_1 |= outParam->horTruncBit << 10;//10-12
    PRZ_control_1 |= outParam->verTruncBit << 13;//13-15
    PRZ_control_1 |= outParam->horTable << 16;//16-20
    PRZ_control_1 |= outParam->verTable << 21;//21-25

    RSZ_horizontal_coeff_step = outParam->coeffStepX;
    RSZ_vertical_coeff_step = outParam->coeffStepY;

    // Write command
    MM_REG_WRITE(command, PRZ_CONTROL_1,                PRZ_control_1,              0x03FFFDF3);
    MM_REG_WRITE(command, PRZ_HORIZONTAL_COEFF_STEP,    RSZ_horizontal_coeff_step,  0x007FFFFF);
    MM_REG_WRITE(command, PRZ_VERTICAL_COEFF_STEP,      RSZ_vertical_coeff_step,    0x007FFFFF);
#else
    // Enable hor and ver always
    outParam->horEnable = 1;
    outParam->verEnable = 1;

    // Retrieve RszOutput parameter
    uint32_t PRZ_control_1 = 0;
    uint32_t RSZ_horizontal_coeff_step = 0;
    uint32_t RSZ_vertical_coeff_step = 0;
    uint32_t RSZ_ibse = 0;
    uint32_t RSZ_tap_adapt = 0;
    //uint32_t RSZ_demo_in_hmask = 0;
    //uint32_t RSZ_demo_in_vmask = 0;

    PRZ_control_1 |= outParam->horEnable << 0;//0
    PRZ_control_1 |= outParam->verEnable << 1;//1
    PRZ_control_1 |= outParam->verticalFirst << 4;//4
    PRZ_control_1 |= outParam->horAlgo << 5;//5-6
    PRZ_control_1 |= outParam->verAlgo << 7;//7-8
    PRZ_control_1 |= outParam->horTruncBit << 10;//10-12
    PRZ_control_1 |= outParam->verTruncBit << 13;//13-15
    PRZ_control_1 |= outParam->horTable << 16;//16-20
    PRZ_control_1 |= outParam->verTable << 21;//21-25

    RSZ_horizontal_coeff_step = outParam->coeffStepX;
    RSZ_vertical_coeff_step = outParam->coeffStepY;

    RSZ_ibse |= m_pRszFW->m_rszReg.IBSEEnable << 30;//30
    RSZ_ibse |= outParam->IBSE_clip_ratio<< 15;//15-19
    RSZ_ibse |= outParam->IBSE_clip_thr<< 7;//7-14
    RSZ_ibse |= outParam->IBSEGainMid << 2;//2-6
    RSZ_tap_adapt |= outParam->tapAdaptEnable << 26;//26
    RSZ_tap_adapt |= outParam->TapAdaptSlope << 0;//0-3

    /*
    RSZ_demo_in_hmask |= outParam->demoInHStart << 16;
    RSZ_demo_in_hmask |= outParam->demoInHEnd << 0;
    RSZ_demo_in_vmask |= outParam->demoInVStart << 16;
    RSZ_demo_in_vmask |= outParam->demoInVEnd << 0;
    */

    MM_REG_WRITE(command, PRZ_IBSE,                     RSZ_ibse,                   0x400FFFFC);
    MM_REG_WRITE(command, PRZ_CONTROL_1,                PRZ_control_1,              0x03FFFDF3);
    MM_REG_WRITE(command, PRZ_HORIZONTAL_COEFF_STEP,    RSZ_horizontal_coeff_step,  0x007FFFFF);
    MM_REG_WRITE(command, PRZ_VERTICAL_COEFF_STEP,      RSZ_vertical_coeff_step,    0x007FFFFF);
    MM_REG_WRITE(command, PRZ_TAP_ADAPT,                RSZ_tap_adapt,              0x0400000F);
#endif
    return true;
}

void PQRSZAdaptor::dumpFWReg(DpPqParam *PQParam, RszInput *input, RszOutput *output, RszReg *swReg)
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
            PQ_LOGI("[PQRSZAdaptor] Dump Exif for ISP_CAPTURE");
        else
            return;
    }
    else
    {
        PQ_LOGI("[PQRSZAdaptor] Unknown scenario = [%d]", PQParam->scenario);
        return;
    }

    char *buffer = new char[2048*4]();
    int  cnt = 0;
    char *pRegDump = NULL;

    cnt += sprintf(buffer, "FW Reg dump begin...\n");

    if (m_identifier == 0)
        cnt += sprintf(buffer + cnt, "Rsz0_fw:\n");
    else if (m_identifier == 1)
        cnt += sprintf(buffer + cnt, "Rsz1_fw:\n");
    else
        PQ_LOGE("[PQRSZAdaptor] Exception: m_identifier = %d", m_identifier);

    cnt += sprintf(buffer + cnt, "RszInput:\n");

    pRegDump = (char*)input;
    for (int i = 0; i < (int)sizeof(RszInput); i++)
    {
        cnt += sprintf(buffer + cnt, "%02X\n", pRegDump[i]);
    }

    cnt += sprintf(buffer + cnt, "RszOutput:\n");
    pRegDump = (char*)output;

    for (int i = 0; i < (int)sizeof(RszOutput); i++)
    {
        cnt += sprintf(buffer + cnt, "%02X\n", pRegDump[i]);
    }

    cnt += sprintf(buffer + cnt, "RszReg:\n");
    pRegDump = (char*)swReg;
    for (int i = 0; i < (int)sizeof(RszReg); i++)
    {
        cnt += sprintf(buffer + cnt, "%02X\n", pRegDump[i]);
    }

    if (!(PQParam->enable & PQ_ULTRARES_EN) && !(PQParam->enable & PQ_DRE_EN))
        cnt += sprintf(buffer + cnt, "FW Reg dump end...\n");

    if (!enableDumpRegister && !forceEnableDump)
        goto EXIF;

    FILE *pFile;

    pFile = fopen(name, "ab");

    if (NULL != pFile)
    {
        fwrite(buffer, cnt, 1, pFile);

        fclose(pFile);

        PQ_LOGD("[PQRSZAdaptor]Dump register to %s\n", name);
    }
    else
    {
        PQ_LOGD("[PQRSZAdaptor]Open %s failed, \n", name);
    }

EXIF:
    if (PQParam->u.isp.p_mdpSetting != NULL && PQParam->u.isp.p_mdpSetting->buffer != NULL)
    {
        char* p_mdpsetting_buf = (char*)PQParam->u.isp.p_mdpSetting->buffer;
        uint32_t *mdpsetting_offset = &(PQParam->u.isp.p_mdpSetting->offset);
        uint32_t mdpsetting_max_size = PQParam->u.isp.p_mdpSetting->size;

        if (*mdpsetting_offset + cnt > mdpsetting_max_size)
        {
            PQ_LOGD("[PQRSZAdaptor] mdpSetting buffer overflow\n");
            delete [] buffer;

            return;
        }

        memcpy(p_mdpsetting_buf + *mdpsetting_offset, buffer, cnt);
        *mdpsetting_offset += cnt;
    }

    delete [] buffer;
}
#ifdef SUPPORT_DUMP_IN_FW
void PQRSZAdaptor::dumpFwRegInFw(DpPqParam *PQParam, RszInput *input, RszOutput *output, RszReg *swReg)
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
            PQ_LOGI("[PQRSZAdaptor] Dump Exif for ISP_CAPTURE");
        else
            return;
    }
    else
    {
        PQ_LOGI("[PQRSZAdaptor] Unknown scenario = [%d]", PQParam->scenario);
        return;
    }

    uint32_t bufferSize_mdp = 10*(sizeof(RszInput) + sizeof(RszOutput) + sizeof(RszReg));
    uint32_t bufferSize_exif = 10*(sizeof(RszInput) + sizeof(RszReg));
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
    if (m_identifier == 0)
        cnt_mdp += sprintf(buffer_mdp + cnt_mdp, "Rsz0_fw:\n");
    else if (m_identifier == 1)
        cnt_mdp += sprintf(buffer_mdp + cnt_mdp, "Rsz1_fw:\n");
    else
        PQ_LOGE("[PQRSZAdaptor] Exception: m_identifier = %d", m_identifier);

    m_pRszFW->DumpReg(input, buffer_mdp, cnt_mdp, bufferSize_mdp, sizeof(RszInput)/sizeof(uint32_t));
    m_pRszFW->DumpReg(output, buffer_mdp, cnt_mdp, bufferSize_mdp, sizeof(RszOutput)/sizeof(uint32_t));
    m_pRszFW->DumpReg(swReg, buffer_mdp, cnt_mdp, bufferSize_mdp, sizeof(RszReg)/sizeof(uint32_t));
    cnt_mdp += sprintf(buffer_mdp + cnt_mdp, "____mdp_section_end____\n");
    PQ_LOGI("[PQRSZAdaptor] DumpReg: cnt_mdp[%d] bufferSize_mdp[%d]", cnt_mdp, bufferSize_mdp);

    FILE *pFile;

    pFile = fopen(name, "ab");

    if (NULL != pFile)
    {
        fwrite(buffer_mdp, cnt_mdp, 1, pFile);

        fclose(pFile);

        PQ_LOGD("[PQRSZAdaptor]Dump register to %s, cnt[%d], bufferSize[%d]\n", name, cnt_mdp, bufferSize_mdp);
    }
    else
    {
        PQ_LOGD("[PQRSZAdaptor]Open %s failed, \n", name);
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
        if (m_identifier == 0)
            cnt_exif += sprintf(buffer_exif + cnt_exif, "Rsz0_fw:\n");
        else if (m_identifier == 1)
            cnt_exif += sprintf(buffer_exif + cnt_exif, "Rsz1_fw:\n");
        else
            PQ_LOGE("[PQRSZAdaptor] Exception: m_identifier = %d", m_identifier);

        m_pRszFW->DumpReg(input, buffer_exif, cnt_exif, bufferSize_exif, sizeof(RszInput)/sizeof(uint32_t));
        m_pRszFW->DumpReg(swReg, buffer_exif, cnt_exif, bufferSize_exif, sizeof(RszReg)/sizeof(uint32_t));
        cnt_exif += sprintf(buffer_exif + cnt_exif, "____mdp_section_end____\n");
        PQ_LOGI("[PQRSZAdaptor] DumpReg: cnt_exif[%d] bufferSize_exif[%d]", cnt_exif, bufferSize_exif);

        if (*mdpsetting_offset + cnt_exif > mdpsetting_max_size)
        {
            PQ_LOGD("[PQRSZAdaptor] mdpSetting buffer overflow\n");
            delete [] buffer_exif;

            return;
        }

        /* only dump the port with ClearZoom enable if scenario is MEDIA_ISP_CAPTURE */
        if (PQParam->scenario == MEDIA_ISP_PREVIEW ||
            (PQParam->scenario == MEDIA_ISP_CAPTURE && (PQParam->enable & PQ_ULTRARES_EN) != 0))
        {
            memcpy(p_mdpsetting_buf + *mdpsetting_offset, buffer_exif, cnt_exif);
            *mdpsetting_offset += cnt_exif;
        }
    }

    delete [] buffer_exif;
}
#endif
