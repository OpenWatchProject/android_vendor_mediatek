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
}

/*******************************************************************************
* Public Functions
********************************************************************************/
int AbsCcuCtrlBase::init(MUINT32 sensorIdx, ESensorDev_T sensorDev)
{
    LOG_DBG("+:\n",__FUNCTION__);

    ICcuMgrPriv *ccuMgr = ICcuMgrPriv::createInstance();
    LOG_DBG("getCcuBufferList+");
    m_BufferList = ccuMgr->getCcuBufferList();
    ccuMgr->destroyInstance();
    LOG_DBG("getCcuBufferList-");

    if(!m_BufferList.isValid)
    {
        LOG_ERR("CCU BufferList is NOT ready yet.");
        return -CCU_CTRL_ERR_GENERAL;
    }
    LOG_DBG("mailboxInBuf va: %p, mva: 0x%x ", m_BufferList.mailboxInBuf.va, m_BufferList.mailboxInBuf.mva);
    LOG_DBG("mailboxOutBuf va: %p, mva: 0x%x ", m_BufferList.mailboxOutBuf.va, m_BufferList.mailboxOutBuf.mva);
    for(int i=0 ; i<CCU_EXT_MSG_COUNT ; i++)
    {
        LOG_DBG("CtrlMsgBufs[%d] va: %p, mva: 0x%x ",i , m_BufferList.CtrlMsgBufs[i].va, m_BufferList.CtrlMsgBufs[i].mva);
    }
    for(int i=0 ; i<6 ; i++)
    {
        LOG_DBG("AF_Buffer[%d] va: %p, mva: 0x%x ",i , m_BufferList.AF_Buf[i].va, m_BufferList.AF_Buf[i].mva);
        LOG_DBG("AF__Reg__[%d] va: %p, mva: 0x%x ",i , m_BufferList.AF_Reg[i].va, m_BufferList.AF_Reg[i].mva);
    }

    m_pDrvCcu = (CcuDrvImp*)CcuDrvImp::createInstance(CCU_A);
    if (m_pDrvCcu == NULL)
    {
        LOG_ERR("createInstance(CCU_A)return %p", m_pDrvCcu);
        return -CCU_CTRL_ERR_GENERAL;
    }

    m_sensorIdx = sensorIdx;
    m_sensorDev = sensorDev;
    LOG_DBG("m_sensorIdx:%d, m_sensorDev:%d", m_sensorIdx, m_sensorDev);

    LOG_DBG("-:\n",__FUNCTION__);

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

    LOG_DBG("-:\n",__FUNCTION__);

    return CCU_CTRL_SUCCEED;
}

int AbsCcuCtrlBase::ccuControl(ccu_msg_id msgId, void *inDataPtr, void *outDataPtr)
{
    LOG_DBG("+:\n",__FUNCTION__);
    android::Mutex::Autolock lock(this->m_CcuccuControlMutex);

    if(msgId < 0 || msgId >= CCU_EXT_MSG_COUNT)
    {
        LOG_ERR("%s: msgId invalid: %x", __FUNCTION__, msgId);
        return -CCU_CTRL_ERR_GENERAL;
    }

    //Get control param descriptor from pre-difined list
    const struct CcuCtrlParamDescriptor *paramDescptr = CCU_CTRL_PARAM_DESCRIPTOR_LIST[msgId];

    LOG_DBG("cotnrol msgId: %d, msg: %s\n", msgId, paramDescptr->msg_literal);

    //Check if need input data
    if(paramDescptr->need_input)
    {
        ccu_control_handle_input(msgId, paramDescptr, inDataPtr);
    }

    if(!ccuCtrlPreprocess(msgId, inDataPtr))
    {
        LOG_DBG("ccuCtrlPreprocess fail, msgId: %d\n", msgId);
        return -CCU_CTRL_ERR_GENERAL;
    }

    if(send_ccu_command(msgId) != CCU_CTRL_SUCCEED)
    {
        return -CCU_CTRL_ERR_GENERAL;
    }
    LOG_DBG("ccuCtrlPreprocess msgId: %d\n", msgId);
    //m_pDrvCcu->DumpSramLog();
    //check if need to copy output data
    if(paramDescptr->need_output)
    {
        LOG_DBG("need output, copy 0x%x bytes\n", paramDescptr->output_non_ptr_total_size);
        memcpy(outDataPtr, m_BufferList.mailboxOutBuf.va, paramDescptr->output_non_ptr_total_size);
    }

    if(!ccuCtrlPostprocess(msgId, outDataPtr))
    {
        LOG_DBG("ccuCtrlPostprocess fail, msgId: %d\n", msgId);
        return -CCU_CTRL_ERR_GENERAL;
    }

    LOG_DBG("-:\n",__FUNCTION__);

    return CCU_CTRL_SUCCEED;
}

int AbsCcuCtrlBase::ccu_control_handle_input(ccu_msg_id msgId, const struct CcuCtrlParamDescriptor *paramDescptr, void *inDataPtr)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    const struct CcuPtrDescriptor *ptrDescptr = paramDescptr->intput_ptr_descs;
    int i = 0;

    //set inData pointer as original input data struct pointer
    int8_t *orig_in_data_ptr = (int8_t *)inDataPtr;

    LOG_DBG("need input, non-ptr size: 0x%x\n", paramDescptr->input_non_ptr_total_size);
    //copy non-ptr types
    LOG_DBG("mailboxInBuf: %p, orig_in_data_ptr: %p\n", m_BufferList.mailboxInBuf.va, orig_in_data_ptr);
    memcpy(m_BufferList.mailboxInBuf.va, orig_in_data_ptr, paramDescptr->input_non_ptr_total_size);

    //check if need do pointer conversion & content copy
    //if all data is non-ptr, this loop will be skiped
    if(ptrDescptr != NULL)
    {
        //set m_BufferList.mailboxInBuf.va pointer as compat input data struct pointer (in compat struct, ptr is convert to 32bit mva)
        int8_t *compat_ptr_member_addr;
        int8_t *orig_ptr_memeber;
        //set ION buffer to store datas (since ION API support only ION buffer to be convert into mva)
        //datas pointed by pointers in input data struct will be copied onto this ION buffer
        int8_t *ion_ptr_content_va = (int8_t *)m_BufferList.CtrlMsgBufs[msgId].va;
        CCU_COMPAT_PTR_T ion_ptr_content_mva = (CCU_COMPAT_PTR_T)m_BufferList.CtrlMsgBufs[msgId].mva;

        for(i=0; ptrDescptr->valid; i++)
        {
            LOG_DBG("ptrDescptr[%d], offst orig: 0x%x\n", i, ptrDescptr->offset_in_orig_struct);
            LOG_DBG("ptrDescptr[%d], offset compat: 0x%x\n", i, ptrDescptr->offset_in_compat_struct);
            LOG_DBG("ptrDescptr[%d], size: 0x%x\n", i, ptrDescptr->size);
            //calculate addr. of pointer member in original input data struct
            compat_ptr_member_addr = ((int8_t *)m_BufferList.mailboxInBuf.va) + ptrDescptr->offset_in_compat_struct;
            orig_ptr_memeber = (int8_t *)*((void **)(orig_in_data_ptr + ptrDescptr->offset_in_orig_struct));
            if(msgId == MSG_TO_CCU_SET_AP_AE_ONCHANGE_DATA)
            {
                LOG_DBG("CCU_ONCHANGE_DBG, m_u4IndexMax: %d\n", ((ccu_ae_onchange_data *)orig_in_data_ptr)->m_u4IndexMax);
                LOG_DBG("CCU_ONCHANGE_DBG, m_u4IndexMin: %d\n", ((ccu_ae_onchange_data *)orig_in_data_ptr)->m_u4IndexMin);
                LOG_DBG("CCU_ONCHANGE_DBG, auto_flicker_en: %d\n", ((ccu_ae_onchange_data *)orig_in_data_ptr)->auto_flicker_en);
                LOG_DBG("CCU_ONCHANGE_DBG, pCurrentTable: %x\n", ((ccu_ae_onchange_data *)orig_in_data_ptr)->pCurrentTable);
            }
            //set pointer member in compat input data struct point to ION buffer mva
            *((CCU_COMPAT_PTR_T *)compat_ptr_member_addr) = ion_ptr_content_mva;

            LOG_DBG("ptrDescptr[%d], orig_in_data_ptr: %p\n", i, orig_in_data_ptr);
            LOG_DBG("ptrDescptr[%d], m_BufferList.mailboxInBuf.va: %p\n", i, m_BufferList.mailboxInBuf.va);
            LOG_DBG("ptrDescptr[%d], compat_ptr_member_addr: %p\n", i, compat_ptr_member_addr);
            LOG_DBG("ptrDescptr[%d], orig_ptr_memeber_addr: %p\n", i, ((void **)(orig_in_data_ptr + ptrDescptr->offset_in_orig_struct)));
            LOG_DBG("ptrDescptr[%d], orig_ptr_memeber: %p\n", i, orig_ptr_memeber);
            LOG_DBG("ptrDescptr[%d], ion_ptr_content_va: %p\n", i, ion_ptr_content_va);

            //copy data pointed by pointer member in original inpust data struct into ION buffer
            memcpy(ion_ptr_content_va, orig_ptr_memeber, ptrDescptr->size);

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

int AbsCcuCtrlBase::send_ccu_command(ccu_msg_id msgId)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    LOG_DBG("sending ccu command :%d\n", msgId);
    enum ccu_tg_info tgInfo = getCcuTgInfo();
    if(tgInfo == CCU_CAM_TG_NONE)
    {
        return -CCU_CTRL_ERR_GENERAL;
    }

    ccu_cmd_st cmd_t = { {msgId, m_BufferList.mailboxInBuf.mva, m_BufferList.mailboxOutBuf.mva, tgInfo}, CCU_ENG_STATUS_INVALID };
    ccu_cmd_st cmd_r;

    if ( !m_pDrvCcu->sendCmd( &cmd_t, &cmd_r, true ) )
    {
        LOG_ERR("cmd(%d) fail \n",cmd_t.task.msg_id);
        return -CCU_CTRL_ERR_GENERAL;
    }

    LOG_DBG("-:%s\n",__FUNCTION__);
    return CCU_CTRL_SUCCEED;
}

enum ccu_tg_info AbsCcuCtrlBase::getCcuTgInfo()
{
    return sensorDevToTgInfo(m_sensorIdx, m_sensorDev);
}

enum ccu_tg_info AbsCcuCtrlBase::sensorDevToTgInfo(MUINT32 sensorIdx, ESensorDev_T sensorDev)
{
    LOG_DBG("+:%s\n",__FUNCTION__);
    // Query TG info
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    if(pIHalSensorList == NULL)
    {
        LOG_ERR("pIHalSensorList is NULL\n");
        return CCU_CAM_TG_NONE;
    }

    IHalSensor* pIHalSensor = pIHalSensorList->createSensor("ccu_ctrl", sensorIdx);
    if(pIHalSensor == NULL)
    {
        LOG_ERR("pIHalSensor is NULL\n");
        return CCU_CAM_TG_NONE;
    }
    SensorDynamicInfo rSensorDynamicInfo;

    switch(sensorDev) {
        case ESensorDev_Main:
            pIHalSensor->querySensorDynamicInfo(SENSOR_DEV_MAIN, &rSensorDynamicInfo);
            break;
        case ESensorDev_Sub:
            pIHalSensor->querySensorDynamicInfo(SENSOR_DEV_SUB, &rSensorDynamicInfo);
            break;
        case ESensorDev_MainSecond:
            pIHalSensor->querySensorDynamicInfo(SENSOR_DEV_MAIN_2, &rSensorDynamicInfo);
            break;
        case ESensorDev_SubSecond:
            pIHalSensor->querySensorDynamicInfo(SENSOR_DEV_SUB_2, &rSensorDynamicInfo);
            break;
        default:    //  Shouldn't happen.
            LOG_ERR("Invalid sensor device: %d", sensorDev);
    }

    if(pIHalSensor) {
        pIHalSensor->destroyInstance("ccu_ctrl");
    }

    LOG_INF( "TG = %d\n", rSensorDynamicInfo.TgInfo);

    switch(rSensorDynamicInfo.TgInfo){
        case CAM_TG_1:
            return CCU_CAM_TG_1;
        case CAM_TG_2:
            return CCU_CAM_TG_2;
        default:
            LOG_ERR("undefined rSensorDynamicInfo.TgInfo = %d", rSensorDynamicInfo.TgInfo);
            return CCU_CAM_TG_NONE;
    }

    LOG_DBG("-:%s\n",__FUNCTION__);
}

};  //namespace NSCcuIf

