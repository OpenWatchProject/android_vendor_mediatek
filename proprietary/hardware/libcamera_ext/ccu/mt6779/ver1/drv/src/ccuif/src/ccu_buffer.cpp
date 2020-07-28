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
#define LOG_TAG "CcuBuffer"

#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>

#include <cutils/properties.h>  // For property_get().
#include "ccu_buffer.h"
#include "ccu_mem_mgr.h"
#include "ccu_cmd_param_info.h"


namespace NSCcuIf {

/**************************************************************************
* Globals
**************************************************************************/
#include "ccu_log.h"

EXTERN_DBG_LOG_VARIABLE(ccu_drv);

CcuBufferList::
CcuBufferList():
    isValid(false),
    m_featureType(CCU_FEATURE_UNDEF)
    {
        mailboxInBuf.reset();
        mailboxOutBuf.reset();

        for(int i=0 ; i<CCU_MSG_CNT_MAX ; i++)
        {
            CtrlMsgBufs[i].reset();
        }
        LOG_DBG("CcuBufferList reset all buffers in constructor");
    }

bool CcuBufferList::init()
{
    //Set mailbox in/out buffer size, and initialize them
    LOG_DBG("init mailboxInBuf");
    if(this->mailboxInBuf.init("mailboxIn", MAILBOX_IN_BUF_SIZE, CCU_BUFTYPE_DDRBIN) == false)
        return false;
    LOG_DBG("init mailboxOutBuf");
    if(this->mailboxOutBuf.init("mailboxOut", MAILBOX_OUT_BUF_SIZE, CCU_BUFTYPE_DDRBIN) == false)
        return false;
    LOG_DBG("init initControlBuffers");
    //Initialize control command buffers, according to CCU_CTRL_PARAM_DESCRIPTOR_LIST
    if(_initControlBuffers() == false)
        return false;

    isValid = true;

    return true;
}

/*******************************************************************************
* Private Functions
********************************************************************************/
bool CcuBufferList::_initControlBuffers()
{
    bool ret = true;
    struct CcuBuffer *buffer;

    struct CcuCtrlCmdSetInfo * cmdSetInfo = CcuCmdParamInfo::getCmdSetInfo(m_featureType);

    if(cmdSetInfo == NULL)
    {
        LOG_ERR("cmdSetInfo is NULL, ftype(%d)", m_featureType);
        return false;
    }

    LOG_DBG("+");

    for(int i = cmdSetInfo->msgIdMin ; i < cmdSetInfo->msgIdMax ; i++)
    {
        //Get control param descriptor from pre-defined list
        const struct CcuCtrlParamDescriptor *paramDescptr = CcuCmdParamInfo::getParamDescriptor(m_featureType, i);

        //map buffer from CtrlMsgBufs and clear buffer info
        buffer = &(this->CtrlMsgBufs[i]);
        memset(buffer, 0, sizeof(struct CcuBuffer));

        //check if command available
        if(paramDescptr == NULL) continue;
        //check if command needs input ptr buffer copying
        if(paramDescptr->intput_ptr_descs == NULL) continue;

        LOG_DBG("init_control_buffers[%d]: %s\n", i, paramDescptr->msg_literal);

        ret = buffer->init(paramDescptr->msg_literal, paramDescptr->buffer_size, CCU_BUFTYPE_DDRBIN, paramDescptr->ring_count);

        if(ret == false)
        {
            LOG_ERR("init_control_buffers[%d] fail\n", i);
            break;
        }
    }

    LOG_DBG("-");

    return ret;
}

/*******************************************************************************
* Imp. of CcuBuffer function
********************************************************************************/
void CcuBuffer::reset()
{
    for(int i=0 ; i < CCU_BUFFER_RING_CAPACITY ; i++)
    {
        this->va[i] = NULL;
        this->mva[i] = 0;
    }
    this->size = 0;
    this->name = NULL;
    this->ringIdx = 0;
    this->ringCnt = 1;
    bufType = CCU_BUFTYPE_NONE;
    cached = false;
}

bool CcuBuffer::init(const char *name, int size, enum CCU_BUFFER_TYPE bufType, uint32_t ringCnt)
{
    bool ret = true;
    LOG_VRB("+");

    CcuMemMgr *memMgr = CcuMemMgr::getInstance();

    this->size = size;
    this->bufType = bufType;
    this->name = name;
    this->ringCnt = ringCnt;
    
    ret = memMgr->allocateCcuBuffer(this);
    if(!ret) goto CCU_BUF_INIT_EXIT;

    LOG_DBG("buffer allocation success, ringCnt(%d)", this->ringCnt);
    for(int i=0 ; i < CCU_BUFFER_RING_CAPACITY ; i++)
    {
        if(i < ringCnt)
        {
            LOG_DBG("ringIdx(%d), size(%d), va(%p), mva(%x)\n", i, this->size, this->va[i], this->mva[i]);
        }
        else
        {
            this->va[i] = NULL;
            this->mva[i] = NULL;
        }
        
    }

    LOG_VRB("-");

CCU_BUF_INIT_EXIT:
    return true;
}

bool CcuBuffer::ring()
{
    LOG_VRB("+");

    uint32_t ringIdxOrigin;

    if(this->ringCnt == 1)
    {
        LOG_DBG("ringCnt is 1, not need to ring");
        return true;
    }
    
    ringIdxOrigin = this->ringIdx;
    this->ringIdx = this->ringIdx + 1;
    
    if(this->ringIdx >= this->ringCnt)
        this->ringIdx = 0;

    LOG_DBG("ring buffer(%s): idx(%d)->(%d), va(%p), mva(%x)\n", 
        this->name, ringIdxOrigin, this->ringIdx, this->va[ringIdx], this->mva[ringIdx]);

    LOG_VRB("-");

CCU_BUF_INIT_EXIT:
    return true;
}

char * CcuBuffer::getVa()
{
    return this->va[this->ringIdx];
}

unsigned int CcuBuffer::getMva()
{
    return this->mva[this->ringIdx];
}

};  //namespace NSCcuIf

