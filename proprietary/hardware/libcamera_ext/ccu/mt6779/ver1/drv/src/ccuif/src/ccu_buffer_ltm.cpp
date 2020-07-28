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
#define LOG_TAG "CcuBufferLtm"

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
#include "ccu_ctrl_ltm.h"
#include "ccu_ext_interface/ccu_ltm_extif.h"
#include "algorithm/ccu_ltm_data.h"
#include "algorithm/ccu_hlr_data.h"

/*******************************************************************************
*
********************************************************************************/
namespace NSCcuIf {

/**************************************************************************
* Globals
**************************************************************************/
#include "ccu_log.h"

EXTERN_DBG_LOG_VARIABLE(ccu_drv);

bool CcuLtmBufferList::init()
{
    bool ret = true;
    m_featureType = CCU_FEATURE_LTM;

    ret = CcuBufferList::init();
    if(ret == false)
        goto INIT_EXIT;

    ret = _initLtmBuffers();
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
bool CcuLtmBufferList::_initLtmBuffers()
{
    bool ret = true;

    LOG_DBG("_initLtmBuffers+");
    LOG_DBG("ltm(hlr)_info_exif/isp");
    for(int i = 0 ; i < CCU_LTM_DBG_RING_SIZE ; i++)
    {
        //ccu_ltm_algo.h : ccu_ltm_algo_data
        ret = ltm_info_exif[i].init("LTM_INFO_EXIF", sizeof(struct ccu_ltm_algo_data), CCU_BUFTYPE_CPUREF);
        if(ret == false)
        {
            LOG_ERR("init ccu ltm_info_exif buffer[%d] fail\n", i);
            return ret;
        }
        //TO-DO: Define EXIF data structure
        ret = ltm_info_isp[i].init("LTM_INFO_ISP", sizeof(struct ccu_ltm_info_isp), CCU_BUFTYPE_CPUREF);
        if(ret == false)
        {
            LOG_ERR("init ccu ltm_info_isp buffer[%d] fail\n", i);
            return ret;
        }
        //ccu_ltm_algo.h : ccu_ltm_algo_data
        ret = hlr_info_exif[i].init("HLR_INFO_EXIF", sizeof(struct ccu_hlr_algo_data), CCU_BUFTYPE_CPUREF);
        if(ret == false)
        {
            LOG_ERR("init ccu hlr_info_exif buffer[%d] fail\n", i);
            return ret;
        }
        //TO-DO: Define EXIF data structure
        ret = hlr_info_isp[i].init("HLR_INFO_ISP", sizeof(struct ccu_hlr_info_isp), CCU_BUFTYPE_CPUREF);
        if(ret == false)
        {
            LOG_ERR("init ccu hlr_info_isp buffer[%d] fail\n", i);
            return ret;
        }
    }
    LOG_DBG("ltmso_ring_buf");
    for(int i = 0 ; i < CCU_LTMSO_RING_SIZE ; i++)
    {
        ret = ltmso_ring_buf[i].init("LTMSO_RBUF", CCU_LTMSO_MAX_BUF_SIZE, CCU_BUFTYPE_CPUREF);
        if(ret == false)
        {
            LOG_ERR("init ccu ltmso_ring_buf buffer fail\n");
            return ret;
        }
    }

    // LOG_DBG("ltm_nvram");
    // ret = ltm_nvram.init("LTM_NVRAM", CCU_LTM_NVRAM_SIZE * CCU_LTM_NVRAM_NUM_MAX, CCU_BUFTYPE_CPUREF);
    // if(ret == false)
    // {
    //     LOG_ERR("init ccu ltm_nvram buffer fail\n");
    //     return ret;
    // }

    // LOG_DBG("hlr_nvram");
    // ret = hlr_nvram.init("HLR_NVRAM", CCU_HLR_NVRAM_SIZE * CCU_HLR_NVRAM_NUM_MAX, CCU_BUFTYPE_CPUREF);
    // if(ret == false)
    // {
    //     LOG_ERR("init ccu hlr_nvram buffer fail\n");
    //     return ret;
    // }


    LOG_DBG("_initLtmBuffers-");

    return ret;
}


};  //namespace NSCcuIf

