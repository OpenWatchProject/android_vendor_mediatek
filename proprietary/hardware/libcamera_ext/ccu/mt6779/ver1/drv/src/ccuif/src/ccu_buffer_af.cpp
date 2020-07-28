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
#define LOG_TAG "CcuBufferAf"

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

bool CcuAfBufferList::init()
{
    bool ret = true;
    m_featureType = CCU_FEATURE_AF;

    ret = CcuBufferList::init();
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

/*******************************************************************************
* Private Functions
********************************************************************************/
bool CcuAfBufferList::_initAFBuffers()
{
    bool ret = true;
    CcuBuffer *buffer;

    LOG_DBG("+_initAFBuffers");


    for(int j=0 ; j<Half_AFO_BUFFER_COUNT ; j++)
    {
        LOG_DBG("init_AFO_buffer[%d]\n", j);

        buffer = &(this->AF_Buf[j]);
        ret = buffer->init("AFOBuf", AFO_BUF_SIZE, CCU_BUFTYPE_CPUREF);
        if(ret == false)
        {
            LOG_ERR("init_AFO_buffer[%d] fail\n", j);
            return ret;
        }
    }

    for(int j=0 ; j<CCU_AF_INSTANCE_CAPACITY ; j++)
    {
        LOG_DBG("init_AF_Hwinit_Output[%d]\n", j);

        buffer = &(this->AF_Hwinit_Output[j]);
        ret = buffer->init("AF_Hwinit_OutputBuf", sizeof(AF_HW_INIT_OUTPUT_T), CCU_BUFTYPE_CPUREF);
        if(ret == false)
        {
            LOG_ERR("init_AF_Hwinit_Output[%d] fail\n", j);
            return ret;
        }
    }

    LOG_DBG("-_initAFBuffers");

    return ret;
}


};  //namespace NSCcuIf

