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
//
#define LOG_TAG "Drv/HWsync"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <cutils/atomic.h>
#include <semaphore.h>
#include <cutils/properties.h>  // For property_get().
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include "hwsync_drv_imp.h"
#include "hwsync_drv_para.h"
#include <cutils/log.h>
#include <system/thread_defs.h>
#include <utils/ThreadDefs.h>
#include <utils/threads.h>
#include <vector>
//for access N3D register in SENINF
#include "seninf_drv.h"
#include "seninf_reg.h"


/*************************************************************************************
* Log Utility
*************************************************************************************/
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG

#undef  __func__
#define __func__    __FUNCTION__


#ifndef USING_MTK_LDVT
#define LOG_VRB(fmt, arg...)       CAM_ULOGMV(DBG_LOG_TAG "[%s] " fmt , __func__, ##arg)
#define LOG_DBG(fmt, arg...)       CAM_ULOGMD(DBG_LOG_TAG "[%s] " fmt , __func__, ##arg)
#define LOG_INF(fmt, arg...)       CAM_ULOGMI(DBG_LOG_TAG "[%s] " fmt , __func__, ##arg)
#define LOG_WRN(fmt, arg...)       CAM_ULOGMW(DBG_LOG_TAG "[%s] WARNING: " fmt , __func__, ##arg)
#define LOG_ERR(fmt, arg...)       CAM_ULOGME(DBG_LOG_TAG "[%s, %s, line%04d] ERROR: " fmt , __FILE__, __func__, __LINE__, ##arg)

#else   // LDVT
#include "uvvf.h"

#define LOG_MSG(fmt, arg...)    VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define LOG_DBG(fmt, arg...)    VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define LOG_INF(fmt, arg...)    VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    VV_MSG("[%s]Warning(%5d): " fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    VV_MSG("[%s]Err(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)

#endif

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_SENSOR);

pthread_mutex_t mStateMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mEnterIdleMutex = PTHREAD_MUTEX_INITIALIZER;

static HWSyncDrvImp singleton;


MINT32 getUs()
{
   struct timeval tv;
   ::gettimeofday(&tv, NULL);
   return tv.tv_sec * 1000000 + tv.tv_usec;
}


/*******************************************************************************
*
********************************************************************************/
HWSyncDrv* HWSyncDrv::createInstance()
{
    return HWSyncDrvImp::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
HWSyncDrvImp::HWSyncDrvImp()
{

}

/*******************************************************************************
*
********************************************************************************/
HWSyncDrvImp::~HWSyncDrvImp()
{
}

/*******************************************************************************
*
********************************************************************************/
HWSyncDrv* HWSyncDrvImp::getInstance(void)
{
    LOG_DBG("singleton[0x%p],getpid[0x%08x],gettid[0x%08x] ",&singleton,getpid() ,gettid());
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void HWSyncDrvImp::destroyInstance(void)
{
}

/*******************************************************************************
*
********************************************************************************/
MINT32 HWSyncDrvImp::init(HW_SYNC_USER_ENUM user,MUINT32 sensorIdx,MINT32 aeCyclePeriod)
{
    user;
    sensorIdx;
    aeCyclePeriod;
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 HWSyncDrvImp::uninit(HW_SYNC_USER_ENUM user,MUINT32 sensorIdx)
{
    user;
    sensorIdx;
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
#define str(x)  ({\
   static char _str[48];\
   _str[0] = '\0';\
   switch(x){\
       case HW_SYNC_CMD_SET_MODE:\
           sprintf(_str,"HW_SYNC_CMD_SET_MODE");\
           break;\
       case HW_SYNC_CMD_DISABLE_SYNC:\
           sprintf(_str,"HW_SYNC_CMD_DISABLE_SYNC");\
           break;\
       case HW_SYNC_CMD_ENABLE_SYNC:\
           sprintf(_str,"HW_SYNC_CMD_ENABLE_SYNC");\
           break;\
       case HW_SYNC_CMD_GET_FRAME_TIME:\
           sprintf(_str,"HW_SYNC_CMD_GET_FRAME_TIME");\
           break;\
       case HW_SYNC_CMD_SET_PARA:\
           sprintf(_str,"HW_SYNC_CMD_SET_PARA");\
           break;\
       case HW_SYNC_CMD_SYNC_TYPE:\
           sprintf(_str,"HW_SYNC_CMD_SYNC_TYPE");\
           break;\
   }\
   _str;\
})

MINT32 HWSyncDrvImp::sendCommand(HW_SYNC_CMD_ENUM cmd,MUINT32 arg1,MUINTPTR arg2,MUINTPTR arg3,MUINTPTR arg4)
{
    LOG_DBG("+ cmd(%s)",str(cmd));
    arg1;
    arg3;
    arg4;
    MINT32 ret=0;
    switch(cmd)
    {
        case HW_SYNC_CMD_SYNC_TYPE:
            *((MUINTPTR*)arg2) = (MUINT32)SYNC_TYPE;
            break;
        default:
            LOG_INF("cmd %s is not supported",str(cmd));
            break;
    }

    LOG_DBG("-cmd(%s)",str(cmd));
    return ret;
}

