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
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CcuDrvIpc"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string.h>
#include <ccu_udrv_stddef.h>
#include <ccu_udrv_ipc.h>
#include <ccu_drvutil.h>
#include <ccu_platform_def.h>

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG
#include "ccu_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
EXTERN_DBG_LOG_VARIABLE(ccu_drv);

using namespace NSCcuIf;

CcuDrvIpc::CcuDrvIpc(void):
m_initialized(MFALSE),
m_ccuIntTriggerPtr(NULL),
m_ccuIpcPtr(NULL),
m_ipcInDataPtr(NULL),
m_ipcOutDataPtr(NULL),
m_ipcInDataAddrCcu(0),
m_ipcOutDataAddrCcu(0)
{};

MBOOL CcuDrvIpc::init(struct shared_buf_map *sb_map_ptr, void *ccuDmBase, void *ccuCtrlBase)
{
    LOG_DBG("+");
    
    m_ccuIntTriggerPtr = (MUINT32 *)ccuCtrlBase + OFFSET_CCU_INT_TRG;
    m_ccuIpcPtr = (struct ap2ccu_ipc_t *)(((MUINT8 *)ccuDmBase) + sb_map_ptr->ipc_base_offset);
    m_ipcInDataPtr = (void *)(((MUINT8 *)ccuDmBase) + sb_map_ptr->ipc_in_data_base_offset);
    m_ipcOutDataPtr = (void *)(((MUINT8 *)ccuDmBase) + sb_map_ptr->ipc_out_data_base_offset);
    m_ipcInDataAddrCcu = sb_map_ptr->ipc_in_data_addr_ccu;
    m_ipcOutDataAddrCcu = sb_map_ptr->ipc_out_data_addr_ccu;
    m_initialized = MTRUE;

    LOG_DBG("ccuDmBase: %p", ccuDmBase);
    LOG_DBG("ccuCtrlBase: %p", ccuCtrlBase);
    LOG_DBG("m_ccuIntTriggerPtr: %p", m_ccuIntTriggerPtr);
    LOG_DBG("m_ccuIpcPtr: %p", m_ccuIpcPtr);
    LOG_DBG("m_ipcInDataPtr: %p", m_ipcInDataPtr);
    LOG_DBG("m_ipcOutDataPtr: %p", m_ipcOutDataPtr);
    
    LOG_DBG("-");

    return MTRUE;
}

MBOOL CcuDrvIpc::getIpcIOBufAddr(void **ipcInDataPtr, void **ipcOutDataPtr, MUINT32 *ipcInDataAddrCcu, MUINT32 *ipcOutDataAddrCcu)
{
    LOG_DBG("+");

    if(m_initialized == MFALSE)
    {
        LOG_ERR("not initialized, invalid operation.");
        return MFALSE;
    }

    *ipcInDataPtr = m_ipcInDataPtr;
    *ipcOutDataPtr = m_ipcOutDataPtr;
    *ipcInDataAddrCcu = m_ipcInDataAddrCcu;
    *ipcOutDataAddrCcu = m_ipcOutDataAddrCcu;
    LOG_DBG("*ipcInDataPtr: %p", *ipcInDataPtr);
    LOG_DBG("*ipcOutDataPtr: %p", *ipcOutDataPtr);
    LOG_DBG("*ipcInDataAddrCcu: %p", *ipcInDataAddrCcu);
    LOG_DBG("*ipcOutDataAddrCcu: %p", *ipcOutDataAddrCcu);

    LOG_DBG("-");

    return MTRUE;
}

MBOOL CcuDrvIpc::sendCmd(struct ccu_msg *msg)
{
    LOG_DBG("+, msg_id(%d)", msg->msg_id);

    if(m_initialized == MFALSE)
    {
        LOG_ERR("not initialized, invalid operation.");
        return MFALSE;
    }

    //since ipc is synchronized, check no previous ipc
    if(m_ccuIpcPtr->read_cnt != m_ccuIpcPtr->write_cnt)
    {
        LOG_ERR("CCU IPC synchronization violation, rcnt:%d, wcnt:%d", m_ccuIpcPtr->read_cnt, m_ccuIpcPtr->write_cnt);
        return MFALSE;
    }

    m_ccuIpcPtr->ack = MFALSE;
    m_ccuIpcPtr->write_cnt = m_ccuIpcPtr->write_cnt + 1;
    m_ccuIpcPtr->msg.msg_id = msg->msg_id;
    m_ccuIpcPtr->msg.in_data_ptr = msg->in_data_ptr;
    m_ccuIpcPtr->msg.out_data_ptr = msg->out_data_ptr;
    m_ccuIpcPtr->msg.tg_info = msg->tg_info;
    *m_ccuIntTriggerPtr = 1; //trigger interrupt

    double ts_start = CcuDrvUtil::getTimeUs();
    double ts_end = ts_start;
    double duration = 0;
    uint32_t loopCount = 0;
    MBOOL ackValue = 0;
    bool timeout = false;
    
    while(m_ccuIpcPtr->ack == MFALSE)
    {
        loopCount++;
        ts_end = CcuDrvUtil::getTimeUs();
        duration = ts_end - ts_start;
        if(duration > CCU_IPC_CMD_TIMEOUT_SPEC)
        {
            ackValue = m_ccuIpcPtr->ack;
            if(ackValue == MFALSE)
            {
                timeout = true;
            }
            else
            {
                LOG_WRN("CCU cmd timeout false alarm");
            }
            break;
        }
    }

    if(timeout)
    {
        LOG_ERR("CCU cmd timeout, msg_id(%d), ackValue(%d), loopCount(%d)", msg->msg_id, ackValue, loopCount);
        return MFALSE;
    }

    LOG_DBG("-, msg_id(%d), duration(%lf), loopCount(%d)", msg->msg_id, duration, loopCount);
    return MTRUE;
}


