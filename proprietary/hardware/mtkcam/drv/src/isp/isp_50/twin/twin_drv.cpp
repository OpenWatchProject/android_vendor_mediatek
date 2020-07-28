/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
#define LOG_TAG "TwinDrv"
//
#include <utils/Errors.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include <utils/threads.h>  // For Mutex::Autolock.
#include <cutils/properties.h>  // For property_get().
#include <cutils/atomic.h>
#include <sys/ioctl.h>

#include "MyUtils.h"
#include "isp_drv.h"
#include "twin_drv.h"
#include <imem_drv.h>
#include <mtkcam/def/BuiltinTypes.h>    // For type definitions.
#include "twin_drv_imp.h"

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        "{TwinDrv} "
#include "drv_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(twin_drv);
//EXTERN_DBG_LOG_VARIABLE(twin_drv);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (twin_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (twin_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (twin_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (twin_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (twin_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (twin_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define TWIN_DRV_INIT_MAX        5


/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/



/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/
//-----------------------------------------------------------------------------
TwinDrvImp::TwinDrvImp()
{
    LOG_DBG("TwinDrvImp()");
    //
    mInitCount = 0;
    mMode = TWIN_SCENARIO_NORMAL;
    memset(&m_dual_isp_in_config, 0, sizeof(m_dual_isp_in_config));
    memset(&m_dual_isp_out_config, 0, sizeof(m_dual_isp_out_config));
    memset(&m_isp_reg_ptr, 0, sizeof(m_isp_reg_ptr));
}


//-----------------------------------------------------------------------------
TwinDrvImp::~TwinDrvImp()
{
    LOG_DBG("~TwinDrvImp()");
}


//-----------------------------------------------------------------------------
TwinDrv*
TwinDrv::createInstance(ISP_HW_MODULE hwModule)
{
    DBG_LOG_CONFIG(drv, twin_drv);

    return TwinDrvImp::getInstance(hwModule);
}

//-----------------------------------------------------------------------------
TwinDrv*
TwinDrvImp::getInstance(ISP_HW_MODULE hwModule)
{
    static TwinDrvImp Singleton[PHY_CAM];

    switch (hwModule) {
    case CAM_A:
    case CAM_B:
        break;
    default:
        LOG_ERR("Unsupport twin_drv on cam: %d\n", hwModule);
        return NULL;
    }
    LOG_DBG("&Singleton(0x%p)\n", &(Singleton[hwModule]));

    return &(Singleton[hwModule]);
}

//-----------------------------------------------------------------------------
void
TwinDrvImp::destroyInstance(void)
{
    LOG_DBG("");
}

//-----------------------------------------------------------------------------
MBOOL
TwinDrvImp::init(E_TWIN_MODE mode)
{
    MBOOL Result = MTRUE;
    MINT32 tmp=0;
    //
    Mutex::Autolock lock(mLock); // Automatic mutex. Declare one of these at the top of a function. It'll be locked when Autolock mutex is constructed and released when Autolock mutex goes out of scope.
    //
    LOG_DBG("mInitCount(%d)",mInitCount);
    //

    //
    if(mInitCount >= TWIN_DRV_INIT_MAX) {
        LOG_ERR("over max mInitCount(%d)",mInitCount);
        Result = MFALSE;
        goto EXIT;
    }
    //
    switch(mode){
        case twin_normal_mode:
            this->mMode = TWIN_SCENARIO_NORMAL;
            break;
        case twin_non_af_mode:
            this->mMode = TWIN_SCENARIO_AF_FAST_P1;
            break;
        default:
            Result = MFALSE;
            goto EXIT;
            break;
    }

    //
    tmp = android_atomic_inc(&mInitCount);

    //
    EXIT:
    LOG_DBG("X. mInitCount(%d)",mInitCount);
    //
    return Result;
}



//-----------------------------------------------------------------------------
MBOOL
TwinDrvImp::uninit(void)
{
    MBOOL Result = MTRUE;
    MINT32 tmp=0;
    //
    Mutex::Autolock lock(mLock);
    //
    LOG_DBG("mInitCount(%d)",mInitCount);
    //
    tmp = android_atomic_dec(&mInitCount);
    //
    if(mInitCount > 0) {
        goto EXIT;
    }
    //

    //
    EXIT:
    LOG_DBG("X. mInitCount(%d)",mInitCount);
    return Result;
}



MBOOL TwinDrvImp::
configTwinPara(void)
{
    MBOOL ret = MTRUE;

    Mutex::Autolock lock(mLock); // acquires a lock on m_mutex

    LOG_DBG("Start to Config Twin Mode");

    //for af mode setting
    m_dual_isp_in_config.SW.TWIN_SCENARIO = this->mMode;

    if ( 0 != dual_cal_platform(&m_isp_reg_ptr, &m_dual_isp_in_config, &m_dual_isp_out_config) ) {
        LOG_ERR("twin cal error!");
        ret = MFALSE;
    }


EXIT:
    return ret;

}


//-----------------------------------------------------------------------------

