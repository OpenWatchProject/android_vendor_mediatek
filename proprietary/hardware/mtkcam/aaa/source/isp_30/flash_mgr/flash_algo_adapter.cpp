/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define LOG_TAG "FlashAlgoAdapter"

#include <array>

/* aaa common headers */
#include "log_utils.h"
#include <private/aaa_utils.h>
#include <private/aaa_hal_private.h>

/* flash headers */
#include "flash_algo_adapter.h"
#include "flash_param.h"
#include "flash_utils.h"

/* custom headers */
#include "flash_tuning_custom.h"
#include "camera_custom_nvram.h"

/* kernel headers */
#include "kd_camera_feature.h"

using namespace NS3A;

FlashAlgoAdapter::FlashAlgoAdapter(int sensorDev)
    : mpFlashAlg(NULL)
{
    /* set debug */
    setDebug();

    int facing = 0, num = 0;
    NS3Av3::mapSensorDevToFace(sensorDev, facing, num);

#if USE_OPEN_SOURCE_FLASH_AE
    if(facing == 0)
        mpFlashAlg = FlashAlg::getInstanceLite(FLASH_CAMERA_REAR_SENSOR);
    else if(facing == 1)
        mpFlashAlg = FlashAlg::getInstanceLite(FLASH_CAMERA_FRONT_SENSOR);
    else
        mpFlashAlg = FlashAlg::getInstanceLite(FLASH_CAMERA_DEFAULT_SENSOR);
#else
    if(facing == 0)
        mpFlashAlg = FlashAlg::getInstance(FLASH_CAMERA_REAR_SENSOR);
    else if(facing == 1)
        mpFlashAlg = FlashAlg::getInstance(FLASH_CAMERA_FRONT_SENSOR);
    else
        mpFlashAlg = FlashAlg::getInstance(FLASH_CAMERA_DEFAULT_SENSOR);
#endif
}

FlashAlgoAdapter::~FlashAlgoAdapter()
{
}

int FlashAlgoAdapter::init()
{
    logI("%s+", __func__);
    return 0;
}

int FlashAlgoAdapter::unInit()
{
    logI("%s+", __func__);
    return 0;
}

#define MY_INST NS3Av3::INST_T<FlashAlgoAdapter>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

FlashAlgoAdapter *FlashAlgoAdapter::getInstance(int sensorDev)
{
    int sensorOpenIndex = NS3Av3::mapSensorDevToIdx(sensorDev);
    if(sensorOpenIndex >= SENSOR_IDX_MAX || sensorOpenIndex < 0) {
        logE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, sensorOpenIndex);
        return NULL;
    }

    MY_INST& rSingleton = gMultiton[sensorOpenIndex];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<FlashAlgoAdapter>(sensorDev);
    } );

    return rSingleton.instance.get();
}

int FlashAlgoAdapter::Estimate(FlashAlgExpPara* exp, FlashAlgFacePos* pFaceInfo, int* isLowRef)
{
    logI("%s+", __func__);
    mpFlashAlg->Estimate(exp, pFaceInfo, isLowRef);
    return 0;
}

int FlashAlgoAdapter::setFlashType(int type)
{
    logI("%s+", __func__);

    mpFlashAlg->init(type);
    return 0;
}

int FlashAlgoAdapter::Reset()
{
    logI("%s+", __func__);
    mpFlashAlg->Reset();
    return 0;
}

int FlashAlgoAdapter::AddStaData10(FlashAlgStaData* pData, FlashAlgExpPara* pExp,
        int* pIsNeedNext, FlashAlgExpPara* pExpNext, bool last)
{
    logI("%s+", __func__);
    mpFlashAlg->AddStaData10(pData, pExp, pIsNeedNext, pExpNext, last);
    return 0;
}

int FlashAlgoAdapter::CalFirstEquAEPara(FlashAlgExpPara* exp, FlashAlgExpPara* EquExp)
{
    logI("%s+", __func__);
    mpFlashAlg->CalFirstEquAEPara(exp, EquExp);
    return 0;
}

int FlashAlgoAdapter::setFlashProfile(FlashAlgStrobeProfile* pr)
{
    logI("%s+", __func__);
    mpFlashAlg->setFlashProfile(pr);
    return 0;
}

int FlashAlgoAdapter::setCapturePLine(PLine* p, int isoAtGain1x)
{
    logI("%s+", __func__);
    mpFlashAlg->setCapturePLine(p, isoAtGain1x);
    return 0;
}

int FlashAlgoAdapter::setPreflashPLine(PLine* p, int isoAtGain1x)
{
    logI("%s+", __func__);
    mpFlashAlg->setPreflashPLine(p, isoAtGain1x);
    return 0;
}

int FlashAlgoAdapter::setEVCompEx(float ev_comp, float tar, float evLevel)
{
    logI("%s+", __func__);
    mpFlashAlg->setEVCompEx(ev_comp, tar, evLevel);
    return 0;
}

int FlashAlgoAdapter::setDefaultPreferences()
{
    logI("%s+", __func__);
    mpFlashAlg->setDefaultPreferences();
    return 0;
}

int FlashAlgoAdapter::setTuningPreferences(FLASH_TUNING_PARA *tuning_p, int lv)
{
    logI("%s+", __func__);
    mpFlashAlg->setTuningPreferences(tuning_p, lv);
    return 0;
}

int FlashAlgoAdapter::setFlashInfo(FlashAlgInputInfoStruct *pFlashInfo)
{
    logI("%s+", __func__);
    mpFlashAlg->setFlashInfo(pFlashInfo);
    return 0;
}

int FlashAlgoAdapter::Flash_Awb_Init(FLASH_AWB_INIT_T &FlashAwbInit)
{
    logI("%s+", __func__);
    mpFlashAlg->Flash_Awb_Init(FlashAwbInit);
    return 0;
}

int FlashAlgoAdapter::Flash_Awb_Algo(int Exp, int Iso, MUINT32 *FlashResultWeight)
{
    logI("%s+", __func__);
    mpFlashAlg->Flash_Awb_Algo(FlashResultWeight);
    return 0;
}

MRESULT FlashAlgoAdapter::DecideCalFlashComb(int CalNum,
        NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT *pNvCali, int totalStep,
        int totalStepLT, ChooseResult *pChoose, bool dualFlashEnable, int caliNvIdx)
{
    (void)caliNvIdx;
    logI("%s+", __func__);
    return mpFlashAlg->DecideCalFlashComb(CalNum,
            pNvCali->yTab, totalStep, totalStepLT, pChoose, dualFlashEnable);
}

MRESULT FlashAlgoAdapter::InterpolateCalData(int CalNum, short *dutyI, short *dutyLtI,
        CalData *pCalData, NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT *pNvCali,
        int totalStep, int totalStepLT, AWB_GAIN_T *outWB, short *outYTab,
        bool dualFlashEnable, int caliNvIdx)
{
    (void)caliNvIdx;
    logI("%s+", __func__);
    return mpFlashAlg->InterpolateCalData(CalNum, dutyI, dutyLtI,
                    pCalData, pNvCali->yTab, pNvCali->flashWBGain,
                    totalStep, totalStepLT, outWB, outYTab, dualFlashEnable);
}

int FlashAlgoAdapter::setIsSaveSimBinFile(int isSaveBin)
{
    logI("%s+", __func__);
    mpFlashAlg->setIsSaveSimBinFile(isSaveBin);
    return 0;
}

int FlashAlgoAdapter::setDebugDir(const char *DirName, const char *PrjName)
{
    logI("%s+", __func__);
    mpFlashAlg->setDebugDir(DirName, PrjName);
    return 0;
}

int FlashAlgoAdapter::getDbgData(FLASH_DEBUG_INFO_T *pDebugInfo)
{
    logI("%s+", __func__);
    mpFlashAlg->getDbgData(pDebugInfo);
    return 0;
}

int FlashAlgoAdapter::checkInputParaError(int* num, int* errBuf)
{
    logI("%s+", __func__);
    return mpFlashAlg->checkInputParaError(num, errBuf);
}
