#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1
#include <cutils/properties.h>
#include <cutils/log.h>
#include <fcntl.h>

#include <PQCommon.h>
#include <PQDREAdaptor.h>
#include "PQTuningBuffer.h"
#include "PQReadBackFactory.h"
#include "PQIspTuning.h"

#include "mdp_reg_aal.h"

#define dre_min(a, b)      (((a) < (b)) ? (a) : (b))
#define ARR_LEN_4BYTE(arr) (sizeof(arr) / 4)

#define DRE30_LAST_POINT   (mdpDRE_LUMA_CURVE_NUM-1)

#define DRE_FW_VERSION "DRE_FW_V1_3\n"
#define DRE_NVRAMINDEX_TAG "CA_LTM_NVRAM_IDX:"
#define DRE_INPUT_TAG "DREInput:"
#define DRE_OUTPUT_TAG "DREOutput:"
#define DRE_INITREG_TAG "DREInitReg:"
#define DRE_SWREG_TAG "DREReg:"
#define ADAPTIVECALTMREG_TAG "TAdaptiveCALTMReg:"
#define ADAPTIVECALTMFACE_EXIF_REG_TAG "TAdaptiveCALTMFace_Exif:"
#define ADAPTIVECALTMFACE_DUMP_REG_TAG "TAdaptiveCALTMFace_Dump:"
#define AAL_TIME_LIMIT 5

enum DRE_DRIVER_DEBUG_FLAG {
    DRE_DRIVER_DUMP_BUFFER = 1 << 0,
    DRE_DRIVER_DUMP_HISTOGRAM = 1 << 1,
    DRE_DRIVER_DUMP_CURVE = 1 << 2,
    DRE_DRIVER_DUMP_TILE = 1 << 3,
    DRE_DRIVER_DEBUG_MODE = 1 << 4,
    DRE_DRIVER_PRINT_TIME = 1 << 5,
    DRE_DRIVER_DUMP_ISP_HISTOGRAM = 1 << 6,
    DRE_DRIVER_DUMP_ISP_CURVE = 1 << 7,
    DRE_DRIVER_DUMP_GETTRANSDATA = 1 << 8,
    DRE_DRIVER_DUMP_SETTRANSDATA = 1 << 9,
    DRE_DRIVER_DUMP_ISP = 1 << 10,
};

PQDREAdaptor* PQDREAdaptor::s_pInstance[] = {};
PQMutex   PQDREAdaptor::s_ALMutex;

PQDREAdaptor* PQDREAdaptor::getInstance(uint32_t identifier)
{
    AutoMutex lock(s_ALMutex);

    if (identifier >= DRE_ENGINE_MAX_NUM)
    {
        return NULL;
    }

    if(NULL == s_pInstance[identifier])
    {
        s_pInstance[identifier] = new PQDREAdaptor(identifier);
        atexit(PQDREAdaptor::destroyInstance);
    }

    return s_pInstance[identifier];
}

void PQDREAdaptor::destroyInstance()
{
    AutoMutex lock(s_ALMutex);

    for (int identifier = 0; identifier < DRE_ENGINE_MAX_NUM; identifier++){
        if (NULL != s_pInstance[identifier])
        {
            delete s_pInstance[identifier];
            s_pInstance[identifier] = NULL;
        }
    }
}

PQDREAdaptor::PQDREAdaptor(uint32_t identifier)
        : m_identifier(identifier),
          PQAlgorithmAdaptor(PROXY_DRE_SWREG,
                             PROXY_DRE_INPUT,
                             PROXY_DRE_OUTPUT,
                             PROXY_DRE_READONLYREG)
{
    PQ_LOGD("[PQDREAdaptor] PQDREAdaptor()... ");

    m_pDREFW = new CDRETopFW;
    m_pDREVPFW = new CDREVPTopFW;
    m_pAdaptiveCALTMFW = new TAdaptiveCALTM(m_pDREFW);

    initDREOutput(&m_defaultDREOutput);
    initDREOutput(&m_defaultDREVPOutput);
    memset(&m_pDREConfig, 0x0, sizeof(DRE_CONFIG_T));
    memset(&m_initDREParam, 0x0, sizeof(DREInitParam));
    memset(&m_lastestDREInfo, 0x0, sizeof(DREReg));
    m_pDREInput.eventFlags = 0x0;
    m_pDREInput.dre_blk_x_num = mdpDRE_BLK_NUM_X;
    m_pDREInput.dre_blk_y_num = mdpDRE_BLK_NUM_Y;
    m_pDREInput.PreWidth = m_pDREInput.CurWidth = m_initDREParam.frame_width = -1;
    m_pDREInput.PreHeight = m_pDREInput.CurHeight = m_initDREParam.frame_height = -1;
    m_pDREInput.IspScenario = 0;
    m_prePQID = 0;
    m_preFrameWidth = 0;
    m_preFrameHeight = 0;
    getDRETable();

    m_enableDumpRegister = DpDriver::getInstance()->getEnableDumpRegister();
}

PQDREAdaptor::~PQDREAdaptor()
{
    PQ_LOGD("[PQDREAdaptor] ~PQDREAdaptor()... ");

    delete m_pAdaptiveCALTMFW;
    delete m_pDREFW;
    delete m_pDREVPFW;
}

void PQDREAdaptor::tuningDREInput(void *input, int32_t scenario)
{
    PQTuningBuffer *p_buffer = m_inputBuffer;
    unsigned int *overwritten_buffer = p_buffer->getOverWrittenBuffer();
    unsigned int *reading_buffer = p_buffer->getReadingBuffer();
    size_t copy_size = sizeof(DRETopInput);

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

bool PQDREAdaptor::tuningDREOutput(void *output, int32_t scenario)
{
    PQTuningBuffer *p_buffer = m_outputBuffer;
    unsigned int *overwritten_buffer = p_buffer->getOverWrittenBuffer();
    unsigned int *reading_buffer = p_buffer->getReadingBuffer();
    size_t copy_size = sizeof(DRETopOutput);

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

void PQDREAdaptor::tuningDRESWReg(CDRETopFW *pDREFW, int32_t scenario)
{
    PQTuningBuffer *p_buffer = m_swRegBuffer;
    unsigned int *overwritten_buffer = p_buffer->getOverWrittenBuffer();
    unsigned int *reading_buffer = p_buffer->getReadingBuffer();

    size_t copy_size = sizeof(DREReg);

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
        memcpy(pDREFW->pDREReg, overwritten_buffer, copy_size);
    } else if (scenario == MEDIA_PICTURE) {
        p_buffer->resetReady();
        memcpy(reading_buffer, pDREFW->pDREReg, copy_size);
        p_buffer->push();
    } else if (p_buffer->toBePrepared()) {
        memcpy(reading_buffer, pDREFW->pDREReg, copy_size);
        p_buffer->push();
    }
}

void PQDREAdaptor::tuningDREVPSWReg(CDREVPTopFW *pDREFW, int32_t scenario)
{
    PQTuningBuffer *p_buffer = m_swRegBuffer;
    unsigned int *overwritten_buffer = p_buffer->getOverWrittenBuffer();
    unsigned int *reading_buffer = p_buffer->getReadingBuffer();
    size_t copy_size = sizeof(DREReg);

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
        memcpy(pDREFW->pDREReg, overwritten_buffer, copy_size);
    } else if (scenario == MEDIA_PICTURE) {
        p_buffer->resetReady();
        memcpy(reading_buffer, pDREFW->pDREReg, copy_size);
        p_buffer->push();
    } else if (p_buffer->toBePrepared()) {
        memcpy(reading_buffer, pDREFW->pDREReg, copy_size);
        p_buffer->push();
    }
}

void PQDREAdaptor::tuningToolReadDREReg(CDREVPTopFW *pDREFW, int32_t scenario)
{
    PQTuningBuffer *p_buffer = m_ReadOnlyBuffer;
    //unsigned int *overwritten_buffer = p_buffer->getOverWrittenBuffer();
    unsigned int *reading_buffer = p_buffer->getReadingBuffer();
    size_t copy_size = sizeof(DREReg_RO);

    if (p_buffer->isValid() == false) {
        return;
    }

    if (copy_size > p_buffer->getModuleSize()) {
        copy_size = p_buffer->getModuleSize();
    }

    /*if (p_buffer->isOverwritten()) {
        if (p_buffer->isSync()) {
            p_buffer->pull();
        }
        memcpy(pDREFW->DREReg_RO, overwritten_buffer, copy_size);
    } else */
    if (scenario == MEDIA_PICTURE) {
        p_buffer->resetReady();
        memcpy(reading_buffer, pDREFW->pDREReg_RO, copy_size);
        p_buffer->push();
    } else if (p_buffer->toBePrepared()) {
        memcpy(reading_buffer, pDREFW->pDREReg_RO, copy_size);
        p_buffer->push();
    }
}

bool PQDREAdaptor::getDRETable()
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;
    /* get register value from ashmem */
    for (int index = 0; index < PROXY_DRE_CUST_MAX; index++)
    {
        offset += size;
        if (index == PROXY_DRE_CUST_REG)
        {
            size = ARR_LEN_4BYTE(m_pDREVPFW->iDRERegEntry);
            isNoError = pPQConfig->getAshmemArray(PROXY_DRE_CUST, offset, &m_pDREVPFW->iDRERegEntry, size);
        }

        if (isNoError < 0)
        {
            break;
        }
    }
    return (isNoError == 1) ? 1 : 0;
}

bool PQDREAdaptor::getDREOutput()
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;
    DRETopOutput temp_Output;
    DRETopOutput zero_Output;
    memset(&zero_Output, 0x0, sizeof(DRETopOutput));
    /* get register value from ashmem */
    for (int index = 0; index < PROXY_DRE_CUST_MAX; index++)
    {
        offset += size;
        if (index == PROXY_DRE_CUST_REG)
        {
            size = ARR_LEN_4BYTE(m_pDREVPFW->iDRERegEntry);
        }
        else if (index == PROXY_DRE_OUTPUT_BUFFER)
        {
            size = ARR_LEN_4BYTE(m_defaultDREVPOutput.DRECurveSet);
            isNoError = pPQConfig->getAshmemArray(PROXY_DRE_CUST, offset, &temp_Output.DRECurveSet, size);
            /* copy register to outParam */
            if ((isNoError == 1) &&
                (memcmp(&temp_Output.DRECurveSet, &zero_Output.DRECurveSet, sizeof(temp_Output.DRECurveSet)) != 0))
            {
                memcpy(&m_defaultDREVPOutput.DRECurveSet, &temp_Output.DRECurveSet, sizeof(temp_Output.DRECurveSet));
            }
        }

        if (isNoError < 0)
        {
            break;
        }
    }
    return (isNoError == 1) ? 1 : 0;
}

bool PQDREAdaptor::getDRETransInfo()
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;
    /* get register value from ashmem */
    for (int index = 0; index < PROXY_DRE_TRANSISTION_MAX; index++)
    {
        offset += size;
        if (index == PROXY_DRE_INFO_BUFFER)
        {
            size = ARR_LEN_4BYTE(m_pDRETransInfo);
            isNoError = pPQConfig->getAshmemArray(PROXY_DRE_TRANSISTION_PREFLOAT_CURVE,
                    offset, &m_pDRETransInfo, size);
        }

        if (isNoError < 0)
        {
            break;
        }
    }
    PQ_LOGI("getDRETransInfo isNoError[%d]", isNoError);
    return (isNoError == 1) ? 1 : 0;
}

bool PQDREAdaptor::getDRETransPrefloatBuffer()
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;
    /* get register value from ashmem */
    for (int index = 0; index < PROXY_DRE_TRANSISTION_MAX; index++)
    {
        offset += size;
        if (index == PROXY_DRE_INFO_BUFFER)
        {
            size = ARR_LEN_4BYTE(m_pDRETransInfo);
        }
        else if (index == PROXY_DRE_PREFLOAT_CURVE_BUFFER)
        {
            size = ARR_LEN_4BYTE(m_pDREsharedata.PreFloatCurve_game);
            isNoError = pPQConfig->getAshmemArray(PROXY_DRE_TRANSISTION_PREFLOAT_CURVE,
                    offset, &m_pDREsharedata.PreFloatCurve_game, size);
        }

        if (isNoError < 0)
        {
            break;
        }
    }
    PQ_LOGI("getDRETransPrefloatBuffer isNoError[%d]", isNoError);
    return (isNoError == 1) ? 1 : 0;
}

bool PQDREAdaptor::getDRETransCurtgtBuffer()
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;
    /* get register value from ashmem */
    for (int index = 0; index < PROXY_DRE_TRANSISTION_MAX; index++)
    {
        offset += size;
        if (index == PROXY_DRE_CURTGT_CURVE_BUFFER)
        {
            size = ARR_LEN_4BYTE(m_pDREsharedata.CurTgtCurve_game);
            isNoError = pPQConfig->getAshmemArray(PROXY_DRE_TRANSISTION_CURTGT_CURVE_FINALlCOEF,
                    offset, &m_pDREsharedata.CurTgtCurve_game, size);
        }
        if (isNoError < 0)
        {
            break;
        }
    }
    PQ_LOGI("getDRETransCurtgtBuffer isNoError[%d]", isNoError);
    return (isNoError == 1) ? 1 : 0;
}

bool PQDREAdaptor::getDRETransFinalCoefBuffer()
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;
    /* get register value from ashmem */
    for (int index = 0; index < PROXY_DRE_TRANSISTION_MAX; index++)
    {
        offset += size;
        if (index == PROXY_DRE_CURTGT_CURVE_BUFFER)
        {
            size = ARR_LEN_4BYTE(m_pDREsharedata.CurTgtCurve_game);
        }
        else if (index == PROXY_DRE_TRANSISTION_FINALlCOEF_BUFFER)
        {
            size = ARR_LEN_4BYTE(m_pDREsharedata.FinalCoef_game);
            isNoError = pPQConfig->getAshmemArray(PROXY_DRE_TRANSISTION_CURTGT_CURVE_FINALlCOEF,
                    offset, &m_pDREsharedata.FinalCoef_game, size);
        }
        if (isNoError < 0)
        {
            break;
        }
    }
    PQ_LOGI("getDRETransFinalCoefBuffer isNoError[%d]", isNoError);
    return (isNoError == 1) ? 1 : 0;
}

bool PQDREAdaptor::getDRESkinWgtSetBuffer()
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;
    /* get register value from ashmem */
    for (int index = 0; index < PROXY_DRE_TRANSISTION_MAX; index++)
    {
        offset += size;
        if (index == PROXY_DRE_CURTGT_CURVE_BUFFER)
        {
            size = ARR_LEN_4BYTE(m_pDREsharedata.CurTgtCurve_game);
        }
        else if (index == PROXY_DRE_TRANSISTION_FINALlCOEF_BUFFER)
        {
            size = ARR_LEN_4BYTE(m_pDREsharedata.FinalCoef_game);
        }
        else if (index == PROXY_DRE_TRANSISTION_DRESKINWGTSET_BUFFER)
        {
            size = ARR_LEN_4BYTE(m_pDREsharedata.DRESkinWgtSet_game);
            isNoError = pPQConfig->getAshmemArray(PROXY_DRE_TRANSISTION_CURTGT_CURVE_FINALlCOEF,
                    offset, &m_pDREsharedata.DRESkinWgtSet_game, size);
        }
        if (isNoError < 0)
        {
            break;
        }
    }
    PQ_LOGI("getDRESkinWgtSetBuffer isNoError[%d]", isNoError);
    return (isNoError == 1) ? 1 : 0;
}

bool PQDREAdaptor::setDREOutput(DRETopOutput *outParam)
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;
    /* set register value from ashmem */
    for (int index = 0; index < PROXY_DRE_CUST_MAX; index++)
    {
        offset += size;
        if (index == PROXY_DRE_CUST_REG)
        {
            size = ARR_LEN_4BYTE(m_pDREVPFW->iDRERegEntry);
        }
        else if (index == PROXY_DRE_OUTPUT_BUFFER)
        {
            size = ARR_LEN_4BYTE(outParam->DRECurveSet);
            isNoError = pPQConfig->setAshmemArray(PROXY_DRE_CUST, offset, &outParam->DRECurveSet, size);
        }

        if (isNoError < 0)
        {
            break;
        }
    }
    return (isNoError == 1) ? 1 : 0;
}

bool PQDREAdaptor::setDRETransInfo(DRETransInfo *transInfo)
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;
    /* set register value from ashmem */
    for (int index = 0; index < PROXY_DRE_TRANSISTION_MAX; index++)
    {
        offset += size;
        if (index == PROXY_DRE_INFO_BUFFER)
        {
            size = ARR_LEN_4BYTE(m_pDRETransInfo);
            isNoError = pPQConfig->setAshmemArray(PROXY_DRE_TRANSISTION_PREFLOAT_CURVE,
                    offset, &m_pDRETransInfo, size);
        }

        if (isNoError < 0)
        {
            break;
        }
    }
    PQ_LOGI("setDRETransInfo isNoError[%d]", isNoError);
    return (isNoError == 1) ? 1 : 0;
}

bool PQDREAdaptor::setDRETransPrefloatBuffer(DREsharedata *transParam)
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;
    /* set register value from ashmem */
    for (int index = 0; index < PROXY_DRE_TRANSISTION_MAX; index++)
    {
        offset += size;
        if (index == PROXY_DRE_INFO_BUFFER)
        {
            size = ARR_LEN_4BYTE(m_pDRETransInfo);
        }
        else if (index == PROXY_DRE_PREFLOAT_CURVE_BUFFER)
        {
            size = ARR_LEN_4BYTE(transParam->PreFloatCurve_game);
            isNoError = pPQConfig->setAshmemArray(PROXY_DRE_TRANSISTION_PREFLOAT_CURVE,
                    offset, &transParam->PreFloatCurve_game, size);
        }
        if (isNoError < 0)
        {
            break;
        }
    }
    PQ_LOGI("setDRETransPrefloatBuffer isNoError[%d]", isNoError);
    return (isNoError == 1) ? 1 : 0;
}

bool PQDREAdaptor::setDRETransCurtgtBuffer(DREsharedata *transParam)
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;
    /* get register value from ashmem */
    for (int index = 0; index < PROXY_DRE_TRANSISTION_MAX; index++)
    {
        offset += size;
        if (index == PROXY_DRE_CURTGT_CURVE_BUFFER)
        {
            size = ARR_LEN_4BYTE(transParam->CurTgtCurve_game);
            isNoError = pPQConfig->setAshmemArray(PROXY_DRE_TRANSISTION_CURTGT_CURVE_FINALlCOEF,
                    offset, &transParam->CurTgtCurve_game, size);
        }
        if (isNoError < 0)
        {
            break;
        }
    }
    PQ_LOGI("setDRETransCurtgtBuffer isNoError[%d]", isNoError);
    return (isNoError == 1) ? 1 : 0;
}

bool PQDREAdaptor::setDRETransFinalCoefBuffer(DREsharedata *transParam)
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;
    /* get register value from ashmem */
    for (int index = 0; index < PROXY_DRE_TRANSISTION_MAX; index++)
    {
        offset += size;
        if (index == PROXY_DRE_CURTGT_CURVE_BUFFER)
        {
            size = ARR_LEN_4BYTE(m_pDREsharedata.CurTgtCurve_game);
        }
        else if (index == PROXY_DRE_TRANSISTION_FINALlCOEF_BUFFER)
        {
            size = ARR_LEN_4BYTE(transParam->FinalCoef_game);
            isNoError = pPQConfig->setAshmemArray(PROXY_DRE_TRANSISTION_CURTGT_CURVE_FINALlCOEF,
                    offset, &transParam->FinalCoef_game, size);
        }
        if (isNoError < 0)
        {
            break;
        }
    }
    PQ_LOGI("setDRETransFinalCoefBuffer isNoError[%d]", isNoError);
    return (isNoError == 1) ? 1 : 0;
}

bool PQDREAdaptor::setDRESkinWgtSetBuffer(DREsharedata *transParam)
{
    PQConfig* pPQConfig = PQConfig::getInstance();
    int32_t offset = 0;
    int32_t size = 0;
    int32_t isNoError = 0;
    /* get register value from ashmem */
    for (int index = 0; index < PROXY_DRE_TRANSISTION_MAX; index++)
    {
        offset += size;
        if (index == PROXY_DRE_CURTGT_CURVE_BUFFER)
        {
            size = ARR_LEN_4BYTE(m_pDREsharedata.CurTgtCurve_game);
        }
        else if (index == PROXY_DRE_TRANSISTION_FINALlCOEF_BUFFER)
        {
            size = ARR_LEN_4BYTE(m_pDREsharedata.FinalCoef_game);
        }
        else if (index == PROXY_DRE_TRANSISTION_DRESKINWGTSET_BUFFER)
        {
            size = ARR_LEN_4BYTE(transParam->DRESkinWgtSet_game);
            isNoError = pPQConfig->setAshmemArray(PROXY_DRE_TRANSISTION_CURTGT_CURVE_FINALlCOEF,
                    offset, &transParam->DRESkinWgtSet_game, size);
        }
        if (isNoError < 0)
        {
            break;
        }
    }
    PQ_LOGI("setDRESkinWgtSetBuffer isNoError[%d]", isNoError);
    return (isNoError == 1) ? 1 : 0;
}
void PQDREAdaptor::initDREOutput(DRETopOutput *outParam)
{
    int blk_x, blk_y, curve_point;

    for (blk_y = 0; blk_y < mdpDRE_BLK_NUM_Y; blk_y++)
    {
        for (blk_x = 0; blk_x < mdpDRE_BLK_NUM_X; blk_x++)
        {
            for (curve_point = 0; curve_point < mdpDRE_LUMA_CURVE_NUM; curve_point++)
            {
                outParam->DRECurveSet[blk_y][blk_x][curve_point] = dre_min(255, 16 * curve_point);
                outParam->CurFloatCurve[blk_y][blk_x][curve_point] = curve_point * 2048;
            }
        }
    }
}

void PQDREAdaptor::getDefaultDREOutput(uint32_t modifyDREBlk, DRETopOutput *outParam)
{
    int modify_x, modify_y, curve_point;

    memcpy(outParam, &m_defaultDREOutput, sizeof(DRETopOutput));

    if (modifyDREBlk > 0 && modifyDREBlk <= mdpDRE_BLK_NUM_Y*mdpDRE_BLK_NUM_X)
    {
        modifyDREBlk = modifyDREBlk - 1;
        modify_y = modifyDREBlk / mdpDRE_BLK_NUM_X;
        modify_x = modifyDREBlk % mdpDRE_BLK_NUM_X;

        PQ_LOGD("[PQDREAdaptor] getDefaultDREOutput, [debug mode], modifyDREBlock[%d] modify_x[%d], modify_y[%d]",
            modifyDREBlk, modify_x, modify_y);

        for (curve_point = 0; curve_point < mdpDRE_LUMA_CURVE_NUM; curve_point++)
        {
            outParam->DRECurveSet[modify_y][modify_x][curve_point] = 255;
        }
    }
}

void PQDREAdaptor::getDefaultDREVPOutput(uint32_t modifyDREBlk, DRETopOutput *outParam)
{
    int modify_x, modify_y, curve_point;
    if (getDREOutput() == 1)
    {
        /* use the previous output curve stored in Ashmem */
        PQ_LOGD("[PQDREAdaptor] getDefaultDREVPOutput, use the previous output curve stored in Ashmem");
        memcpy(outParam, &m_defaultDREVPOutput, sizeof(DRETopOutput));
    }
    else
    {
        memcpy(outParam, &m_defaultDREVPOutput, sizeof(DRETopOutput));

        if (modifyDREBlk > 0 && modifyDREBlk <= mdpDRE_BLK_NUM_Y*mdpDRE_BLK_NUM_X)
        {
            modifyDREBlk = modifyDREBlk - 1;
            modify_y = modifyDREBlk / mdpDRE_BLK_NUM_X;
            modify_x = modifyDREBlk % mdpDRE_BLK_NUM_X;

            PQ_LOGD("[PQDREAdaptor] getDefaultDREVPOutput, [debug mode], modifyDREBlock[%d] modify_x[%d], modify_y[%d]",
                modifyDREBlk, modify_x, modify_y);

            for (curve_point = 0; curve_point < mdpDRE_LUMA_CURVE_NUM; curve_point++)
            {
                outParam->DRECurveSet[modify_y][modify_x][curve_point] = 255;
            }
        }
    }
}


void PQDREAdaptor::initDREFWparam(const int32_t scenario, int32_t imWidth, int32_t imHeight,
    int32_t frameWidth, int32_t frameHeight)
{
    if (m_initDREParam.frame_width == -1 || m_initDREParam.frame_height == -1)
    {
        m_initDREParam.frame_width = imWidth;
        m_initDREParam.frame_height = imHeight;
        if (scenario == MEDIA_VIDEO)
        {
            m_pDREVPFW->onInitCommon(m_initDREParam, frameWidth, frameHeight);
        }
        else
        {
            m_pDREFW->onInitCommon(m_initDREParam);
        }
    }
    else
    {
        m_initDREParam.frame_width = imWidth;
        m_initDREParam.frame_height = imHeight;
        if (scenario == MEDIA_VIDEO)
        {
            m_pDREVPFW->DREBlkNumCal(m_initDREParam, frameWidth, frameHeight);
        }

        if (scenario == MEDIA_VIDEO &&
            (m_initDREParam.frame_width != imWidth || m_initDREParam.frame_height != imHeight))
        {
            m_pDREVPFW->DREFrameHWRegCal(m_initDREParam);
        }
    }

    PQ_LOGI("initDREFWparam, frame_width[%d], frame_height[%d]", m_initDREParam.frame_width, m_initDREParam.frame_height);
    if (scenario == MEDIA_VIDEO)
    {
        PQ_LOGI("initDREFWparam, DRE block number [y, x] [%d, %d]", m_pDREVPFW->pDREInitReg->dre_blk_y_num, m_pDREVPFW->pDREInitReg->dre_blk_x_num);
    }
    else
    {
        PQ_LOGI("initDREFWparam, DRE block number [y, x] [%d, %d]", m_pDREFW->pDREInitReg->dre_blk_y_num, m_pDREFW->pDREInitReg->dre_blk_x_num);
    }
}

void PQDREAdaptor::onCalculateIspImpl(const bool isBypass, DpPqParam &PQParam, DRETopOutput *outParam, CDRETopFW *pDREFW)
{
    TAdaptiveCALTM_ExtraInfo CALTM_ExtraInfo;
    unsigned int ispTuningFlag = 0;

    if (PQParam.scenario == MEDIA_ISP_CAPTURE)
    {
        CALTM_ExtraInfo.Scenario = ADAPTIVE_CALTM_SCENARIO_PICTURE;
    }
    else
    {
        CALTM_ExtraInfo.Scenario = ADAPTIVE_CALTM_SCENARIO_VIDEO;
    }
    CALTM_ExtraInfo.ISO = PQParam.u.isp.iso;
    CALTM_ExtraInfo.LV = PQParam.u.isp.LV;
    CALTM_ExtraInfo.LCSO = PQParam.u.isp.LCSO;
    CALTM_ExtraInfo.LCSO_Size = PQParam.u.isp.LCSO_Size;
    CALTM_ExtraInfo.DCE = PQParam.u.isp.DCE;
    CALTM_ExtraInfo.DCE_Size = PQParam.u.isp.DCE_Size;
    CALTM_ExtraInfo.LCE = PQParam.u.isp.LCE;
    CALTM_ExtraInfo.LCE_Size = PQParam.u.isp.LCE_Size;
    CALTM_ExtraInfo.mtkCameraFaceMetadata = PQParam.u.isp.p_faceInfor;

    if (m_pDREConfig.ispTuningFlag != 0)
    {
        PQIspTuning *pPQIspTuning = PQIspTuning::getInstance();

        if (pPQIspTuning != NULL)
        {
            if (pPQIspTuning->loadIspTuningFile() == true)
            {
                pPQIspTuning->getDreTuningValue(pDREFW->pDREToolReg, m_pAdaptiveCALTMFW->AdaptiveCALTMReg, m_pAdaptiveCALTMFW->adaptiveCALTMFace_dump);
                PQ_LOGD("[PQDREAdaptor] onCalculateIspImpl, tuning mode, scenario[%d]", PQParam.scenario);
                // Load DRE SW register
                pDREFW->LoadFWRegSettings();
                // Load adaptive CALTM SW register
                memcpy(m_pAdaptiveCALTMFW->adaptiveCALTMFace_exif, &(m_pAdaptiveCALTMFW->adaptiveCALTMFace_dump->adaptiveCALTMFace_Exif), sizeof(TAdaptiveCALTMFace_Exif));

                ispTuningFlag = 1;
            }
        }
    }

    m_pAdaptiveCALTMFW->AdaptiveCALTMReg->DebugFlag = m_pDREConfig.adaptiveCaltmFlag;

    if (isBypass == true)
    {
        PQ_LOGI("[PQDREAdaptor] adaptive CALTM onCalculateHW start, Scenario[%d]", CALTM_ExtraInfo.Scenario);
        m_pAdaptiveCALTMFW->onCalculateHW(m_initDREParam, CALTM_ExtraInfo, PQParam.u.isp.dpDREParam.p_customSetting, ispTuningFlag);
    }
    else
    {
        PQ_LOGI("[PQDREAdaptor] adaptive CALTM onCalculateSW start, Scenario[%d]", CALTM_ExtraInfo.Scenario);
        m_pAdaptiveCALTMFW->onCalculateSW(m_initDREParam, m_pDREInput, outParam, CALTM_ExtraInfo, PQParam.u.isp.dpDREParam.p_customSetting,
            ispTuningFlag, PQParam.u.isp.dpDREParam.activeWidth, PQParam.u.isp.dpDREParam.activeHeight);
    }
}

void PQDREAdaptor::onCalculateVideoImpl(PQSession *pPQSession, DpPqParam &PQParam, const bool isBypass, DRETopOutput *outParam, CDREVPTopFW *pDREFW,
                                        const bool isHDR, const uint32_t selTable)
{
    PQTimeValue    begin;
    PQTimeValue    end;
    int32_t        diff;
    PQTimeValue    time_g_s;
    PQTimeValue    time_g_e;
    int32_t        time_g_d;
    PQ_TIMER_GET_CURRENT_TIME(begin);
#ifdef SUPPORT_GAME_DRE
    if ((PQParam.u.video.videoScenario == INFO_GAME) && (pPQSession->getDREReg(&PQParam, &m_lastestDREInfo)))
    {
        memcpy(pDREFW->pDREReg, &m_lastestDREInfo, sizeof(DREReg));
    }
    else
#endif
    {
        pDREFW->onLoadTable(isHDR, selTable);
    }
    PQ_LOGI("[PQDREAdaptor] onCalculateVideoImpl, selTable[%d] dre_dync_spike_th[%d] LLPValue[%d] LLPRatio[%d] APLCompRatioLow[%d]",
        selTable, pDREFW->pDREReg->dre_dync_spike_th, pDREFW->pDREReg->LLPValue, pDREFW->pDREReg->LLPRatio, pDREFW->pDREReg->APLCompRatioLow);
    tuningDREVPSWReg(pDREFW, MEDIA_VIDEO);
    pDREFW->DREFrameHWRegCal(m_initDREParam);
    if (isBypass == false)
    {
        {
            DP_TRACE_BEGIN("pDREFW->onCalculate");
            pDREFW->onCalculate(m_pDREInput, outParam);
            DP_TRACE_END();
        }
        {
            DP_TRACE_BEGIN("pDREFW->getShareMemory");
            pDREFW->getShareMemory(&m_pDREsharedata);
            DP_TRACE_END();
        }
        PQ_LOGI("Update DRE share data\n");
        {
            PQ_TIMER_GET_CURRENT_TIME(time_g_s);
            DP_TRACE_BEGIN("SetDataToMemory");
            setDRETransPrefloatBuffer(&m_pDREsharedata);
            setDRETransFinalCoefBuffer(&m_pDREsharedata);
            setDRETransCurtgtBuffer(&m_pDREsharedata);
            setDRESkinWgtSetBuffer(&m_pDREsharedata);
            DP_TRACE_END();
            PQ_TIMER_GET_CURRENT_TIME(time_g_e);
            PQ_TIMER_GET_DURATION_IN_MS(time_g_s, time_g_e, time_g_d);
            if (time_g_d > 1)
                PQ_LOGD("Get Data fro Memory, time %d ms\n", time_g_d);
        }

    }
    if ((m_pDREConfig.driverDebugFlag & DRE_DRIVER_DUMP_GETTRANSDATA) != 0)
    {
        debugDumpTransData(&m_pDRETransInfo, &m_pDREsharedata);
    }
    PQ_LOGI("[PQDREAdaptor] onCalculateVideoImpl, isHDR[%d], selTable[%d]", (int)isHDR, selTable);
    PQ_TIMER_GET_CURRENT_TIME(end);
    PQ_TIMER_GET_DURATION_IN_MS(begin, end, diff);
    if (diff > AAL_TIME_LIMIT)
    {
        PQ_LOGD("onCalculateVideoImpl time %d ms\n", diff);
    }
    else
    {
        PQ_LOGI("onCalculateVideoImpl time %d ms\n", diff);
    }
#ifdef SUPPORT_GAME_DRE
    tuningToolReadDREReg(pDREFW, MEDIA_VIDEO);
#endif
}

void PQDREAdaptor::onCalculate(PQSession *pPQSession, DpPqParam &PQParam, const bool isBypass, const uint32_t modifyDREBlock,
    DRETopOutput *outParam, bool *isApplyFw, int32_t frameWidth, int32_t frameHeight)
{
    int32_t scenario = PQParam.scenario;
    uint32_t DRE_Cmd = DpDREParam::Cmd::Nothing;
    uint32_t buffer_ID = DRE_ILLEGAL_BUFFER_ID;
    bool isHistAvailable = false;
    unsigned long long userId = DRE_DEFAULT_USERID;
    void *user_buffer = NULL;
    PQDREHistogramAdaptor *pPQDREHistogramAdaptor = NULL;
    CDRETopFW *pDREFW = m_pDREFW;
    CDREVPTopFW *pDREVPFW = m_pDREVPFW;
    DpPqConfig *PQConfig;
    bool isHDR;
    uint32_t selTable = 0;
    uint64_t PQSessionId = pPQSession->getID();

    pPQSession->getDpPqConfig(&PQConfig);
    isHDR = (bool)PQConfig->enHDR;

    if (scenario == MEDIA_ISP_PREVIEW || scenario == MEDIA_ISP_CAPTURE)
    {
        DRE_Cmd = PQParam.u.isp.dpDREParam.cmd;
        userId = PQParam.u.isp.dpDREParam.userId;
        user_buffer = PQParam.u.isp.dpDREParam.buffer;
        pPQDREHistogramAdaptor = PQReadBackFactory::getInstance()->getDRE(0);
        if (pPQDREHistogramAdaptor == NULL)
        {
            PQ_LOGE("[PQDREAdaptor] onCalculate, pPQDREHistogramAdaptor is NULL in scenario[%d]", scenario);
        }
    }
    else if (scenario == MEDIA_VIDEO)
    {
        DRE_Cmd = PQParam.u.video.dpDREParam.cmd;
        userId = PQParam.u.video.dpDREParam.userId;
        user_buffer = PQParam.u.video.dpDREParam.buffer;
        selTable = PQParam.u.video.paramTable;
        pPQDREHistogramAdaptor = PQReadBackFactory::getInstance()->getDRE(0);
        if (pPQDREHistogramAdaptor == NULL)
        {
            PQ_LOGE("[PQDREAdaptor] onCalculate, pPQDREHistogramAdaptor is NULL in scenario[%d]", scenario);
        }
    }

    if (pPQDREHistogramAdaptor != NULL)
    {
        if (isBypass == false)
        {
            pPQDREHistogramAdaptor->registerBuffer(user_buffer, userId, DRE_Cmd, &buffer_ID);
            pPQDREHistogramAdaptor->getBufferData(user_buffer, buffer_ID, DRE_Cmd, &isHistAvailable);
        }
        // Dump DRE Buffer Pool for debug
        if ((m_pDREConfig.driverDebugFlag & DRE_DRIVER_DUMP_BUFFER) != 0)
        {
            pPQDREHistogramAdaptor->dumpBufferPool();
        }
    }

    PQ_LOGI("[PQDREAdaptor] onCalculate, scenario[%d], DRE_Cmd[0x%x], userId[0x%llx], buffer_ID[%d], isBypass[%d], isHistAvailable[%d]",
        scenario, DRE_Cmd, userId, buffer_ID, isBypass, isHistAvailable);

    if (isHistAvailable == false || isBypass == true)
    {
        if (scenario == MEDIA_ISP_PREVIEW || scenario == MEDIA_ISP_CAPTURE)
        {
            onCalculateIspImpl(true, PQParam, NULL, pDREFW);
            getDefaultDREOutput(modifyDREBlock, outParam);
        }
        else if (scenario == MEDIA_VIDEO)
        {
            onCalculateVideoImpl(pPQSession, PQParam, true, NULL, pDREVPFW, isHDR, selTable);
            getDefaultDREVPOutput(modifyDREBlock, outParam);
        }

        PQ_LOGI("[PQDREAdaptor] use bypass setting! PQ Session ID[%llx]", PQSessionId);
    }
    else
    {
        PQTimeValue    time_s;
        PQTimeValue    time_e;
        int32_t        time_d;
        PQTimeValue    time_g_s;
        PQTimeValue    time_g_e;
        int32_t        time_g_d;
        int            dre_blk_x_num;
        int            dre_blk_y_num;
        getDRETransInfo();
        if ((m_pDRETransInfo.user == INFO_HWC || m_pDRETransInfo.user == INFO_GPU ) &&
                (m_pDRETransInfo.user != PQParam.u.video.userScenario) && (scenario == MEDIA_VIDEO))
        {
                {
                    PQ_TIMER_GET_CURRENT_TIME(time_g_s);
                    DP_TRACE_BEGIN("GetDataFromMemory");
                    getDRETransPrefloatBuffer();
                    getDRETransFinalCoefBuffer();
                    getDRETransCurtgtBuffer();
                    getDRESkinWgtSetBuffer();
                    DP_TRACE_END();
                    PQ_TIMER_GET_CURRENT_TIME(time_g_e);
                    PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d);
                }
                if (time_d > 1)
                    PQ_LOGD("Get Data from Memory, time %d ms\n", time_d);
                if ((m_pDREConfig.driverDebugFlag & DRE_DRIVER_DUMP_GETTRANSDATA) != 0)
                {
                    debugDumpTransData(&m_pDRETransInfo, &m_pDREsharedata);
                }
                pDREVPFW->setShareMemory(&m_pDREsharedata);
        }
        else
            PQ_LOGI("User is the same m_pDRETransInfo[%d] userScenario[%d]",
                    m_pDRETransInfo.user, PQParam.u.video.userScenario);


        m_pDRETransInfo.user = PQParam.u.video.userScenario;
        m_pDRETransInfo.dre_blk_x_num = pDREVPFW->pDREInitReg->dre_blk_x_num;
        m_pDRETransInfo.dre_blk_y_num = pDREVPFW->pDREInitReg->dre_blk_y_num;
        setDRETransInfo(&m_pDRETransInfo);

        if (scenario == MEDIA_VIDEO)
        {
            dre_blk_x_num = pDREVPFW->pDREInitReg->dre_blk_x_num;
            dre_blk_y_num = pDREVPFW->pDREInitReg->dre_blk_y_num;
        }
        else
        {
            dre_blk_x_num = pDREFW->pDREInitReg->dre_blk_x_num;
            dre_blk_y_num = pDREFW->pDREInitReg->dre_blk_y_num;
        }

        pPQDREHistogramAdaptor->getDREInput(user_buffer, buffer_ID, dre_blk_x_num, dre_blk_y_num, &m_pDREInput);
#ifdef CALTM_VERSION_6779
        memset(&m_pDREInput.DREChromaHist, 0, sizeof(m_pDREInput.DREChromaHist));
#endif
        m_pDREInput.CurWidth = m_initDREParam.frame_width;
        m_pDREInput.CurHeight = m_initDREParam.frame_height;
        m_pDREInput.dre_blk_x_num = dre_blk_x_num;
        m_pDREInput.dre_blk_y_num = dre_blk_y_num;
        m_pDREInput.IspScenario = 0;
        if (scenario == MEDIA_ISP_PREVIEW)
        {
            m_pDREInput.IspScenario = 2;
        }
        else if (scenario == MEDIA_ISP_CAPTURE)
        {
            m_pDREInput.IspScenario = 1;
        }

        PQ_LOGI("[PQDREAdaptor] onCalculate, scenario[%d], IspScenario[%d] PreWidth[%d], PreHeight[%d] CurWidth[%d], CurHeight[%d]\n",
            scenario, m_pDREInput.IspScenario, m_pDREInput.PreWidth, m_pDREInput.PreHeight, m_pDREInput.CurWidth, m_pDREInput.CurHeight);

        tuningDREInput(&m_pDREInput, scenario);

        // Dump DRE input histogram for debug
        if ((m_pDREConfig.driverDebugFlag & DRE_DRIVER_DUMP_HISTOGRAM) != 0)
        {
            debugDumpFWInput(&m_pDREInput);
        }

        if (scenario == MEDIA_VIDEO)
        {
            /* get tuning register after onLoadTable */
            //tuningDREVPSWReg(pDREVPFW, scenario);
            pDREVPFW->setDebugFlags(m_pDREConfig.debugFlag);
        }
        else
        {
            tuningDRESWReg(pDREFW, scenario);
            pDREFW->setDebugFlags(m_pDREConfig.debugFlag);
        }

        PQ_TIMER_GET_CURRENT_TIME(time_s);
        if (scenario == MEDIA_ISP_PREVIEW || scenario == MEDIA_ISP_CAPTURE)
        {
            DP_TRACE_CALL();
            onCalculateIspImpl(false, PQParam, outParam, pDREFW);
        }
        else if (scenario == MEDIA_VIDEO)
        {
            DP_TRACE_CALL();
            /* call ALGO init fuction is PQSession ID is changed */
            if (PQParam.u.video.videoScenario == INFO_VIDEO &&
                    (frameWidth != m_preFrameWidth || frameHeight != m_preFrameHeight))
            {
                m_prePQID = PQSessionId;
                m_preFrameWidth = frameWidth;
                m_preFrameHeight = frameHeight;
                pDREVPFW->onInitPart(m_initDREParam, outParam);
            }
            onCalculateVideoImpl(pPQSession, PQParam, false, outParam, pDREVPFW, isHDR, selTable);
            setDREOutput(outParam);
        }
#if 0     // there are no other scenarios now
        else
        {
            pDREFW->onCalculate(m_pDREInput, outParam);
        }
#endif
        *isApplyFw = true;
        PQ_TIMER_GET_CURRENT_TIME(time_e);
        PQ_TIMER_GET_DURATION_IN_MS(time_s, time_e, time_d);
        if ((m_pDREConfig.driverDebugFlag & DRE_DRIVER_PRINT_TIME) != 0)
        {
            PQ_LOGD("[PQDREAdaptor] pDREFW->onCalculate, time %d ms\n", time_d);
        }
        else
        {
            PQ_LOGI("[PQDREAdaptor] pDREFW->onCalculate, time %d ms\n", time_d);
        }
    }

    if (pPQDREHistogramAdaptor != NULL)
    {
        if (isBypass == false)
        {
            pPQDREHistogramAdaptor->storeDREOutput(user_buffer, buffer_ID, outParam);
            pPQDREHistogramAdaptor->unregisterBuffer(userId, DRE_Cmd);
        }
        else
        {
            pPQDREHistogramAdaptor->setBypass(user_buffer, userId, outParam);
        }
    }
}

int32_t PQDREAdaptor::getPQScenario(PQSession *pPQSession)
{
    int32_t scenario;
    DpPqParam PQParam;

    pPQSession->getPQParam(&PQParam);

    scenario = PQParam.scenario;
    PQ_LOGI("[PQDREAdaptor] getPQScenario, scenario =  %d\n", scenario);

    return scenario;
}

void PQDREAdaptor::calRegs(PQSession* pPQSession, DpCommand &command, int32_t imWidth, int32_t imHeight,
    int32_t frameWidth, int32_t frameHeight, int32_t *pFrameConfigLabel, const bool curveOnly)
{
    DpPqConfig *PQConfig;
    DpPqParam PQParam;
    DREInitReg initDREReg;
    DRETopOutput outParam;
    int32_t scenario;
    uint32_t dre_enable = 0;
    uint32_t modifyDREBlock = 0;
    bool isDebugMode = false;
    bool bypassHWAccess = false;
    PQTimeValue    begin;
    PQTimeValue    end;
    int32_t        diff;

    pPQSession->getDpPqConfig(&PQConfig);
    pPQSession->getPQParam(&PQParam);

    scenario = PQParam.scenario;
    PQ_LOGI("[PQDREAdaptor] calRegs, scenario =  %d\n", scenario);

    {
        AutoMutex lock(s_ALMutex);

        PQ_TIMER_GET_CURRENT_TIME(begin);

        pPQSession->getDREConfig(&m_pDREConfig);

        dre_enable = PQConfig->enDRE;

        /* check debug system property */
        if ((scenario == MEDIA_VIDEO) && (PQParam.u.video.videoScenario == INFO_GAME))
        {
             dre_enable = 1;
             PQ_LOGI("[PQDREAdaptor] Game Scenario dre_enable always on\n");
        }
        else if (scenario == MEDIA_VIDEO && PQConfig->enHDR == 1 &&
            (m_pDREConfig.HDR_VP_ENABLE == 0 || m_pDREConfig.HDR_VP_ENABLE == 1))
        {
            dre_enable = m_pDREConfig.HDR_VP_ENABLE;
        }
        else if (scenario == MEDIA_VIDEO && PQConfig->enHDR == 0 &&
            (m_pDREConfig.VP_ENABLE == 0 || m_pDREConfig.VP_ENABLE == 1))
        {
            dre_enable = m_pDREConfig.VP_ENABLE;
        }
        else if ((scenario == MEDIA_ISP_PREVIEW || scenario == MEDIA_ISP_CAPTURE) &&
            (m_pDREConfig.ENABLE == 0 || m_pDREConfig.ENABLE == 1))
        {
            dre_enable = m_pDREConfig.ENABLE;
        }

        PQ_LOGI("[PQDREAdaptor] dre_enable[%d], debugFlag[%d], driverdebugFlag[%d], demoWinX[0x%08x], driverBLKFlag[%d]",
            dre_enable, m_pDREConfig.debugFlag, m_pDREConfig.driverDebugFlag, m_pDREConfig.demoWinX, m_pDREConfig.driverBLKFlag);
        PQ_LOGI("[PQDREAdaptor] frameWidth[%d], frameHeight[%d]", frameWidth, frameHeight);

        if ((m_pDREConfig.driverDebugFlag & DRE_DRIVER_DEBUG_MODE) != 0)
        {
            isDebugMode = true;
            modifyDREBlock = m_pDREConfig.driverBLKFlag;
        }
        m_initDREParam.demo_win_x_start = m_pDREConfig.demoWinX & 0x1FFF;
        m_initDREParam.demo_win_x_end = (m_pDREConfig.demoWinX >> 16) & 0x1FFF;
#ifdef CALTM_VERSION_6779
        m_initDREParam.demo_win_y_start = 0;
        m_initDREParam.demo_win_y_end = 0x1FFF;
#endif

        initDREFWparam(scenario, imWidth, imHeight, frameWidth, frameHeight);

        bool isBypass = (dre_enable == 1 && isDebugMode == false) ? false:true;
        bool isApplyFw = false;
        onCalculate(pPQSession, PQParam, isBypass, modifyDREBlock, &outParam, &isApplyFw, frameWidth, frameHeight);

        // Copy pDREInitReg
        if (scenario == MEDIA_VIDEO)
        {
            memcpy(&initDREReg, m_pDREVPFW->pDREInitReg, sizeof(DREInitReg));
        }
        else
        {
            memcpy(&initDREReg, m_pDREFW->pDREInitReg, sizeof(DREInitReg));
        }

        bypassHWAccess = tuningDREOutput(&outParam, scenario);

        if (scenario == MEDIA_ISP_PREVIEW || scenario == MEDIA_ISP_CAPTURE)
        {
#ifdef SUPPORT_DUMP_IN_FW
            dumpFwRegInFw(&PQParam);
#else
            dumpFwReg(isApplyFw, &PQParam, &m_pDREInput, &outParam, m_pDREFW->pDREInitReg, m_pDREFW->pDREReg,
                m_pAdaptiveCALTMFW->AdaptiveCALTMReg, m_pAdaptiveCALTMFW->adaptiveCALTMFace_exif, m_pAdaptiveCALTMFW->adaptiveCALTMFace_dump);
#endif
        }

        // Dump DRE output curve for debug
        if ((m_pDREConfig.driverDebugFlag & DRE_DRIVER_DUMP_CURVE) != 0)
        {
            debugDumpFrameReg(&outParam, &m_initDREParam, &initDREReg);
        }

        PQ_TIMER_GET_CURRENT_TIME(end);
        PQ_TIMER_GET_DURATION_IN_MS(begin, end, diff);
        if ((m_pDREConfig.driverDebugFlag & DRE_DRIVER_PRINT_TIME) != 0)
        {
            PQ_LOGD("[PQDREAdaptor] calRegs, onCalculate, time %d ms\n", diff);
        }
        else
        {
            PQ_LOGI("[PQDREAdaptor] calRegs, onCalculate, time %d ms\n", diff);
        }
    }

    uint32_t relay = (dre_enable == 0 && isDebugMode == false) ? 1:0;

    if (curveOnly == false)
    {
        // Set MDP_AAL Relay mode
        MM_REG_WRITE(command, MDP_AAL_CFG, relay, 0x1);
        command.addMetLog("MDP_AAL__MDP_AAL_CFG", relay);
    }

    PQ_LOGI("[PQDREAdaptor] relay[%d], dre_enable[%d], isDebugMode[%d], bypassHWAccess[%d], modifyDREBlock[%d]",
        relay, dre_enable, isDebugMode, bypassHWAccess, modifyDREBlock);

    if (relay == 1 || (isDebugMode == true && modifyDREBlock == 0) || bypassHWAccess == true)
    {
        return;
    }

    PQ_TIMER_GET_CURRENT_TIME(begin);

    if (curveOnly == false)
    {
        const uint32_t dre_h_slope = 0x8;
        const uint32_t dre_s_slope = 0x4;
        const uint32_t dre_y_slope = 0x4;
        const uint32_t vp_dre_h_slope = 0x4;

        // Change MDP_AAL alg mode
        MM_REG_WRITE(command, MDP_AAL_CFG_MAIN, (1 << 5) | (1 << 4), (1 << 5) | (1 << 4));
        // Configure initial DRE setting
        MM_REG_WRITE(command, MDP_AAL_DRE_BLOCK_INFO_01, ((initDREReg.dre_blk_y_num&0x1F) << 5)|(initDREReg.dre_blk_x_num&0x1F), MDP_AAL_DRE_BLOCK_INFO_01_MASK);
        MM_REG_WRITE(command, MDP_AAL_DRE_BLOCK_INFO_02, ((initDREReg.dre_blk_height&0x1FFF) << 13)|(initDREReg.dre_blk_width&0x1FFF), MDP_AAL_DRE_BLOCK_INFO_02_MASK);
        MM_REG_WRITE(command, MDP_AAL_DRE_BLOCK_INFO_03, ((initDREReg.dre_pxl_diff_slope_for_flat_pxl&0xFF) << 24)|((initDREReg.dre_pxl_diff_th_for_flat_pxl&0xFF) << 16)|((initDREReg.dre_pxl_diff_slope&0xFF) << 8)|(initDREReg.dre_pxl_diff_th&0xFF),
            MDP_AAL_DRE_BLOCK_INFO_03_MASK);
        MM_REG_WRITE(command, MDP_AAL_DRE_BLOCK_INFO_04, ((initDREReg.dre_flat_length_slope&0x3FF) << 13)|(initDREReg.dre_flat_length_th&0x1FFF),
            MDP_AAL_DRE_BLOCK_INFO_04_MASK);
        MM_REG_WRITE(command, MDP_AAL_DRE_BLOCK_INFO_05, initDREReg.dre_blk_area, MDP_AAL_DRE_BLOCK_INFO_05_MASK);
        MM_REG_WRITE(command, MDP_AAL_DRE_BLOCK_INFO_06, initDREReg.dre_blk_area_min, MDP_AAL_DRE_BLOCK_INFO_06_MASK);
        MM_REG_WRITE(command, MDP_AAL_DRE_ALPHA_BLEND_00, ((initDREReg.dre_y_alpha_shift_bit&0xF) << 25)|((initDREReg.dre_y_alpha_base&0x1FF) << 16)|((initDREReg.dre_x_alpha_shift_bit&0xF) << 9)|(initDREReg.dre_x_alpha_base&0x1FF),
            MDP_AAL_DRE_ALPHA_BLEND_00_MASK);
        MM_REG_WRITE(command, MDP_AAL_DRE_CHROMA_HIST_00, ((initDREReg.dre_s_upper&0xFF) << 24)|((initDREReg.dre_s_lower&0xFF) << 16)|((initDREReg.dre_y_upper&0xFF) << 8)|(initDREReg.dre_y_lower&0xFF),
            MDP_AAL_DRE_CHROMA_HIST_00_MASK);
        if (scenario == MEDIA_VIDEO) {
            MM_REG_WRITE(command, MDP_AAL_DRE_CHROMA_HIST_01, ((vp_dre_h_slope&0xF) << 24)|((dre_s_slope&0xF) << 20)|((dre_y_slope&0xF) << 16)|((initDREReg.dre_h_upper&0xFF) << 8)|(initDREReg.dre_h_lower&0xFF),
                0xFFFFFFF);
        } else {
            MM_REG_WRITE(command, MDP_AAL_DRE_CHROMA_HIST_01, ((dre_h_slope&0xF) << 24)|((dre_s_slope&0xF) << 20)|((dre_y_slope&0xF) << 16)|((initDREReg.dre_h_upper&0xFF) << 8)|(initDREReg.dre_h_lower&0xFF),
                0xFFFFFFF);
        }
        MM_REG_WRITE(command, MDP_AAL_DRE_BITPLUS_00, ((initDREReg.dre_bitplus_signchange_count_slope&0xFF) << 8)|((initDREReg.dre_bitplus_signchange_count_th&0xF) << 4),
            (0xFF<<8)|(0xF<<4));
        MM_REG_WRITE(command, MDP_AAL_DRE_BITPLUS_01, ((initDREReg.dre_bitplus_noise_range_slope&0xFF) << 24)|((initDREReg.dre_bitplus_noise_range_th&0xFF) << 16)|((initDREReg.dre_bitplus_contour_range_slope&0xFF) << 8)|(initDREReg.dre_bitplus_contour_range_th&0xFF),
            MDP_AAL_DRE_BITPLUS_01_MASK);
        MM_REG_WRITE(command, MDP_AAL_DRE_BITPLUS_02, ((initDREReg.dre_bitplus_diff_count_slope&0xFF) << 24)|((initDREReg.dre_bitplus_diff_count_th&0x1F) << 16)|((initDREReg.dre_bitplus_pxl_diff_slope&0xFF) << 8)|(initDREReg.dre_bitplus_pxl_diff_th&0xFF),
            MDP_AAL_DRE_BITPLUS_02_MASK);
        MM_REG_WRITE(command, MDP_AAL_DRE_BITPLUS_03, ((initDREReg.dre_bitplus_high_lvl_out_oft&0xF) << 16)|((initDREReg.dre_bitplus_high_lvl_pxl_slope&0xFF) << 8)|(initDREReg.dre_bitplus_high_lvl_pxl_th&0xFF),
            MDP_AAL_DRE_BITPLUS_03_MASK);
        MM_REG_WRITE(command, MDP_AAL_DRE_BITPLUS_04, ((initDREReg.dre_bitplus_to_ali_wgt&0xF) << 20)|((initDREReg.dre_bitplus_high_lvl_out_oft2&0xF) << 16)|((initDREReg.dre_bitplus_high_lvl_pxl_slope2&0xFF) << 8)|(initDREReg.dre_bitplus_high_lvl_pxl_th2&0xFF),
            MDP_AAL_DRE_BITPLUS_04_MASK);

        if (initDREReg.dre_blk_width > 50)
        {
            // Disable dre_map_bypass
            MM_REG_WRITE(command, MDP_AAL_DRE_MAPPING_00, 0 << 4, 1 << 4);
        }
        else
        {
            // dre_map_bypass
            PQ_LOGI("[PQDREAdaptor] dre_map_bypass, dre_blk_width[%d]", initDREReg.dre_blk_width);
            MM_REG_WRITE(command, MDP_AAL_DRE_MAPPING_00, 1 << 4, 1 << 4);
        }

        // Configure SRAM
        uint32_t configSRAM = 0;
        if (scenario == MEDIA_ISP_PREVIEW || scenario == MEDIA_ISP_CAPTURE)
        {
            if (PQParam.u.isp.dpDREParam.SRAMId == DpDREParam::DRESRAM::SRAM00 ||
                PQParam.u.isp.dpDREParam.SRAMId == DpDREParam::DRESRAM::SRAM01)
            {
                configSRAM = PQParam.u.isp.dpDREParam.SRAMId;
            }

            PQ_LOGI("[PQDREAdaptor] Write curve: SRAMId[%d], userId[0x%llx], cmd[0x%x], scenario[%d]",
                configSRAM, PQParam.u.isp.dpDREParam.userId, PQParam.u.isp.dpDREParam.cmd, scenario);
        }
        if (scenario == MEDIA_VIDEO)
        {
            if (PQParam.u.video.dpDREParam.SRAMId == DpDREParam::DRESRAM::SRAM00 ||
                PQParam.u.video.dpDREParam.SRAMId == DpDREParam::DRESRAM::SRAM01)
            {
                configSRAM = PQParam.u.video.dpDREParam.SRAMId;
            }

            PQ_LOGI("[PQDREAdaptor] Write curve: SRAMId[%d], userId[0x%llx], cmd[0x%x], scenario[%d]",
                configSRAM, PQParam.u.video.dpDREParam.userId, PQParam.u.video.dpDREParam.cmd, scenario);
        }
        MM_REG_WRITE(command, MDP_AAL_SRAM_CFG, (configSRAM << 6)|(configSRAM << 5)|(1 << 4), (0x7 << 4));
    }

    int32_t index = 0;
    // Write DRE 3.0 curve
    writeDRE30Curve(command, &outParam, initDREReg.dre_blk_x_num, initDREReg.dre_blk_y_num, pFrameConfigLabel, index, curveOnly);

    PQ_TIMER_GET_CURRENT_TIME(end);
    PQ_TIMER_GET_DURATION_IN_MS(begin, end, diff);
    if ((m_pDREConfig.driverDebugFlag & DRE_DRIVER_PRINT_TIME) != 0)
    {
        PQ_LOGD("[PQDREAdaptor] calRegs, Config frame, time %d ms\n", diff);
    }
    else
    {
        PQ_LOGI("[PQDREAdaptor] calRegs, Config frame, time %d ms\n", diff);
    }

    return;
}

void PQDREAdaptor::calTileRegs(DpCommand &command,
    const uint32_t hSize, const uint32_t vSize,
    const uint32_t inTileStart, const uint32_t inTileEnd,
    const uint32_t outTileStart, const uint32_t outTileEnd,
    const uint32_t winXStart, const uint32_t winXEnd,
    const uint32_t roiHSize, const uint32_t roiVSize, int32_t scenario)
{
    AutoMutex lock(s_ALMutex);
    DREInitReg *pDREInitReg;

    if ((uint32_t)m_initDREParam.frame_width != hSize || (uint32_t)m_initDREParam.frame_height != vSize)
    {
        m_initDREParam.frame_width = hSize;
        m_initDREParam.frame_height = vSize;
        if (scenario == MEDIA_VIDEO)
            m_pDREVPFW->DREFrameHWRegCal(m_initDREParam);
        else
            m_pDREFW->DREFrameHWRegCal(m_initDREParam);
    }
#ifdef CALTM_VERSION_6771
    m_initDREParam.act_win_x_start = winXStart;
    m_initDREParam.act_win_x_end = winXEnd;
    m_initDREParam.tile_pxl_start = inTileStart;
    m_initDREParam.tile_pxl_end = inTileEnd;
    m_initDREParam.isLastTile = ((uint32_t)inTileEnd+1 >= hSize) ? 1:0;
#elif CALTM_VERSION_6779
    m_initDREParam.act_win_x_start = winXStart;
    m_initDREParam.act_win_x_end = winXEnd;
    m_initDREParam.tile_pxl_x_start = inTileStart;
    m_initDREParam.tile_pxl_x_end = inTileEnd;
    m_initDREParam.isLastTile_x = ((uint32_t)inTileEnd+1 >= hSize) ? 1:0;
    /* tile y is from 0 ~ vSize-1 */
    m_initDREParam.act_win_y_start = 0;
    m_initDREParam.act_win_y_end = roiVSize - 1;
    m_initDREParam.tile_pxl_y_start = 0;
    m_initDREParam.tile_pxl_y_end = roiVSize - 1;
    m_initDREParam.isLastTile_y = ((uint32_t)m_initDREParam.tile_pxl_y_end+1 >= vSize) ? 1:0;
    uint32_t roi_x_end = roiHSize - 1;
    uint32_t roi_x_start = 0;
    uint32_t roi_y_end = roiVSize - 1;
    uint32_t roi_y_start = 0;
#endif

    if (scenario == MEDIA_VIDEO)
    {
        m_pDREVPFW->DRETileHWRegCal(m_initDREParam);
        pDREInitReg = m_pDREVPFW->pDREInitReg;
    }
    else
    {
        m_pDREFW->DRETileHWRegCal(m_initDREParam);
        pDREInitReg = m_pDREFW->pDREInitReg;
    }
#ifdef CALTM_VERSION_6771
    // Set histogram window
    MM_REG_WRITE(command, MDP_AAL_DRE_BLOCK_INFO_00 , (pDREInitReg->act_win_x_end << 13)
                                                     | pDREInitReg->act_win_x_start
                                                    , MDP_AAL_DRE_BLOCK_INFO_00_MASK);

    // Tile Setting
    MM_REG_WRITE(command, MDP_AAL_TILE_00           , (pDREInitReg->blk_cnt_end << 13)
                                                     | pDREInitReg->blk_cnt_start
                                                    , MDP_AAL_TILE_00_MASK);
    MM_REG_WRITE(command, MDP_AAL_TILE_01           , (pDREInitReg->last_tile_flag << 10)
                                                     |(pDREInitReg->blk_num_end << 5)
                                                     | pDREInitReg->blk_num_start
                                                    , MDP_AAL_TILE_01_MASK);
    // Set demo window X
    MM_REG_WRITE(command, MDP_AAL_WIN_X_MAIN        , (pDREInitReg->win_x_end << 16)
                                                     | pDREInitReg->win_x_start
                                                    , MDP_AAL_WIN_X_MAIN_MASK);
#elif CALTM_VERSION_6779
    // Set histogram window
    MM_REG_WRITE(command, MDP_AAL_DRE_BLOCK_INFO_00 , (pDREInitReg->act_win_x_end << 13)
                                                     | pDREInitReg->act_win_x_start
                                                    , MDP_AAL_DRE_BLOCK_INFO_00_MASK);
    MM_REG_WRITE(command, MDP_AAL_DRE_BLOCK_INFO_07 , (pDREInitReg->act_win_y_end << 13)
                                                     | pDREInitReg->act_win_y_start
                                                    , MDP_AAL_DRE_BLOCK_INFO_07_MASK);
    // Tile Setting
    MM_REG_WRITE(command, MDP_AAL_TILE_00       , (pDREInitReg->last_tile_x_flag << 21)
                                                 | (pDREInitReg->last_tile_y_flag << 20)
                                                 | (pDREInitReg->blk_num_x_end << 15)
                                                 | (pDREInitReg->blk_num_x_start << 10)
                                                 | (pDREInitReg->blk_num_y_end << 5)
                                                 | pDREInitReg->blk_num_y_start
                                                , MDP_AAL_TILE_00_MASK);
    MM_REG_WRITE(command, MDP_AAL_TILE_01       , (pDREInitReg->blk_cnt_x_end << 13)
                                                 | pDREInitReg->blk_cnt_x_start
                                                , MDP_AAL_TILE_01_MASK);
    MM_REG_WRITE(command, MDP_AAL_TILE_02       , (pDREInitReg->blk_cnt_y_end << 13)
                                                 | pDREInitReg->blk_cnt_y_start
                                                , MDP_AAL_TILE_02_MASK);
    MM_REG_WRITE(command, MDP_AAL_DRE_ROI_00    , (roi_x_end << 13)
                                                 | roi_x_start
                                                , MDP_AAL_DRE_ROI_00_MASK);
    MM_REG_WRITE(command, MDP_AAL_DRE_ROI_01    , (roi_y_end << 13)
                                                 | roi_y_start
                                                , MDP_AAL_DRE_ROI_01_MASK);
    // Set demo window
    MM_REG_WRITE(command, MDP_AAL_WIN_X_MAIN    , (pDREInitReg->win_x_end << 16)
                                                 | pDREInitReg->win_x_start
                                                , MDP_AAL_WIN_X_MAIN_MASK);
    MM_REG_WRITE(command, MDP_AAL_WIN_Y_MAIN    , (pDREInitReg->win_y_end << 16)
                                                 | pDREInitReg->win_y_start
                                                , MDP_AAL_WIN_Y_MAIN_MASK);
#endif
    if ((m_pDREConfig.driverDebugFlag & DRE_DRIVER_DUMP_TILE) != 0)
    {
        debugDumpTileReg(&m_initDREParam, pDREInitReg, outTileStart, outTileEnd);
    }
}

void PQDREAdaptor::sramWrite(DpCommand &command, unsigned int addr, unsigned int value,
    int32_t *pFrameConfigLabel, int32_t &index, const bool curveOnly)
{
    if (curveOnly == false)
    {
        MM_REG_WRITE(command, MDP_AAL_SRAM_RW_IF_0, addr, MDP_AAL_SRAM_RW_IF_0_MASK);
        MM_REG_POLL(command, MDP_AAL_SRAM_STATUS, (0x1 << 16), (0x1 << 16));
        MM_REG_WRITE(command, MDP_AAL_SRAM_RW_IF_1, value, MDP_AAL_SRAM_RW_IF_1_MASK,
            &pFrameConfigLabel[index++]);
    }
    else
    {
        MM_REG_WRITE(command, MDP_AAL_SRAM_RW_IF_1, value, MDP_AAL_SRAM_RW_IF_1_MASK,
            NULL, pFrameConfigLabel[index++]);
    }
}

void PQDREAdaptor::writeBlock(DpCommand &command, const DRETopOutput *output,
    const uint32_t block_x, const uint32_t block_y,
    const uint32_t dre_blk_x_num, const uint32_t dre_blk_y_num,
    int32_t *pFrameConfigLabel, int32_t &index, const bool curveOnly)
{
    uint32_t write_value;
    uint32_t block_offset = 4 * 4 * (block_y * dre_blk_x_num + block_x);

    write_value = ((output->DRECurveSet[block_y][block_x][0] & 0xff) |
            ((output->DRECurveSet[block_y][block_x][1] & 0xff) << 8) |
            ((output->DRECurveSet[block_y][block_x][2] & 0xff) << 16) |
            ((output->DRECurveSet[block_y][block_x][3] & 0xff) << 24));
    sramWrite(command, DRE30_GAIN_START + block_offset, write_value, pFrameConfigLabel, index, curveOnly);

    block_offset += 4;
    write_value = ((output->DRECurveSet[block_y][block_x][4] & 0xff) |
            ((output->DRECurveSet[block_y][block_x][5] & 0xff) << 8) |
            ((output->DRECurveSet[block_y][block_x][6] & 0xff) << 16) |
            ((output->DRECurveSet[block_y][block_x][7] & 0xff) << 24));
    sramWrite(command, DRE30_GAIN_START + block_offset, write_value, pFrameConfigLabel, index, curveOnly);

    block_offset += 4;
    write_value = ((output->DRECurveSet[block_y][block_x][8] & 0xff) |
            ((output->DRECurveSet[block_y][block_x][9] & 0xff) << 8) |
            ((output->DRECurveSet[block_y][block_x][10] & 0xff) << 16) |
            ((output->DRECurveSet[block_y][block_x][11] & 0xff) << 24));
    sramWrite(command, DRE30_GAIN_START + block_offset, write_value, pFrameConfigLabel, index, curveOnly);

    block_offset += 4;
    write_value = ((output->DRECurveSet[block_y][block_x][12] & 0xff) |
            ((output->DRECurveSet[block_y][block_x][13] & 0xff) << 8) |
            ((output->DRECurveSet[block_y][block_x][14] & 0xff) << 16) |
            ((output->DRECurveSet[block_y][block_x][15] & 0xff) << 24));
    sramWrite(command, DRE30_GAIN_START + block_offset, write_value, pFrameConfigLabel, index, curveOnly);
}

void PQDREAdaptor::writeCurve16(DpCommand &command, const DRETopOutput *output,
    const uint32_t dre_blk_x_num, const uint32_t dre_blk_y_num,
    int32_t *pFrameConfigLabel, int32_t &index, const bool curveOnly)
{
    uint32_t blk_x, blk_y;
    const uint32_t blk_num_max = dre_blk_x_num * dre_blk_y_num;
    uint32_t write_value = 0x0;
    uint32_t bit_shift = 0;
    uint32_t block_offset = 0;

    for (blk_y = 0; blk_y < dre_blk_y_num; blk_y++)
    {
        for (blk_x = 0; blk_x < dre_blk_x_num; blk_x++)
        {
            write_value |= ((output->DRECurveSet[blk_y][blk_x][DRE30_LAST_POINT] & 0xff) << (8*bit_shift));
            bit_shift++;

            if (bit_shift >= 4)
            {
                sramWrite(command, DRE30_GAIN_POINT16_START + block_offset, write_value, pFrameConfigLabel, index, curveOnly);
                block_offset += 4;

                write_value = 0x0;
                bit_shift = 0;
            }
        }
    }

    if ((blk_num_max>>2)<<2 != blk_num_max)
    {
        /* configure last curve */
        sramWrite(command, DRE30_GAIN_POINT16_START + block_offset, write_value, pFrameConfigLabel, index, curveOnly);
    }
}

void PQDREAdaptor::writeDRE30Curve(DpCommand &command, const DRETopOutput *output,
    const uint32_t dre_blk_x_num, const uint32_t dre_blk_y_num,
    int32_t *pFrameConfigLabel, int32_t &index, const bool curveOnly)
{
    uint32_t blk_x, blk_y;

    for (blk_y = 0; blk_y < dre_blk_y_num; blk_y++)
    {
        for (blk_x = 0; blk_x < dre_blk_x_num; blk_x++)
        {
            /* write each block dre curve */
            writeBlock(command, output, blk_x, blk_y, dre_blk_x_num, dre_blk_y_num, pFrameConfigLabel, index, curveOnly);
        }
    }
    /* write each block dre curve last point */
    writeCurve16(command, output, dre_blk_x_num, dre_blk_y_num, pFrameConfigLabel, index, curveOnly);
}

#ifdef PQ_TRANS_BY_SMEN
//Game_PQ
void PQDREAdaptor::debugDumpTransData(const DRETransInfo *initDRETransInfo,
        const DREsharedata *initDREsharedata)
{
    int blk_x, blk_y;
    int dre_blk_x_num = initDRETransInfo->dre_blk_x_num;
    int dre_blk_y_num = initDRETransInfo->dre_blk_y_num;

    PQ_LOGD("[PQDREAdaptor] debugDumpTransData start");
    PQ_LOGD("[PQDREAdaptor] User[%d], DRE block number [y, x] [%d, %d]",
            initDRETransInfo->user, dre_blk_y_num, dre_blk_x_num);

    for (blk_y = 0; blk_y < dre_blk_y_num; blk_y++)
    {
        for (blk_x = 0; blk_x < dre_blk_x_num; blk_x++)
        {
            PQ_LOGD("[PQDREAdaptor] DRE block[%d, %d] PreFloatCurve_game[ 0.. 8]: [ %d, %d, %d,"\
                    "%d, %d, %d, %d, %d, %d]",
                    blk_y, blk_x,
                    initDREsharedata->PreFloatCurve_game[blk_y][blk_x][0],
                    initDREsharedata->PreFloatCurve_game[blk_y][blk_x][1],
                    initDREsharedata->PreFloatCurve_game[blk_y][blk_x][2],
                    initDREsharedata->PreFloatCurve_game[blk_y][blk_x][3],
                    initDREsharedata->PreFloatCurve_game[blk_y][blk_x][4],
                    initDREsharedata->PreFloatCurve_game[blk_y][blk_x][5],
                    initDREsharedata->PreFloatCurve_game[blk_y][blk_x][6],
                    initDREsharedata->PreFloatCurve_game[blk_y][blk_x][7],
                    initDREsharedata->PreFloatCurve_game[blk_y][blk_x][8]);
            PQ_LOGD("[PQDREAdaptor] DRE block[%d, %d] PreFloatCurve_game[ 9.. 16]: [ %d, %d, %d,"\
                    "%d, %d, %d, %d, %d]",
                    blk_y, blk_x,
                    initDREsharedata->PreFloatCurve_game[blk_y][blk_x][9],
                    initDREsharedata->PreFloatCurve_game[blk_y][blk_x][10],
                    initDREsharedata->PreFloatCurve_game[blk_y][blk_x][11],
                    initDREsharedata->PreFloatCurve_game[blk_y][blk_x][12],
                    initDREsharedata->PreFloatCurve_game[blk_y][blk_x][13],
                    initDREsharedata->PreFloatCurve_game[blk_y][blk_x][14],
                    initDREsharedata->PreFloatCurve_game[blk_y][blk_x][15],
                    initDREsharedata->PreFloatCurve_game[blk_y][blk_x][16]);
            PQ_LOGD("[PQDREAdaptor] DRE block[%d, %d] CurTgtCurve_game[ 0.. 8]: [ %d, %d, %d, %d,"\
                    "%d, %d, %d, %d, %d]",
                    blk_y, blk_x,
                    initDREsharedata->CurTgtCurve_game[blk_y][blk_x][0],
                    initDREsharedata->CurTgtCurve_game[blk_y][blk_x][1],
                    initDREsharedata->CurTgtCurve_game[blk_y][blk_x][2],
                    initDREsharedata->CurTgtCurve_game[blk_y][blk_x][3],
                    initDREsharedata->CurTgtCurve_game[blk_y][blk_x][4],
                    initDREsharedata->CurTgtCurve_game[blk_y][blk_x][5],
                    initDREsharedata->CurTgtCurve_game[blk_y][blk_x][6],
                    initDREsharedata->CurTgtCurve_game[blk_y][blk_x][7],
                    initDREsharedata->CurTgtCurve_game[blk_y][blk_x][8]);
            PQ_LOGD("[PQDREAdaptor] DRE block[%d, %d] CurTgtCurve_game[ 9.. 16]: [ %d, %d, %d, %d,"\
                    "%d, %d, %d, %d]",
                    blk_y, blk_x,
                    initDREsharedata->CurTgtCurve_game[blk_y][blk_x][9],
                    initDREsharedata->CurTgtCurve_game[blk_y][blk_x][10],
                    initDREsharedata->CurTgtCurve_game[blk_y][blk_x][11],
                    initDREsharedata->CurTgtCurve_game[blk_y][blk_x][12],
                    initDREsharedata->CurTgtCurve_game[blk_y][blk_x][13],
                    initDREsharedata->CurTgtCurve_game[blk_y][blk_x][14],
                    initDREsharedata->CurTgtCurve_game[blk_y][blk_x][15],
                    initDREsharedata->CurTgtCurve_game[blk_y][blk_x][16]);
            PQ_LOGD("DRE block[%d, %d] FinalCoef_game: %d",
                    blk_y, blk_x, initDREsharedata->FinalCoef_game[blk_y][blk_x]);
        }
    }
    PQ_LOGD("Scenario[%d] debugDumpTransData End");
}

#endif
void PQDREAdaptor::debugDumpFWInput(const DRETopInput *inParam)
{
    int blk_x, blk_y, dre_blk_x_num, dre_blk_y_num;

    PQ_LOGD("Frame Resolution [PreWidth, PreHeight] [%d, %d]", inParam->PreWidth, inParam->PreHeight);
    PQ_LOGD("Frame Resolution [CurWidth, CurHeight] [%d, %d]", inParam->CurWidth, inParam->CurHeight);

    dre_blk_y_num = inParam->dre_blk_y_num;
    dre_blk_x_num = inParam->dre_blk_x_num;
    PQ_LOGD("His block number [y, x] [%d, %d]", dre_blk_y_num, dre_blk_x_num);

    for (blk_y = 0; blk_y < dre_blk_y_num; blk_y++)
    {
        for (blk_x = 0; blk_x < dre_blk_x_num; blk_x++)
        {
            PQ_LOGD("His block[%d, %d] hist[ 0.. 8]: [ %d, %d, %d, %d, %d, %d, %d, %d, %d]",
                blk_y, blk_x,
                inParam->DREMaxHisSet[blk_y][blk_x][0], inParam->DREMaxHisSet[blk_y][blk_x][1], inParam->DREMaxHisSet[blk_y][blk_x][2],
                inParam->DREMaxHisSet[blk_y][blk_x][3], inParam->DREMaxHisSet[blk_y][blk_x][4], inParam->DREMaxHisSet[blk_y][blk_x][5],
                inParam->DREMaxHisSet[blk_y][blk_x][6], inParam->DREMaxHisSet[blk_y][blk_x][7], inParam->DREMaxHisSet[blk_y][blk_x][8]);
            PQ_LOGD("His block[%d, %d] hist[ 9.. 16]: [ %d, %d, %d, %d, %d, %d, %d, %d]",
                blk_y, blk_x,
                inParam->DREMaxHisSet[blk_y][blk_x][9], inParam->DREMaxHisSet[blk_y][blk_x][10], inParam->DREMaxHisSet[blk_y][blk_x][11],
                inParam->DREMaxHisSet[blk_y][blk_x][12], inParam->DREMaxHisSet[blk_y][blk_x][13], inParam->DREMaxHisSet[blk_y][blk_x][14],
                inParam->DREMaxHisSet[blk_y][blk_x][15], inParam->DREMaxHisSet[blk_y][blk_x][16]);
            PQ_LOGD("His block[%d, %d] rgb_max_sum = %d, large_diff_count_set = %d, max_diff_set = %d, chroma_hist = %d, flat_line_count_set = %d",
                blk_y, blk_x,
                inParam->DRERGBMaxSum[blk_y][blk_x], inParam->DRELargeDiffCountSet[blk_y][blk_x],
                inParam->DREMaxDiffSet[blk_y][blk_x], inParam->DREChromaHist[blk_y][blk_x],
                inParam->DREFlatLineCountSet[blk_y][blk_x]);
            PQ_LOGD("His block[%d, %d] PreFloatCurve[ 0.. 8]: [ %d, %d, %d, %d, %d, %d, %d, %d, %d]",
                blk_y, blk_x,
                inParam->PreFloatCurve[blk_y][blk_x][0], inParam->PreFloatCurve[blk_y][blk_x][1], inParam->PreFloatCurve[blk_y][blk_x][2],
                inParam->PreFloatCurve[blk_y][blk_x][3], inParam->PreFloatCurve[blk_y][blk_x][4], inParam->PreFloatCurve[blk_y][blk_x][5],
                inParam->PreFloatCurve[blk_y][blk_x][6], inParam->PreFloatCurve[blk_y][blk_x][7], inParam->PreFloatCurve[blk_y][blk_x][8]);
            PQ_LOGD("His block[%d, %d] PreFloatCurve[ 9.. 16]: [ %d, %d, %d, %d, %d, %d, %d, %d]",
                blk_y, blk_x,
                inParam->PreFloatCurve[blk_y][blk_x][9], inParam->PreFloatCurve[blk_y][blk_x][10], inParam->PreFloatCurve[blk_y][blk_x][11],
                inParam->PreFloatCurve[blk_y][blk_x][12], inParam->PreFloatCurve[blk_y][blk_x][13], inParam->PreFloatCurve[blk_y][blk_x][14],
                inParam->PreFloatCurve[blk_y][blk_x][15], inParam->PreFloatCurve[blk_y][blk_x][16]);
        }
    }
}

void PQDREAdaptor::debugDumpFrameReg(const DRETopOutput *outParam, const DREInitParam *initDREParam, const DREInitReg *initDREReg)
{
    int blk_x, blk_y;
    int dre_blk_x_num = initDREReg->dre_blk_x_num;
    int dre_blk_y_num = initDREReg->dre_blk_y_num;

    PQ_LOGD("frame_width[%d], frame_height[%d]", initDREParam->frame_width, initDREParam->frame_height);
    PQ_LOGD("DRE block number [y, x] [%d, %d]", dre_blk_y_num, dre_blk_x_num);
    PQ_LOGD("blk_width[%d], blk_height[%d]", initDREReg->dre_blk_width, initDREReg->dre_blk_height);

    for (blk_y = 0; blk_y < dre_blk_y_num; blk_y++)
    {
        for (blk_x = 0; blk_x < dre_blk_x_num; blk_x++)
        {
            PQ_LOGD("DRE block[%d, %d] curve[ 0.. 8]: [ %d, %d, %d, %d, %d, %d, %d, %d, %d]",
                blk_y, blk_x,
                outParam->DRECurveSet[blk_y][blk_x][0], outParam->DRECurveSet[blk_y][blk_x][1], outParam->DRECurveSet[blk_y][blk_x][2],
                outParam->DRECurveSet[blk_y][blk_x][3], outParam->DRECurveSet[blk_y][blk_x][4], outParam->DRECurveSet[blk_y][blk_x][5],
                outParam->DRECurveSet[blk_y][blk_x][6], outParam->DRECurveSet[blk_y][blk_x][7], outParam->DRECurveSet[blk_y][blk_x][8]);
            PQ_LOGD("DRE block[%d, %d] curve[ 9.. 16]: [ %d, %d, %d, %d, %d, %d, %d, %d]",
                blk_y, blk_x,
                outParam->DRECurveSet[blk_y][blk_x][9], outParam->DRECurveSet[blk_y][blk_x][10], outParam->DRECurveSet[blk_y][blk_x][11],
                outParam->DRECurveSet[blk_y][blk_x][12], outParam->DRECurveSet[blk_y][blk_x][13], outParam->DRECurveSet[blk_y][blk_x][14],
                outParam->DRECurveSet[blk_y][blk_x][15], outParam->DRECurveSet[blk_y][blk_x][16]);
            PQ_LOGD("DRE block[%d, %d] CurFloatCurve[ 0.. 8]: [ %d, %d, %d, %d, %d, %d, %d, %d, %d]",
                blk_y, blk_x,
                outParam->CurFloatCurve[blk_y][blk_x][0], outParam->CurFloatCurve[blk_y][blk_x][1], outParam->CurFloatCurve[blk_y][blk_x][2],
                outParam->CurFloatCurve[blk_y][blk_x][3], outParam->CurFloatCurve[blk_y][blk_x][4], outParam->CurFloatCurve[blk_y][blk_x][5],
                outParam->CurFloatCurve[blk_y][blk_x][6], outParam->CurFloatCurve[blk_y][blk_x][7], outParam->CurFloatCurve[blk_y][blk_x][8]);
            PQ_LOGD("DRE block[%d, %d] CurFloatCurve[ 9.. 16]: [ %d, %d, %d, %d, %d, %d, %d, %d]",
                blk_y, blk_x,
                outParam->CurFloatCurve[blk_y][blk_x][9], outParam->CurFloatCurve[blk_y][blk_x][10], outParam->CurFloatCurve[blk_y][blk_x][11],
                outParam->CurFloatCurve[blk_y][blk_x][12], outParam->CurFloatCurve[blk_y][blk_x][13], outParam->CurFloatCurve[blk_y][blk_x][14],
                outParam->CurFloatCurve[blk_y][blk_x][15], outParam->CurFloatCurve[blk_y][blk_x][16]);
        }
    }

    PQ_LOGD("demo_win_x_start[%d], demo_win_x_end[%d]", initDREParam->demo_win_x_start, initDREParam->demo_win_x_end);
#ifdef CALTM_VERSION_6779
   PQ_LOGD("demo_win_y_start[%d], demo_win_y_end[%d]", initDREParam->demo_win_y_start, initDREParam->demo_win_y_end);
#endif

}

void PQDREAdaptor::debugDumpTileReg(const DREInitParam *initDREParam, const DREInitReg *initDREReg,
    const uint32_t outTileStart, const uint32_t outTileEnd)
{
#ifdef CALTM_VERSION_6771
    PQ_LOGD("PARA: frame_width[%d], frame_height[%d]", initDREParam->frame_width, initDREParam->frame_height);
    PQ_LOGD("PARA: tile_pxl_start[%d], tile_pxl_end[%d]", initDREParam->tile_pxl_start, initDREParam->tile_pxl_end);

    PQ_LOGD("OUT: outTileStart[%d], outTileEnd[%d]", outTileStart, outTileEnd);

    PQ_LOGD("REG: act_win_x_start[%d], act_win_x_end[%d]", initDREReg->act_win_x_start, initDREReg->act_win_x_end);
    PQ_LOGD("REG: blk_num_start[%d], blk_num_end[%d]", initDREReg->blk_num_start, initDREReg->blk_num_end);
    PQ_LOGD("REG: blk_cnt_start[%d], blk_cnt_end[%d]", initDREReg->blk_cnt_start, initDREReg->blk_cnt_end);

    PQ_LOGD("REG: last_tile_flag[%d]", initDREReg->last_tile_flag);

    PQ_LOGD("REG: win_x_start[%d], win_x_end[%d]", initDREReg->win_x_start, initDREReg->win_x_end);

    PQ_LOGD("Frame REG: dre_blk_width[%d], dre_blk_height[%d]", initDREReg->dre_blk_width, initDREReg->dre_blk_height);
#elif CALTM_VERSION_6779
    PQ_LOGD("PARA: frame_width[%d], frame_height[%d]", initDREParam->frame_width, initDREParam->frame_height);
    PQ_LOGD("PARA: tile_pxl_x_start[%d], tile_pxl_x_end[%d]", initDREParam->tile_pxl_x_start, initDREParam->tile_pxl_x_end);
    PQ_LOGD("PARA: tile_pxl_y_start[%d], tile_pxl_y_end[%d]", initDREParam->tile_pxl_y_start, initDREParam->tile_pxl_y_end);

    PQ_LOGD("OUT: outTileStart[%d], outTileEnd[%d]", outTileStart, outTileEnd);

    PQ_LOGD("REG: act_win_x_start[%d], act_win_x_end[%d]", initDREReg->act_win_x_start, initDREReg->act_win_x_end);
    PQ_LOGD("REG: act_win_y_start[%d], act_win_y_end[%d]", initDREReg->act_win_y_start, initDREReg->act_win_y_end);
    PQ_LOGD("REG: blk_num_x_start[%d], blk_num_x_end[%d]", initDREReg->blk_num_x_start, initDREReg->blk_num_x_end);
    PQ_LOGD("REG: blk_num_y_start[%d], blk_num_y_end[%d]", initDREReg->blk_num_y_start, initDREReg->blk_num_y_end);
    PQ_LOGD("REG: blk_cnt_x_start[%d], blk_cnt_x_end[%d]", initDREReg->blk_cnt_x_start, initDREReg->blk_cnt_x_end);
    PQ_LOGD("REG: blk_cnt_y_start[%d], blk_cnt_y_end[%d]", initDREReg->blk_cnt_y_start, initDREReg->blk_cnt_y_end);

    PQ_LOGD("REG: last_tile_x_flag[%d], last_tile_y_flag[%d]", initDREReg->last_tile_x_flag, initDREReg->last_tile_y_flag);

    PQ_LOGD("REG: win_x_start[%d], win_x_end[%d]", initDREReg->win_x_start, initDREReg->win_x_end);
    PQ_LOGD("REG: win_y_start[%d], win_y_end[%d]", initDREReg->win_y_start, initDREReg->win_y_end);

    PQ_LOGD("Frame REG: dre_blk_width[%d], dre_blk_height[%d]", initDREReg->dre_blk_width, initDREReg->dre_blk_height);
#endif
}

template <class regType>
void PQDREAdaptor::writeStringWithTag(const char *pTag, const regType *pSrc, char *pDst, int &cnt)
{
    char *pRegDump = (char*)pSrc;
    unsigned long i;

    cnt += sprintf(pDst + cnt, "%s\n", pTag);

    if (pSrc != NULL)
    {
        for (i = 0; i < sizeof(regType); i++)
        {
            cnt += sprintf(pDst + cnt, "%02X\n", pRegDump[i]);
        }
    }
    else
    {
        cnt += sprintf(pDst + cnt, "No Data since first frame or bypass...\n");
    }
}

void PQDREAdaptor::dumpFwReg(const bool isApplyFw, const DpPqParam *PQParam,
    const DRETopInput *inReg, const DRETopOutput *outReg, const DREInitReg *initReg, const DREReg *swReg,
    const TAdaptiveCALTMReg *adaptiveCALTMReg, const TAdaptiveCALTMFace_Exif *adaptiveCALTMFace_exif, const TAdaptiveCALTMFace_Dump *adaptiveCALTMFace_dump)
{
    DP_TRACE_CALL();

    char name[128] = {0};
    bool forceEnableDump = PQParam->u.isp.enableDump;

#ifdef DEBUG_DUMP_REG
    m_enableDumpRegister = DpDriver::getInstance()->getEnableDumpRegister();
#endif
    if (!m_enableDumpRegister && !forceEnableDump &&
        (PQParam->u.isp.p_mdpSetting == NULL || PQParam->u.isp.p_mdpSetting->buffer == NULL))
        return;

    if (PQParam->scenario == MEDIA_ISP_PREVIEW || PQParam->scenario == MEDIA_ISP_CAPTURE)
    {
        if (!(PQParam->u.isp.dpDREParam.cmd & DpDREParam::Cmd::Default) &&
            !(PQParam->u.isp.dpDREParam.cmd & DpDREParam::Cmd::Apply))
        {
            return;
        }

        if (forceEnableDump ||
            (((m_enableDumpRegister == DUMP_ISP_PRV && PQParam->scenario == MEDIA_ISP_PREVIEW) ||
            (m_enableDumpRegister == DUMP_ISP_CAP && PQParam->scenario == MEDIA_ISP_CAPTURE) ||
            (m_enableDumpRegister == DUMP_ISP_PRV_CAP)) &&
            PQParam->u.isp.timestamp != 0xFFFFFFFF))
            sprintf(name, "/data/vendor/camera_dump/%09d-%04d-%04d-MDP-%s-%d-%s.mdp",
                PQParam->u.isp.timestamp,
                PQParam->u.isp.requestNo,
                PQParam->u.isp.frameNo,
                (PQParam->scenario == MEDIA_ISP_PREVIEW) ? "Prv" : "CAP",
                PQParam->u.isp.lensId,
                PQParam->u.isp.userString);
        else if (PQParam->scenario == MEDIA_ISP_CAPTURE)
            PQ_LOGI("[PQDREAdaptor] Dump Exif for ISP_CAPTURE");
        else
            return;
    }
    else
    {
        PQ_LOGI("[PQDREAdaptor] Unknown scenario = [%d]", PQParam->scenario);
        return;
    }

    uint32_t bufferSize = 256 + 3*(sizeof(DRETopInput) + sizeof(DRETopOutput) + sizeof(DREInitReg) + sizeof(DREReg) + sizeof(TAdaptiveCALTMReg) + sizeof(TAdaptiveCALTMFace_Exif) + sizeof(TAdaptiveCALTMFace_Dump));
    char *buffer = new char[bufferSize];
    int cnt = 0;

    if (!m_enableDumpRegister && !forceEnableDump)
        goto EXIF;

    writeStringWithTag(DRE_NVRAMINDEX_TAG, &PQParam->u.isp.dpDREParam.customIndex, buffer, cnt);
    if (isApplyFw == true)
    {
        writeStringWithTag(DRE_INPUT_TAG, inReg, buffer, cnt);
    }
    writeStringWithTag(DRE_OUTPUT_TAG, outReg, buffer, cnt);
    writeStringWithTag(DRE_INITREG_TAG, initReg, buffer, cnt);
    writeStringWithTag(DRE_SWREG_TAG, swReg, buffer, cnt);
    writeStringWithTag(ADAPTIVECALTMREG_TAG, adaptiveCALTMReg, buffer, cnt);
    writeStringWithTag(ADAPTIVECALTMFACE_DUMP_REG_TAG, adaptiveCALTMFace_dump, buffer, cnt);

    cnt += sprintf(buffer + cnt, DRE_FW_VERSION);
    cnt += sprintf(buffer + cnt, "FW Reg dump end...\n");

    FILE *pFile;

    pFile = fopen(name, "ab");

    if (NULL != pFile)
    {
        fwrite(buffer, cnt, 1, pFile);

        fclose(pFile);

        PQ_LOGD("[PQDREAdaptor]Dump register to %s, cnt[%d], bufferSize[%d]\n", name, cnt, bufferSize);
    }
    else
    {
        PQ_LOGD("[PQDREAdaptor]Open %s failed, \n", name);
    }

EXIF:
    if (PQParam->u.isp.p_mdpSetting != NULL && PQParam->u.isp.p_mdpSetting->buffer != NULL)
    {
        char* p_mdpsetting_buf = (char*)PQParam->u.isp.p_mdpSetting->buffer;
        uint32_t *mdpsetting_offset = &(PQParam->u.isp.p_mdpSetting->offset);
        uint32_t mdpsetting_max_size = PQParam->u.isp.p_mdpSetting->size;

        cnt = 0;
        writeStringWithTag(DRE_NVRAMINDEX_TAG, &PQParam->u.isp.dpDREParam.customIndex, buffer, cnt);
        writeStringWithTag(DRE_INITREG_TAG, initReg, buffer, cnt);
        writeStringWithTag(DRE_SWREG_TAG, swReg, buffer, cnt);
        writeStringWithTag(ADAPTIVECALTMREG_TAG, adaptiveCALTMReg, buffer, cnt);
        writeStringWithTag(ADAPTIVECALTMFACE_EXIF_REG_TAG, adaptiveCALTMFace_exif, buffer, cnt);

        cnt += sprintf(buffer + cnt, DRE_FW_VERSION);
        cnt += sprintf(buffer + cnt, "FW Reg dump end...\n");

        if (*mdpsetting_offset + cnt > mdpsetting_max_size)
        {
            PQ_LOGD("[PQDREAdaptor] mdpSetting buffer overflow, offset[%d], cnt[%d], mdpsetting_max_size[%d]\n",
                *mdpsetting_offset, cnt, mdpsetting_max_size);

            delete []buffer;

            return;
        }

        memcpy(p_mdpsetting_buf + *mdpsetting_offset, buffer, cnt);
        *mdpsetting_offset += cnt;
    }

    delete []buffer;
}

#ifdef SUPPORT_DUMP_IN_FW
void PQDREAdaptor::dumpFwRegInFw(const DpPqParam *PQParam)
{
    DP_TRACE_CALL();

    char name[128] = {0};
    bool forceEnableDump = PQParam->u.isp.enableDump;

#ifdef DEBUG_DUMP_REG
    m_enableDumpRegister = DpDriver::getInstance()->getEnableDumpRegister();
#endif
    if (!m_enableDumpRegister && !forceEnableDump &&
        (PQParam->u.isp.p_mdpSetting == NULL || PQParam->u.isp.p_mdpSetting->buffer == NULL))
        return;

    if (PQParam->scenario == MEDIA_ISP_PREVIEW || PQParam->scenario == MEDIA_ISP_CAPTURE)
    {
        if (!(PQParam->u.isp.dpDREParam.cmd & DpDREParam::Cmd::Default) &&
            !(PQParam->u.isp.dpDREParam.cmd & DpDREParam::Cmd::Apply))
        {
            return;
        }

        if (forceEnableDump ||
            (((m_enableDumpRegister == DUMP_ISP_PRV && PQParam->scenario == MEDIA_ISP_PREVIEW) ||
            (m_enableDumpRegister == DUMP_ISP_CAP && PQParam->scenario == MEDIA_ISP_CAPTURE) ||
            (m_enableDumpRegister == DUMP_ISP_PRV_CAP)) &&
            PQParam->u.isp.timestamp != 0xFFFFFFFF))
            sprintf(name, "/data/vendor/camera_dump/%09d-%04d-%04d-MDP-%s-%d-%s.mdp",
                PQParam->u.isp.timestamp,
                PQParam->u.isp.requestNo,
                PQParam->u.isp.frameNo,
                (PQParam->scenario == MEDIA_ISP_PREVIEW) ? "Prv" : "CAP",
                PQParam->u.isp.lensId,
                PQParam->u.isp.userString);
        else if (PQParam->scenario == MEDIA_ISP_CAPTURE)
            PQ_LOGI("[PQDREAdaptor] Dump Exif for ISP_CAPTURE");
        else
            return;
    }
    else
    {
        PQ_LOGI("[PQDREAdaptor] Unknown scenario = [%d]", PQParam->scenario);
        return;
    }

    uint32_t bufferSize_mdp = 200000;
    uint32_t bufferSize_exif = 20000;
    char *buffer_mdp = NULL;
    char *buffer_exif = new char[bufferSize_exif];
    int cnt_mdp = 0;
    int cnt_exif = 0;

    if (m_enableDumpRegister || forceEnableDump)
    {
        buffer_mdp = new char[bufferSize_mdp];
    }
    if (buffer_mdp != NULL)
    {
        cnt_mdp += sprintf(buffer_mdp + cnt_mdp, "____mdp_section_start____\n");
    }
    cnt_exif += sprintf(buffer_exif + cnt_exif, "____mdp_section_start____\n");
    m_pAdaptiveCALTMFW->DumpReg(buffer_exif, buffer_mdp, cnt_exif, cnt_mdp, bufferSize_exif, bufferSize_mdp);
    if (buffer_mdp != NULL)
    {
        cnt_mdp += sprintf(buffer_mdp + cnt_mdp, "____mdp_section_end____\n");
    }
    cnt_exif += sprintf(buffer_exif + cnt_exif, "____mdp_section_end____\n");
    PQ_LOGI("[PQDREAdaptor] dumpFwRegInFw: cnt_exif[%d] cnt_mdp[%d] bufferSize_exif[%d] bufferSize_mdp[%d]",
        cnt_exif, cnt_mdp, bufferSize_exif, bufferSize_mdp);

    if (!m_enableDumpRegister && !forceEnableDump)
        goto EXIF;

    FILE *pFile;

    pFile = fopen(name, "ab");

    if (NULL != pFile)
    {
        fwrite(buffer_mdp, cnt_mdp, 1, pFile);

        fclose(pFile);

        PQ_LOGD("[PQDREAdaptor]Dump register to %s, cnt[%d], bufferSize[%d]\n", name, cnt_mdp, bufferSize_mdp);
    }
    else
    {
        PQ_LOGD("[PQDREAdaptor]Open %s failed, \n", name);
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

        if (*mdpsetting_offset + cnt_exif > mdpsetting_max_size)
        {
            PQ_LOGD("[PQDREAdaptor] mdpSetting buffer overflow, offset[%d], cnt[%d], mdpsetting_max_size[%d]\n",
                *mdpsetting_offset, cnt_exif, mdpsetting_max_size);

            delete []buffer_exif;

            return;
        }

        memcpy(p_mdpsetting_buf + *mdpsetting_offset, buffer_exif, cnt_exif);
        *mdpsetting_offset += cnt_exif;
    }
    delete []buffer_exif;
}
#endif
