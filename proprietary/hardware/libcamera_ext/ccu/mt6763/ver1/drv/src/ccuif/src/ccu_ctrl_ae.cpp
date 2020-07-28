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
#define LOG_TAG "ICcuCtrlAe"

#include "ccu_ctrl.h"
#include "ccu_ctrl_ae.h"
#include <cutils/properties.h>  // For property_get().
#include "ccu_log.h"

namespace NSCcuIf {

/**************************************************************************
* Globals
**************************************************************************/
EXTERN_DBG_LOG_VARIABLE(ccu_drv);

/*******************************************************************************
* Factory Function
********************************************************************************/
ICcuCtrlAe *ICcuCtrlAe::createInstance()
{
	return new CcuCtrlAe();
}

/*******************************************************************************
* Public Functions
********************************************************************************/
int CcuCtrlAe::init(MUINT32 sensorIdx, ESensorDev_T sensorDev)
{
	LOG_DBG("+:%s\n",__FUNCTION__);
	WHICH_SENSOR_T ccuSensorDev;

	int ret = AbsCcuCtrlBase::init(sensorIdx, sensorDev);
	if(ret != CCU_CTRL_SUCCEED)
	{
		return ret;
	}

	if (sensorDev & ESensorDev_Main){
        ccuSensorDev = WHICH_SENSOR_MAIN;
    }else if (sensorDev & ESensorDev_Sub){
        ccuSensorDev = WHICH_SENSOR_SUB;
    }else if (sensorDev & ESensorDev_MainSecond){
        ccuSensorDev = WHICH_SENSOR_MAIN_2;
    }else{
        LOG_WRN("invalid sensorDev = %d\n",sensorDev);
        return -CCU_CTRL_ERR_GENERAL;
    }
	if (!m_pDrvCcu->loadSensorBin(ccuSensorDev))
    {
        LOG_WRN("loadSensorBin fail \n");
        return -CCU_CTRL_ERR_GENERAL;
    }

    LOG_DBG("-:%s\n",__FUNCTION__);
    return CCU_CTRL_SUCCEED;
}

void CcuCtrlAe::destroyInstance(void)
{
	LOG_DBG("+:%s\n",__FUNCTION__);
	delete this;
	LOG_DBG("-:%s\n",__FUNCTION__);
}

/*******************************************************************************
* Overridden Functions
********************************************************************************/
bool CcuCtrlAe::ccuCtrlPreprocess(ccu_msg_id msgId, void *inDataPtr)
{
    bool result = true;
    switch(msgId)
    {
        case MSG_TO_CCU_SENSOR_INIT:
        {
            result = ccuCtrlPreprocess_SensorInit(inDataPtr);
            break;
        }
        case MSG_TO_CCU_AE_INIT:
        {
            result = ccuCtrlPreprocess_AeInit(inDataPtr);
            break;
        }
        default : break;
    }
    return result;
}

bool CcuCtrlAe::ccuCtrlPostprocess(ccu_msg_id msgId, void *outDataPtr)
{
    bool result = true;
    switch(msgId)
    {
        case MSG_TO_CCU_SENSOR_INIT:
        {
            result = ccuCtrlPostprocess_SensorInit(outDataPtr);
            break;
        }
        default : break;
    }
    return result;
}

/*******************************************************************************
* Private Functions
********************************************************************************/
bool CcuCtrlAe::ccuCtrlPreprocess_AeInit(void *inDataPtr)
{
    // CcuCtrlAeInitParam *input = (CcuCtrlAeInitParam *)inDataPtr;
    // CcuCtrlAeInitParam *initParam = (CcuCtrlAeInitParam *)m_BufferList.mailboxInBuf.va;

    // //Pick CCU AE Algo. required data from AE_PARAM_T, fill into CcuCtrlAeInitParam.ccuAeInitParam.m_AeTuningParam
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bEnableRotateWeighting = input->aeParam->strAEParasetting.bEnableRotateWeighting;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bEnableFaceAE = input->aeParam->strAEParasetting.bEnableFaceAE;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bEnableMeterAE = input->aeParam->strAEParasetting.bEnableMeterAE;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bEnableHDRLSB = input->aeParam->strAEParasetting.bEnableHDRLSB;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.uBlockNumX = input->aeParam->strAEParasetting.uBlockNumX;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.uBlockNumY = input->aeParam->strAEParasetting.uBlockNumY;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.u4MinYLowBound = input->aeParam->strAEParasetting.u4MinYLowBound;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.u4MaxYHighBound = input->aeParam->strAEParasetting.u4MaxYHighBound;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.u4MeterWeight = input->aeParam->strAEParasetting.u4MeterWeight;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.u4MinCWRecommend = input->aeParam->strAEParasetting.u4MinCWRecommend;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.u4MaxCWRecommend = input->aeParam->strAEParasetting.u4MaxCWRecommend;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.m_TgValidBlkHeight = 84; //Hard-code at first phase
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.bAEv4p0MeterEnable = input->aeParam->bAEv4p0MeterEnable;
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pABLabsDiff, input->aeParam->pABLabsDiff, sizeof(CCU_strABL_absDiff));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pHSFlatSkyEnhance, input->aeParam->pHSFlatSkyEnhance, sizeof(CCU_strHSFlatSkyCFG));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pHSMultiStep, input->aeParam->pHSMultiStep, sizeof(CCU_strHSMultiStep));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pNonCWRAcc, input->aeParam->pNonCWRAcc, sizeof(CCU_strNonCWRAcc));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pAOERefBV, input->aeParam->pAOERefBV, sizeof(CCU_strAOEBVRef));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pOverExpoAOECFG, input->aeParam->pOverExpoAOECFG, sizeof(CCU_strOverExpoAOECFG));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pMainTargetCFG, input->aeParam->pMainTargetCFG, sizeof(CCU_strMainTargetCFG));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pHSv4p0CFG, input->aeParam->pHSv4p0CFG, sizeof(CCU_strHSv4p0CFG));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pNSBVCFG, input->aeParam->pNSBVCFG, sizeof(CCU_strNSBVCFG));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pAETempSmooth, input->aeParam->pAETempSmooth, sizeof(CCU_strAECWRTempSmooth));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pFaceSmooth, input->aeParam->pFaceSmooth, sizeof(CCU_strFaceSmooth));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pNsCdfRatio, input->aeParam->pNsCdfRatio, sizeof(CCU_strNS_CDF));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pFaceLocSizecheck, input->aeParam->pFaceLocSizecheck, sizeof(CCU_strFaceLocSize));
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.bPerFrameAESmooth = input->aeParam->bPerFrameAESmooth;
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pAEStableThd, input->aeParam->pAEStableThd, sizeof(CCU_strAEStableThd));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pPerframeCFG, input->aeParam->pPerframeCFG, sizeof(CCU_strPerframeCFG));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pHistStableCFG, input->aeParam->pHistStableCFG, sizeof(CCU_strHistStableCFG));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pPSOConverge, input->aeParam->pPSOConverge, sizeof(CCU_strPSOConverge));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pBVAccRatio, input->aeParam->pBVAccRatio, sizeof(CCU_strBVAccRatio));

    // //Pick CCU AE Algo. required data from AE_NVRAM_T, fill into CcuCtrlAeInitParam.m_AeCCTConfig
    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.u4AETarget = input->nvramData->rCCTConfig.u4AETarget;
    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.i4BVOffset = input->nvramData->rCCTConfig.i4BVOffset;
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.rMeteringSpec, &input->nvramData->rCCTConfig.rMeteringSpec, sizeof(CCU_AE_METER_SPEC_T));

    // LOG_DBG_MUST("AEIDBG m_AeTuningParam.bEnableRotateWeighting: %x", initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bEnableRotateWeighting);
    // LOG_DBG_MUST("AEIDBG m_AeTuningParam.pBVAccRatio: %x", initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pBVAccRatio.i4DeltaBVRatio);
    // LOG_DBG_MUST("AEIDBG m_AeTuningParam.pBVAccRatio: %x", initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pBVAccRatio.u4B2T_Target);

    // LOG_DBG_MUST("AEIDBG m_AeCCTConfig.u4MeteringStableMax: %x", initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.rMeteringSpec.rTOUCHFD_Spec.u4MeteringStableMax);
    // LOG_DBG_MUST("AEIDBG m_AeCCTConfig.u4MeteringStableMin: %x", initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.rMeteringSpec.rTOUCHFD_Spec.u4MeteringStableMin);

    LOG_DBG("+:%s\n",__FUNCTION__);

    return true;
}

bool CcuCtrlAe::ccuCtrlPreprocess_SensorInit(void *inDataPtr)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    SENSOR_INFO_IN_T *infoIn = (SENSOR_INFO_IN_T *)inDataPtr;
    MUINT32 i2cBufferMva;
    MUINT32 i2cBufferPaH;
    MUINT32 i2cBufferPaL;
	MUINT32 i2cId;
    MUINT32 i2cBufferLength = 4096; //i2c driver buffer default 4k
    enum CCU_I2C_CHANNEL channel;
    int32_t current_fps;
    int32_t sensorI2cSlaveAddr[3] = {-1 , -1 , -1};

    //======== Set i2c channel of cam type to make kernel driver select correct i2c driver =========
    LOG_INF_MUST("Sensor Type: %x\n",infoIn->eWhichSensor);
    if(infoIn->eWhichSensor == WHICH_SENSOR_MAIN)
    {
        channel = CCU_I2C_CHANNEL_MAINCAM;
    }
    else if(infoIn->eWhichSensor == WHICH_SENSOR_SUB)
    {
        channel = CCU_I2C_CHANNEL_SUBCAM;
    }
    else
    {
        LOG_ERR("Sensor Type is not main nor sub : %x\n", infoIn->eWhichSensor);
        return false;
    }

    if (!m_pDrvCcu->setI2CChannel(channel))
    {
        LOG_ERR("setI2CChannel fail \n");
        return false;
    }

    //======== Get i2c dma buffer mva =========
    if (!m_pDrvCcu->getI2CDmaBufferAddr(&i2cBufferMva, &i2cBufferPaH, &i2cBufferPaL, &i2cId))
    {
        LOG_ERR("getI2CDmaBufferAddr fail \n");
        return false;
    }
    LOG_DBG("i2c_get_dma_buffer_addr: %x\n", i2cBufferMva);
    LOG_DBG("i2c_get_dma_buffer_pa_h: %x\n", i2cBufferPaH);
    LOG_DBG("i2c_get_dma_buffer_pa_l: %x\n", i2cBufferPaL);
    LOG_DBG_MUST("i2c_get_dma_buffer_i2c_id: %x\n", i2cId);
	
    //======== Get current fps =========
    if (!m_pDrvCcu->getCurrentFps(&current_fps))
    {
        LOG_ERR("getCurrentFps fail \n");
        return false;
    }
    LOG_DBG("current_fps: %d\n", current_fps);
    if(current_fps == -1)
    {
        LOG_ERR("current_fps is not catched from kd_sensorlist!\n");
        current_fps = 0;
        //return false;
    }

    //======== Get Sensor I2c Slave Address =========
    if (!m_pDrvCcu->getSensorI2cSlaveAddr(&sensorI2cSlaveAddr[0]))
    {
        LOG_ERR("getSensorI2cSlaveAddr fail \n");
        return false;
    }
    LOG_DBG("[CCU If]Main  Sensor slave addr : %d\n", sensorI2cSlaveAddr[0]);
    LOG_DBG("[CCU If]Sub   Sensor slave addr : %d\n", sensorI2cSlaveAddr[1]);
    LOG_DBG("[CCU If]Main2 Sensor slave addr : %d\n", sensorI2cSlaveAddr[2]);
    if(infoIn->eWhichSensor == WHICH_SENSOR_MAIN)
    {
        if(sensorI2cSlaveAddr[0] == -1)
            return false;
    }
    else if(infoIn->eWhichSensor == WHICH_SENSOR_SUB)
    {
        if(sensorI2cSlaveAddr[1] == -1)
            return false;
    }

    //======== Call CCU to initial sensor drv =========
    //Convert to compatible structure
    COMPAT_SENSOR_INFO_IN_T *compatInfoIn = (COMPAT_SENSOR_INFO_IN_T *)m_BufferList.mailboxInBuf.va;

    //compatInfoIn->u32SensorId = infoIn->u32SensorId;
    compatInfoIn->eWhichSensor = infoIn->eWhichSensor;
    compatInfoIn->u16FPS = current_fps; //--todo: check where to get the fps
    compatInfoIn->eScenario = infoIn->eScenario;
    compatInfoIn->pu8BufferVAddr = i2cBufferMva;//buffer len is byte size
	compatInfoIn->u32BufferPAddrH = i2cBufferPaH;
    compatInfoIn->u32BufferPAddrL = i2cBufferPaL;
	compatInfoIn->u32I2CId = i2cId;
    compatInfoIn->u16BufferLen = i2cBufferLength;
    if(infoIn->eWhichSensor == WHICH_SENSOR_MAIN)
        compatInfoIn->sensorI2cSlaveAddr = sensorI2cSlaveAddr[0];
    else if(infoIn->eWhichSensor == WHICH_SENSOR_SUB)
        compatInfoIn->sensorI2cSlaveAddr = sensorI2cSlaveAddr[1];

    //LOG_DBG("ccu sensor in, compatInfoIn->u32SensorId: %x\n", compatInfoIn->u32SensorId);
    LOG_DBG("ccu sensor in, compatInfoIn->eWhichSensor: %x\n", compatInfoIn->eWhichSensor);
    LOG_DBG("ccu sensor in, compatInfoIn->u16FPS: %x\n", compatInfoIn->u16FPS);
    LOG_DBG("ccu sensor in, compatInfoIn->eScenario: %x\n", compatInfoIn->eScenario);
    LOG_DBG("ccu sensor in, compatInfoIn->pu8BufferVAddr: %x\n", compatInfoIn->pu8BufferVAddr);
	LOG_DBG_MUST("ccu sensor in, compatInfoIn->u32BufferPAddrH: %x\n", compatInfoIn->u32BufferPAddrH);
    LOG_DBG_MUST("ccu sensor in, compatInfoIn->u32BufferPAddrL: %x\n", compatInfoIn->u32BufferPAddrL);
	LOG_DBG_MUST("ccu sensor in, compatInfoIn->u32I2CBaseAddr: %x\n", compatInfoIn->u32I2CId);
    LOG_DBG_MUST("ccu sensor in, compatInfoIn->u16BufferLen: %x\n", compatInfoIn->u16BufferLen);
    //LOG_DBG("ccu sensor in, compatInfoIn->sensorI2cSlaveAddr : %x\n", compatInfoIn->sensorI2cSlaveAddr);

    LOG_DBG("-:%s\n",__FUNCTION__);
    return true;
}

bool CcuCtrlAe::ccuCtrlPostprocess_SensorInit(void *outDataPtr)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    COMPAT_SENSOR_INFO_IN_T *compatInfoIn = (COMPAT_SENSOR_INFO_IN_T *)m_BufferList.mailboxInBuf.va;
    SENSOR_INFO_OUT_T *infoOut = (SENSOR_INFO_OUT_T *)outDataPtr;

    if(infoOut->u8SupportedByCCU == 0)
    {
        LOG_ERR("ccu not support sensor\n");
        return false;
    }

    LOG_DBG("ccu sensor init cmd done\n");
    LOG_DBG("ccu sensor out, u16TransferLen: %d\n", infoOut->u16TransferLen);
    //LOG_DBG("ccu sensor out, u8SlaveAddr: %x\n", infoOut->u8SlaveAddr);
    LOG_DBG("ccu sensor out, u8SupportedByCCU: %x\n", infoOut->u8SupportedByCCU);

    //======== Set i2c controller initial settings: slave addr, transfer length =========
    LOG_DBG("setI2CMode, compatInfoIn->sensorI2cSlaveAddr : %x\n", compatInfoIn->sensorI2cSlaveAddr);
    if (!m_pDrvCcu->setI2CMode((unsigned int)(compatInfoIn->sensorI2cSlaveAddr), (unsigned int)infoOut->u16TransferLen))
    {
        LOG_ERR("setI2CMode fail \n");
        return false;
    }
    LOG_DBG("-:%s\n",__FUNCTION__);

    return true;
}

};  //namespace NSCcuIf