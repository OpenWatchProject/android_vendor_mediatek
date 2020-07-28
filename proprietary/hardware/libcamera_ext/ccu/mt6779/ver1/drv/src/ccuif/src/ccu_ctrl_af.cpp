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
#ifdef CCU_AF_ENABLE
#define LOG_TAG "ICcuCtrlAf"

#include "ccu_ctrl.h"
#include "ccu_ctrl_af.h"
#include <cutils/properties.h>  // For property_get().
#include "ccu_log.h"
#include "ccu_drvutil.h"
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include "utilSystrace.h"
#include "ccu_mem_mgr.h"
#include <af_param.h>
#include "memory.h"
#include "ccu_ext_interface/ccu_sensor_extif.h"

namespace NSCcuIf {

/**************************************************************************
* Globals
**************************************************************************/

EXTERN_DBG_LOG_VARIABLE(ccu_drv);
extern "C"
NSCam::NSIoPipe::NSCamIOPipe::INormalPipe* createInstance_INormalPipe(MUINT32 SensorIndex, char const* szCallerName);

/*******************************************************************************
* Factory Function
********************************************************************************/
ICcuCtrlAf *ICcuCtrlAf::createInstance(uint8_t const szUsrName[32],ESensorDev_T sensorDev)
{
    LOG_INF_MUST("+ User: %s(%d)\n", szUsrName,sensorDev);

    if (sensorDev == ESensorDev_MainSecond)
    {
        static CcuCtrlAf ccuafsingletonMain2;
        return (ICcuCtrlAf *)&ccuafsingletonMain2;
    }
    if (sensorDev == ESensorDev_MainThird)
    {
        static CcuCtrlAf ccuafsingletonMain3;
        return (ICcuCtrlAf *)&ccuafsingletonMain3;
    }
    else if (sensorDev == ESensorDev_Sub)
    {
        static CcuCtrlAf ccuafsingletonSub1;
        return (ICcuCtrlAf *)&ccuafsingletonSub1;
    }
    else if (sensorDev == ESensorDev_SubSecond)
    {
        static CcuCtrlAf ccuafsingletonSub2;
        return (ICcuCtrlAf *)&ccuafsingletonSub2;
    }
    else if (sensorDev == ESensorDev_MainFourth)
    {
        static CcuCtrlAf ccuafsingletonMain4;
        return (ICcuCtrlAf *)&ccuafsingletonMain4;
    }
    else
    {
        if (sensorDev != ESensorDev_Main)
        {
            LOG_WRN("%s sensorDev invalid : %d", __FUNCTION__, sensorDev);
        }
        static CcuCtrlAf ccuafsingletonMain1;
        return (ICcuCtrlAf *)&ccuafsingletonMain1;
    }
}

/*******************************************************************************
* Public Functions
********************************************************************************/
void CcuCtrlAf::destroyInstance(void)
{
    //delete this;
}

int CcuCtrlAf::init(MUINT32 sensorIdx, ESensorDev_T sensorDev)
{
    LOG_DBG("+(%d).\n", sensorDev);

    if((sensorIdx < IMGSENSOR_SENSOR_IDX_MIN_NUM) || (sensorIdx >= IMGSENSOR_SENSOR_IDX_MAX_NUM))
    {
        LOG_WRN("sensorIdx(%d) invalid, init fail.", sensorIdx);
        return -CCU_CTRL_ERR_GENERAL;
    }

    int ret = AbsCcuCtrlBase::init(sensorIdx, sensorDev);
    if(ret != CCU_CTRL_SUCCEED)
    {
        return ret;
    }

    m_afBufferList = (CcuAfBufferList *)m_pBufferList;

    for(int j=0 ; j<Half_AFO_BUFFER_COUNT ; j++)
    {
        LOG_DBG("AF_Buffer[%d] va: %p, mva: 0x%x ",j , m_afBufferList->AF_Buf[j].getVa(), m_afBufferList->AF_Buf[j].getMva());
    }

    LOG_DBG("-(%d)\n", sensorDev);
    return CCU_CTRL_SUCCEED;
}

/*******************************************************************************
* Overridden Functions
********************************************************************************/
enum ccu_feature_type CcuCtrlAf::_getFeatureType()
{
    return CCU_FEATURE_AF;
}

bool CcuCtrlAf::ccu_af_enque_afo_buffer(void *AfoBufferPtr)
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAf is not initialized yet, skip", __FUNCTION__);
        return false;
    }

    UTIL_TRACE_BEGIN(__FUNCTION__);
    LOG_DBG("%s\n", __FUNCTION__);
    UTIL_TRACE_END();

    return true;
}

void CcuCtrlAf::vaTomva(int device,char * bufinfo)
{
    UTIL_TRACE_BEGIN(__FUNCTION__);
    LOG_DBG("%s\n", __FUNCTION__);
    UTIL_TRACE_END();
}

bool CcuCtrlAf::ccu_af_set_hw_regs(void *AfAlgoOutput)
{
    UTIL_TRACE_BEGIN(__FUNCTION__);
    LOG_DBG("%s\n", __FUNCTION__);
    UTIL_TRACE_END();

    return true;
}

void CcuCtrlAf::ccu_af_deque_afo_buffer(AFAcquireBuf_Reg_INFO_OUT_AP_T* rAFAcquireBufout)
{
    UTIL_TRACE_BEGIN(__FUNCTION__);
    LOG_DBG("%s\n", __FUNCTION__);
    UTIL_TRACE_END();
}

bool CcuCtrlAf::ccuCtrlPreprocess(uint32_t msgId, void *inDataPtr, void *inDataBuf)
{
    if(msgId != MSG_TO_CCU_AF_INIT)
        return 1;
    LOG_DBG("+:%s\n",__FUNCTION__);
#if 1
    AFOINIT_INFO_IN_T *AFIN = (AFOINIT_INFO_IN_T *)inDataPtr;
    LENS_INFO_IN_T *infoIn = &AFIN->lens_info;
    MUINT32 sensor_nm_idx = CcuDrvUtil::sensorIdxToNonmapIdx(m_sensorIdx);

    struct ccu_i2c_buf_mva_ioarg i2cBufIoArg;
   //======== Set i2c i2cControllerId of cam type to make kernel driver select correct i2c driver =========
    LOG_INF_MUST("Lens Type: %d, %d\n", m_sensorIdx, sensor_nm_idx);
    i2cBufIoArg.sensor_idx = sensor_nm_idx;

    uint8_t lens_slot;

    if (!m_pDrvCcu->allocSensorSlot(m_sensorIdx, &lens_slot)){
        LOG_WRN("allocSensorSlot fail \n");
        return -CCU_CTRL_ERR_GENERAL;
    }

    if (!m_pDrvCcu->loadLensBin(m_sensorIdx, lens_slot, infoIn->szLensName))
    {
        LOG_WRN("loadLensBin fail \n");
        return -CCU_CTRL_ERR_GENERAL;
    } 

    //======== Get i2c dma buffer mva =========
    LOG_DBG_MUST("skip get i2c bufaddr\n");
    if (!m_pDrvCcu->getI2CDmaBufferAddr(&i2cBufIoArg))
    {
        LOG_ERR("getI2CDmaBufferAddr fail \n");
        m_pDrvCcu->TriggerAee("getI2CDmaBufferAddr failed");
        return false;
    }

    i2cBufIoArg.mva+=2*PAGE_SIZE;
    i2cBufIoArg.va_l+=2*PAGE_SIZE;

    if ((m_sensorIdx == IMGSENSOR_SENSOR_IDX_MAIN) || (m_sensorIdx == IMGSENSOR_SENSOR_IDX_SUB))
    {
        i2cBufIoArg.mva+=PAGE_SIZE;
        i2cBufIoArg.va_l+=PAGE_SIZE;
    }

    LOG_DBG("i2c_get_dma_buffer_addr: %x\n" , i2cBufIoArg.mva);
    LOG_DBG("i2c_get_dma_buffer_va_h: %x\n" , i2cBufIoArg.va_h);
    LOG_DBG("i2c_get_dma_buffer_va_l: %x\n" , i2cBufIoArg.va_l);
    LOG_DBG("i2c_get_dma_buffer_i2c_id:%x\n", i2cBufIoArg.i2c_id);
    
    //======== Call CCU to initial sensor drv =========
    //Convert to compatible structure
    COMPAT_AFOINIT_INFO_IN_T *compatInfoIn = (COMPAT_AFOINIT_INFO_IN_T *)inDataBuf;

    //compatInfoIn->u32SensorId = infoIn->u32SensorId;
    compatInfoIn->lens_info.dma_buf_mva  = i2cBufIoArg.mva;
    compatInfoIn->lens_info.dma_buf_va_h = i2cBufIoArg.va_h;
    compatInfoIn->lens_info.dma_buf_va_l = i2cBufIoArg.va_l;
    compatInfoIn->lens_info.u32I2CId     = i2cBufIoArg.i2c_id;
    compatInfoIn->lens_info.sensor_slot  = lens_slot;

    //LOG_DBG("ccu sensor in, compatInfoIn->u32SensorId: %x\n", compatInfoIn->u32SensorId);
    LOG_DBG_MUST("ccu sensor in, compatInfoIn->dma_buf_mva: %x\n",  compatInfoIn->lens_info.dma_buf_mva);
    LOG_DBG_MUST("ccu sensor in, compatInfoIn->dma_buf_va_h: %x\n", compatInfoIn->lens_info.dma_buf_va_h);
    LOG_DBG_MUST("ccu sensor in, compatInfoIn->dma_buf_va_l: %x\n", compatInfoIn->lens_info.dma_buf_va_l);
    LOG_DBG_MUST("ccu sensor in, compatInfoIn->u32I2CId: %x\n",     compatInfoIn->lens_info.u32I2CId);
#endif
    LOG_DBG("-:%s\n",__FUNCTION__);
    return true;
}


bool CcuCtrlAf::ccuCtrlPostprocess(uint32_t msgId, void *outDataPtr, void *inDataBuf)
{
    return 1;
/*
    LOG_DBG("+:%s\n",__FUNCTION__);
    if(msgId != 0)
        return 1;
#if 1
    enum CCU_I2C_CHANNEL i2cControllerId;
    COMPAT_AFOINIT_INFO_IN_T *compatInfoIn = (COMPAT_AFOINIT_INFO_IN_T *)inDataBuf;
    LENS_INFO_OUT_T *infoOut = (LENS_INFO_OUT_T *)outDataPtr;

    LOG_DBG_MUST("ccu lens init cmd done, Sensor Type: %x\n", compatInfoIn->lens_info.eWhichSensor);
    LOG_DBG_MUST("ccu lens out, u8SupportedByCCU: %x\n", infoOut->u8SupportedByCCU);
#if 0
    if(infoOut->u8SupportedByCCU == 0)
    {
        LOG_ERR("ccu not support lens\n");
        m_pDrvCcu->TriggerAee("ccu not support lens");
        return false;
    }  
#endif
    //Init i2c controller, call i2c driver to do initialization
    i2cControllerId = m_pDrvCcu->whichSesnorToI2cChannel(compatInfoIn->lens_info.eWhichSensor);
    if(i2cControllerId == CCU_I2C_CHANNEL_UNDEF)
    {
        m_pDrvCcu->PrintReg();
        m_pDrvCcu->DumpDramLog();
        m_pDrvCcu->DumpSramLog();
        m_pDrvCcu->TriggerAee("eWhichSensor error");
        return false;
    }
    LOG_DBG_MUST("skip i2c init\n");
    if (!m_pDrvCcu->initI2cController(i2cControllerId))
    {
        LOG_ERR("initI2cController fail \n");
        m_pDrvCcu->TriggerAee("initI2cController failed");
        return false;
    }
#endif
    LOG_DBG("-:%s\n",__FUNCTION__);
    return true;
*/
}

bool CcuCtrlAf::ccu_af_abort()
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAf is not initialized yet, skip", __FUNCTION__);
        return false;
    }
    //enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("(%d)+\n",0);

    ccuControl(MSG_TO_CCU_AF_ABORT, NULL, NULL);

    LOG_DBG("(%d)-\n",0);
    return true;
}

bool CcuCtrlAf::ccu_af_suspend()
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAf is not initialized yet, skip", __FUNCTION__);
        return false;
    }

    enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("ccu_af_suspend(%d)+\n", tgInfo);
    ccu_af_stop();
    return true;
}

bool CcuCtrlAf::ccu_af_resume()
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAf is not initialized yet, skip", __FUNCTION__);
        return false;
    }

    MINT32 err = 0;
    enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("ccu_af_resume(%d)+\n", tgInfo);
    ccu_af_start(err);
    return true;
}

bool CcuCtrlAf::ccu_af_initialize()
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAf is not initialized yet, skip", __FUNCTION__);
        return false;
    }

    afCtrlInitBuf(m_sensorIdx);

    //get instance index of corresponding CCU AE object, and bind it with this CcuCtrlAe instance
    struct ccu_af_bind_instance_output bind_instance_info = {0};
    ccuControl(MSG_TO_CCU_AF_BIND_INSTANCE, NULL, &bind_instance_info);
    m_ccuInstanceIdx = bind_instance_info.instance_idx;
    LOG_DBG_MUST("Bind this instance of sensorIdx(%d) to CCU AF instanceIdx(%d)", m_sensorIdx, m_ccuInstanceIdx);

    return  true;
}

void CcuCtrlAf::afCtrlInitBuf(MUINT32 sensorIdx)
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAf is not initialized yet, skip", __FUNCTION__);
        return;
    }

    AFOINIT_INFO_IN_T rAFInitparam;

    for(int i=0 ; i < Half_AFO_BUFFER_COUNT ; i++)
    {
        rAFInitparam.AFOBufsAddr[i] = m_afBufferList->AF_Buf[i].getMva();

        memcpy((void *)&AFO_BUF[i], (void *)&m_afBufferList->AF_Buf[i], sizeof(CcuBuffer));

        LOG_DBG( "AFInitparamin AFO_BUF[%d] = 0x%x\n", sensorIdx, i, m_afBufferList->AF_Buf[i].getMva());
    }
    for(int i=0 ; i < Half_AF_REG_COUNT ; i++)
    {
        rAFInitparam.AFORegAddr[i] = m_sensorIdx;
        rAFInitparam.AFORegAddr[i] = rAFInitparam.AFORegAddr[i] << 2;
        rAFInitparam.AFORegAddr[i] = rAFInitparam.AFORegAddr[i]+i;
    }

    /////////////////query from P1/////////////////
    NSCam::NSIoPipe::NSCamIOPipe::INormalPipe*  mpNormalPipe;

    NSCam::NSIoPipe::NSCamIOPipe::V_NormalPipe_MagReg magic;
    NSCam::NSIoPipe::NSCamIOPipe::V_NormalPipe_TwinReg twinreg;
    mpNormalPipe = createInstance_INormalPipe(m_sensorIdx, "ccu");

    mpNormalPipe->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_MAGIC_REG_ADDR,(MINTPTR)&magic,0,0);
    mpNormalPipe->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_TWIN_REG_ADDR,(MINTPTR)&twinreg,0,0);
    for(MUINT32 i=0;i<magic.size();i++)
    LOG_DBG("ccu:magic:%d,0x%x\n", magic.at(i).Pipe_path, magic.at(i).regAddr);
    for(MUINT32 i=0;i<twinreg.size();i++)
    LOG_DBG("ccu:twin:%d,0x%x\n", twinreg.at(i).Pipe_path, twinreg.at(i).regAddr);
/*
    NSCam::NSIoPipe::NSCamIOPipe::V_NormalPipe_CROP_INFO info;
    info.clear();
    mpNormalPipe->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_RCP_SIZE,(MINTPTR)&info,0,0);
    for(MUINT32 i=0;i<info.size();i++){
        LOG_DBG("ccu:path:0x%x",info.at(i).Pipe_path);
        for(MUINT32 j=0;j<info.at(i).v_rect.size();j++){
            LOG_DBG("ccu:crop:%d_%d_%d_%d\n",info.at(i).v_rect.at(j).p.x,info.at(i).v_rect.at(j).p.y,\
                info.at(i).v_rect.at(j).s.w,info.at(i).v_rect.at(j).s.h);
        }
    }
*/
    mpNormalPipe->destroyInstance("ccu");
    ///////////////////////////////////////////////////////////////
    rAFInitparam.magic_reg = magic.at(0).regAddr;
    rAFInitparam.twin_reg = 0x50;//twinreg.at(0).regAddr;/*P1 bug*/

    memset(rAFInitparam.lens_info.szLensName, '\0', sizeof(rAFInitparam.lens_info.szLensName));
    strncpy(rAFInitparam.lens_info.szLensName, mcuName_in, strlen(mcuName_in));

    LOG_DBG_MUST("(%s)(%d)+\n", mcuName_in, sensorIdx);
    LOG_DBG_MUST("(%s)(%d)+\n", rAFInitparam.lens_info.szLensName,m_sensorIdx);

    ccuControl(MSG_TO_CCU_AF_INIT, &rAFInitparam, NULL);

}

//init ccu af control
bool CcuCtrlAf::ccu_af_control_initialize(char* mcuName)
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAf is not initialized yet, skip", __FUNCTION__);
        return false;
    }

    LOG_DBG("sensorIdx(%d)+\n", m_sensorIdx);

    LOG_DBG_MUST("lens name from mgr(%s)\n", mcuName);
    
    memset(mcuName_in, '\0', sizeof(mcuName_in));
    strncpy(mcuName_in, mcuName, strlen(mcuName));

    LOG_DBG_MUST("lens name to ccu(%s)\n", mcuName_in);
    ccu_af_initialize();

    struct shared_buf_map *sharedBufMap = m_pDrvCcu->getSharedBufMap();
    
    sharedBufMap->af_shared_buf[m_ccuInstanceIdx].lens_curpos              = 0;
    sharedBufMap->af_shared_buf[m_ccuInstanceIdx].move_lens_curpos_time_L  = 0;
    sharedBufMap->af_shared_buf[m_ccuInstanceIdx].move_lens_curpos_time_H  = 0;
    sharedBufMap->af_shared_buf[m_ccuInstanceIdx].lens_lastpos             = 0;
    sharedBufMap->af_shared_buf[m_ccuInstanceIdx].move_lens_lastpos_time_L = 0;
    sharedBufMap->af_shared_buf[m_ccuInstanceIdx].move_lens_lastpos_time_H = 0;

    LOG_DBG("sensorIdx(%d)-\n", m_sensorIdx);
    return 0;
}
//init AFalgo and enable afo interrupt
#define CCU_AFALGOINIT_CMD_TIMEOUT_SPEC 30000
bool CcuCtrlAf::ccu_af_init_hwconfig(ConfigAFInput_T *firstAFhwconfig, AF_HW_INIT_OUTPUT_T *configOutput)
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAf is not initialized yet, skip", __FUNCTION__);
        return false;
    }

    enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("TG(%d) (%d)+\n", tgInfo, m_sensorIdx);

    LOG_DBG("ccu_af_init_hwconfig enableLPB(%x) ConfigAFInput_T(%d) AF_HW_INIT_INPUT_T(%d) sTG_SZ.i4W(%d) sTG_SZ.i4H(%d)\n", 
        firstAFhwconfig->initHWConfigInput.enableLPB,
        sizeof(ConfigAFInput_T), 
        sizeof(AF_HW_INIT_INPUT_T), 
        firstAFhwconfig->initHWConfigInput.sDefaultHWCfg.sTG_SZ.i4W,
        firstAFhwconfig->initHWConfigInput.sDefaultHWCfg.sTG_SZ.i4H
        );

    struct shared_buf_map *sharedBufMap = m_pDrvCcu->getSharedBufMap();
    sharedBufMap->af_shared_buf[m_ccuInstanceIdx].af_hwinit_done = m_afBufferList->AF_Hwinit_Output[m_ccuInstanceIdx].getMva();
    LOG_DBG_MUST("af_hwinit_done  = %d", sharedBufMap->af_shared_buf[m_ccuInstanceIdx].af_hwinit_done);
    ccuControl(MSG_TO_CCU_AF_HW_INTI, &firstAFhwconfig->initHWConfigInput, configOutput);

    volatile double ts_start = CcuDrvUtil::getTimeUs();
    volatile double ts_end = ts_start;
    volatile double duration = 0;
    uint32_t loopCount = 0;

    while(sharedBufMap->af_shared_buf[m_ccuInstanceIdx].af_hwinit_done == m_afBufferList->AF_Hwinit_Output[m_ccuInstanceIdx].getMva())
    {
        loopCount++;
        ts_end = CcuDrvUtil::getTimeUs();
        duration = ts_end - ts_start;
        if(duration > CCU_AFALGOINIT_CMD_TIMEOUT_SPEC)
        {
            if(sharedBufMap->af_shared_buf[m_ccuInstanceIdx].af_hwinit_done == m_afBufferList->AF_Hwinit_Output[m_ccuInstanceIdx].getMva())
            {
                LOG_WRN("CCU ccu_af_init_hwconfig duration(%lf) ts_end(%lf) ts_start(%lf) sensorId(%d) instence(%d) ", duration, ts_end, ts_start, m_sensorIdx, m_ccuInstanceIdx);
            }
            break;
        }
    }

    LOG_DBG("ccu_af_init_hwconfig Out Data IO buffer copy+");

    MUINT32 *interIoBuf = (MUINT32 *)configOutput;
    MUINT32 *realIoBuf = (MUINT32 *)m_afBufferList->AF_Hwinit_Output[m_ccuInstanceIdx].getVa();

    //copy from _interIBuf to real IO buffer in unit of 4bytes, to avoid APB bus constraint of non-4byte-aligned data access
    for(int i=0 ; i<(sizeof(AF_HW_INIT_OUTPUT_T)/4) ; i++)
    {
        interIoBuf[i] = realIoBuf[i];
    }

    LOG_DBG("ccu_af_init_hwconfig Out Data IO buffer copy-");

    LOG_DBG("afccu ccu_af_init_hwconfig configOutput(%x) ConfigAFInput_T(%d) AF_HW_INIT_OUTPUT_T(%d) sTG_SZ.i4W(%d) sTG_SZ.i4H(%d)\n", 
        configOutput,
        sizeof(ConfigAFInput_T), 
        sizeof(AF_HW_INIT_OUTPUT_T), 
        configOutput->sInitHWCfg.sTG_SZ.i4W,
        configOutput->sInitHWCfg.sTG_SZ.i4H);

    LOG_DBG("TG(%d) (%d)-\n", tgInfo, m_sensorIdx);
    return 0;
}
//init AFalgo and enable afo interrupt
bool CcuCtrlAf::ccu_af_start_and_init_algo(CCUStartAFInput_T *InitAFInput, StartAFOutput_T *InitAFOutput)
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAf is not initialized yet, skip", __FUNCTION__);
        return false;
    }

    enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("TG(%d) (%d)+\n", tgInfo, m_sensorIdx);

    struct shared_buf_map *sharedBufMap = m_pDrvCcu->getSharedBufMap();
    LOG_DBG("(%d)afccu ccu_af_start_and_init_algo CCUStartAFInput_T(%d) AlgoCommand_T(%d) AF_INIT_INPUT_T(%d)\n", 
        tgInfo,
        sizeof(AlgoCommand_T),
        sizeof(CCUStartAFInput_T),
        sizeof(AF_INIT_INPUT_T));

    sharedBufMap->af_shared_buf[m_ccuInstanceIdx].af_init_done = false;

    ccuControl(MSG_TO_CCU_AF_START_ALGO_INIT, InitAFInput, InitAFOutput);

    volatile double ts_start = CcuDrvUtil::getTimeUs();
    volatile double ts_end = ts_start;
    volatile double duration = 0;
    uint32_t loopCount = 0;

    while(sharedBufMap->af_shared_buf[m_ccuInstanceIdx].af_init_done == false)
    {
        loopCount++;
        ts_end = CcuDrvUtil::getTimeUs();
        duration = ts_end - ts_start;
        if(duration > CCU_AFALGOINIT_CMD_TIMEOUT_SPEC)
        {
            if(sharedBufMap->af_shared_buf[m_ccuInstanceIdx].af_init_done == false)
            {
                LOG_WRN("CCU ccu_af_start_and_init_algo duration(%lf) ts_end(%lf) ts_start(%lf) sensorId(%d) instence(%d) ", duration, ts_end, ts_start, m_sensorIdx, m_ccuInstanceIdx);
            }
            break;
        }
    }

    LOG_DBG("ccu_af_start_and_init_algo Out Data IO buffer copy+");

    MUINT32 *interIoBuf = (MUINT32 *)InitAFOutput;
    MUINT32 *realIoBuf = (MUINT32 *)&sharedBufMap->af_shared_buf[m_ccuInstanceIdx].afstartOutput;

    //copy from _interIBuf to real IO buffer in unit of 4bytes, to avoid APB bus constraint of non-4byte-aligned data access
    for(int i=0 ; i<(sizeof(StartAFOutput_T)/4) ; i++)
    {
        interIoBuf[i] = realIoBuf[i];
    }

    LOG_DBG("ccu_af_start_and_init_algo Out Data IO buffer copy-");

    LOG_DBG("TG(%d) (%d)-\n", tgInfo, m_sensorIdx);
    return 0;
}

bool CcuCtrlAf::ccu_af_start(MUINT32 initialpos)
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAf is not initialized yet, skip", __FUNCTION__);
        return false;
    }

    enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("TG(%d) (%d)+\n", tgInfo, m_sensorIdx);

    struct shared_buf_map *sharedBufMap = m_pDrvCcu->getSharedBufMap();

    sharedBufMap->af_shared_buf[m_ccuInstanceIdx].lens_curpos = initialpos;

    ccuControl(MSG_TO_CCU_AF_START, NULL/*(MINTPTR)&magic*/, NULL);
    LOG_DBG_MUST("initialpos(%d)\n", initialpos);
    
    LOG_DBG("TG(%d) (%d)-\n", tgInfo, m_sensorIdx);
    return 0;
}
//init AFalgo to update_nvram
bool CcuCtrlAf::ccu_af_update_nvram(StartAFInput_t InitAFInput, StartAFOutput_t *InitAFOutput)
{
    return true;
}
//for intput of command and data from AFmgr to algo and wait output for per frame state update
void CcuCtrlAf::ccu_af_doAF(DoAFInput_T *doAFinput)
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAf is not initialized yet, skip", __FUNCTION__);
        return;
    }

    enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("TG(%d) (%d)+\n", tgInfo, m_sensorIdx);

    LOG_DBG("ccu_af_doAF DoAFInput_T(%d) AlgoCommand_T(%d) AF_INPUT_T(%d)\n", 
        sizeof(DoAFInput_T),
        sizeof(AlgoCommand_T),
        sizeof(AF_INPUT_T));

    LOG_DBG("ccu_af_doAF u4AfoStatMode(%x) PixelInLine(%d) PixelClk(%d) i4Count(%d) i4W(%d) i4H(%d)\n", 
        doAFinput->afInput.sStatProfile.u4AfoStatMode,
        doAFinput->afInput.PixelInLine,
        doAFinput->afInput.PixelClk,
        doAFinput->afInput.sAFArea.i4Count,
        doAFinput->afInput.sAFArea.sRect[0].i4W,
        doAFinput->afInput.sAFArea.sRect[0].i4H);

    LOG_DBG("algoCommand requestNum(%d) afLibMode(%d) mfPos(%d) eLockAlgo(%d)\n", 
        doAFinput->algoCommand.requestNum,
        doAFinput->algoCommand.afLibMode,
        doAFinput->algoCommand.mfPos,
        doAFinput->algoCommand.eLockAlgo);

    LOG_DBG("algoCommand bTrigger(%d) bCancel(%d) bTargetAssistMove(%d) isSkipAf(%d)\n", 
        doAFinput->algoCommand.bTrigger,
        doAFinput->algoCommand.bCancel,
        doAFinput->algoCommand.bTargetAssistMove,
        doAFinput->algoCommand.isSkipAf);

    doAFinput->afInput.sStatProfile.u4AfoStatMode = 1;
    ccuControl(MSG_TO_CCU_AF_DOAF, doAFinput, NULL);

    int ret = m_pDrvCcu->dequeueAFO(m_sensorIdx);
    LOG_DBG_MUST("gotafodone(%d)\n",ret);
    if(ret == MSG_TO_APMCU_CAM_AFO_ABORT)
        LOG_DBG("dequeueAFO abort\n");

    LOG_DBG("TG(%d) (%d)-\n", tgInfo, m_sensorIdx);
}
//for capture or query result
bool CcuCtrlAf::ccu_af_update_doAFoutput(DoAFOutput_T *doAFoutput)
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAf is not initialized yet, skip", __FUNCTION__);
        return false;
    }

    enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("TG(%d) (%d)+\n", tgInfo, m_sensorIdx);

    LOG_DBG("ccu_af_update_doAFoutput DoAFOutput_T(%d) AF_OUTPUT_T(%d) ConfigAFOutput_T(%d) LENS_INFO_T(%d)\n", 
        sizeof(DoAFOutput_T),
        sizeof(AF_OUTPUT_T),
        sizeof(ConfigAFOutput_T),
        sizeof(LENS_INFO_T));

    ccuControl(MSG_TO_CCU_AF_GET_DOAF_OUTPUT, NULL, doAFoutput);

    LOG_DBG("TG(%d) (%d)-\n", tgInfo, m_sensorIdx);
    return 0;
}

//for PDAF
bool CcuCtrlAf::ccu_af_get_curposition(MINT32 *curposition, MINT32 *curtime_H, MINT32 *curtime_L, MINT32 *lastposition, MINT32 *lasttime_H, MINT32 *lasttime_L)
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAf is not initialized yet, skip", __FUNCTION__);
        return false;
    }

    enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("TG(%d) (%d)+\n", tgInfo, m_sensorIdx);

    struct shared_buf_map *sharedBufMap = m_pDrvCcu->getSharedBufMap();

    *curposition = sharedBufMap->af_shared_buf[m_ccuInstanceIdx].lens_curpos;
    *curtime_L   = sharedBufMap->af_shared_buf[m_ccuInstanceIdx].move_lens_curpos_time_L;
    *curtime_H   = sharedBufMap->af_shared_buf[m_ccuInstanceIdx].move_lens_curpos_time_H;

    *lastposition = sharedBufMap->af_shared_buf[m_ccuInstanceIdx].lens_lastpos;
    *lasttime_L   = sharedBufMap->af_shared_buf[m_ccuInstanceIdx].move_lens_lastpos_time_L;
    *lasttime_H   = sharedBufMap->af_shared_buf[m_ccuInstanceIdx].move_lens_lastpos_time_H;

    LOG_DBG("ccu_af_get_curposition curposition(%d) curtime_L(%x) curtime_H(%x) lastposition(%d) lasttime_L(%x) lasttime_H(%x)\n", 
        *curposition,
        *curtime_L,
        *curtime_H,
        *lastposition,
        *lasttime_L,
        *lasttime_H);

    LOG_DBG("TG(%d) (%d)-\n", tgInfo, m_sensorIdx);
    return 0;
}

bool CcuCtrlAf::ccu_af_get_PD_block(GetPdBlockInput_T pdBlockInut, GetPdBlockOutput_T* pdBlockOutput)
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAf is not initialized yet, skip", __FUNCTION__);
        return false;
    }

    enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("TG(%d) (%d)+\n", tgInfo, m_sensorIdx);

    ccuControl(MSG_TO_CCU_AF_GET_PDBLOCK, &pdBlockInut, pdBlockOutput);

    LOG_DBG("TG(%d) (%d)-\n", tgInfo, m_sensorIdx);
    return 0;
}

bool CcuCtrlAf::ccu_af_get_DebugInfo(EXT_AF_DEBUG_INFO_T *a_DebugInfo)
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAf is not initialized yet, skip", __FUNCTION__);
        return false;
    }

    enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("TG(%d) (%d)+\n", tgInfo, m_sensorIdx);

    ccuControl(MSG_TO_CCU_AF_GET_DEBUGINFO, NULL, a_DebugInfo);

    LOG_DBG("TG(%d) (%d)-\n", tgInfo, m_sensorIdx);
    return 0;
}

bool CcuCtrlAf::ccu_af_stop()
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAf is not initialized yet, skip", __FUNCTION__);
        return false;
    }

    enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("TG(%d) (%d)+\n", tgInfo, m_sensorIdx);

    ccuControl(MSG_TO_CCU_AF_STOP, NULL, NULL);

    LOG_DBG("TG(%d) (%d)-\n", tgInfo, m_sensorIdx);
    return true;
}

bool CcuCtrlAf::ccu_af_get_AFOinfo(AFAcquireBuf_Reg_INFO_OUT_AP_T AFOinfo)
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAf is not initialized yet, skip", __FUNCTION__);
        return false;
    }

    char* AFO_va = 0;
    enum ccu_tg_info tgInfo = getCcuTgInfo();
    struct shared_buf_map *sharedBufMap = m_pDrvCcu->getSharedBufMap();
    int k = 0;
    for(k = 0; k < Half_AFO_BUFFER_COUNT; k++)
    {
        LOG_DBG_MUST("m_afBufferList->AF_Buf[%d][%d].mva : %x", tgInfo-1, k, AFO_BUF[k].getMva());
        if(sharedBufMap->af_shared_buf[m_ccuInstanceIdx].AFO_info.AFOBufmva == AFO_BUF[k].getMva())
        {
            AFO_va = AFO_BUF[k].getVa();
            LOG_DBG_MUST("CCU_AFOBufs CCU to AP = va(0x%x) mva(0x%x)\n", AFO_va, sharedBufMap->af_shared_buf[m_ccuInstanceIdx].AFO_info.AFOBufmva);
            break;
        }
    }

    //AFOinfo = sharedBufMap->AFO_info[tgInfo-1];
    //AFOinfo.AFOBufAddr = AFO_va;

    CcuMemMgr *memMgr = CcuMemMgr::getInstance();
    int ionDevFd;
    ionDevFd = memMgr->getIonDevFd();

/*
    ionDevFd = mt_ion_open(__FILE__);

    ion_user_handle_t buf_handle;
    if (ion_import(ionDevFd, memMgr->getIonShareFd(), &buf_handle) != 0)
    {
        LOG_ERR("[%s] Fail to get import share buffer fd", LOG_TAG);
    }
*/
    LOG_DBG_MUST("(%p)CCU ccu_af_get_AFOinfo::init :(%d)", memMgr, ionDevFd);
    struct ion_sys_data sys_data;
    LOG_DBG_MUST("CCU ion_cache_sync invalidd:(%d)", ionDevFd);
    sys_data.sys_cmd = ION_SYS_CACHE_SYNC;
    sys_data.cache_sync_param.handle = memMgr->getIonHandle();
    sys_data.cache_sync_param.sync_type = ION_CACHE_INVALID_BY_RANGE;
    sys_data.cache_sync_param.va = AFO_BUF[k].getVa(); //please input correct kernel/user va of this buffer.
    sys_data.cache_sync_param.size = AFO_BUF_SIZE; // if you have special usage, can not do all ion buffer flush ,please contact me.

    if (ion_custom_ioctl(ionDevFd, ION_CMD_SYSTEM, (void *)&sys_data))//fd is get from ion_open
        LOG_ERR("CCU ion_cache_sync err info here\n");

    MUINT32 TotalH0=0;
    MUINT32 TotalGsum=0;
    AF_STAT_T* ptrAfo = (AF_STAT_T*)AFO_va;
    LOG_DBG_MUST("afccu u4NumBlkX(%x)u4NumBlkY(%x)ptrAfo(%x) end \n", 
    sharedBufMap->af_shared_buf[m_ccuInstanceIdx].AFO_info.u4NumBlkX,
    sharedBufMap->af_shared_buf[m_ccuInstanceIdx].AFO_info.u4NumBlkY
    , ptrAfo);
 
    for(int j=0; j<sharedBufMap->af_shared_buf[m_ccuInstanceIdx].AFO_info.u4NumBlkY-1; j++)
    {
        ptrAfo += j * (sharedBufMap->af_shared_buf[m_ccuInstanceIdx].AFO_info.u4NumBlkX);
    LOG_DBG_MUST("afccu TotalGsum(%x)TotalH0(%x)ptrAfo(%x) \n", TotalGsum, TotalH0, ptrAfo);
        for(int i=0; i<sharedBufMap->af_shared_buf[m_ccuInstanceIdx].AFO_info.u4NumBlkX-1; i++)
        {
            //TotalH0 += ptrAfo[i].u4FILH0;
            //TotalGsum += ptrAfo[i].u4GSum;
        }
    }

    LOG_DBG_MUST("TotalGsum(%x)TotalH0(%x)ptrAfo(%x) end \n", TotalGsum, TotalH0, ptrAfo);


    LOG_DBG_MUST("ccu_af_get_AFOinfo AFOaddrva(%x) AFOaddrmva(%x) AFOBufStride(%x) AFOBufConfigNum(%x) AFOBufMagic_num(%x) u4NumBlkX(%x) u4NumBlkY(%x) u4SizeBlkX(%x) u4SizeBlkY(%x) \n",
        AFO_va,
        sharedBufMap->af_shared_buf[m_ccuInstanceIdx].AFO_info.AFOBufmva,
        sharedBufMap->af_shared_buf[m_ccuInstanceIdx].AFO_info.AFOBufStride,
        sharedBufMap->af_shared_buf[m_ccuInstanceIdx].AFO_info.AFOBufConfigNum,
        sharedBufMap->af_shared_buf[m_ccuInstanceIdx].AFO_info.AFOBufMagic_num,
        sharedBufMap->af_shared_buf[m_ccuInstanceIdx].AFO_info.u4NumBlkX,
        sharedBufMap->af_shared_buf[m_ccuInstanceIdx].AFO_info.u4NumBlkY,
        sharedBufMap->af_shared_buf[m_ccuInstanceIdx].AFO_info.u4SizeBlkX,
        sharedBufMap->af_shared_buf[m_ccuInstanceIdx].AFO_info.u4SizeBlkY);
    return 0;
}

bool CcuCtrlAf::ccu_af_syncAf_process(MINT32 masterDev, MINT32 masterIdx, MINT32 slaveDev, MINT32 slaveIdx, MINT32 sync2AMode, AF_SyncInfo_T *syncInfo)
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAf is not initialized yet, skip", __FUNCTION__);
        return false;
    }

    enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("TG(%d) (%d)+\n", tgInfo, m_sensorIdx);

    LOG_DBG_MUST("m_sensorIdx(%d)m_sensorDev(%d)slaveIdx(%d)slaveDev(%d)+\n", masterIdx,masterDev,slaveIdx,slaveDev);
    CCUAFsync_info_T af_sync_info = {0};

    //af_sync_info.masterDev  = (MINT32)getCcuTgInfowithsensorDev((ESensorDev_T)m_sensorDev, m_sensorIdx);//masterDev;
    //af_sync_info.slaveDev   = (MINT32)getCcuTgInfowithsensorDev((ESensorDev_T)slaveDev, slaveIdx);//slaveDev;
    LOG_DBG_MUST("af_sync_info TG masterDev(%d)slaveDev(%d)+\n", af_sync_info.masterDev, af_sync_info.slaveDev);

    af_sync_info.masterDev  = (MINT32)getCcuTgInfowithsensorDev((ESensorDev_T)m_sensorDev, m_sensorIdx)-1;//masterDev;
    af_sync_info.slaveDev   = 1-af_sync_info.masterDev;//3-(MINT32)getCcuTgInfowithsensorDev((ESensorDev_T)slaveDev, m_sensorIdx)-1;//slaveDev;
    af_sync_info.sync2AMode = sync2AMode;
    af_sync_info.masterIdx = masterIdx;
    af_sync_info.slaveIdx = slaveIdx;
    //af_sync_info.syncInfo = syncInfo;
    memcpy((void *)&af_sync_info.syncInfo, (void *)syncInfo, sizeof(AF_SyncInfo_T));

    LOG_DBG_MUST("af_sync_info masterDev(%d)slaveDev(%d)+\n", af_sync_info.masterDev, af_sync_info.slaveDev);

    ccuControl(MSG_TO_CCU_AF_SYNC, &af_sync_info, NULL);

    LOG_DBG("TG(%d) (%d)-\n", tgInfo, m_sensorIdx);
    return 0;
}
/*******************************************************************************
* Private Functions
********************************************************************************/

};  //namespace NSCcuIf
#endif //CCU_AF_ENABLE
