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
#define LOG_TAG "ICcuCtrlAf"

#include "ccu_ctrl.h"
#include "ccu_ctrl_af.h"
#include <cutils/properties.h>  // For property_get().
#include "ccu_log.h"
#include "ccu_ext_interface/ccu_af_reg.h"

#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>

namespace NSCcuIf {

/**************************************************************************
* Globals
**************************************************************************/
CcuBuffer AFO_BUF[6];
CcuBuffer AFO_REG[6];
/*
CcuBuffer AFO_BUF2[3];
CcuBuffer AFO_REG2[3];
*/
/*
static U32 APAFORegAddr;
static U32 APAFORegAddr2;
*/
static U32 APAFORegAddr[2];
static bool Abort_A = 0;
static bool Abort_B = 0;

EXTERN_DBG_LOG_VARIABLE(ccu_drv);
extern "C"
NSCam::NSIoPipe::NSCamIOPipe::INormalPipe* createInstance_INormalPipe(MUINT32 SensorIndex, char const* szCallerName);


/*******************************************************************************
* Factory Function
********************************************************************************/
ICcuCtrlAf *ICcuCtrlAf::createInstance(uint8_t const szUsrName[32],ESensorDev_T sensorDev)
{
    LOG_INF_MUST("+ User: %s(%d)\n", szUsrName,sensorDev);
/*
    if( == NULL)
    {
         = new CcuCtrlAf();
    }
*/
    LOG_INF_MUST("-\n");
	return new CcuCtrlAf();
}

/*******************************************************************************
* Public Functions
********************************************************************************/
void CcuCtrlAf::destroyInstance(void)
{
	delete this;
}

/*******************************************************************************
* Overridden Functions
********************************************************************************/

bool CcuCtrlAf::ccu_af_stop()
{
    enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("(%d)+:%s\n",tgInfo,__FUNCTION__);

    if(tgInfo == CCU_CAM_TG_1)
        Abort_A = 0;
    else if(tgInfo == CCU_CAM_TG_2)
        Abort_B = 0;

    ccuControl(MSG_TO_CCU_AF_STOP, NULL, NULL);

    LOG_DBG("(%d)-:%s\n",tgInfo,__FUNCTION__);
    return true;
}

bool CcuCtrlAf::ccu_af_enque_afo_buffer(void *AfoBufferPtr)
{
    char * bufinfo;
    enum ccu_tg_info tgInfo = getCcuTgInfo();
    bufinfo = (char*)AfoBufferPtr;

    LOG_DBG("(%d)+:%s:%x\n",tgInfo,__FUNCTION__,bufinfo);

    switch ( tgInfo )
    {
        case CCU_CAM_TG_1:
        case CCU_CAM_TG_2:

            vaTomva(tgInfo-1, bufinfo);
            break;

        default:
            LOG_DBG("warn wrong tg number : %d",tgInfo);
            break;
    }

    LOG_DBG("(%d)-:%s\n",tgInfo,__FUNCTION__);
    return true;
}

void CcuCtrlAf::vaTomva(int device,char * bufinfo)
{
    int j;
    for(j=device*3;j<device*3+3;j++)
    {
        LOG_DBG("AFO_BUF[%d].va:%x\n",j,AFO_BUF[j].va);
        LOG_DBG("AFO_BUF[%d].mva:%x\n",j,AFO_BUF[j].mva);
        if(bufinfo == AFO_BUF[j].va)
        {
            LOG_DBG("AFO_BUF[%d].mva;:%x\n",j,AFO_BUF[j].mva);
            break;
        }
            //AFO_BUFm_BufferList.AF_Buf[j].mva;
    }
    LOG_DBG("AF return Buf[%d]:%x\n",j,AFO_BUF[j].mva);
    ccuControl(MSG_TO_CCU_AF_ALGO_DONE, &AFO_BUF[j].mva, NULL);
}

#include "memory.h"

//#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
bool CcuCtrlAf::ccu_af_abort()
{
    enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("(%d)+:%s\n",tgInfo,__FUNCTION__);

    if(tgInfo == CCU_CAM_TG_1)
        Abort_A = 1;
    else if(tgInfo == CCU_CAM_TG_2)
        Abort_B = 1;

    LOG_DBG("(%d)-:%s\n",tgInfo,__FUNCTION__);
    return true;
}
bool CcuCtrlAf::ccu_af_set_hw_regs(void *AfAlgoOutput)
{
    enum ccu_tg_info tgInfo = getCcuTgInfo();
    int i;

    LOG_DBG("(%d)+:%s\n",tgInfo,__FUNCTION__);

    CAM_REG_AF_FMT *ptr = (CAM_REG_AF_FMT*)AfAlgoOutput;

    //cam_reg_af_t* pIsp_reg_af = &ptr->data.reg;
    //LOG_DBG("CCU_S : Addr : %04x\tData : %x", pIsp_reg_af->CAM_CTL_EN.Raw , pIsp_reg_af->CAM_CTL_EN.Raw );

    //ptr->data.val[0] = 0x860000;
    //ptr->data_a.val[0] = 0x860000;
    //ptr->data_b.val[0] = 0x860000;

    if((tgInfo == CCU_CAM_TG_1) && Abort_A)
    {
        LOG_DBG("CamA Abort\n");
        return true;
    }
    else if((tgInfo == CCU_CAM_TG_2) && Abort_B)
    {
        LOG_DBG("CamB Abort\n");
        return true;
    }

    cal_dual_af(tgInfo,ptr);

    CAM_REG_AF_T rAFAcquireBufout;

    memcpy((void *)&rAFAcquireBufout.addr, (void *)&ptr->addr, sizeof(CAM_REG_AF_DATA_T));
    memcpy((void *)&rAFAcquireBufout.data, (void *)&ptr->data.val, sizeof(CAM_REG_AF_DATA_T));
    memcpy((void *)&rAFAcquireBufout.data_a, (void *)&ptr->data_a.val, sizeof(CAM_REG_AF_DATA_T));
    memcpy((void *)&rAFAcquireBufout.data_b, (void *)&ptr->data_b.val, sizeof(CAM_REG_AF_DATA_T));
    memcpy((void *)&rAFAcquireBufout.ori_data, (void *)ptr, sizeof(CAM_REG_AF_FMT));

    switch ( tgInfo )
    {
        case CCU_CAM_TG_1:
        case CCU_CAM_TG_2:

            rAFAcquireBufout.AFORegAddr = APAFORegAddr[tgInfo-1];
            LOG_DBG("SetAFReg:%x\n",APAFORegAddr[tgInfo-1]);
            break;

        default:
            LOG_DBG("warn wrong tg number : %d",tgInfo);
            break;
    }

    rAFAcquireBufout.Config_num = ptr->configNum;
    LOG_DBG("configNum:%x\n",ptr->configNum);

    //for (i = 0; i < REG_AF_NUM; i++)
    //{
        LOG_DBG("apCCU_S %d : Addr : %04x\tData : %x", tgInfo, ptr->addr[21] , ptr->data.val[21]);
        LOG_DBG("apCCU_Sa %d : Addr : %04x\tData : %x", tgInfo, ptr->addr[21] , ptr->data_a.val[21]);
        LOG_DBG("apCCU_Sb %d : Addr : %04x\tData : %x", tgInfo, ptr->addr[21] , ptr->data_b.val[21]);
    //}

    ccuControl(MSG_TO_CCU_AF_SET_HW_REG, &rAFAcquireBufout, NULL);
    LOG_DBG("(%d)-:%s\n",tgInfo,__FUNCTION__);
    return true;
}

bool CcuCtrlAf::ccu_af_start()
{
    enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("(%d)+:%s\n",tgInfo,__FUNCTION__);
    bool result = true;
/*
    if(tgInfo == CCU_CAM_TG_1)
        Abort_A = 0;
    else if(tgInfo == CCU_CAM_TG_2)
        Abort_B = 0;
*/
    

    ccuControl(MSG_TO_CCU_AF_START, NULL/*(MINTPTR)&magic*/, NULL);
    LOG_DBG("(%d)-:%s\n",tgInfo,__FUNCTION__);
    return result;
}

void CcuCtrlAf::ccu_af_deque_afo_buffer(AFAcquireBuf_Reg_INFO_OUT_AP_T* rAFAcquireBufout)
{
    int i,j;
    i = 0;
    //AFAcquireBuf_Reg_INFO_OUT_T AFAcquireBufout;

    enum ccu_tg_info tgInfo = getCcuTgInfo();

    LOG_DBG("(%d)+:%s\n",tgInfo,__FUNCTION__);

    if(tgInfo == CCU_CAM_TG_1)
        i = m_pDrvCcu->AFcreateThread();
    else if(tgInfo == CCU_CAM_TG_2)
        i = m_pDrvCcu->AFBcreateThread();
    else
        LOG_DBG("warn wrong tg number : %d",tgInfo);

	LOG_DBG("ap recieved afo done : %d",i);
    if(i==3)
    {
        LOG_DBG("wait_afo_done_success");
        ccuControl(MSG_TO_CCU_AF_ACQUIRE_AFO_BUFFER, NULL, rAFAcquireBufout);

        LOG_DBG("CCU_AFOBufs : %x",rAFAcquireBufout->AFOBufmva);
        LOG_DBG("CCU_AFORegs : %x",rAFAcquireBufout->AFORegAddr);
        APAFORegAddr[0] = rAFAcquireBufout->AFORegAddr;
        for(j=0;j<3;j++)
        {
            LOG_DBG("m_BufferList.AF_Buf[j].mva : %x",AFO_BUF[j].mva);
            if(rAFAcquireBufout->AFOBufmva== AFO_BUF[j].mva)
            {
                rAFAcquireBufout->AFOBufAddr = AFO_BUF[j].va;
                LOG_DBG("CCU_AFOBufs CCU to AP = 0x%x\n", rAFAcquireBufout->AFOBufAddr);
            }

        }
        /*
        rAFAcquireBufout->AFORegAddr = AFAcquireBufout.AFORegAddr;
        rAFAcquireBufout->AFOBufStride = AFAcquireBufout.AFOBufStride;
        rAFAcquireBufout->AFOBufMagic_num = AFAcquireBufout.AFOBufMagic_num;
        rAFAcquireBufout->AFOBufConfigNum = AFAcquireBufout.AFOBufConfigNum;
*/
    }
    else if(i==4)
    {
        LOG_DBG("wait_afoB_done_success");
        ccuControl(MSG_TO_CCU_AF_ACQUIRE_AFO_BUFFER, NULL, rAFAcquireBufout);

        LOG_DBG("CCU_AFOBufs : %x",rAFAcquireBufout->AFOBufmva);
        LOG_DBG("CCU_AFORegs : %x",rAFAcquireBufout->AFORegAddr);
        APAFORegAddr[1] = rAFAcquireBufout->AFORegAddr;
        for(j=3;j<6;j++)
        {
            LOG_DBG("m_BufferList.AF_Buf[j].mva : %x",AFO_BUF[j].mva);

            if(rAFAcquireBufout->AFOBufmva== AFO_BUF[j].mva)
            {
                rAFAcquireBufout->AFOBufAddr = AFO_BUF[j].va;
                LOG_DBG("CCU_AFOBufs CCU to AP = 0x%x\n", rAFAcquireBufout->AFOBufAddr);
            }
        }
        /*
        rAFAcquireBufout->AFORegAddr = AFAcquireBufout.AFORegAddr;
        rAFAcquireBufout->AFOBufStride = AFAcquireBufout.AFOBufStride;
        rAFAcquireBufout->AFOBufMagic_num = AFAcquireBufout.AFOBufMagic_num;
        rAFAcquireBufout->AFOBufConfigNum = AFAcquireBufout.AFOBufConfigNum;
*/
    }
    else
    {
        LOG_DBG("wait_afo_done_fail");

    }
    LOG_DBG("(%d)-:%s\n",tgInfo,__FUNCTION__);
}
/*
void CcuCtrlAf::mvaTova(int device,AFAcquireBuf_Reg_INFO_OUT_T AFAcquireBufout,AFAcquireBuf_Reg_INFO_OUT_AP_T* rAFAcquireBufout)
{

    for(int j=device;j<device+3;j++)
    {
        LOG_DBG("m_BufferList.AF_Buf[j].mva : %x",AFO_BUF[j].mva);

        if(AFAcquireBufout.AFOBufAddr == AFO_BUF[j].mva)
        {
            rAFAcquireBufout->AFOBufAddr = AFO_BUF[j].va;
            LOG_DBG("CCU_AFOBufs CCU to AP = 0x%x\n", rAFAcquireBufout->AFOBufAddr);
        }
    }
    rAFAcquireBufout->AFORegAddr = AFAcquireBufout.AFORegAddr;
    rAFAcquireBufout->AFOBufStride = AFAcquireBufout.AFOBufStride;
    rAFAcquireBufout->AFOBufMagic_num = AFAcquireBufout.AFOBufMagic_num;
    rAFAcquireBufout->AFOBufConfigNum = AFAcquireBufout.AFOBufConfigNum;

}
*/
bool CcuCtrlAf::ccu_af_initialize()
{
    U32 AFORegAddr;
    enum ccu_tg_info tgInfo = getCcuTgInfo();

    LOG_DBG("(%d)+:%s\n",tgInfo,__FUNCTION__);

    switch ( tgInfo )
    {
        case CCU_CAM_TG_1:
        case CCU_CAM_TG_2:

            afCtrlInitBuf(tgInfo-1,&AFORegAddr);

            LOG_DBG( "AFInitparamout AFO_AcquireReg = 0x%x\n", AFORegAddr);
            APAFORegAddr[tgInfo-1] = AFORegAddr;

            break;

        default:
            LOG_DBG("warn wrong tg number : %d",tgInfo);
            break;
    }

    LOG_DBG("(%d)-:%s\n",getCcuTgInfo(),__FUNCTION__);
        return  true;
}

void CcuCtrlAf::afCtrlInitBuf(int device, U32 *AFORegAddr)
{
    AFOINIT_INFO_IN_T rAFInitparam;
    AFOINIT_INFO_OUT_T rAFInitparamout;

    for(int i=device*3;i<device*3+3;i++)
    {
        rAFInitparam.AFOBufsAddr[i] = m_BufferList.AF_Buf[i].mva;
        rAFInitparam.AFORegAddr[i] = m_BufferList.AF_Reg[i].mva;

        memcpy((void *)&AFO_BUF[i], (void *)&m_BufferList.AF_Buf[i], sizeof(CcuBuffer));
        memcpy((void *)&AFO_REG[i], (void *)&m_BufferList.AF_Reg[i], sizeof(CcuBuffer));

        LOG_DBG( "AFInitparamin AFO_BUF[%d] = 0x%x\n",i, m_BufferList.AF_Buf[i].mva);
        LOG_DBG( "AFInitparamin AFO_Reg[%d] = 0x%x\n",i, m_BufferList.AF_Reg[i].mva);
    }

    //query from P1
    ///////////////////////////////////////////////////////////////
    NSCam::NSIoPipe::NSCamIOPipe::INormalPipe*  mpNormalPipe;

    NSCam::NSIoPipe::NSCamIOPipe::V_NormalPipe_MagReg magic;
    NSCam::NSIoPipe::NSCamIOPipe::V_NormalPipe_CROP_INFO info;
    NSCam::NSIoPipe::NSCamIOPipe::V_NormalPipe_TwinReg twinreg;
    mpNormalPipe = createInstance_INormalPipe(m_sensorIdx, "ccu");
    info.clear();
    mpNormalPipe->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_RCP_SIZE,(MINTPTR)&info,0,0);
    mpNormalPipe->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_MAGIC_REG_ADDR,(MINTPTR)&magic,0,0);
    //mpNormalPipe->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_TWIN_REG_ADDR,(MINTPTR)&twinreg,0,0);
    for(MUINT32 i=0;i<magic.size();i++)
    LOG_DBG("ccu:magic:%d,0x%x\n",magic.at(i).Pipe_path,magic.at(i).regAddr);
    for(MUINT32 i=0;i<twinreg.size();i++)
    LOG_DBG("ccu:twin:%d,0x%x\n",twinreg.at(i).Pipe_path,twinreg.at(i).regAddr);
/*
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
    rAFInitparam.twin_reg = 0x50; //twinreg.at(0).regAddr;

    ccuControl(MSG_TO_CCU_AF_INIT, &rAFInitparam, &rAFInitparamout);
    *AFORegAddr = rAFInitparamout.AFORegAddr;
}

bool CcuCtrlAf::ccuCtrlPreprocess(ccu_msg_id msgId, void *inDataPtr)
{
    bool result = true;

    return result;
}

bool CcuCtrlAf::ccuCtrlPostprocess(ccu_msg_id msgId, void *outDataPtr)
{
    bool result = true;

    return result;
}

/*******************************************************************************
* Private Functions
********************************************************************************/


};  //namespace NSCcuIf
