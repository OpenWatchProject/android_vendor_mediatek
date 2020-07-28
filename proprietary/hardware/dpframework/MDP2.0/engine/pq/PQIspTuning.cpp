#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1

#include "PQIspTuning.h"
#include "PQLogger.h"
#include "PQServiceCommon.h"

#include <fstream>
#include <cutils/properties.h>

PQIspTuning* PQIspTuning::s_pInstance = NULL;
PQMutex   PQIspTuning::s_ALMutex;

#define PQ_ISPTUNING_STR "vendor.debug.camera.dumpin.path.config"
#define PQ_ISPTUNING_DEFAULT ""
#define PQ_TUNING_CFG_TYPE "mdp"
#define PQ_TUNING_CFG_FILENAME_NUM (3)

#define DS_SWREG_MIN (MOD_DS_SWREG << 16)
#define DS_SWREG_MAX (DS_SWREG_MIN + sizeof(DSReg))
#define RSZ_SWREG_MIN (MOD_RSZ_SWREG << 16)
#define RSZ_SWREG_MAX (RSZ_SWREG_MIN + sizeof(RszReg))
#define DS_INPUT_MIN (MOD_DS_INPUT << 16)
#define DS_INPUT_MAX (DS_INPUT_MIN + sizeof(DSInput))
#define RSZ_INPUT_MIN (MOD_RSZ_INPUT << 16)
#define RSZ_INPUT_MAX (RSZ_INPUT_MIN + sizeof(RszInput))
#define HFG_INPUT_MIN (MOD_HFG_INPUT << 16)
#define HFG_INPUT_MAX (HFG_INPUT_MIN + sizeof(HFGInput_frame))
#define DRE_SWREG_MIN (0x20003000)
#define DRE_SWREG_MAX (DRE_SWREG_MIN + sizeof(DREToolReg))
#define ADAPTIVECALTM_REG_MIN (0x32000000)
#define ADAPTIVECALTM_REG_MAX (ADAPTIVECALTM_REG_MIN + sizeof(TAdaptiveCALTMReg))
#define ADAPTIVECALTMFACE_REG_MIN (ADAPTIVECALTM_REG_MAX)
#define ADAPTIVECALTMFACE_REG_MAX (ADAPTIVECALTMFACE_REG_MIN + sizeof(TAdaptiveCALTMFace_Dump))
#define TUNING_STRING_BUFFER_MAX (256)

static inline int valueOf(const char ch)
{
    if ('0' <= ch && ch <= '9')
        return (ch - '0');
    else if ('a' <= ch && ch <= 'z')
        return (ch - 'a') + 10;
    else if ('A' <= ch && ch <= 'Z')
        return (ch - 'A') + 10;

    return 0;
}

PQIspTuning* PQIspTuning::getInstance()
{
    AutoMutex lock(s_ALMutex);

    if(NULL == s_pInstance)
    {
        s_pInstance = new PQIspTuning();
        atexit(PQIspTuning::destroyInstance);
    }

    return s_pInstance;
}

void PQIspTuning::destroyInstance()
{
    AutoMutex lock(s_ALMutex);

    if (NULL != s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = NULL;
    }
}

PQIspTuning::PQIspTuning()
{
    PQ_LOGD("[PQIspTuning] PQIspTuning()... ");

    m_pTuningDreReg = new DREToolReg;
    memset(m_pTuningDreReg, 0, sizeof(DREToolReg));

    m_pTuningAdaptiveCaltmReg = new TAdaptiveCALTMReg;
    memset(m_pTuningAdaptiveCaltmReg, 0, sizeof(TAdaptiveCALTMReg));

    m_pTuningAdaptiveCaltmFaceReg = new TAdaptiveCALTMFace_Dump;
    memset(m_pTuningAdaptiveCaltmFaceReg, 0, sizeof(TAdaptiveCALTMFace_Dump));

    m_pTuningRszReg = new RszReg;
    memset(m_pTuningRszReg, 0, sizeof(RszReg));

    m_pTuningDSReg = new DSReg;
    memset(m_pTuningDSReg, 0, sizeof(DSReg));

    m_pTuningRszInput = new RszInput;
    memset(m_pTuningRszInput, 0, sizeof(RszInput));

    m_pTuningDSInput = new DSInput;
    memset(m_pTuningDSInput, 0, sizeof(DSInput));

    m_pTuningHFGInput = new HFGInput_frame;
    memset(m_pTuningHFGInput, 0, sizeof(HFGInput_frame));

    m_loadFileSuccess = false;
}

PQIspTuning::~PQIspTuning()
{
    PQ_LOGD("[PQIspTuning] ~PQIspTuning()... ");

    delete m_pTuningDreReg;
    delete m_pTuningAdaptiveCaltmReg;
    delete m_pTuningAdaptiveCaltmFaceReg;
    delete m_pTuningRszReg;
    delete m_pTuningDSReg;
    delete m_pTuningRszInput;
    delete m_pTuningDSInput;
    delete m_pTuningHFGInput;
}

bool PQIspTuning::getTuningValue(const char *str, uint32_t &tuningAddress, uint32_t &tuningValue)
{
    uint32_t val = 0;
    uint32_t validNum = 0;
    uint32_t arrayIndex;
    const uint32_t base = 16;
    const char *char_p = str;

    for (arrayIndex = 0; arrayIndex < TUNING_STRING_BUFFER_MAX; arrayIndex++)
    {
        if (char_p[arrayIndex] == '\n' || char_p[arrayIndex] == '\0')
        {
            break;
        }
        else if (char_p[arrayIndex] == '=')
        {
            tuningAddress = val;
            val = 0;
            validNum++;
            continue;
        }
        else if (char_p[arrayIndex] == ',')
        {
            tuningValue = val;
            validNum++;
            break;
        }
        else if (char_p[arrayIndex] == ' ')
        {
            continue;
        }

        val = val * base + valueOf(char_p[arrayIndex]);
    }

    return ((validNum >= 2) ? true : false);
}

bool PQIspTuning::loadIspTuningFile(void)
{
    AutoMutex lock(s_ALMutex);

    std::ifstream cfgFile;
    std::ifstream tuningFile;
    char propertyString[PROPERTY_VALUE_MAX];
    char fileNameString[TUNING_STRING_BUFFER_MAX] = PQ_ISPTUNING_DEFAULT;
    char tuningString[TUNING_STRING_BUFFER_MAX];
    uint32_t tuningAddress, tuningValue;

    m_loadFileSuccess = false;

    property_get(PQ_ISPTUNING_STR, propertyString, PQ_ISPTUNING_DEFAULT);
    if (strncmp(propertyString, PQ_ISPTUNING_DEFAULT, sizeof(propertyString)) == 0)
    {
        PQ_LOGE("[PQIspTuning] onIspTuning, property[%s] is NULL", PQ_ISPTUNING_STR);

        return m_loadFileSuccess;
    }

    cfgFile.open(propertyString);
    if (cfgFile.is_open())
    {
        char strType[TUNING_STRING_BUFFER_MAX];
        char strFile[TUNING_STRING_BUFFER_MAX];

        PQ_LOGD("[PQIspTuning] onIspTuning, load cfgFile[%s] sucess", propertyString);
        while (!cfgFile.eof())
        {
            cfgFile.getline(tuningString, (TUNING_STRING_BUFFER_MAX-1));
            sscanf(tuningString,"%[^,],%*[^,],%*[^;];%s\n", strType, strFile);

            if (strncmp(strType, PQ_TUNING_CFG_TYPE, sizeof(strType)) == 0)
            {
                strncpy(fileNameString, strFile, sizeof(fileNameString)-1);
            }
        }
        cfgFile.close();
    }
    else
    {
        PQ_LOGE("[PQIspTuning] onIspTuning, load cfgFile[%s] fail", propertyString);

        return m_loadFileSuccess;
    }

    uint32_t *pDreReg = (uint32_t*)m_pTuningDreReg;
    uint32_t *pAdaptiveCaltmReg = (uint32_t*)m_pTuningAdaptiveCaltmReg;
    uint32_t *pAdaptiveCaltmFaceReg = (uint32_t*)m_pTuningAdaptiveCaltmFaceReg;
    uint32_t *pRszReg = (uint32_t*)m_pTuningRszReg;
    uint32_t *pDSReg = (uint32_t*)m_pTuningDSReg;
    uint32_t *pRszInput = (uint32_t*)m_pTuningRszInput;
    uint32_t *pDSInput = (uint32_t*)m_pTuningDSInput;
    uint32_t *pHFGInput = (uint32_t*)m_pTuningHFGInput;

    tuningFile.open(fileNameString);
    if (tuningFile.is_open())
    {
        PQ_LOGD("[PQIspTuning] onIspTuning, load tuning file[%s] sucess", fileNameString);
        while (!tuningFile.eof())
        {
            tuningFile.getline(tuningString, (TUNING_STRING_BUFFER_MAX-1));
            if (getTuningValue(tuningString, tuningAddress, tuningValue) == true)
            {
                if ((tuningAddress & 0x3) != 0)
                {
                    PQ_LOGI("[PQIspTuning] onIspTuning, Invalid Address[0x%08x]:Value[0x%08x]", tuningAddress, tuningValue);
                    continue;
                }

                if (tuningAddress >= DRE_SWREG_MIN && tuningAddress < DRE_SWREG_MAX)
                {
                    PQ_LOGI("[PQIspTuning] onIspTuning, Overwrite DRE Address[0x%08x]:Value[0x%08x]", tuningAddress, tuningValue);
                    *(pDreReg + (tuningAddress-DRE_SWREG_MIN)/4) = tuningValue;
                }
                else if (tuningAddress >= ADAPTIVECALTM_REG_MIN && tuningAddress < ADAPTIVECALTM_REG_MAX)
                {
                    PQ_LOGI("[PQIspTuning] onIspTuning, Overwrite AdaptiveCALTM Address[0x%08x]:Value[0x%08x]", tuningAddress, tuningValue);
                    *(pAdaptiveCaltmReg + (tuningAddress-ADAPTIVECALTM_REG_MIN)/4) = tuningValue;
                }
                else if (tuningAddress >= ADAPTIVECALTMFACE_REG_MIN && tuningAddress < ADAPTIVECALTMFACE_REG_MAX)
                {
                    PQ_LOGI("[PQIspTuning] onIspTuning, Overwrite AdaptiveCALTM Face Address[0x%08x]:Value[0x%08x]", tuningAddress, tuningValue);
                    *(pAdaptiveCaltmFaceReg + (tuningAddress-ADAPTIVECALTMFACE_REG_MIN)/4) = tuningValue;
                }
                else if (tuningAddress >= RSZ_SWREG_MIN && tuningAddress < RSZ_SWREG_MAX)
                {
                    PQ_LOGI("[PQIspTuning] onIspTuning, Overwrite RSZReg Address[0x%08x]:Value[0x%08x]", tuningAddress, tuningValue);
                    *(pRszReg + (tuningAddress-RSZ_SWREG_MIN)/4) = tuningValue;
                }
                else if (tuningAddress >= DS_SWREG_MIN && tuningAddress < DS_SWREG_MAX)
                {
                    PQ_LOGI("[PQIspTuning] onIspTuning, Overwrite DSReg Address[0x%08x]:Value[0x%08x]", tuningAddress, tuningValue);
                    *(pDSReg + (tuningAddress-DS_SWREG_MIN)/4) = tuningValue;
                }
                else if (tuningAddress >= RSZ_INPUT_MIN && tuningAddress < RSZ_INPUT_MAX)
                {
                    PQ_LOGI("[PQIspTuning] onIspTuning, Overwrite RszInput Address[0x%08x]:Value[0x%08x]", tuningAddress, tuningValue);
                    *(pRszInput + (tuningAddress-RSZ_INPUT_MIN)/4) = tuningValue;
                }
                else if (tuningAddress >= DS_INPUT_MIN && tuningAddress < DS_INPUT_MAX)
                {
                    PQ_LOGI("[PQIspTuning] onIspTuning, Overwrite DSInput Address[0x%08x]:Value[0x%08x]", tuningAddress, tuningValue);
                    *(pDSInput + (tuningAddress-DS_INPUT_MIN)/4) = tuningValue;
                }
                else if (tuningAddress >= HFG_INPUT_MIN && tuningAddress < HFG_INPUT_MAX)
                {
                    PQ_LOGI("[PQIspTuning] onIspTuning, Overwrite HFGInput_frame Address[0x%08x]:Value[0x%08x]", tuningAddress, tuningValue);
                    *(pHFGInput + (tuningAddress-HFG_INPUT_MIN)/4) = tuningValue;
                }
                else
                {
                    PQ_LOGI("[PQIspTuning] onIspTuning, Unknown Address[0x%08x]:Value[0x%08x]", tuningAddress, tuningValue);
                }

            }
        }

        tuningFile.close();

        m_loadFileSuccess = true;
    }
    else
    {
        PQ_LOGE("[PQIspTuning] onIspTuning, load tuning file[%s] fail", fileNameString);
    }

    return m_loadFileSuccess;
}

bool PQIspTuning::getIspTuningFileStatus(void)
{
    AutoMutex lock(s_ALMutex);

    return m_loadFileSuccess;
}

void PQIspTuning::getDreTuningValue(DREToolReg *appliedDreReg, TAdaptiveCALTMReg *appliedAdaptiveCALTM, TAdaptiveCALTMFace_Dump *appliedAdaptiveCALTMFace_dump)
{
    AutoMutex lock(s_ALMutex);

    memcpy(appliedDreReg, m_pTuningDreReg, sizeof(DREToolReg));
    memcpy(appliedAdaptiveCALTM, m_pTuningAdaptiveCaltmReg, sizeof(TAdaptiveCALTMReg));
    memcpy(&(appliedAdaptiveCALTMFace_dump->adaptiveCALTMFace_Exif), &(m_pTuningAdaptiveCaltmFaceReg->adaptiveCALTMFace_Exif), sizeof(TAdaptiveCALTMFace_Exif));
}

void PQIspTuning::getRszTuningValue(RszReg *appliedRszReg, RszInput *appliedRszInput)
{
    AutoMutex lock(s_ALMutex);

    memcpy(appliedRszReg, m_pTuningRszReg, sizeof(RszReg));
    memcpy(&(appliedRszInput->iHWReg), &(m_pTuningRszInput->iHWReg), sizeof(RszRegClearZoom));
}

void PQIspTuning::getDSTuningValue(DSReg *appliedDSReg, DSInput *appliedDSInput)
{
    AutoMutex lock(s_ALMutex);

    memcpy(appliedDSReg, m_pTuningDSReg, sizeof(DSReg));
    memcpy(appliedDSInput, m_pTuningDSInput, sizeof(DSInput));
}

void PQIspTuning::getHFGTuningValue(HFGInput_frame *appliedHFGInput)
{
    AutoMutex lock(s_ALMutex);

    memcpy(appliedHFGInput, m_pTuningHFGInput, sizeof(HFGInput_frame));
}

