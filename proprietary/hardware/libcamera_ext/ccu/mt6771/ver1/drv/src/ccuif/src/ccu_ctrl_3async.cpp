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
#define LOG_TAG "ICcuCtrl3ASync"

#include "ccu_ctrl.h"
#include "ccu_mgr.h"
#include "ccu_ctrl_3async.h"
#include <cutils/properties.h>  // For property_get().
#include "ccu_log.h"
#include "kd_camera_feature.h"/*for IMGSENSOR_SENSOR_IDX*/
#include "kd_ccu_i2c.h"
#include "ccu_drvutil.h"
#include "utilSystrace.h"


namespace NSCcuIf {

/**************************************************************************
* Globals
**************************************************************************/
CcuCtrl3ASync _CcuCtrl3ASyncSingleton;

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
ICcuCtrl3ASync *ICcuCtrl3ASync::createInstance()
{
	return &_CcuCtrl3ASyncSingleton;
}

/*******************************************************************************
* Public Functions
********************************************************************************/
int CcuCtrl3ASync::init(MUINT32 sensorIdx, ESensorDev_T sensorDev)
{
	LOG_DBG("+:%s\n",__FUNCTION__);

	int ret = AbsCcuCtrlBase::init(sensorIdx, sensorDev);
	if(ret != CCU_CTRL_SUCCEED)
	{
		return ret;
	}

    LOG_DBG("-:%s\n",__FUNCTION__);
    return CCU_CTRL_SUCCEED;
}

void CcuCtrl3ASync::destroyInstance(void)
{
	LOG_DBG("%s\n", __FUNCTION__);
}

MBOOL CcuCtrl3ASync::isSupportFrameSync(void)
{
	if(CcuMgr::IsCcuMgrEnabled)
		return MTRUE;
	else
		return MFALSE;
}

MBOOL CcuCtrl3ASync::isSupport3ASync(void)
{
	if(CcuMgr::IsCcuMgrEnabled)
		return MTRUE;
	else
		return MFALSE;
}

enum ccu_tg_info CcuCtrl3ASync::getCcuTgInfo()
{
	//for 3a sync control, no need tg info
	return CCU_CAM_TG_1;
}


/*******************************************************************************
* Overridden Functions
********************************************************************************/
enum ccu_feature_type CcuCtrl3ASync::_getFeatureType()
{
    return CCU_FEATURE_3ASYNC;
}

bool CcuCtrl3ASync::ccuCtrlPreprocess(ccu_msg_id msgId, void *inDataPtr, void *inDataBuf)
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

bool CcuCtrl3ASync::ccuCtrlPreprocess_SetSyncState(void *inDataPtr, void *inDataBuf)
{
	struct ccu_3a_sync_state *input = (struct ccu_3a_sync_state *)inDataBuf;
	
	if(input->mode == FREE_RUN)
	{
		//don't care master dev/idx when free-run mode
		return true;
	}
	
	input->master_sensor_tg = CcuDrvUtil::sensorDevToTgInfo((NSIspTuning::ESensorDev_T)input->master_sensor_dev, input->master_sensor_idx);
	LOG_DBG("masterDev(%d), masterIdx(%d, masterTg(%d)", input->master_sensor_dev, input->master_sensor_idx, input->master_sensor_tg);

	if(input->master_sensor_tg == CCU_CAM_TG_NONE)
		return false;
	else
		return true;
}

bool CcuCtrl3ASync::ccuCtrlPreprocess_SyncAeSetteings(void *inDataPtr, void *inDataBuf)
{
	struct ccu_sync_ae_settings_data *input = (struct ccu_sync_ae_settings_data *)inDataBuf;
	input->master_sensor_tg = CcuDrvUtil::sensorDevToTgInfo((NSIspTuning::ESensorDev_T)input->master_sensor_dev, input->master_sensor_idx);
	LOG_DBG("masterDev(%d), masterIdx(%d, masterTg(%d)", input->master_sensor_dev, input->master_sensor_idx, input->master_sensor_tg);

	if(input->master_sensor_tg == CCU_CAM_TG_NONE)
		return false;
	else
		return true;
}

void CcuCtrl3ASync::getDebugInfo(N3D_AE_DEBUG_INFO_T *a_rN3dAeDebugInfo)
{
    UTIL_TRACE_BEGIN(__FUNCTION__);
    //m_CcuAeExifDataPtr
    enum ccu_tg_info tg_info = getCcuTgInfo();
    struct shared_buf_map * sharedBufMap = m_pDrvCcu->getSharedBufMap();
    CCU_N3DAE_STRUCT *n3dAeDataPtr = NULL;
    CCU_strAEOutput *n3dOutputPtr = NULL;
    CCU_N3D_AE_INIT_STRUCT *n3dAeInitPtr = NULL;

    n3dAeDataPtr = (CCU_N3DAE_STRUCT *)m_pDrvCcu->ccuAddrToVa(sharedBufMap->exif_data_addrs[CCU_TG2IDX(tg_info)].n3d_ae_addr);
    n3dOutputPtr = (CCU_strAEOutput *)m_pDrvCcu->ccuAddrToVa(sharedBufMap->exif_data_addrs[CCU_TG2IDX(tg_info)].n3d_output_addr);
    n3dAeInitPtr = (CCU_N3D_AE_INIT_STRUCT *)m_pDrvCcu->ccuAddrToVa(sharedBufMap->exif_data_addrs[CCU_TG2IDX(tg_info)].n3d_init_addr);

    LOG_DBG("AE algo dataPtrs, n3dAeDataPtr(%p), n3dOutputPtr(%p), n3dAeInitPtr(%p)", n3dAeDataPtr, n3dOutputPtr, n3dAeInitPtr);

    if((n3dAeDataPtr == NULL) || (n3dOutputPtr == NULL) || (n3dAeInitPtr == NULL))
    {
        LOG_ERR("AE algo dataPtrs error, skip fillUpN3AedDebugInfo");
        LOG_ERR("AE algo dataPtrs, n3dAeDataPtr(%p), n3dOutputPtr(%p), n3dAeInitPtr(%p)", n3dAeDataPtr, n3dOutputPtr, n3dAeInitPtr);
        return;
    }

    fillUpN3AedDebugInfo(a_rN3dAeDebugInfo, n3dAeInitPtr, n3dAeDataPtr, n3dOutputPtr);

    LOG_DBG("getN3dDebugInfo done.");
    UTIL_TRACE_END();
    return;
}

void CcuCtrl3ASync::fillUpN3AedDebugInfo(N3D_AE_DEBUG_INFO_T *a_rN3dAeDebugInfo, CCU_N3D_AE_INIT_STRUCT *gN3d_ae_init_info, CCU_N3DAE_STRUCT *m_n3dAE, CCU_strAEOutput *m_AE_Output)
{
    memset(a_rN3dAeDebugInfo, 0, sizeof(N3D_AE_DEBUG_INFO_T));


// N3D AWB tag version
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_TAG_DEBUG_VERSION, (MUINT32)N3D_AE_DEBUG_VERSION);
	setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_SYNCNUM, gN3d_ae_init_info->SyncNum);

	setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_SYNCMODE, m_n3dAE->i4SyncMode);
	setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_SYNCPOLICY, gN3d_ae_init_info->SyncAeTuningParam.syncPolicy);

    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_LOWER_BV, m_n3dAE->Lower.nBV);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_UPPER_BV, m_n3dAE->Upper.nBV);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_MASTER_IDX, m_n3dAE->u4AEMasterIdx);

    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_CAL_GAIN_R, m_n3dAE->CalGain.i4R);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_CAL_GAIN_G, m_n3dAE->CalGain.i4G);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_CAL_GAIN_B, m_n3dAE->CalGain.i4B);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_CAL_YGAIN, m_n3dAE->i4CalYGain);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_CAL_OFFSET, m_n3dAE->i4CalOffset);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_REG_OFFSET, m_n3dAE->i4RegrOffset);

    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_0_AEIDX,   m_AE_Output[0].u4Index);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_0_AFEGAIN, m_AE_Output[0].EvSetting.u4AfeGain);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_0_SHUTTER, m_AE_Output[0].EvSetting.u4Eposuretime);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_0_ISPGAIN, m_AE_Output[0].EvSetting.u4IspGain);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_0_CWY, m_AE_Output[0].u4CWValue);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_0_SYNCGAIN, m_n3dAE->u4SyncGain[0]);

    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_1_AEIDX,   m_AE_Output[1].u4Index);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_1_AFEGAIN, m_AE_Output[1].EvSetting.u4AfeGain);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_1_SHUTTER, m_AE_Output[1].EvSetting.u4Eposuretime);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_1_ISPGAIN, m_AE_Output[1].EvSetting.u4IspGain);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_1_CWY, m_AE_Output[1].u4CWValue);
    setDebugTagN3D(a_rN3dAeDebugInfo, N3D_AE_1_SYNCGAIN, m_n3dAE->u4SyncGain[1]);
    return;
}

};  //namespace NSCcuIf
