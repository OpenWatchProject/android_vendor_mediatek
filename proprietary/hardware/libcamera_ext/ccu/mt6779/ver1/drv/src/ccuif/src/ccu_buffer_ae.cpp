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
#define LOG_TAG "CcuBufferAe"

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
#include "ccu_ctrl_ae.h"

/*******************************************************************************
*
********************************************************************************/
namespace NSCcuIf {

/**************************************************************************
* Globals
**************************************************************************/
#include "ccu_log.h"

EXTERN_DBG_LOG_VARIABLE(ccu_drv);

bool CcuAeBufferList::init()
{
    bool ret = true;
    m_featureType = CCU_FEATURE_AE;

    ret = CcuBufferList::init();
    if(ret == false)
        goto INIT_EXIT;

    ret = _initAeBuffers();
    if(ret == false)
    {
        isValid = false;
        goto INIT_EXIT;
    }

    isValid = true;

INIT_EXIT:
    return ret;
}

/*******************************************************************************
* Private Functions
********************************************************************************/
bool CcuAeBufferList::_initAeBuffers()
{
    bool ret = true;
    CcuBuffer *buffer;

    LOG_DBG("+");

    for(int i=0 ; i<CCU_HDR_DATA_BUF_CNT ; i++)
    {
        LOG_DBG("init ccu HDR data buffer[%d]\n", i);

        buffer = &(this->hdrDataBuffer[i]);
        ret = buffer->init("HDR_DATA", HDR_SENSOR_DATA_SIZE, CCU_BUFTYPE_CPUREF);
        if(ret == false)
        {
            LOG_ERR("init ccu HDR data buffer[%d] fail\n", i);
            return ret;
        }
    }

    for(int i=0 ; i<AE_ACTOUT_RING_SIZE ; i++)
    {
        LOG_DBG("init ActoutAeAlgData buffer[%d]\n", i);
        ret = actoutAeAlgoData[i].init("ActoutAeAlgData", sizeof(CCU_AeAlgo), CCU_BUFTYPE_DDRBIN);
        if(ret == false)
        {
            LOG_ERR("init CCU ActoutAeAlgData buffer[%d] fail\n", i);
            return ret;
        }

        LOG_DBG("init ActoutAeOutput buffer[%d]\n", i);
        ret = actoutAeOutput[i].init("ActoutAeOutput", sizeof(struct ccu_ae_output), CCU_BUFTYPE_DDRBIN);
        if(ret == false)
        {
            LOG_ERR("init CCU ActoutAeOutput buffer[%d] fail\n", i);
            return ret;
        }

        LOG_DBG("init ActoutAeStat buffer[%d]\n", i);
        ret = actoutAeStat[i].init("ActoutAeStat", sizeof(CCU_AE_STAT), CCU_BUFTYPE_DDRBIN);
        if(ret == false)
        {
            LOG_ERR("init CCU ActoutAeStat buffer[%d] fail\n", i);
            return ret;
        }
    }



    LOG_DBG("-");

    return ret;
}


};  //namespace NSCcuIf

