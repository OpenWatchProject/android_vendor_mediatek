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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "ICcuCtrl3ACtrl"

#include "ccu_ctrl.h"
#include "ccu_mgr.h"
#include "ccu_ctrl_3actrl.h"
#include <cutils/properties.h>  // For property_get().
#include "ccu_log.h"
#include "kd_camera_feature.h"/*for IMGSENSOR_SENSOR_IDX*/
#include "kd_ccu_i2c.h"
#include "ccu_drvutil.h"
#include "utilSystrace.h"
#include "iccu_ctrl_ae.h"
#include "iccu_ctrl_ltm.h"
#include "algorithm/ccu_ae_sync_utility.h"

#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>

namespace NSCcuIf {

/**************************************************************************
* Globals
**************************************************************************/
CcuCtrl3ACtrl _CcuCtrl3ACtrlSingleton;

EXTERN_DBG_LOG_VARIABLE(ccu_drv);

/*******************************************************************************
* Static Functions
********************************************************************************/
static inline void setDebugTagN3D(N3D_AE_DEBUG_INFO_T *a_rSyncAeDebugInfo, MINT32 a_i4ID, MINT32 a_i4Value)
{
    a_rSyncAeDebugInfo->Tag[a_i4ID].u4FieldID = N3DAAATAG(DEBUG_N3D_AE_MODULE_ID, a_i4ID, 0);
    a_rSyncAeDebugInfo->Tag[a_i4ID].u4FieldValue = a_i4Value;
}

/*******************************************************************************
* Factory Function
********************************************************************************/
ICcuCtrl3ACtrl *ICcuCtrl3ACtrl::createInstance()
{
	return &_CcuCtrl3ACtrlSingleton;
}

/*******************************************************************************
* Public Functions
********************************************************************************/
extern "C"
NSCam::NSIoPipe::NSCamIOPipe::INormalPipe* createInstance_INormalPipe(MUINT32 SensorIndex, char const* szCallerName);

static void setupTgInt1(MUINT32 sensorIdx)
{
    LOG_DBG_MUST("+");

    NSCam::NSIoPipe::NSCamIOPipe::INormalPipe*  mpNormalPipe;

    mpNormalPipe = createInstance_INormalPipe(sensorIdx, "ccuMgr");
    mpNormalPipe->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_SET_TG_INT_LINE, 0, ePixMode_NONE, 0);
    mpNormalPipe->destroyInstance("ccuMgr");

    LOG_DBG_MUST("-");
}

int CcuCtrl3ACtrl::init(MUINT32 sensorIdx, ESensorDev_T sensorDev)
{
	LOG_DBG("+:%s\n",__FUNCTION__);

	int ret = AbsCcuCtrlBase::init(sensorIdx, sensorDev);
	if(ret != CCU_CTRL_SUCCEED)
	{
		return ret;
	}

    setupTgInt1(sensorIdx);

    LOG_DBG("-:%s\n",__FUNCTION__);
    return CCU_CTRL_SUCCEED;
}

void CcuCtrl3ACtrl::destroyInstance(void)
{
	LOG_DBG("%s\n", __FUNCTION__);
}

MBOOL CcuCtrl3ACtrl::isSupportFrameSync(void)
{
	if(CcuMgr::IsCcuMgrEnabled)
		return MTRUE;
	else
		return MFALSE;
}

MBOOL CcuCtrl3ACtrl::isSupport3ASync(void)
{
	if(CcuMgr::IsCcuMgrEnabled)
		return MTRUE;
	else
		return MFALSE;
}

enum ccu_tg_info CcuCtrl3ACtrl::getCcuTgInfo()
{
	//for 3a sync control, no need tg info
	return CCU_CAM_TG_1;
}


/*******************************************************************************
* Overridden Functions
********************************************************************************/
enum ccu_feature_type CcuCtrl3ACtrl::_getFeatureType()
{
    return CCU_FEATURE_3ACTRL;
}

bool CcuCtrl3ACtrl::ccuCtrlPreprocess(uint32_t msgId, void *inDataPtr, void *inDataBuf)
{
    bool result = true;
    switch(msgId)
    {
        case MSG_TO_CCU_SET_3A_SYNC_STATE:
        {
            result = ccuCtrlPreprocess_SetSyncState(inDataPtr, inDataBuf);
            break;
        }
        case MSG_TO_CCU_SYNC_AE_SETTING:
        {
            result = ccuCtrlPreprocess_SyncAeSetteings(inDataPtr, inDataBuf);
            break;
        }
        default : break;
    }
    return result;
}

bool CcuCtrl3ACtrl::ccuCtrlPreprocess_SetSyncState(void *inDataPtr, void *inDataBuf)
{
	struct ccu_3a_sync_state *input = (struct ccu_3a_sync_state *)inDataBuf;
	
	if(input->mode == FREE_RUN)
	{
		//don't care master dev/idx when free-run mode
		return true;
	}

	LOG_DBG_MUST("masterDev(%d), masterIdx(%d, masterTg(%d)", input->master_sensor_dev, input->master_sensor_idx, input->master_sensor_tg);
	input->master_sensor_tg = CcuDrvUtil::sensorDevToTgInfo((NSIspTuning::ESensorDev_T)input->master_sensor_dev, input->master_sensor_idx);

	if(input->master_sensor_tg == CCU_CAM_TG_NONE)
		return false;
	else
		return true;
}

bool CcuCtrl3ACtrl::ccuCtrlPreprocess_SyncAeSetteings(void *inDataPtr, void *inDataBuf)
{
	struct ccu_sync_ae_settings_data *input = (struct ccu_sync_ae_settings_data *)inDataBuf;

    LOG_DBG_MUST("masterDev(%d), masterIdx(%d, masterTg(%d)", input->master_sensor_dev, input->master_sensor_idx, input->master_sensor_tg);
	input->master_sensor_tg = CcuDrvUtil::sensorDevToTgInfo((NSIspTuning::ESensorDev_T)input->master_sensor_dev, input->master_sensor_idx);

	if(input->master_sensor_tg == CCU_CAM_TG_NONE)
		return false;
	else
		return true;
}

void CcuCtrl3ACtrl::getDebugInfo(N3D_AE_DEBUG_INFO_T *a_rN3dAeDebugInfo)
{
    UTIL_TRACE_BEGIN(__FUNCTION__);

    if(!ICcuMgrPriv::isCcuBooted())
    {
        LOG_WRN("[%s] CCU is not booted yet, skip", __FUNCTION__);
        return;
    }
    //m_CcuAeExifDataPtr
    enum ccu_tg_info tg_info = getCcuTgInfo();
    struct shared_buf_map * sharedBufMap = m_pDrvCcu->getSharedBufMap();
    COMPAT_CCU_SYNC_AE_INPUT_T *pAeSyncAlgoInPtr = NULL;
    CCU_SYNC_AE_OUTPUT_T *pAeSyncAlgoOutPtr = NULL;

    pAeSyncAlgoInPtr = (COMPAT_CCU_SYNC_AE_INPUT_T *)m_pDrvCcu->ccuAddrToVa(sharedBufMap->exif_data_addrs[CCU_TG2IDX(tg_info)].aesync_algo_in_addr);
    pAeSyncAlgoOutPtr = (CCU_SYNC_AE_OUTPUT_T *)m_pDrvCcu->ccuAddrToVa(sharedBufMap->exif_data_addrs[CCU_TG2IDX(tg_info)].aesync_algo_out_addr);
    LOG_DBG("AE sync algo dataPtrs, pAeSyncAlgoInPtr(%x), pAeSyncAlgoOutPtr(%x)", pAeSyncAlgoInPtr, pAeSyncAlgoOutPtr);

    if ((NULL == pAeSyncAlgoInPtr) || (NULL == pAeSyncAlgoOutPtr))
    {
        LOG_ERR("AE sync algo dataPtrs error, skip fillUpN3AedDebugInfo");
        LOG_ERR("AE sync algo dataPtrs, pAeSyncAlgoInPtr(%x), pAeSyncAlgoOutPtr(%x)", pAeSyncAlgoInPtr, pAeSyncAlgoOutPtr);
        return;
    }

    fillUpN3AedDebugInfo(a_rN3dAeDebugInfo, pAeSyncAlgoInPtr, pAeSyncAlgoOutPtr);

    LOG_DBG("getN3dDebugInfo done.");
    UTIL_TRACE_END();
    return;
}

void CcuCtrl3ACtrl::fillUpN3AedDebugInfo(N3D_AE_DEBUG_INFO_T *a_rN3dAeDebugInfo, COMPAT_CCU_SYNC_AE_INPUT_T *a_rAesyncInData, CCU_SYNC_AE_OUTPUT_T *a_rAesyncOutData)
{
    memset(a_rN3dAeDebugInfo, 0, sizeof(N3D_AE_DEBUG_INFO_T));


    // N3D AWB tag version
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_DEBUG_VERSION, (MUINT32)N3D_AE_DEBUG_VERSION);

    //input data abnormal need to check
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_SYNCNUM, (MINT32)a_rAesyncInData->u2SyncNum);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_MASTERID, (MINT32)a_rAesyncInData->u2MasterId);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_0_CAMTYPE, (MINT32)a_rAesyncInData->rCamInput[0].eCamType);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_0_AVGY, a_rAesyncInData->rCamInput[0].u4AvgY);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_0_EXPUNIT, a_rAesyncInData->rCamInput[0].u4ExpUnit);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_0_SHUTTERBASE, a_rAesyncInData->rCamInput[0].u4ShutterBase);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_0_AEIDX, a_rAesyncInData->rCamInput[0].rInput.u4Index);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_0_SHUTTER, a_rAesyncInData->rCamInput[0].rInput.EvSetting.u4Eposuretime);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_0_AFEGAIN, a_rAesyncInData->rCamInput[0].rInput.EvSetting.u4AfeGain);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_0_ISPGAIN, a_rAesyncInData->rCamInput[0].rInput.EvSetting.u4IspGain);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_1_CAMTYPE, (MINT32)a_rAesyncInData->rCamInput[1].eCamType);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_1_AVGY, a_rAesyncInData->rCamInput[1].u4AvgY);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_1_EXPUNIT, a_rAesyncInData->rCamInput[1].u4ExpUnit);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_1_SHUTTERBASE, a_rAesyncInData->rCamInput[1].u4ShutterBase);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_1_AEIDX, a_rAesyncInData->rCamInput[1].rInput.u4Index);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_1_SHUTTER, a_rAesyncInData->rCamInput[1].rInput.EvSetting.u4Eposuretime);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_1_AFEGAIN, a_rAesyncInData->rCamInput[1].rInput.EvSetting.u4AfeGain);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_1_ISPGAIN, a_rAesyncInData->rCamInput[1].rInput.EvSetting.u4IspGain);

    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_OUT_0_OFFSETBASE, a_rAesyncOutData->rCamOutput[0].u4OffsetBase);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_OUT_0_REGGAIN, a_rAesyncOutData->rCamOutput[0].u4RegGain);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_OUT_0_APPLYGAIN, a_rAesyncOutData->rCamOutput[0].u4ApplyGain);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_OUT_0_EVDIFF, a_rAesyncOutData->rCamOutput[0].i4EvDiff);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_OUT_0_CWV, a_rAesyncOutData->rCamOutput[0].Output.u4CWValue);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_OUT_0_AEIDX, a_rAesyncOutData->rCamOutput[0].Output.u4Index);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_OUT_0_SHUTTER, a_rAesyncOutData->rCamOutput[0].Output.EvSetting.u4Eposuretime);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_OUT_0_AFEGAIN, a_rAesyncOutData->rCamOutput[0].Output.EvSetting.u4AfeGain);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_OUT_0_ISPGAIN, a_rAesyncOutData->rCamOutput[0].Output.EvSetting.u4IspGain);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_OUT_1_OFFSETBASE, a_rAesyncOutData->rCamOutput[1].u4OffsetBase);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_OUT_1_REGGAIN, a_rAesyncOutData->rCamOutput[1].u4RegGain);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_OUT_1_APPLYGAIN, a_rAesyncOutData->rCamOutput[1].u4ApplyGain);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_OUT_1_EVDIFF, a_rAesyncOutData->rCamOutput[1].i4EvDiff);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_OUT_1_CWV, a_rAesyncOutData->rCamOutput[1].Output.u4CWValue);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_OUT_1_AEIDX, a_rAesyncOutData->rCamOutput[1].Output.u4Index);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_OUT_1_SHUTTER, a_rAesyncOutData->rCamOutput[1].Output.EvSetting.u4Eposuretime);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_OUT_1_AFEGAIN, a_rAesyncOutData->rCamOutput[1].Output.EvSetting.u4AfeGain);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_OUT_1_ISPGAIN, a_rAesyncOutData->rCamOutput[1].Output.EvSetting.u4IspGain);

    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_INFO_0, AE_SYNC_ALGO_VER_FULL);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_INFO_1, AE_SYNC_ALGO_IF_VER_FULL);
/*
    if (a_rAesyncInData->rCamInput[0].prAeSyncNvram != NULL)
    {
        setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_0_ENGAINREG, a_rAesyncInData->rCamInput[0].prAeSyncNvram->bGainRegression);
        setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_0_REGTYPE, a_rAesyncInData->rCamInput[0].prAeSyncNvram->u2AlignMode);
        setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_0_ALIGNMODE, a_rAesyncInData->rCamInput[0].prAeSyncNvram->u2AlignMode);
        setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_0_EVOFFSET, a_rAesyncInData->rCamInput[0].prAeSyncNvram->i4EvOffset[0]);
        setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_0_RGB2YCOEF_0, a_rAesyncInData->rCamInput[0].prAeSyncNvram->u4RGB2YCoef[0]);
        setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_0_RGB2YCOEF_1, a_rAesyncInData->rCamInput[0].prAeSyncNvram->u4RGB2YCoef[1]);
        setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_0_RGB2YCOEF_2, a_rAesyncInData->rCamInput[0].prAeSyncNvram->u4RGB2YCoef[2]);
        setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_0_FIXSYNCGAIN, a_rAesyncInData->rCamInput[0].prAeSyncNvram->u4FixSyncGain);
        setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_1_ENGAINREG, a_rAesyncInData->rCamInput[1].prAeSyncNvram->bGainRegression);
        setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_1_REGTYPE, a_rAesyncInData->rCamInput[1].prAeSyncNvram->u2AlignMode);
        setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_1_ALIGNMODE, a_rAesyncInData->rCamInput[1].prAeSyncNvram->u2AlignMode);
        setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_1_EVOFFSET, a_rAesyncInData->rCamInput[1].prAeSyncNvram->i4EvOffset[0]);
        setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_1_RGB2YCOEF_0, a_rAesyncInData->rCamInput[1].prAeSyncNvram->u4RGB2YCoef[0]);
        setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_1_RGB2YCOEF_1, a_rAesyncInData->rCamInput[1].prAeSyncNvram->u4RGB2YCoef[1]);
        setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_1_RGB2YCOEF_2, a_rAesyncInData->rCamInput[1].prAeSyncNvram->u4RGB2YCoef[2]);
        setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_IN_1_FIXSYNCGAIN, a_rAesyncInData->rCamInput[1].prAeSyncNvram->u4FixSyncGain);
    }*/
    
    return;
}

uint32_t CcuCtrl3ACtrl::getAeDebugInfo(MUINT32 sensorIdx, ESensorDev_T sensorDev, AE_DEBUG_INFO_T *a_rAEDebugInfo, AE_DEBUG_DATA_T *a_rAEDebugData)
{
    ICcuCtrlAe *ccuCtrlAe;

    if(!ICcuMgrPriv::isCcuBooted())
    {
        LOG_WRN("[%s] CCU is not booted yet, skip", __FUNCTION__);
        return -CCU_CTRL_ERR_GENERAL;
    }

    ccuCtrlAe = ICcuCtrlAe::getInstance(sensorIdx, sensorDev);
    if(ccuCtrlAe == NULL)
        return -1;
    return ccuCtrlAe->getDebugInfo(a_rAEDebugInfo, a_rAEDebugData);
}

uint32_t CcuCtrl3ACtrl::getAeInfo(MUINT32 sensorIdx, ESensorDev_T sensorDev, AE_INFO_T *a_rAEInfo, MUINT64 *u8RealEposuretime)
{
    ICcuCtrlAe *ccuCtrlAe;
    struct ccu_ae_output ae_output;
    if(!ICcuMgrPriv::isCcuBooted())
    {
        LOG_WRN("[%s] CCU is not booted yet, skip", __FUNCTION__);
        return -CCU_CTRL_ERR_GENERAL;
    }

    ccuCtrlAe = ICcuCtrlAe::getInstance(sensorIdx, sensorDev);
    if(ccuCtrlAe == NULL)
        return -1;

    if (u8RealEposuretime != NULL)
    {
        ccuCtrlAe->getCcuAeOutput(&ae_output);
        *u8RealEposuretime = (MUINT64)ae_output.real_setting.EvSetting.u4Eposuretime * 1000;
        LOG_DBG("[%s] u4Eposuretime(%d)->u8RealEposuretime(%llu)",
        __FUNCTION__, ae_output.real_setting.EvSetting.u4Eposuretime, *u8RealEposuretime);
    }

    return ccuCtrlAe->getAeInfoForIsp(a_rAEInfo);
}

uint32_t CcuCtrl3ACtrl::getLtmDebugInfo(MUINT32 sensorIdx, ESensorDev_T sensorDev, NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo *ltm_debug_info)
{
    ICcuCtrlLtm *ccuCtrlLtm;

    if(!ICcuMgrPriv::isCcuBooted())
    {
        LOG_WRN("[%s] CCU is not booted yet, skip", __FUNCTION__);
        return -CCU_CTRL_ERR_GENERAL;
    }

    ccuCtrlLtm = ICcuCtrlLtm::getInstance(sensorIdx, sensorDev);
    if(ccuCtrlLtm == NULL)
        return -1;
    return ccuCtrlLtm->getLtmInfoExif(ltm_debug_info);
}

uint32_t CcuCtrl3ACtrl::getLtmInfo(MUINT32 sensorIdx, ESensorDev_T sensorDev, struct ccu_ltm_info_isp *ltm_info)
{
    ICcuCtrlLtm *ccuCtrlLtm;

    if(!ICcuMgrPriv::isCcuBooted())
    {
        LOG_WRN("[%s] CCU is not booted yet, skip", __FUNCTION__);
        return -CCU_CTRL_ERR_GENERAL;
    }

    ccuCtrlLtm = ICcuCtrlLtm::getInstance(sensorIdx, sensorDev);
    if(ccuCtrlLtm == NULL)
        return -1;
    return ccuCtrlLtm->getLtmInfoIsp(ltm_info);
}

uint32_t CcuCtrl3ACtrl::getHlrDebugInfo(MUINT32 sensorIdx, ESensorDev_T sensorDev, NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo *hlr_debug_info)
{
    ICcuCtrlLtm *ccuCtrlLtm;

    if(!ICcuMgrPriv::isCcuBooted())
    {
        LOG_WRN("[%s] CCU is not booted yet, skip", __FUNCTION__);
        return -CCU_CTRL_ERR_GENERAL;
    }

    ccuCtrlLtm = ICcuCtrlLtm::getInstance(sensorIdx, sensorDev);
    if(ccuCtrlLtm == NULL)
        return -1;
    return ccuCtrlLtm->getHlrInfoExif(hlr_debug_info);
}

uint32_t CcuCtrl3ACtrl::getHlrInfo(MUINT32 sensorIdx, ESensorDev_T sensorDev, struct ccu_hlr_info_isp *hlr_info)
{
    ICcuCtrlLtm *ccuCtrlLtm;

    if(!ICcuMgrPriv::isCcuBooted())
    {
        LOG_WRN("[%s] CCU is not booted yet, skip", __FUNCTION__);
        return -CCU_CTRL_ERR_GENERAL;
    }

    ccuCtrlLtm = ICcuCtrlLtm::getInstance(sensorIdx, sensorDev);
    if(ccuCtrlLtm == NULL)
        return -1;
    return ccuCtrlLtm->getHlrInfoIsp(hlr_info);
}

};  //namespace NSCcuIf
