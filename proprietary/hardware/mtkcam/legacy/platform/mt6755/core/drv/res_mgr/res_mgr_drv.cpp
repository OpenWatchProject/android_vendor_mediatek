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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "ResMgrDrv"
//-----------------------------------------------------------------------------
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <cutils/atomic.h>
#include <sys/ioctl.h>
#include <utils/threads.h>
#include <cutils/properties.h>
//
#include <list>
#include <queue>
using namespace std;
//
#include "bandwidth_control.h"
#include <mtkcam/common.h>
#include <mtkcam/utils/sys/CpuCtrl.h>
#define MTK_LOG_ENABLE 1
//
#include "res_mgr_drv_imp.h"
//-----------------------------------------------------------------------------
static Mutex               resMgrDrvImplock;
static list<ResMgrDrvImp*> vpResMgrDrvImpSingleton;
//-----------------------------------------------------------------------------
ResMgrDrvImp::
ResMgrDrvImp(MUINT32 sensorIdx)
{
    //MY_LOGD("");
    mUser = 0;
    mSensorIdx = sensorIdx;
    mBwcParams.step = MMDVFS_STEP_LOW;
    mBwcParams.profileType = BWCPT_NONE;
    mBwcParams.sensorSize = 0;
    mBwcParams.sensorFps = 30;
    mBwcParams.bPip = MFALSE;
    mBwcParams.bVfb = MFALSE;
    mBwcParams.bEis20 = MFALSE;
    mBwcParams.bIvhdr = MFALSE;
    mBwcParams.bStereo = MFALSE;
    mBwcParams.bThirdParty = MFALSE;
    //
    mpCpuCtrl = CpuCtrl::createInstance();
    if (mpCpuCtrl){
        mpCpuCtrl->init();
        mbCpuCtrl = MFALSE;
    }
    //
    mEngMode = -1;
    if (mEngMode == -1)
    {
        char value[PROPERTY_VALUE_MAX];
        property_get("ro.build.type", value, "eng");
        if (0 == strcmp(value, "eng")) {
            mEngMode = 1;
        }
        else
        {
            mEngMode = 0;
        }
    }
}
//----------------------------------------------------------------------------
ResMgrDrvImp::
~ResMgrDrvImp()
{
    //MY_LOGD("");
    if(mpCpuCtrl)
    {
        mpCpuCtrl->disable();
        mpCpuCtrl->uninit();
        mpCpuCtrl->destroyInstance();
        mpCpuCtrl = NULL;
    }
}
//-----------------------------------------------------------------------------
ResMgrDrv*
ResMgrDrv::
createInstance(MUINT32 sensorIdx)
{
    return ResMgrDrvImp::getInstance(sensorIdx);
}
//-----------------------------------------------------------------------------
ResMgrDrv*
ResMgrDrvImp::
getInstance(MUINT32 sensorIdx)
{
    Mutex::Autolock _l(resMgrDrvImplock);
    //
    ResMgrDrvImp* pResMgrDrvImp = NULL;
    list<ResMgrDrvImp*>::const_iterator iter;
    for(iter = vpResMgrDrvImpSingleton.begin(); iter != vpResMgrDrvImpSingleton.end(); iter++)
    {
        if((*iter)->getSensorIdx() == sensorIdx)
            pResMgrDrvImp = (*iter);
    }
    //
    if(pResMgrDrvImp == NULL)
    {
        //create new
        pResMgrDrvImp = new ResMgrDrvImp(sensorIdx);
        vpResMgrDrvImpSingleton.push_back(pResMgrDrvImp);
    }
    //
    pResMgrDrvImp->mUser++;
    MY_LOGD("this(%p), userCnt(%d)",
            pResMgrDrvImp,
            pResMgrDrvImp->mUser);
    //
    return pResMgrDrvImp;
}
//----------------------------------------------------------------------------
MVOID
ResMgrDrvImp::
destroyInstance()
{
    Mutex::Autolock _l(resMgrDrvImplock);
    //
    mUser--;
    MY_LOGD("this(%p), userCnt(%d)",
            this,
            mUser);
    //
    if(mUser == 0)
    {
        list<ResMgrDrvImp*>::iterator iter;
        for(iter = vpResMgrDrvImpSingleton.begin(); iter != vpResMgrDrvImpSingleton.end(); iter++)
        {
            if((*iter) == this)
            {
                vpResMgrDrvImpSingleton.erase(iter);
                break;
            }
        }
        delete this;
    }
}
//----------------------------------------------------------------------------
MBOOL
ResMgrDrvImp::
init()
{
    Mutex::Autolock lock(mLock);
    //

    //
    //

    return MTRUE;
}
//----------------------------------------------------------------------------
MBOOL
ResMgrDrvImp::
uninit()
{
    Mutex::Autolock lock(mLock);
    //
    //

    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
ResMgrDrvImp::
getMode(MODE_STRUCT* pMode)
{
    Mutex::Autolock lock(mLock);
    //
    MBOOL Result = MTRUE;
    //
    MY_LOGD("scenSw(%d),dev(%d)",
            mMode.scenSw,
            mMode.dev);
    //
    if(pMode)
    {
        pMode->scenSw = mMode.scenSw;
        pMode->dev = mMode.dev;
    }
    //

    //MY_LOGD("Result(%d)",Result);
    return Result;

}
//-----------------------------------------------------------------------------
MBOOL
ResMgrDrvImp::
setMode(MODE_STRUCT* pMode)
{
    Mutex::Autolock lock(mLock);
    //
    MBOOL Result = MTRUE;
    //
    MY_LOGD("scenSw(%d),dev(%d)",
            pMode->scenSw,
            pMode->dev);
    //
    if( mMode.scenSw == (pMode->scenSw) &&
        mMode.dev == (pMode->dev))
    {
        MY_LOGD("OK:Same(%d/%d,%d/%d)",
                mMode.scenSw,
                mMode.dev,
                pMode->scenSw,
                pMode->dev);
        goto EXIT;
    }
    //
    if(Result)
    {
        BWC BwcIns;
        //
        if(pMode->dev == DEV_VT)
        {
            if( pMode->scenSw == SCEN_SW_NONE ||
                pMode->scenSw == SCEN_SW_CAM_IDLE)
            {
                BwcIns.Profile_Change(BWCPT_VIDEO_TELEPHONY,false);
            }
            else
            {
                BwcIns.Profile_Change(BWCPT_VIDEO_TELEPHONY,true);
                mBwcParams.profileType = BWCPT_VIDEO_TELEPHONY;
            }
        }
        else
        {
            switch(mMode.scenSw)
            {
                case SCEN_SW_CAM_PRV:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_PREVIEW,false);
                    break;
                }
                case SCEN_SW_VIDEO_PRV:
                case SCEN_SW_VIDEO_REC:
                {
                    BwcIns.Profile_Change(BWCPT_VIDEO_RECORD_CAMERA,false);
                    break;
                }
                case SCEN_SW_VIDEO_REC_HS:
                {
                    BwcIns.Profile_Change(BWCPT_VIDEO_RECORD_SLOWMOTION,false);
                    break;
                }
                case SCEN_SW_VIDEO_VSS:
                {
                    BwcIns.Profile_Change(BWCPT_VIDEO_SNAPSHOT,false);
                    break;
                }
                case SCEN_SW_CAM_CAP:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_CAPTURE,false);
                    break;
                }
                case SCEN_SW_CAM_CSHOT:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_ICFP,false);
                    break;
                }
                case SCEN_SW_ZSD:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_ZSD,false);
                    break;
                }
                default:
                {
                    //do nothing.
                }
            }
            //
            switch(pMode->scenSw)
            {
                case SCEN_SW_NONE:
                case SCEN_SW_CAM_IDLE:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_PREVIEW,false);
                    break;
                }
                case SCEN_SW_CAM_PRV:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_PREVIEW,true);
                    mBwcParams.profileType = BWCPT_CAMERA_PREVIEW;
                    break;
                }
                case SCEN_SW_VIDEO_PRV:
                case SCEN_SW_VIDEO_REC:
                {
                    BwcIns.Profile_Change(BWCPT_VIDEO_RECORD_CAMERA,true);
                    mBwcParams.profileType = BWCPT_VIDEO_RECORD_CAMERA;
                    break;
                }
                case SCEN_SW_VIDEO_REC_HS:
                {
                    BwcIns.Profile_Change(BWCPT_VIDEO_RECORD_SLOWMOTION,true);
                    mBwcParams.profileType = BWCPT_VIDEO_RECORD_SLOWMOTION;
                    break;
                }
                case SCEN_SW_VIDEO_VSS:
                {
                    BwcIns.Profile_Change(BWCPT_VIDEO_SNAPSHOT,true);
                    mBwcParams.profileType = BWCPT_VIDEO_SNAPSHOT;
                    break;
                }
                case SCEN_SW_CAM_CAP:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_CAPTURE,true);
                    mBwcParams.profileType = BWCPT_CAMERA_CAPTURE;
                    break;
                }
                case SCEN_SW_CAM_CSHOT:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_ICFP,true);
                    mBwcParams.profileType = BWCPT_CAMERA_ICFP;
                    break;
                }
                case SCEN_SW_ZSD:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_ZSD,true);
                    mBwcParams.profileType = BWCPT_CAMERA_ZSD;
                    break;
                }
                default:
                {
                    //do nothing.
                }
            }
        }
    }
    //
    mMode.scenSw = pMode->scenSw;
    mMode.dev = pMode->dev;
    //
    EXIT:
    //MY_LOGD("Result(%d)",Result);
    return Result;

}
//-----------------------------------------------------------------------------
MUINT32
ResMgrDrvImp::
getSensorIdx()
{
    return mSensorIdx;
}
//-----------------------------------------------------------------------------
MBOOL
ResMgrDrvImp::
setItem(ITEM_ENUM item, MUINT32 value)
{
    Mutex::Autolock lock(mLock);
    //
    MY_LOGD("item(%d),value(%d)",
            item,
            value);
    //
    switch(item)
    {
        case ITEM_SENSOR_SIZE:
        {
            if(value != 0)
            {
                mBwcParams.sensorSize = value;
            }
            break;
        }
        case ITEM_SENSOR_FPS:
        {
            if(value != 0)
            {
                mBwcParams.sensorFps = value;
            }
            break;
        }
        case ITEM_PIP:
        {
            mBwcParams.bPip = value;
            break;
        }
        case ITEM_VFB:
        {
            mBwcParams.bVfb = value;
            break;
        }
        case ITEM_EIS20:
        {
            mBwcParams.bEis20 = value;
            break;
        }
        case ITEM_IVHDR:
        {
            mBwcParams.bIvhdr = value;
            break;
        }
        case ITEM_STEREO:
        {
            mBwcParams.bStereo = value;
            break;
        }
        case ITEM_THIRD_PARTY:
        {
            mBwcParams.bThirdParty = value;
            break;
        }
    }
    //
    return MTRUE;
}
//-----------------------------------------------------------------------------
ResMgrDrvImp::CONTROL_ENUM
ResMgrDrvImp::
queryCtrlType()
{
    Mutex::Autolock lock(mLock);
    //
    MY_LOGD("+");
    if( mBwcParams.profileType == BWCPT_NONE ||
        mBwcParams.sensorSize == 0 ||
        mBwcParams.sensorFps == 0)
    {
        MY_LOGD("profileType(%d),sensorSize(%d),sensorFps(%d)",
                mBwcParams.profileType,
                mBwcParams.sensorSize,
                mBwcParams.sensorFps);
        return CONTROL_BEFORE;
    }
    //
    mBwcParams.step = mmdvfs_query(
                            mBwcParams.profileType,
                            MMDVFS_SENSOR_SIZE,         mBwcParams.sensorSize,
                            MMDVFS_SENSOR_FPS,          mBwcParams.sensorFps,
                            MMDVFS_CAMERA_MODE_PIP,     mBwcParams.bPip,
                            MMDVFS_CAMERA_MODE_VFB,     mBwcParams.bVfb,
                            MMDVFS_CAMERA_MODE_EIS_2_0, mBwcParams.bEis20,
                            MMDVFS_CAMERA_MODE_IVHDR,   mBwcParams.bIvhdr,
                            MMDVFS_CAMERA_MODE_STEREO, mBwcParams.bStereo,
                            MMDVFS_PARAMETER_EOF);
    //
    MY_LOGD("step(%d),profileType(%d),sensorSize(%d),sensorFps(%d),bPip(%d),bVfb(%d),bEis20(%d),bIvhdr(%d),bStereo(%d)",
            mBwcParams.step,
            mBwcParams.profileType,
            mBwcParams.sensorSize,
            mBwcParams.sensorFps,
            mBwcParams.bPip,
            mBwcParams.bVfb,
            mBwcParams.bEis20,
            mBwcParams.bIvhdr,
            mBwcParams.bStereo);
    //
    if(mBwcParams.step == MMDVFS_STEP_HIGH2LOW)
    {
        return CONTROL_AFTER;
    }
    else
    {
        return CONTROL_BEFORE;
    }
}
//-----------------------------------------------------------------------------
MBOOL
ResMgrDrvImp::
triggerCtrl()
{
    Mutex::Autolock lock(mLock);
    //
    //MY_LOGD("+");
    if( mBwcParams.profileType == BWCPT_NONE ||
        mBwcParams.sensorSize == 0 ||
        mBwcParams.sensorFps == 0)
    {
        MY_LOGD("profileType(%d),sensorSize(%d),sensorFps(%d)",
                mBwcParams.profileType,
                mBwcParams.sensorSize,
                mBwcParams.sensorFps);
        return MFALSE;
    }
    //
    MY_LOGD("profileType(%d),sensorSize(%d),sensorFps(%d),bPip(%d),bVfb(%d),bEis20(%d),bIvhdr(%d),bStereo(%d)",
            mBwcParams.profileType,
            mBwcParams.sensorSize,
            mBwcParams.sensorFps,
            mBwcParams.bPip,
            mBwcParams.bVfb,
            mBwcParams.bEis20,
            mBwcParams.bIvhdr,
            mBwcParams.bStereo);
    //
    mmdvfs_set(
        mBwcParams.profileType,
        MMDVFS_SENSOR_SIZE,         mBwcParams.sensorSize,
        MMDVFS_SENSOR_FPS,          mBwcParams.sensorFps,
        MMDVFS_CAMERA_MODE_PIP,     mBwcParams.bPip,
        MMDVFS_CAMERA_MODE_VFB,     mBwcParams.bVfb,
        MMDVFS_CAMERA_MODE_EIS_2_0, mBwcParams.bEis20,
        MMDVFS_CAMERA_MODE_IVHDR,   mBwcParams.bIvhdr,
        MMDVFS_CAMERA_MODE_STEREO, mBwcParams.bStereo,
        MMDVFS_PARAMETER_EOF);
    //
    if( (   mBwcParams.profileType == BWCPT_VIDEO_RECORD_CAMERA ||
            mBwcParams.profileType == BWCPT_VIDEO_SNAPSHOT) &&
        mBwcParams.bVfb == MFALSE &&
        mBwcParams.bEis20 == MFALSE &&
        mBwcParams.bThirdParty == MFALSE )
    {
        if( mpCpuCtrl &&
            mbCpuCtrl == MFALSE &&
            mEngMode != 1)
        {
            CpuCtrl::CPU_CTRL_RANGE_INFO cpuRangeInfo;
            //
            cpuRangeInfo.bigCoreMin = 0;
            cpuRangeInfo.littleCoreMin = 4;
            //
            cpuRangeInfo.bigCoreMax = 0;
            cpuRangeInfo.littleCoreMax = 4;
            //
            cpuRangeInfo.bigFreqMin = -1;
            cpuRangeInfo.littleFreqMin = -1;
            //
            cpuRangeInfo.bigFreqMax = -1;
            cpuRangeInfo.littleFreqMax = -1;
            //
            cpuRangeInfo.timeout = 0;
            mpCpuCtrl->enable(cpuRangeInfo);
            mbCpuCtrl = MTRUE;
        }
    }
    else
    {
        if( mpCpuCtrl &&
            mbCpuCtrl == MTRUE)
        {
            mpCpuCtrl->disable();
            mbCpuCtrl = MFALSE;
        }
    }
    MY_LOGD("-");
    return MTRUE;
}
//-----------------------------------------------------------------------------

