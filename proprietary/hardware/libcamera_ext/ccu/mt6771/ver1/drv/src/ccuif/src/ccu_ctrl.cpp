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
#define LOG_TAG "AbsCcuCtrlBase"

#include <string.h>
#include "ccu_ctrl.h"
#include <cutils/properties.h>  // For property_get().
#include "ccu_log.h"
#include <mtkcam/drv/IHalSensor.h>
#include "ccu_drvutil.h"
#include "ccu_platform_def.h"
#include "utilSystrace.h"

/*******************************************************************************
*
********************************************************************************/
using namespace NSIspTuning;
using namespace NSCam;

namespace NSCcuIf {

/**************************************************************************
* Globals
**************************************************************************/
EXTERN_DBG_LOG_VARIABLE(ccu_drv);

android::Mutex      AbsCcuCtrlBase::m_CcuccuControlMutex;

/*******************************************************************************
* Constructor
********************************************************************************/
AbsCcuCtrlBase::AbsCcuCtrlBase(void)
{
    DBG_LOG_CONFIG(ccuif, ccu_drv);
    m_pBufferList = NULL;
    m_initialized = false;
}

/*******************************************************************************
* Public Functions
********************************************************************************/
int AbsCcuCtrlBase::init(MUINT32 sensorIdx, ESensorDev_T sensorDev)
{
    enum ccu_tg_info tgInfo;

    LOG_DBG("+:\n",__FUNCTION__);

    m_sensorIdx = sensorIdx;
    m_sensorDev = sensorDev;
    LOG_DBG("m_sensorIdx:%d, m_sensorDev:%d", m_sensorIdx, m_sensorDev);

    m_featureType = _getFeatureType();
    tgInfo = getCcuTgInfo();
    if(tgInfo <= CCU_CAM_TG_NONE || tgInfo >= CCU_CAM_TG_MAX)
        return -CCU_CTRL_ERR_GENERAL;

    LOG_DBG("getCcuBufferList+");
    ICcuMgrPriv *ccuMgr = ICcuMgrPriv::createInstance();
    if(ccuMgr == NULL)
    {
        LOG_WRN("[%s] CCU is not enabled, skip", __FUNCTION__);
        return -CCU_CTRL_ERR_GENERAL;
    }
    if(!ccuMgr->isCcuBooted())
    {
        LOG_WRN("[%s] CCU is not booted yet, skip", __FUNCTION__);
        return -CCU_CTRL_ERR_GENERAL;
    }
    m_pBufferList = ccuMgr->getCcuBufferList(m_featureType, tgInfo);
    ccuMgr->destroyInstance();
    LOG_DBG("getCcuBufferList-");

    if(!m_pBufferList->isValid)
    {
        LOG_WRN("[%s] CCU BufferList is not ready, skip", __FUNCTION__);
        return -CCU_CTRL_ERR_GENERAL;
    }

    LOG_DBG("mailboxInBuf va: %p, mva: 0x%x ", m_pBufferList->mailboxInBuf.va, m_pBufferList->mailboxInBuf.mva);
    LOG_DBG("mailboxOutBuf va: %p, mva: 0x%x ", m_pBufferList->mailboxOutBuf.va, m_pBufferList->mailboxOutBuf.mva);
    
    for(int i=0 ; i<CCU_EXT_MSG_COUNT ; i++)
    {
        LOG_DBG("CtrlMsgBufs[%d] va: %p, mva: 0x%x ",i , m_pBufferList->CtrlMsgBufs[i].va, m_pBufferList->CtrlMsgBufs[i].mva);
    }
    

    m_pDrvCcu = (CcuDrvImp*)CcuDrvImp::createInstance(CCU_A);
    if (m_pDrvCcu == NULL)
    {
        LOG_ERR("createInstance(CCU_A)return %p", m_pDrvCcu);
        return -CCU_CTRL_ERR_GENERAL;
    }

    LOG_DBG("-:\n",__FUNCTION__);

    m_initialized = true;

    return CCU_CTRL_SUCCEED;
}

int AbsCcuCtrlBase::uninit()
{
    LOG_DBG("+:\n",__FUNCTION__);

    if(m_pDrvCcu)
    {
        m_pDrvCcu->destroyInstance();
        m_pDrvCcu = NULL;
    }

    m_initialized = false;

    LOG_DBG("-:\n",__FUNCTION__);

    return CCU_CTRL_SUCCEED;
}

static MUINT8 _interIBuf[CCU_IPC_IBUF_CAPACITY];
static MUINT8 _interOBuf[CCU_IPC_OBUF_CAPACITY];
MBOOL AbsCcuCtrlBase::resolveCmdIOBuf(const struct CcuCtrlParamDescriptor *paramDescptr, CcuIOBufInfo *inDataBufInfo, CcuIOBufInfo *outDataBufInfo)
{
    MBOOL ret;
    MUINT32 inDataSize = paramDescptr->input_non_ptr_total_size + sizeof(MUINT32)*paramDescptr->ptr_count;
    MUINT32 outDataSize = paramDescptr->output_non_ptr_total_size;

    if(m_pDrvCcu == NULL)
    {
        LOG_ERR("m_pDrvCcu invalid., m_pDrvCcu(%p)", m_pDrvCcu);
        return MFALSE;
    }

    ret = m_pDrvCcu->getIpcIOBuf((void **)&inDataBufInfo->addr_ap, (void **)&outDataBufInfo->addr_ap, &inDataBufInfo->addr_ccu, &outDataBufInfo->addr_ccu);
    if(ret == MFALSE)
    {
        return ret;
    }

    LOG_DBG("inDataSize(%d), sramInBufCapacity(%d), dramInBufCapacity(%d)", inDataSize, CCU_IPC_IBUF_CAPACITY, MAILBOX_IN_BUF_SIZE);
    LOG_DBG("outDataSize(%d), sramOutBufCapacity(%d), dramOutBufCapacity(%d)", outDataSize, CCU_IPC_OBUF_CAPACITY, MAILBOX_OUT_BUF_SIZE);

    if((inDataSize > MAILBOX_IN_BUF_SIZE) && (inDataSize > CCU_IPC_IBUF_CAPACITY))
    {
        LOG_ERR("inDataSize excceed buffer capacity., sramBufCap(%d), dramBufCap(%d)", CCU_IPC_IBUF_CAPACITY, MAILBOX_IN_BUF_SIZE);
        return MFALSE;
    }
    else if((inDataSize > CCU_IPC_IBUF_CAPACITY))
    {
        LOG_DBG("using Dram input buffer");
        inDataBufInfo->bufType = SYS_DRAM_IO;
        inDataBufInfo->addr_ap = (MUINT8 *)m_pBufferList->mailboxInBuf.va;
        inDataBufInfo->addr_ap_interm = (MUINT8 *)m_pBufferList->mailboxInBuf.va;
        inDataBufInfo->addr_ccu = m_pBufferList->mailboxInBuf.mva;
    }
    else
    {
        inDataBufInfo->bufType = CCU_SRAM_IO;
        inDataBufInfo->addr_ap_interm = _interIBuf;
        LOG_DBG("using Sram input buffer");
    }

    if(outDataSize > MAILBOX_OUT_BUF_SIZE)
    {
        LOG_ERR("outDataSize excceed buffer capacity.");
        return MFALSE;
    }
    else if((outDataSize > CCU_IPC_OBUF_CAPACITY))
    {
        LOG_DBG("using Dram output buffer");
        outDataBufInfo->bufType = SYS_DRAM_IO;
        outDataBufInfo->addr_ap  = (MUINT8 *)m_pBufferList->mailboxOutBuf.va;
        outDataBufInfo->addr_ccu = m_pBufferList->mailboxOutBuf.mva;
    }
    else
    {
        outDataBufInfo->bufType = CCU_SRAM_IO;
        outDataBufInfo->addr_ap_interm = _interOBuf;
        LOG_DBG("using Sram output buffer");
    }

    return MTRUE;
}

int AbsCcuCtrlBase::ccuControl(ccu_msg_id msgId, void *inDataPtr, void *outDataPtr)
{
    LOG_DBG("+:\n",__FUNCTION__);

    if(!m_initialized)
    {
        LOG_ERR("CcuCtrl not initialized.");
        return -CCU_CTRL_ERR_GENERAL;
    }

    android::Mutex::Autolock lock(this->m_CcuccuControlMutex);

    if(msgId < 0 || msgId >= CCU_EXT_MSG_COUNT)
    {
        LOG_ERR("%s: msgId invalid: %x", __FUNCTION__, msgId);
        return -CCU_CTRL_ERR_GENERAL;
    }

    //Get control param descriptor from pre-difined list
    const struct CcuCtrlParamDescriptor *paramDescptr = CCU_CTRL_PARAM_DESCRIPTOR_LIST[msgId];

    UTIL_TRACE_BEGIN(paramDescptr->msg_literal);

    LOG_DBG("cotnrol msgId: %d, msg: %s\n", msgId, paramDescptr->msg_literal);

    //resolve IO buf
    CcuIOBufInfo inDataBufInfo;
    CcuIOBufInfo outDataBufInfo;
    if(resolveCmdIOBuf(paramDescptr, &inDataBufInfo, &outDataBufInfo) == MFALSE)
    {
        LOG_ERR("resolveCmdIOBuf failed, cmd abort.");
        return -CCU_CTRL_ERR_GENERAL;
    }
    LOG_DBG("inDataBufInfo.addr_ap: %p", inDataBufInfo.addr_ap);
    LOG_DBG("inDataBufInfo.addr_ap_interm: %p", inDataBufInfo.addr_ap_interm);

    //Check if need input data
    if(paramDescptr->need_input)
    {
        ccu_control_handle_input(msgId, paramDescptr, inDataPtr, inDataBufInfo.addr_ap_interm);
    }

    if(!ccuCtrlPreprocess(msgId, inDataPtr, inDataBufInfo.addr_ap_interm))
    {
        LOG_DBG("ccuCtrlPreprocess fail, msgId: %d\n", msgId);
        return -CCU_CTRL_ERR_GENERAL;
    }

    //copy from _interIBuf to real IO buffer in unit of 4bytes, to avoid APB bus constraint of non-4byte-aligned data access
    if(inDataBufInfo.bufType == CCU_SRAM_IO)
    {
        LOG_DBG("Intermediate IO buffer copy+");

        volatile MUINT32 *interIoBuf = (MUINT32 *)inDataBufInfo.addr_ap_interm;
        volatile MUINT32 *realIoBuf = (MUINT32 *)inDataBufInfo.addr_ap;

        for(int i=0 ; i<(CCU_IPC_IBUF_CAPACITY/4) ; i++)
        {
            realIoBuf[i] = interIoBuf[i];
        }

        if(msgId == MSG_TO_CCU_SENSOR_INIT)
        {
            COMPAT_SENSOR_INFO_IN_T *InterCompatInfoIn = (COMPAT_SENSOR_INFO_IN_T *)interIoBuf;
            COMPAT_SENSOR_INFO_IN_T *RealCompatInfoIn = (COMPAT_SENSOR_INFO_IN_T *)realIoBuf;
            LOG_DBG_MUST("interIoBuf compatInfoIn->u16FPS: %x\n", InterCompatInfoIn->u16FPS);
            LOG_DBG_MUST("interIoBuf compatInfoIn->eScenario: %x\n", InterCompatInfoIn->eScenario);
            LOG_DBG_MUST("interIoBuf compatInfoIn->dma_buf_mva: %x\n", InterCompatInfoIn->dma_buf_mva);

            LOG_DBG_MUST("realIoBuf compatInfoIn->u16FPS: %x\n", RealCompatInfoIn->u16FPS);
            LOG_DBG_MUST("realIoBuf compatInfoIn->eScenario: %x\n", RealCompatInfoIn->eScenario);
            LOG_DBG_MUST("realIoBuf compatInfoIn->dma_buf_mva: %x\n", RealCompatInfoIn->dma_buf_mva);

            LOG_DBG_MUST("inDataBufInfo.addr_ap: %p", inDataBufInfo.addr_ap);
            LOG_DBG_MUST("inDataBufInfo.addr_ap_interm: %p", inDataBufInfo.addr_ap_interm);
        }

        LOG_DBG("Intermediate IO buffer copy-");
    }

    send_ccu_command_ipc(msgId, inDataBufInfo.addr_ccu, outDataBufInfo.addr_ccu);

    //check if need to copy output data
    if(paramDescptr->need_output)
    {
        volatile MUINT32 *ipcOutBuf = (MUINT32 *)outDataBufInfo.addr_ap;
        volatile MUINT32 *intermOutBuf = (MUINT32 *)outDataBufInfo.addr_ap_interm;
        volatile MUINT32 *outBuf = (MUINT32 *)outDataPtr;

        LOG_DBG("need output 0x%x bytes, copying 0x%x bytes to Intermediate buffer\n", paramDescptr->output_non_ptr_total_size, CCU_IPC_OBUF_CAPACITY);       
        for(int i=0 ; i<(CCU_IPC_OBUF_CAPACITY/4) ; i++)
        {
            intermOutBuf[i] = ipcOutBuf[i];
        }

        LOG_DBG("copying 0x%x bytes to out buffer\n", paramDescptr->output_non_ptr_total_size);
        memcpy((MUINT32 *)outBuf, (MUINT32 *)intermOutBuf, paramDescptr->output_non_ptr_total_size);
    }

    if(msgId == MSG_TO_CCU_SENSOR_INIT)
    {
        COMPAT_SENSOR_INFO_IN_T *RealCompatInfoIn = (COMPAT_SENSOR_INFO_IN_T *)inDataBufInfo.addr_ap;
        LOG_DBG_MUST("realIoBufAft compatInfoIn->u16FPS: %x\n", RealCompatInfoIn->u16FPS);
        LOG_DBG_MUST("realIoBufAft compatInfoIn->eScenario: %x\n", RealCompatInfoIn->eScenario);
        LOG_DBG_MUST("realIoBufAft compatInfoIn->dma_buf_mva: %x\n", RealCompatInfoIn->dma_buf_mva);

        LOG_DBG_MUST("inDataBufInfoAft.addr_ap: %p", inDataBufInfo.addr_ap);
        LOG_DBG_MUST("inDataBufInfoAft.addr_ap_interm: %p", inDataBufInfo.addr_ap_interm);
    }

    if(!ccuCtrlPostprocess(msgId, outDataPtr, inDataBufInfo.addr_ap))
    {
        LOG_DBG("ccuCtrlPostprocess fail, msgId: %d\n", msgId);
        return -CCU_CTRL_ERR_GENERAL;
    }

    LOG_DBG("-:\n",__FUNCTION__);

    UTIL_TRACE_END();
    return CCU_CTRL_SUCCEED;
}

int AbsCcuCtrlBase::ccu_control_handle_input(ccu_msg_id msgId, const struct CcuCtrlParamDescriptor *paramDescptr, void *inDataPtr, void *inDataBuf)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    const struct CcuPtrDescriptor *ptrDescptr = paramDescptr->intput_ptr_descs;
    int i = 0;

    //set inData pointer as original input data struct pointer
    int8_t *orig_in_data_ptr = (int8_t *)inDataPtr;

    LOG_DBG("need input, non-ptr size: 0x%x\n", paramDescptr->input_non_ptr_total_size);
    //copy non-ptr types
    LOG_DBG("inDataBuf: %p, orig_in_data_ptr: %p\n", inDataBuf, orig_in_data_ptr);
    memcpy(inDataBuf, orig_in_data_ptr, paramDescptr->input_non_ptr_total_size);

    //check if need do pointer conversion & content copy
    //if all data is non-ptr, this loop will be skiped
    if(ptrDescptr != NULL)
    {
        //set m_pBufferList->mailboxInBuf.va pointer as compat input data struct pointer (in compat struct, ptr is convert to 32bit mva)
        int8_t *compat_ptr_member_addr;
        int8_t *orig_ptr_memeber;
        //set ION buffer to store datas (since ION API support only ION buffer to be convert into mva)
        //datas pointed by pointers in input data struct will be copied onto this ION buffer
        int8_t *ion_ptr_content_va = (int8_t *)m_pBufferList->CtrlMsgBufs[msgId].va;
        CCU_COMPAT_PTR_T ion_ptr_content_mva = (CCU_COMPAT_PTR_T)m_pBufferList->CtrlMsgBufs[msgId].mva;

        for(i=0; ptrDescptr->valid; i++)
        {
            LOG_DBG("ptrDescptr[%d], offst orig: 0x%x\n", i, ptrDescptr->offset_in_orig_struct);
            LOG_DBG("ptrDescptr[%d], offset compat: 0x%x\n", i, ptrDescptr->offset_in_compat_struct);
            LOG_DBG("ptrDescptr[%d], size: 0x%x\n", i, ptrDescptr->size);
            //calculate addr. of pointer member in original input data struct
            compat_ptr_member_addr = ((int8_t *)inDataBuf) + ptrDescptr->offset_in_compat_struct;
            orig_ptr_memeber = (int8_t *)*((void **)(orig_in_data_ptr + ptrDescptr->offset_in_orig_struct));

            //set pointer member in compat input data struct point to ION buffer mva
            *((CCU_COMPAT_PTR_T *)compat_ptr_member_addr) = ion_ptr_content_mva;
            if(msgId == MSG_TO_CCU_SET_AP_AE_ONCHANGE_DATA)
            {
                LOG_DBG("CCU_ONCHANGE_DBG, m_u4Prvflare: %d\n", ((ccu_ae_onchange_data *)orig_in_data_ptr)->m_u4Prvflare);

            }
            LOG_DBG("ptrDescptr[%d], orig_in_data_ptr: %p\n", i, orig_in_data_ptr);
            LOG_DBG("ptrDescptr[%d], inDataBuf: %p\n", i, inDataBuf);
            LOG_DBG_MUST("ptrDescptr[%d], compat_ptr_member_addr: %p\n", i, compat_ptr_member_addr);
            LOG_DBG_MUST("ptrDescptr[%d], compat_ptr_member_val: %x\n", i, *((CCU_COMPAT_PTR_T *)compat_ptr_member_addr));
            LOG_DBG("ptrDescptr[%d], orig_ptr_memeber_addr: %p\n", i, ((void **)(orig_in_data_ptr + ptrDescptr->offset_in_orig_struct)));
            LOG_DBG("ptrDescptr[%d], orig_ptr_memeber: %p\n", i, orig_ptr_memeber);
            LOG_DBG("ptrDescptr[%d], ion_ptr_content_va: %p\n", i, ion_ptr_content_va);
            LOG_DBG_MUST("ptrDescptr[%d], ion_ptr_content_mva: %x\n", i, ion_ptr_content_mva);

            //copy data pointed by pointer member in original inpust data struct into ION buffer
            if(orig_ptr_memeber == 0)
            {
                LOG_ERR("error: ptrDescptr[%d], orig_ptr_memeber: %p is zero, skip copy\n", i, orig_ptr_memeber);
            }
            else
            {
                memcpy(ion_ptr_content_va, orig_ptr_memeber, ptrDescptr->size);
            }

            //increase ION buffer by data size of this pointer member
            ion_ptr_content_va += ptrDescptr->size;
            ion_ptr_content_mva += ptrDescptr->size;
            //traverse to next pointer member in original input data struct
            ptrDescptr = ptrDescptr + 1;
        }
    }
    LOG_DBG("ptrDescptr list end\n");

    LOG_DBG("-:%s\n",__FUNCTION__);

    return CCU_CTRL_SUCCEED;
}

int AbsCcuCtrlBase::send_ccu_command_ipc(ccu_msg_id msgId, MUINT32 inDataAddrCcu, MUINT32 outDataAddrCcu)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    LOG_DBG("sending ccu ipc command :%d\n", msgId);
    enum ccu_tg_info tgInfo = getCcuTgInfo();
    if(tgInfo == CCU_CAM_TG_NONE)
    {
        return -CCU_CTRL_ERR_GENERAL;
    }

    struct ccu_msg msg = {msgId, inDataAddrCcu, outDataAddrCcu, tgInfo};

    if(!m_pDrvCcu->sendCmdIpc(&msg))
    {
        LOG_ERR("ccu ipc cmd(%d) fail \n", msg.msg_id);
        return -CCU_CTRL_ERR_GENERAL;
    }

    LOG_DBG("-:%s\n",__FUNCTION__);
    return CCU_CTRL_SUCCEED;
}

enum ccu_tg_info AbsCcuCtrlBase::getCcuTgInfo()
{
    return CcuDrvUtil::sensorDevToTgInfo(m_sensorDev, m_sensorIdx);
}

};  //namespace NSCcuIf


