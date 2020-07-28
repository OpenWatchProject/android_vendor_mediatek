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
#include "ccu_ctrl.h"
#include "ccu_ctrl_af.h"

/*******************************************************************************
*
********************************************************************************/
namespace NSCcuIf {

/**************************************************************************
* Globals
**************************************************************************/
#include "ccu_log.h"

EXTERN_DBG_LOG_VARIABLE(ccu_drv);

bool CcuAfBufferList::init(int ionDevFd)
{
    bool ret = true;

    ret = CcuBufferList::init(ionDevFd);
    if(ret == false)
        goto INIT_EXIT;

    isValid = false;

    ret = _initAFBuffers();
    if(ret == false)
        goto INIT_EXIT;

    isValid = true;

INIT_EXIT:
    return ret;
}

bool CcuAfBufferList::uninit()
{
    bool ret = true;

    ret = _uninitAFBuffers();
    
    if(ret == false)
        goto INIT_EXIT;
    
    ret = CcuBufferList::uninit();

    if(ret == false)
        goto INIT_EXIT;

INIT_EXIT:
    return ret;
}

/*******************************************************************************
* Private Functions
********************************************************************************/
bool CcuAfBufferList::_initAFBuffers()
{
    bool ret = true;
    CcuBuffer *buffer;

    LOG_DBG("+_initAFBuffers");

    for(int i=0 ; i<2 ; i++)
    for(int j=0 ; j<3 ; j++)
    {
        LOG_DBG("init_AFO_buffer[%d]\n", i);

        buffer = &(this->AF_Buf[i][j]);
        buffer->size = AFO_BUF_SIZE;
        ret = buffer->init(m_IonDevFD, 1);
        if(ret == false)
        {
            LOG_ERR("init_AFO_buffer[%d][%d] fail\n", i,j);
            return ret;
        }
    }
    /*
    for(int i=0 ; i<6 ; i++)
    {
        LOG_DBG("init_AF_reg[%d]\n", i);

        buffer = &(this->AF_Reg[i]);
        buffer->size = sizeof(CAM_REG_AF_FMT);
        ret = this->_initBuffer(buffer,0);
        if(ret == false)
        {
            LOG_ERR("init_AF_reg[%d] fail\n", i);
            return ret;
        }
    }
    */
    LOG_DBG("-_initAFBuffers");

    return ret;
}

bool CcuAfBufferList::_uninitAFBuffers()
{
    bool ret = true;
    CcuBuffer *buffer;

    LOG_DBG("+");

    for(int i=0 ; i<2 ; i++)
    for(int j=0 ; j<3 ; j++)
    {
        buffer = &(this->AF_Buf[i][j]);

        LOG_DBG("uninit_AFO_buffers[%d][%d]\n", i,j);

        ret = buffer->uninit();
        if(ret == MFALSE)
        {
            LOG_ERR("uninit_AFO_buffers[%d][%d] fail\n", i,j);
            break;
        }
    }
/*
    for(int i=0 ; i<6 ; i++)
    {
        buffer = &(this->AF_Reg[i]);

        LOG_DBG("uninit_AF_regs[%d]\n", i);

        ret = this->_uninitBuffer(buffer);
        if(ret == MFALSE)
        {
            LOG_ERR("uninit_AF_regs[%d] fail\n", i);
            break;
        }
    }
*/
    LOG_DBG("-");

    return ret;
}

};  //namespace NSCcuIf

