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
#include "ccu_ext_interface/ccu_af_reg.h"
#include "ccu_ext_interface/ccu_af_extif.h"

#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include "utilSystrace.h"

namespace NSCcuIf {

/**************************************************************************
* Globals
**************************************************************************/
CcuBuffer AFO_BUF[RAW_number][Half_AFO_BUFFER_COUNT];
static U32 APAFORegAddr[RAW_number];

EXTERN_DBG_LOG_VARIABLE(ccu_drv);
extern "C"
NSCam::NSIoPipe::NSCamIOPipe::INormalPipe* createInstance_INormalPipe(MUINT32 SensorIndex, char const* szCallerName);


/*******************************************************************************
* Factory Function
********************************************************************************/
ICcuCtrlAf *ICcuCtrlAf::createInstance(uint8_t const szUsrName[32], ESensorDev_T sensorDev)
{
    LOG_INF_MUST("+ User: %s(%d)\n", szUsrName, sensorDev);
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

int CcuCtrlAf::init(MUINT32 sensorIdx, ESensorDev_T sensorDev)
{
    LOG_DBG("+\n");

    int ret = AbsCcuCtrlBase::init(sensorIdx, sensorDev);
    if (ret != CCU_CTRL_SUCCEED)
    {
        return ret;
    }

    m_afBufferList = (CcuAfBufferList *)m_pBufferList;
    LOG_DBG("getCcuBufferList-%d", m_pBufferList->m_IonDevFD);
    LOG_DBG("m_afBufferList-%d", m_afBufferList->m_IonDevFD);

    for (int i = 0 ; i < RAW_number ; i++)
        for (int j = 0 ; j < Half_AFO_BUFFER_COUNT ; j++)
        {
            LOG_DBG("AF_Buffer[%d][%d] va: %p, mva: 0x%x ", i , j , m_afBufferList->AF_Buf[i][j].va, m_afBufferList->AF_Buf[i][j].mva);
            LOG_DBG("AF__Reg__[%d][%d] va: %p, mva: 0x%x ", i , j , m_afBufferList->AF_Reg[i][j].va, m_afBufferList->AF_Reg[i][j].mva);
        }

    LOG_DBG("-\n");
    return CCU_CTRL_SUCCEED;
}

/*******************************************************************************
* Overridden Functions
********************************************************************************/
enum ccu_feature_type CcuCtrlAf::_getFeatureType()
{
    return CCU_FEATURE_AF;
}

bool CcuCtrlAf::ccu_af_initialize()
{
    U32 AFORegAddr;
    enum ccu_tg_info tgInfo = getCcuTgInfo();

    LOG_DBG("(%d)+\n", tgInfo);

    switch ( tgInfo )
    {
    case CCU_CAM_TG_1:
    case CCU_CAM_TG_2:

        afCtrlInitBuf(tgInfo - 1, &AFORegAddr);

        LOG_DBG("AFInitparamout AFO_AcquireReg = 0x%x\n", AFORegAddr);
        APAFORegAddr[tgInfo - 1] = AFORegAddr;

        break;

    default:
        LOG_DBG("warn wrong tg number : %d", tgInfo);
        break;
    }

    LOG_DBG("(%d)-\n", tgInfo);
    return  true;
}

void CcuCtrlAf::afCtrlInitBuf(int device, U32 *AFORegAddr)
{
    AFOINIT_INFO_IN_T rAFInitparam;
    AFOINIT_INFO_OUT_T rAFInitparamout;

    for (int i = 0; i < Half_AFO_BUFFER_COUNT; i++)
    {
        rAFInitparam.AFOBufsAddr[device][i] = m_afBufferList->AF_Buf[device][i].mva;
        rAFInitparam.AFORegAddr[device][i] = 0xaf000000 + i + device * 3; //m_afBufferList->AF_Reg[i].mva;

        memcpy((void *)&AFO_BUF[device][i], (void *)&m_afBufferList->AF_Buf[device][i], sizeof(CcuBuffer));
        //memcpy((void *)&AFO_REG[i], (void *)&m_afBufferList->AF_Reg[i], sizeof(CcuBuffer));

        LOG_DBG( "AFInitparamin AFO_BUF[%d][%d] = 0x%x\n", device, i, m_afBufferList->AF_Buf[device][i].mva);
        //LOG_DBG( "AFInitparamin AFO_Reg[%d] = 0x%x\n",i, m_afBufferList->AF_Reg[i].mva);
    }

    //query from P1
    ///////////////////////////////////////////////////////////////
    NSCam::NSIoPipe::NSCamIOPipe::INormalPipe*  mpNormalPipe;

    NSCam::NSIoPipe::NSCamIOPipe::V_NormalPipe_MagReg magic;
    NSCam::NSIoPipe::NSCamIOPipe::V_NormalPipe_TwinReg twinreg;
    mpNormalPipe = createInstance_INormalPipe(m_sensorIdx, "ccu");

    mpNormalPipe->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_MAGIC_REG_ADDR, (MINTPTR)&magic, 0, 0);
    mpNormalPipe->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_TWIN_REG_ADDR, (MINTPTR)&twinreg, 0, 0);
    for (MUINT32 i = 0; i < magic.size(); i++)
        LOG_DBG("ccu:magic:%d,0x%x\n", magic.at(i).Pipe_path, magic.at(i).regAddr);
    for (MUINT32 i = 0; i < twinreg.size(); i++)
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

    ccuControl(MSG_TO_CCU_AF_INIT, &rAFInitparam, &rAFInitparamout);
    *AFORegAddr = rAFInitparamout.AFORegAddr;
}

bool CcuCtrlAf::ccu_af_stop()
{
    enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("(%d)+\n", tgInfo);

    ccuControl(MSG_TO_CCU_AF_STOP, NULL, NULL);

    LOG_DBG("(%d)-\n", tgInfo);
    return true;
}

bool CcuCtrlAf::ccu_af_enque_afo_buffer(void *AfoBufferPtr)
{
    UTIL_TRACE_BEGIN(__FUNCTION__);
    char * bufinfo;
    enum ccu_tg_info tgInfo = getCcuTgInfo();
    bufinfo = (char*)AfoBufferPtr;

    LOG_DBG("(%d)+:%x\n", tgInfo, bufinfo);

    switch ( tgInfo )
    {
    case CCU_CAM_TG_1:
    case CCU_CAM_TG_2:

        vaTomva(tgInfo - 1, bufinfo);
        break;

    default:
        LOG_DBG("warn wrong tg number : %d", tgInfo);
        break;
    }

    LOG_DBG("(%d)-\n", tgInfo);
    UTIL_TRACE_END();
    return true;
}

void CcuCtrlAf::vaTomva(int device, char * bufinfo)
{
    UTIL_TRACE_BEGIN(__FUNCTION__);
    int j;
    for (j = 0; j < Half_AFO_BUFFER_COUNT; j++)
    {
        LOG_DBG("AFO_BUF[%d][%d].va:%x\n", device, j, AFO_BUF[device][j].va);
        LOG_DBG("AFO_BUF[%d][%d].mva:%x\n", device, j, AFO_BUF[device][j].mva);
        if (bufinfo == AFO_BUF[device][j].va)
        {
            LOG_DBG("AFO_BUF[%d][%d].mva;:%x\n", device, j, AFO_BUF[device][j].mva);
            break;
        }
        //AFO_BUFm_afBufferList->AF_Buf[j].mva;
    }
    LOG_DBG("AF return Buf[%d][%d]:%x\n", device, j, AFO_BUF[device][j].mva);
    ccuControl(MSG_TO_CCU_AF_ALGO_DONE, &AFO_BUF[device][j].mva, NULL);
    UTIL_TRACE_END();
}

#include "memory.h"

//#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
bool CcuCtrlAf::ccu_af_set_hw_regs(void *AfAlgoOutput)
{
    UTIL_TRACE_BEGIN(__FUNCTION__);

    enum ccu_tg_info tgInfo = getCcuTgInfo();
    int i;

    LOG_DBG("(%d)+\n", tgInfo);

    CAM_REG_AF_FMT *ptr = (CAM_REG_AF_FMT*)AfAlgoOutput;
    LOG_DBG("afo_done_valid : (%d)\n", CcuCtrlAf::afo_done_valid);

    CAM_REG_AF_FMT ptr2;
    memcpy((void *)&ptr2, (void *)ptr, sizeof(CAM_REG_AF_FMT));

    switch ( tgInfo )
    {
    case CCU_CAM_TG_1:
    case CCU_CAM_TG_2:

        LOG_DBG("SetAFReg:%x\n", APAFORegAddr[tgInfo - 1]);
        break;

    default:
        LOG_DBG("warn wrong tg number : %d", tgInfo);
        break;
    }

    LOG_DBG("configNum:%x\n", ptr->configNum);

    //for (i = 0; i < REG_AF_NUM; i++)
    //{
    LOG_DBG("apCCU_S %d : Addr : %04x\tData : %x", tgInfo, ptr->addr[21] , ptr->data.val[21]);
    LOG_DBG("apCCU_Sa %d : Addr : %04x\tData : %x", tgInfo, ptr->addr[21] , ptr->data_a.val[21]);
    LOG_DBG("apCCU_Sb %d : Addr : %04x\tData : %x", tgInfo, ptr->addr[21] , ptr->data_b.val[21]);
    //}

    ccuControl(MSG_TO_CCU_AF_SET_HW_REG, &ptr2, NULL);

    LOG_DBG("(%d)-\n", tgInfo);

    UTIL_TRACE_END();

    return true;
}

bool CcuCtrlAf::ccu_af_start()
{
    enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("(%d)+\n", tgInfo);
    bool result = true;

    ccuControl(MSG_TO_CCU_AF_START, NULL/*(MINTPTR)&magic*/, NULL);
    LOG_DBG("(%d)-\n", tgInfo);
    return result;
}

void CcuCtrlAf::ccu_af_deque_afo_buffer(AFAcquireBuf_Reg_INFO_OUT_AP_T* rAFAcquireBufout)
{
    UTIL_TRACE_BEGIN(__FUNCTION__);

    int i = 0, j = 0, k = 0;

    CcuCtrlAf::afo_done_valid = false;
    enum ccu_tg_info tgInfo = getCcuTgInfo();

    LOG_DBG("(%d)+\n", tgInfo);

    if (tgInfo == CCU_CAM_TG_1)
        i = m_pDrvCcu->dequeueAFO_A();
    else if (tgInfo == CCU_CAM_TG_2)
        i = m_pDrvCcu->dequeueAFO_B();
    else
        LOG_DBG("warn wrong tg number : %d", tgInfo);

    struct ion_sys_data sys_data;

    if (i == 3 || i == 4)
    {
        j = i - 3;

        CcuCtrlAf::afo_done_valid = 1;
        LOG_DBG("wait_afo_%d_done_success", j);
        ccuControl(MSG_TO_CCU_AF_ACQUIRE_AFO_BUFFER, NULL, rAFAcquireBufout);

        LOG_DBG("CCU_AFOBufs : %x", rAFAcquireBufout->AFOBufmva);
        LOG_DBG("CCU_AFORegs : %x", rAFAcquireBufout->AFORegAddr);
        LOG_DBG("CCU_AFOAFOBufStride : %x", rAFAcquireBufout->AFOBufStride);
        LOG_DBG("CCU_AFOBufMagic_num : %x", rAFAcquireBufout->AFOBufMagic_num);
        LOG_DBG("CCU_AFOBufConfigNum : %x", rAFAcquireBufout->AFOBufConfigNum);
        APAFORegAddr[j] = rAFAcquireBufout->AFORegAddr;
        for (k = 0; k < Half_AFO_BUFFER_COUNT; k++)
        {
            LOG_DBG("m_afBufferList->AF_Buf[%d][%d].mva : %x", j, k, AFO_BUF[j][k].mva);
            if (rAFAcquireBufout->AFOBufmva == AFO_BUF[j][k].mva)
            {
                rAFAcquireBufout->AFOBufAddr = (long long)AFO_BUF[j][k].va;
                LOG_DBG("CCU_AFOBufs CCU to AP = 0x%x\n", rAFAcquireBufout->AFOBufAddr);
                break;
            }
        }

        LOG_DBG_MUST("CCU ion_cache_sync invalid");
        sys_data.sys_cmd = ION_SYS_CACHE_SYNC;
        sys_data.cache_sync_param.handle = (ion_user_handle_t)AFO_BUF[j][k].ion_handle;
        sys_data.cache_sync_param.sync_type = ION_CACHE_INVALID_BY_RANGE;
        sys_data.cache_sync_param.va = AFO_BUF[j][k].va; //please input correct kernel/user va of this buffer.
        sys_data.cache_sync_param.size = AFO_BUF_SIZE; // if you have special usage, can not do all ion buffer flush ,please contact me.

        if (ion_custom_ioctl(m_afBufferList->m_IonDevFD, ION_CMD_SYSTEM, (void *)&sys_data))//fd is get from ion_open
            LOG_ERR("CCU ion_cache_sync err info here\n");
    }
    else if (i == 5)
    {
        LOG_WRN("wait_afo_done_abort by user");
    }
    else
    {
        LOG_WRN("wait_afo_done_fail");
    }
    LOG_DBG("(%d)-\n", tgInfo);

    UTIL_TRACE_END();
}
/*
void CcuCtrlAf::mvaTova(int device,AFAcquireBuf_Reg_INFO_OUT_T AFAcquireBufout,AFAcquireBuf_Reg_INFO_OUT_AP_T* rAFAcquireBufout)
{

    for(int j=device;j<device+3;j++)
    {
        LOG_DBG("m_afBufferList->AF_Buf[j].mva : %x",AFO_BUF[j].mva);

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

bool CcuCtrlAf::ccu_af_abort()
{
    //enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("(%d)+\n", 0);

    ccuControl(MSG_TO_CCU_AF_ABORT, NULL, NULL);

    LOG_DBG("(%d)-\n", 0);
    return true;
}

bool CcuCtrlAf::ccu_af_suspend()
{
    enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("ccu_af_suspend(%d)+\n", tgInfo);
    ccu_af_stop();
    return true;
}

bool CcuCtrlAf::ccu_af_resume()
{
    enum ccu_tg_info tgInfo = getCcuTgInfo();
    LOG_DBG("ccu_af_resume(%d)+\n", tgInfo);
    ccu_af_start();
    return true;
}

/*******************************************************************************
* Private Functions
********************************************************************************/


};  //namespace NSCcuIf
#endif //CCU_AF_ENABLE
