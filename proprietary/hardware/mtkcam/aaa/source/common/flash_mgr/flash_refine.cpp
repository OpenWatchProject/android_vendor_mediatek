#define LOG_TAG "FlashRefine"

/***********************************************************
 * Headers
 **********************************************************/

/* aaa headers */
#include "ae_mgr.h"

/* aaa common headers */
#include "property_utils.h"
#include "log_utils.h"

/* custom headers */
#include "flash_tuning_custom.h"

/* flash headers */
#include "flash_mgr_m.h"
#include "flash_hal.h"
#include "flash_pline.h"
#include "flash_duty.h"
#include "flash_custom_adapter.h"
#include "flash_platform_adapter.h"

#define PLINE_AFE_GAIN_BASE 1024.0
#define PLINE_ISP_GAIN_BASE 1024.0

/* misc */
#include "math.h"

/***********************************************************
 * Define macros
 **********************************************************/
/* android property */
#define PROP_FLASH_LOW_REF             "vendor.flash_low_ref" /* new flow */

bool FlashMgrM::isEisStable(){
    /* not implemented */
    return true;
}

bool FlashMgrM::isAcceGyroStable(){
    /* not implemented */
    return true;
}

bool FlashMgrM::isLowReflectance(){
    int propLowRef = 0;
    getPropInt(PROP_FLASH_LOW_REF, &propLowRef, 0);
    logI("propLowRef = %d, mIsLowRef = %d\n", propLowRef, mIsLowRef);
    return (propLowRef || ((mSensorDev == ESensorDev_Sub || mSensorDev == ESensorDev_SubSecond) && FlashHal::getInstance(mSensorDev)->getSubFlashCustomization()));
}

bool FlashMgrM::isLowConfidence(){
    return (!isEisStable()) || (!isAcceGyroStable()) || (isLowReflectance());
}

bool FlashMgrM::isWellControl(){
    logI("i4DeltaBV = %d, bAEStable = %d\n",
        m_strTorchAEOutput.i4DeltaBV, m_strTorchAEOutput.bAEStable);
    return (m_strTorchAEOutput.i4DeltaBV == 0) && (m_strTorchAEOutput.bAEStable);
}

void FlashMgrM::CalculateEtGainMainFlash(){
    double C1 = (double)m_strEtGainNormal.u4Eposuretime / m_strEtGainTorchAE.u4Eposuretime
                       *m_strEtGainNormal.u4AfeGain / m_strEtGainTorchAE.u4AfeGain
                       *m_strEtGainNormal.u4IspGain / m_strEtGainTorchAE.u4IspGain;
    double C2 = (double)m_i4EngMainflash / m_i4EngPreflash;

    logI("C1/C2=%lf/%lf\n", C1, C2);

    strEvSetting EtGainTemp = m_strEtGainTorchAE;
    if(C1 <= 1.07)
    {
      logI("C1=%lf\n", C1);
      EtGainTemp.u4Eposuretime /= C2;
    }
    else
    {
      logI("C1=%lf\n", C1);
      EtGainTemp.u4Eposuretime *= (C1 / ((C2 * C1) - C2 + 1));
    }
    searchPlineIndex(EtGainTemp, m_strEtGainMainFlash, 0);
}

void FlashMgrM::CWVControl(){
    CalculateEtGainMainFlash();
}

void FlashMgrM::EtGainTrans4(){
    CalculateEtGainMainFlash();
    searchPlineIndex(m_strEtGainMainFlash, m_strEtGainMainFlash, m_strTorchAEOutput.i4DeltaBV);
}

void FlashMgrM::decideMainFlashEng(){
    int dutyIndex = 0;
    dutyIndex = FlashCustomAdapter::getInstance(mSensorDev)->getDecideMfDutyIdx();
    m_i4EngMainflash = FlashCustomAdapter::getInstance(mSensorDev)->getYTabByDutyIndex(dutyIndex);
    mAlgoExpPara.duty = dutyIndex;
    logI("Eng-P/M(%d/%d)\n", m_i4EngPreflash, m_i4EngMainflash);
}

void FlashMgrM::dumpDeciderInfo()
{
    logI("DDD Eng-P/M(%d/%d)\n", m_i4EngPreflash, m_i4EngMainflash);
    logI("DDD i4DeltaBV=%d bAEStable=%d\n", m_strTorchAEOutput.i4DeltaBV, m_strTorchAEOutput.bAEStable);
    logI("DDD normal exp/afe/isp=%d/%d/%d\n", m_strEtGainNormal.u4Eposuretime, m_strEtGainNormal.u4AfeGain, m_strEtGainNormal.u4IspGain);
    logI("DDD torch  exp/afe/isp=%d/%d/%d\n", m_strEtGainTorchAE.u4Eposuretime, m_strEtGainTorchAE.u4AfeGain, m_strEtGainTorchAE.u4IspGain);
    logI("DDD main   exp/afe/isp=%d/%d/%d\n", m_strEtGainMainFlash.u4Eposuretime, m_strEtGainMainFlash.u4AfeGain, m_strEtGainMainFlash.u4IspGain);
}

void FlashMgrM::searchPlineIndex(strEvSetting &a_rEvSettingInput, strEvSetting &a_rEvSettingOutput, int i4DeltaBV){
    strAETable pfPlineTab = {};
    strAETable capPlineTab = {};
    strAETable strobePlineTab = {};
    strAFPlineInfo pfPlineInfo = {};
    AeMgr::getInstance(mSensorDev).getCurrentPlineTable(pfPlineTab, capPlineTab, strobePlineTab, pfPlineInfo);
    int exp;
    int afe;
    int isp;
    exp = a_rEvSettingInput.u4Eposuretime;
    afe = a_rEvSettingInput.u4AfeGain;
    isp = a_rEvSettingInput.u4IspGain;
    logI("input exp/afe/isp=%d/%d/%d\n", exp, afe, isp);
    if(i4DeltaBV != 0){
        logI("i4DeltaBV=%d exp=%d\n", i4DeltaBV, exp);
        exp = exp * pow(2, (double)i4DeltaBV/10);
        logI("exp_new=%d\n", exp);
    }
    int bestInd = 0;
    int ispBase = FlashPlatformAdapter::getInstance(mSensorDev)->getIspGainBase();
    int err;
    err = FlashPlineTool::searchAePlineIndex(&bestInd, &capPlineTab, exp, afe, isp);
    if(err!=0 || capPlineTab.pCurrentTable == NULL)
    {
        logE("searchAePlineIndex err");
        return;
    }
    exp = a_rEvSettingOutput.u4Eposuretime = capPlineTab.pCurrentTable->sPlineTable[bestInd].u4Eposuretime;
    afe = a_rEvSettingOutput.u4AfeGain = capPlineTab.pCurrentTable->sPlineTable[bestInd].u4AfeGain;
    isp = a_rEvSettingOutput.u4IspGain = capPlineTab.pCurrentTable->sPlineTable[bestInd].u4IspGain*ispBase/PLINE_ISP_GAIN_BASE;
    logI("output exp/afe/isp=%d/%d/%d\n", exp, afe, isp);
    return;
}
