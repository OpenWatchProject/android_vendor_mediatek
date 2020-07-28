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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkMain.cpp

#define LOG_TAG "AcdkCctMain"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
#include <utils/Errors.h>
#include <utils/threads.h>

#include <cutils/properties.h>

extern "C" {
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <semaphore.h>
#include <linux/mtkfb.h>
}

#include "AcdkErrCode.h"
#include "AcdkLog.h"
#include "AcdkCallback.h"
#include "AcdkSurfaceView.h"
#include "AcdkUtility.h"

#include "AcdkCctBase.h"
#include "AcdkCctMain.h"
#include "AcdkCctMhalBase.h"

#include <system/camera.h>
#include <hardware/camera.h>
#include <dlfcn.h>
#include <mtkcam/utils/fwk/MtkCamera.h>
#include <CameraParameters.h>

#include <mtkcam/drv/IHalSensor.h>

#define CCT_TUNING_SUPPORT 0

#define CAPTURE_WAIT_AF_DONE    (0)

#include <private/aaa_hal_private.h>
#include "camera_custom_nvram.h"
#include <nvbuf_util.h>

using namespace android;
using namespace NSACDK;
using namespace NSAcdkCctMhal;


/******************************************************************************
* Define Value
*******************************************************************************/
#define LOG_TAG "AcdkCctMain"

#define MEDIA_PATH "/mnt/vendor/cct"

#define SWAP(a, b) {MUINT32 c = (a); (a) = (b); (b) = c; }

#define CAM_FULLSIZE_PREVIEW
//#define CAM_VIDEO_MODE_TEST

static MINT32 g_acdkMainDebug = 0;
static MINT32 g_dumpRAW = 0;
MINT32 NSACDK::AcdkCctMain::mSensorDev = 0;

/*******************************************************************************
*
********************************************************************************/
static MUINT32 get_boot_mode(void)
{
    MUINT32 fd;
    size_t s;
    char boot_mode[4] = {'0'};

    //MT_NORMAL_BOOT 0 , MT_META_BOOT 1, MT_RECOVERY_BOOT 2, MT_SW_REBOOT 3
    //MT_FACTORY_BOOT 4, MT_ADVMETA_BOOT 5

    fd = open("/sys/class/BOOT/BOOT/boot/boot_mode", O_RDONLY);
    if (fd < 0)
    {
        ACDK_LOGE("fail to open: %s\nError Code: %d\n", "/sys/class/BOOT/BOOT/boot/boot_mode", fd);
        return 0;
    }

    s = read(fd, (void *)&boot_mode, sizeof(boot_mode) - 1);
    close(fd);

    if(s <= 0)
    {
        ACDK_LOGE("could not read boot mode sys file\n");
        return 0;
    }

    boot_mode[s] = '\0';
    ACDK_LOGD("Boot Mode %d\n",atoi(boot_mode));

    return atoi(boot_mode);
}

/******************************************************************************
* Global Variable
*******************************************************************************/
//static AcdkCctMain *g_pAcdkCctMainObj = NULL;
/*******************************************************************************
*
********************************************************************************/
void AcdkCctMain::destroyInstance()
{
    delete this;
}

/*******************************************************************************
*Constructor
********************************************************************************/
AcdkCctMain::AcdkCctMain (ACDK_CCT_BYPASS_FLAG bypassFlag)
    :AcdkCctBase()
    ,m_eAcdkCctMainState(ACDK_CCT_MAIN_NONE)
    ,mOperaMode(ACDK_OPT_NONE_MODE)
    ,mFocusDone(0)
    ,mFrameCnt(0)
    ,mPrvWidth(320)
    ,mPrvHeight(240)
    ,mPrvStartX(0)
    ,mPrvStartY(0)
    ,mOrientation(0)
    ,mIsFacotory(MFALSE)
    ,mTestPatternOut(0)
    ,mCapWidth(0)
    ,mCapHeight(0)
    ,mCapType(0)
    ,mQVWidth(0)
    ,mQVHeight(0)
    ,mUnPack(MFALSE)
    ,mIsSOI(MFALSE)
    ,mMFLL_EN(MFALSE)
    ,mHDR_EN(0)
    ,mEMultiNR(EMultiNR_Off)
    ,mLCMOrientation(0)
    ,mSurfaceIndex(0)
    ,m_pSensorHalObj(NULL)
    ,mSensorCallerName("ACDK")
    ,mSensorInit(MFALSE)
    ,mSupportedSensorDev(0)
    ,mSensorType(0)
    ,mSensorOrientation(0)
    ,mSensorVFlip(0)
    ,mSensorHFlip(0)
    ,mSetShutTime(0)
    ,mGetShutTime(0)
    ,mGetAFInfo(0)
    ,mSensorModeNum(0)
    ,mOperationMode(0)
    ,mIsISPEng(MFALSE)
    ,mpIHal3A(0)
    ,Capture_Size(0)
    ,mGetCheckSumValue(0)
    ,PureRaw_BufAddr(NULL)
    ,Capture_BufAddr(NULL)
    ,mu4SensorDelay(0)
    ,mSensorIndex(0)
{
    ACDK_LOGD("+");
    memset(&mSensorResolution, 0, sizeof(ACDK_SENSOR_RESOLUTION_INFO_STRUCT));
    memset(&mSensorFormatInfo, 0, sizeof(SensorRawImageInfo_t));

    //====== SurfaceView ======

    m_pAcdkSurfaceViewObj = NULL;
    if (bypassFlag & ACDK_CCT_BYPASS_SURFACEVIEW) {
        m_pAcdkSurfaceViewObj = AcdkSurfaceView::createInstance(MTRUE);
    }
    else {
        m_pAcdkSurfaceViewObj = AcdkSurfaceView::createInstance();
    }
    if(m_pAcdkSurfaceViewObj == NULL)
    {
        ACDK_LOGE("Can not create surface view obj");
    }

    //====== AcdkMhal ======

    m_pAcdkCctMhalObj = NULL;
    m_pAcdkCctMhalObj = AcdkCctMhalBase::createInstance(0);
    if(m_pAcdkCctMhalObj == NULL)
    {
        ACDK_LOGE("Can not create AcdkMhal obj");
    }

    m_pAcdkCctMhalObjEng = NULL;
    m_pAcdkCctMhalObjEng = AcdkCctMhalBase::createInstance(1);
    if(m_pAcdkCctMhalObjEng == NULL)
    {
        ACDK_LOGE("Can not create AcdkMhal obj");
    }

    //====== AcdkUtility =====

    m_pAcdkUtilityObj = NULL;
    m_pAcdkUtilityObj = AcdkUtility::createInstance();
    if(m_pAcdkUtilityObj == NULL)
    {
        ACDK_LOGE("Can not create AcdkUtility obj");
    }

    //====== Memory ======

    //IMEM
    m_pIMemDrv = IMem3A::createInstance();

    for(MINT32 i = 0; i < OVERLAY_BUFFER_CNT; ++i)
    {
       // Pass1 : Pure Raw buffer
       mPrvIMemInfo[i].size = mPrvIMemInfo[i].virtAddr = mPrvIMemInfo[i].phyAddr = 0;
       mPrvIMemInfo[i].memID = -5;
       // Pass1 : FullG buffer
       mPrvIMemInfoRRZO[i].size = mPrvIMemInfoRRZO[i].virtAddr = mPrvIMemInfoRRZO[i].phyAddr = 0;
       mPrvIMemInfoRRZO[i].memID = -5;
       // Pass2 output buffer
       mDispIMemInfo[i].size = mDispIMemInfo[i].virtAddr = mDispIMemInfo[i].phyAddr = 0;
       mDispIMemInfo[i].memID = -5;
    }

    for(MINT32 i = 0; i < SURFACE_NUM; ++i)
    {
        //overlay buffer
        mSurfaceIMemInfo[i].size = mSurfaceIMemInfo[i].virtAddr = mSurfaceIMemInfo[i].phyAddr = 0;
        mSurfaceIMemInfo[i].memID = -5;
    }

    mRawIMemInfo.size = mRawIMemInfo.virtAddr = mRawIMemInfo.phyAddr = 0;
    mRawIMemInfo.memID = -5;

    mJpgIMemInfo.size = mJpgIMemInfo.virtAddr = mJpgIMemInfo.phyAddr = 0;
    mJpgIMemInfo.memID = -5;

    mQvIMemInfo.size = mQvIMemInfo.virtAddr = mQvIMemInfo.phyAddr = 0;
    mQvIMemInfo.memID = -5;

    mCapIMemInfo.size = mCapIMemInfo.virtAddr = mCapIMemInfo.phyAddr = 0;
    mCapIMemInfo.memID = -5;
    //====== Global Variable ======

    //g_pAcdkCctMainObj = this;

    ACDK_LOGD("-");
}

/*******************************************************************************
*Destructor
********************************************************************************/
AcdkCctMain::~AcdkCctMain()
{
    ACDK_LOGD("+");

    //g_pAcdkCctMainObj = NULL;

    if(m_pAcdkSurfaceViewObj != NULL)
    {
        m_pAcdkSurfaceViewObj->destroyInstance();
        m_pAcdkSurfaceViewObj = NULL;
    }

    if(m_pAcdkCctMhalObj != NULL)
    {
        m_pAcdkCctMhalObj->destroyInstance();
        m_pAcdkCctMhalObj = NULL;
    }

    if(m_pAcdkCctMhalObjEng != NULL)
    {
        m_pAcdkCctMhalObjEng->destroyInstance();
        m_pAcdkCctMhalObjEng = NULL;
    }

    if(m_pAcdkUtilityObj != NULL)
    {
        m_pAcdkUtilityObj->destroyInstance();
        m_pAcdkUtilityObj = NULL;
    }

    if(m_pSensorHalObj != NULL)
    {
        m_pSensorHalObj->destroyInstance();
        m_pSensorHalObj = NULL;
    }

    if(m_pIMemDrv != NULL)
    {
        m_pIMemDrv->destroyInstance();
        m_pIMemDrv = NULL;
    }

    g_acdkMainDebug = 0;
    g_dumpRAW = 0;
    mGetCheckSumValue = 0;
    PureRaw_BufAddr = 0;
    Capture_BufAddr = 0;
    Capture_Size = 0;

    ACDK_LOGD("-");
}

/*******************************************************************************
* acdkCctMainGetState
* brif : set state of AcdkCctMain
*******************************************************************************/
MVOID AcdkCctMain::acdkCctMainSetState(AcdkCctMainState_e newState)
{
    Mutex::Autolock lock(mLock);

    ACDK_LOGD("Now(0x%04x), Next(0x%04x)", m_eAcdkCctMainState, newState);

    if(newState == ACDK_CCT_MAIN_ERROR)
    {
        goto ACDKMAIN_SET_STATE_EXIT;
    }

    switch(m_eAcdkCctMainState)
    {
    case ACDK_CCT_MAIN_NONE:
        switch(newState)
        {
        case ACDK_CCT_MAIN_INIT:
        case ACDK_CCT_MAIN_UNINIT:
            break;
        default:
            ACDK_LOGE("State error ACDK_CCT_MAIN_NONE");
            break;
        }
        break;
    case ACDK_CCT_MAIN_INIT:
        switch(newState)
        {
        case ACDK_CCT_MAIN_IDLE:
            break;
        default:
            ACDK_LOGE("State error ACDK_CCT_MAIN_INIT");
            break;
        }
        break;
    case ACDK_CCT_MAIN_IDLE:
        switch(newState)
        {
        case ACDK_CCT_MAIN_IDLE:
        case ACDK_CCT_MAIN_PREVIEW:
        case ACDK_CCT_MAIN_CAPTURE:
        case ACDK_CCT_MAIN_UNINIT:
            break;
        default:
            ACDK_LOGE("State error ACDK_CCT_MAIN_IDLE");
            break;
        }
        break;
    case ACDK_CCT_MAIN_PREVIEW:
        switch(newState)
        {
        case ACDK_CCT_MAIN_IDLE:
        case ACDK_CCT_MAIN_PREVIEW:
            break;
        default:
            ACDK_LOGE("State error ACDK_CCT_MAIN_PREVIEW");
            break;
        }
        break;
    case ACDK_CCT_MAIN_CAPTURE:
        switch(newState)
        {
        case ACDK_CCT_MAIN_IDLE:
            break;
        default:
            ACDK_LOGE("State error ACDK_CCT_MAIN_CAPTURE");
            break;
        }
        break;
    case ACDK_CCT_MAIN_ERROR:
        switch(newState)
        {
        case ACDK_CCT_MAIN_IDLE:
        case ACDK_CCT_MAIN_UNINIT:
            break;
        default:
            ACDK_LOGE("State error ACDK_CCT_MAIN_ERROR");
            break;
        }
        break;
    default:
        ACDK_LOGE("Unknown state");
        break;
    }

ACDKMAIN_SET_STATE_EXIT:

    m_eAcdkCctMainState = newState;

    ACDK_LOGD("X, state(0x%04x)", m_eAcdkCctMainState);
}

/*******************************************************************************
* acdkCctMainGetState
* brif : get state of AcdkCctMain
*******************************************************************************/
AcdkCctMainState_e AcdkCctMain::acdkCctMainGetState()
{
    Mutex::Autolock _l(mLock);
    return m_eAcdkCctMainState;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::stateMgr(AcdkCctMainState_e nextState)
{
    MINT32 err = ACDK_RETURN_NO_ERROR;
    AcdkCctMainState_e curtState = acdkCctMainGetState();

    ACDK_LOGD("curtState(0x%x),nextState(0x%x)",curtState,nextState);

    if(curtState != ACDK_CCT_MAIN_IDLE)
    {
        if(curtState == ACDK_CCT_MAIN_PREVIEW)  // in preview state
        {
            if(nextState == ACDK_CCT_MAIN_PREVIEW)
            {
                ACDK_LOGD("Already preview");
                err = ACDK_RETURN_ERROR_STATE;
            }
            else if(nextState == ACDK_CCT_MAIN_CAPTURE)
            {
                ACDK_LOGD("OK for Capture");
            }
            else
            {
                ACDK_LOGE("ACDK_CCT_MAIN_PREVIEW wrong state(0x%x)",nextState);
                err = ACDK_RETURN_INVALID_PARA;
            }
        }
        else if(curtState == ACDK_CCT_MAIN_CAPTURE) // in capture state
        {
            if(nextState == ACDK_CCT_MAIN_PREVIEW)
            {
                ACDK_LOGD("Capturing");
                while(ACDK_CCT_MAIN_CAPTURE == acdkCctMainGetState())
                {
                    usleep(200);
                }
                ACDK_LOGD("Capture done");
            }
            else if(nextState == ACDK_CCT_MAIN_CAPTURE)
            {
                ACDK_LOGD("Already capture");
                err = ACDK_RETURN_ERROR_STATE;
            }
            else
            {
                ACDK_LOGE("ACDK_CCT_MAIN_CAPTURE wrong state(0x%x)",nextState);
                err = ACDK_RETURN_INVALID_PARA;
            }
        }
        else
        {
            ACDK_LOGE("wrong state(0x%x)",nextState);
            err = ACDK_RETURN_INVALID_PARA;
        }
    }

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::init()
{
    MINT32 err = ACDK_RETURN_NO_ERROR;
    MUINT32 width = 0, height = 0;
    char value[PROPERTY_VALUE_MAX] = {'\0'};

    ACDK_LOGD("+");

    //===== set p2 raw dump ====
    ACDK_LOGD("setprop vendor.debug.camera.p2plug.log 2");
    String8 t_key = String8("vendor.debug.camera.p2plug.log");
    String8 t_value = String8("2");
    property_set(t_key, t_value);

    DIR *dir;
    if ((dir=opendir("/mnt/vendor/cct")) == NULL)
    {
        ACDK_LOGE("Open dir failed: /mnt/vendor/cct");
    }
    else
    {
        struct dirent *ptr;
        int result;
        while ((ptr=readdir(dir)) != NULL)
        {
            char filename[256];
            char pathname[300];
            strncpy(filename, ptr->d_name, 255);
            filename[255] = 0;
            sprintf(pathname, "%s/%s", "/mnt/vendor/cct", filename);

            ACDK_LOGD("file list=%s", pathname);
            if( filename == strstr(filename, "fcell_dst_") && 0 != strstr(filename, ".raw") ) // start with "fcell_dst_" + contains ".raw"
            {
                result = remove( pathname );
                ACDK_LOGD("remove file=%s, ret=%d", pathname, result);
            }
        }
        closedir(dir);
    }

    //====== Set State ======
    m_eAcdkCctMainState = ACDK_CCT_MAIN_INIT;
    //====== Get System boot mode ======

    mOperationMode = get_boot_mode();

    //======  Object Checking =====

    if(m_pAcdkSurfaceViewObj == NULL)
    {
        ACDK_LOGE("Null SurfaceView Obj");
        err = ACDK_RETURN_NULL_OBJ;
        goto INIT_Exit;
    }

    if(m_pAcdkCctMhalObj == NULL)
    {
        ACDK_LOGE("Null AcdkMhal Obj");
        err = ACDK_RETURN_NULL_OBJ;
        goto INIT_Exit;
    }

    if(m_pAcdkCctMhalObjEng== NULL)
    {
        ACDK_LOGE("Null AcdkMhal Obj");
        err = ACDK_RETURN_NULL_OBJ;
        goto INIT_Exit;
    }

    if(m_pAcdkUtilityObj == NULL)
    {
        ACDK_LOGE("Null AcdkUtilityObj Obj");
        err = ACDK_RETURN_NULL_OBJ;
        goto INIT_Exit;
    }

    if(m_pIMemDrv == NULL)
    {
        ACDK_LOGE("Null IMemDrv Obj");
        err = ACDK_RETURN_NULL_OBJ;
        goto INIT_Exit;
    }

    //====== IMEM Init ======

    if(!m_pIMemDrv->init())
    {
        ACDK_LOGE("mpIMemDrv->init() error");
        err = ACDK_RETURN_NULL_OBJ;
        goto INIT_Exit;
    }

    //======  Surface View Setting ======

    err = m_pAcdkSurfaceViewObj->init();
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("Faile to init surfaceview err(0x%x)", err);
        goto INIT_Exit;
    }

    m_pAcdkSurfaceViewObj->getSurfaceInfo(width, height, mLCMOrientation);

    ACDK_LOGD("width(%u),height(%u),mLCMOrientation(%u)",width,height,mLCMOrientation);

    //======  Sensor Init and Get Sensor Info======

    err = sensorInit();
    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("Sensor setting Fail. err(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto INIT_Exit;
    }

    err = getSensorInfo();
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("getSensorInfo error(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto INIT_Exit;
    }

    #if 0
    if(mLCMOrientation == 180)
    {
        mLCMOrientation = 0;
    }

    if(mSensorDev == SENSOR_DEV_SUB)
    {
        mOrientation = (mLCMOrientation - mSensorOrientation + 360) % 360;
    }
    else
    {
        mOrientation = (mLCMOrientation + mSensorOrientation) % 360;
    }
    #else
    //For LCM 270or90 & subcam used(tablet)
    if((mSensorDev == SENSOR_DEV_SUB) && (mLCMOrientation == 270||mLCMOrientation == 90))
    {
        mOrientation = (mLCMOrientation + mSensorOrientation + 180) % 360;
    }
    else
    {
        mOrientation = (mLCMOrientation + mSensorOrientation) % 360;
    }
    #endif

    calcPreviewWin(width, height, mPrvStartX, mPrvStartY, mPrvWidth, mPrvHeight);

    ACDK_LOGD("mOrientation(%u), mSensorOrientation(%u)", mOrientation, mSensorOrientation);
    ACDK_LOGD("prvStartX(%u), prvStartY(%u)", mPrvStartX, mPrvStartY);
    ACDK_LOGD("prvWidth(%u), prvHeight(%u)" ,mPrvWidth, mPrvHeight);

    //if(mOperationMode == 1 && m_pSensorHalObj != NULL)
    {// power on sensor for LSC to communicate with sensor
        m_pSensorHalObj->powerOn(mSensorCallerName, 1, &msensorArray[0]);
        ACDK_LOGD("Power on sensor");
    }

    //====== 3A init ======
    mpIHal3A = MAKE_Hal3A(mSensorIndex, "ACDK");
    mpIHal3A->send3ACtrl(E3ACtrl_Enable3ASetParams, MFALSE, 0);
    mpIHal3A->send3ACtrl(E3ACtrl_SetOperMode, NSIspTuning::EOperMode_Meta, 0);

    //if(mOperationMode == 1 && m_pSensorHalObj != NULL)
    {// power off sensor in case multiple poweron hereafter
        m_pSensorHalObj->powerOff(mSensorCallerName, 1, &msensorArray[0]);
        ACDK_LOGD("Power off sensor");
    }

    //====== Initialize AcdkMhal ======
    err = m_pAcdkCctMhalObj->acdkCctMhalInit();
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("acdkCctMhalInit Fail(0x%x)", err);
        goto INIT_Exit;
    }

    err = m_pAcdkCctMhalObjEng->acdkCctMhalInit();
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("acdkCctMhalInit Fail(0x%x)", err);
        goto INIT_Exit;
    }

    //====== Get Debug Property ======

    property_get("vendor.camera.acdk.debug", value, "0");
    g_acdkMainDebug = atoi(value);

    ACDK_LOGD("g_acdkMainDebug(%d)", g_acdkMainDebug);

    //====== Set State ======

    acdkCctMainSetState(ACDK_CCT_MAIN_IDLE);

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;

INIT_Exit:

    if(m_pAcdkCctMhalObj)
    {
        m_pAcdkCctMhalObj->acdkCctMhalUninit();
    }

    if(m_pAcdkCctMhalObjEng)
    {
        m_pAcdkCctMhalObjEng->acdkCctMhalUninit();
    }

    if(m_pAcdkSurfaceViewObj)
    {
        m_pAcdkSurfaceViewObj->uninit();
    }

    //====== Set State ======

    m_eAcdkCctMainState = ACDK_CCT_MAIN_NONE;

    ACDK_LOGD("Fail X");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::uninit()
{
    ACDK_LOGD("+");

    MINT32 err = ACDK_RETURN_NO_ERROR;

    //====== Set State ======

    m_eAcdkCctMainState = ACDK_CCT_MAIN_UNINIT;

    //====== AcdkMhal Uninit ======
    if(m_pAcdkCctMhalObj != NULL)
    {
        err = m_pAcdkCctMhalObj->acdkCctMhalUninit();
        if (err != ACDK_RETURN_NO_ERROR)
        {
            ACDK_LOGE("Faile to uninit acdkCctMhalUninit(0x%x)", err);
        }
    }

    if(m_pAcdkCctMhalObjEng!= NULL)
    {
        err = m_pAcdkCctMhalObjEng->acdkCctMhalUninit();
        if (err != ACDK_RETURN_NO_ERROR)
        {
            ACDK_LOGE("Faile to uninit acdkCctMhalUninit(0x%x)", err);
        }
    }

    //====== AcdkSurface =====

    if(m_pAcdkSurfaceViewObj != NULL)
    {
        err = m_pAcdkSurfaceViewObj->uninit();
        if(err != ACDK_RETURN_NO_ERROR)
        {
            ACDK_LOGE("Faile to uninit surfaceview(0x%x)", err);
        }
    }

    //====== AcdkUtility =====

    if(m_pAcdkUtilityObj != NULL)
    {
        m_pAcdkUtilityObj->destroyInstance();
        m_pAcdkUtilityObj = NULL;
    }

    if (NULL != mpIHal3A)
    {
        mpIHal3A->send3ACtrl(E3ACtrl_Enable3ASetParams, MTRUE, 0);
        mpIHal3A->destroyInstance("ACDK");
    }

    if(m_pSensorHalObj != NULL)
    {
        m_pSensorHalObj->powerOff(mSensorCallerName, 1, &msensorArray[0]);
        m_pSensorHalObj->destroyInstance(mSensorCallerName);
        m_pSensorHalObj= NULL;
    }

    //====== Free Surface Memory ======
    if(mSurfaceIMemInfo[0].virtAddr != 0)
    {
        destroyMemBuf(SURFACE_NUM, mSurfaceIMemInfo);
    }

    if(m_pIMemDrv != NULL)
    {
        if(!m_pIMemDrv->uninit())
        {
            ACDK_LOGE("Faile to uninit m_pIMemDrv");
        }
    }

    //====== Set State ======

    m_eAcdkCctMainState = ACDK_CCT_MAIN_NONE;

    ACDK_LOGD("-");

    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::sensorInit()
{
    ACDK_LOGD("+");

    MINT32  err = ACDK_RETURN_NO_ERROR;
    MINT32  index = 0, sensorFound = 0;
    MUINT   sensorNum;
    MainSensorInfo_t camSensorInfo[8];
    IHalSensorList* pHalSensorList;

    pHalSensorList = MAKE_HalSensorList();

    pHalSensorList->searchSensors();

    sensorNum = pHalSensorList->queryNumberOfSensors();

    //Get sensor information
    for (MUINT i = 0; i < sensorNum; i++){
        ACDK_LOGD("name:%s type:%d", pHalSensorList->queryDriverName(i), pHalSensorList->queryType(i));//debug
        ACDK_LOGD("index:%d, SensorDevIdx:%d",i ,pHalSensorList->querySensorDevIdx(i));//debug
        mSupportedSensorDev |= pHalSensorList->querySensorDevIdx(i);
    }

    pHalSensorList->querySensorStaticInfo(SENSOR_DEV_MAIN,&sensorStaticInfo[0]);
    ACDK_LOGD("Main preview width = %d, height = %d\n",sensorStaticInfo[0].previewWidth,sensorStaticInfo[0].previewHeight);

    if (sensorNum > 0) {
        sensorFound = 1;
    }

    ACDK_LOGD("mSupportedSensorDev:0x%x", mSupportedSensorDev);

    // set each sensor type info
    if(mSupportedSensorDev & SENSOR_DEV_MAIN)
    {
        camSensorInfo[index].facing  = 0;    // back
        camSensorInfo[index].devType = SENSOR_DEV_MAIN;
        ++index;
    }

    if(mSupportedSensorDev & SENSOR_DEV_SUB)
    {
        camSensorInfo[index].facing  = 1;   // front
        camSensorInfo[index].devType = SENSOR_DEV_SUB;
        //subIndex = index;
        ++index;
    }

    /*
    if(mSupportedSensorDev & SENSOR_DEV_ATV)
    {
        camSensorInfo[index].facing  = 0;
        camSensorInfo[index].devType = SENSOR_DEV_ATV;
        ++index;
    }*/

    if(mSupportedSensorDev & SENSOR_DEV_MAIN_2)
    {
        camSensorInfo[index].facing  = 0;
        camSensorInfo[index].devType = SENSOR_DEV_MAIN_2;
        ++index;
    }

    // set Main as default
    if(mSensorDev == SENSOR_DEV_NONE)
    {
        mSensorDev = SENSOR_DEV_MAIN;
        mSensorVFlip = 0;
        mSensorHFlip = 0;
    }

    // set current sensor type
    for(MINT32 i = 0; i < index; ++i)
    {
        if (camSensorInfo[i].devType == mSensorDev)
        {
            sensorFound = 1;
            break;
        }
    }

    if(sensorFound == 0)
    {
        ACDK_LOGE("sensor not found");
        err = ACDK_RETURN_INVALID_SENSOR;
        goto sensorInitExit;
    }
    else
    {
        setSrcDev( mSensorDev );

        ACDK_LOGD("mSensorDev   = %d",mSensorDev);
        ACDK_LOGD("mSensorVFlip = %d",mSensorVFlip);
        ACDK_LOGD("mSensorHFlip = %d",mSensorHFlip);
    }

    // Open sensor device
    msensorArray[0] = 0xff;

    for (MUINT i = 0; i < sensorNum; i++){
        if(pHalSensorList->querySensorDevIdx(i) == static_cast<MUINT32>(mSensorDev))
        {
            msensorArray[0] = i;
            mSensorIndex = i;
            break;
        }
    }

    ACDK_LOGD("mSensorIndex = %d",mSensorIndex);
    if(msensorArray[0] == 0xff)
    {
       ACDK_LOGE("Sensor Not found");
       err = ACDK_RETURN_INVALID_SENSOR;
       goto sensorInitExit;
    }

    //====== Sensor Init======
    //set current sensor device
    m_pSensorHalObj = pHalSensorList->createSensor(mSensorCallerName, 1, &msensorArray[0]);//modify

    if(m_pSensorHalObj == NULL)
    {
       ACDK_LOGE("m_pSensorHalObj is NULL");
       err = ACDK_RETURN_NULL_OBJ;
       goto sensorInitExit;
    }

    //MT_NORMAL_BOOT 0 , MT_META_BOOT 1, MT_RECOVERY_BOOT 2, MT_SW_REBOOT 3
    //MT_FACTORY_BOOT 4, MT_ADVMETA_BOOT 5
    #if 0
    if(mOperationMode == 4)
    {
        // Open sensor number decide by sensorArray
        //m_pSensorHalObj->powerOn(mSensorCallerName, 1, &msensorArray[0]);
    }
    #endif
    mSensorInit = MTRUE;

    // Get used preview width / height
    pHalSensorList->querySensorStaticInfo(mSensorDev,&sensorStaticInfo[0]);
    //sensor support mode number
    mSensorModeNum = sensorStaticInfo[0].SensorModeNum;

sensorInitExit:

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::getSensorInfo()
{
    ACDK_LOGD("+");

    MINT32 err = ACDK_RETURN_NO_ERROR;
    MINT32 mode;

    //====== Check Object ======

    if(m_pSensorHalObj == NULL)
    {
        ACDK_LOGE("m_pSensorHalObj is NULL");
        err = ACDK_RETURN_NULL_OBJ;
        goto getSensorInfoExit;
    }
    MUINT previewWidth;
    MUINT previewHeight;
    MUINT captureWidth;
    MUINT captureHeight;
    MUINT videoWidth;
    MUINT videoHeight;
    //====== Get Sensor Info ======
    // get preview range
    mSensorResolution.SensorPreviewWidth = sensorStaticInfo[0].previewWidth;
    mSensorResolution.SensorPreviewHeight = sensorStaticInfo[0].previewHeight;
    // get sensor full range
    mSensorResolution.SensorFullWidth = sensorStaticInfo[0].captureWidth;
    mSensorResolution.SensorFullHeight = sensorStaticInfo[0].captureHeight;
    // get sensor video range
    mSensorResolution.SensorVideoWidth = sensorStaticInfo[0].videoWidth;
    mSensorResolution.SensorVideoHeight = sensorStaticInfo[0].videoHeight;
    // get sensor video1 range
    mSensorResolution.SensorHighSpeedVideoWidth = sensorStaticInfo[0].video1Width;
    mSensorResolution.SensorHighSpeedVideoHeight = sensorStaticInfo[0].video1Height;
    // get sensor video2 range
    mSensorResolution.SensorSlimVideoWidth = sensorStaticInfo[0].video2Width;
    mSensorResolution.SensorSlimVideoHeight = sensorStaticInfo[0].video2Height;
    // get sensor video3 range
    mSensorResolution.SensorCustom1Width = sensorStaticInfo[0].SensorCustom1Width;
    mSensorResolution.SensorCustom1Height = sensorStaticInfo[0].SensorCustom1Height;
    // get sensor video4 range
    mSensorResolution.SensorCustom2Width = sensorStaticInfo[0].SensorCustom2Width;
    mSensorResolution.SensorCustom2Height = sensorStaticInfo[0].SensorCustom2Height;
    // get sensor video5 range
    mSensorResolution.SensorCustom3Width = sensorStaticInfo[0].SensorCustom3Width;
    mSensorResolution.SensorCustom3Height = sensorStaticInfo[0].SensorCustom3Height;
    // get sensor video6 range
    mSensorResolution.SensorCustom4Width = sensorStaticInfo[0].SensorCustom4Width;
    mSensorResolution.SensorCustom4Height = sensorStaticInfo[0].SensorCustom4Height;
    // get sensor video7 range
    mSensorResolution.SensorCustom5Width = sensorStaticInfo[0].SensorCustom5Width;
    mSensorResolution.SensorCustom5Height = sensorStaticInfo[0].SensorCustom5Height;
    // get RAW or YUV
    mSensorType = sensorStaticInfo[0].sensorType;

    //get sernsor orientation angle
    mSensorOrientation = sensorStaticInfo[0].orientationAngle;
    //get sensor format info
    mSensorFormatInfo.u4BitDepth = sensorStaticInfo[0].rawSensorBit;
    mSensorFormatInfo.u4Order = sensorStaticInfo[0].sensorFormatOrder;

    //get sensro delay frame count
    mu4SensorDelay = 0;//sensorStaticInfo[0].previewDelayFrame;
    //get sensor test pattern checksum value
    /*
    err = m_pSensorHalObj->sendCommand( mSensorDev,
                                        SENSOR_CMD_GET_TEST_PATTERN_CHECKSUM_VALUE,
                                        (MUINTPTR)&mGetCheckSumValue,
                                        0,
                                        0);
    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("SENSOR_CMD_GET_TEST_PATTERN_CHECKSUM_VALUE fail(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto getSensorInfoExit;
    }*/

    ACDK_LOGD("mSensorDev(%d)",mSensorDev);
    ACDK_LOGD("1-RAW,2-YUV(%d)",mSensorType);
    ACDK_LOGD("preview size : w(%u),h(%u)", mSensorResolution.SensorPreviewWidth, mSensorResolution.SensorPreviewHeight);
    ACDK_LOGD("full size    : w(%u),h(%u)", mSensorResolution.SensorFullWidth, mSensorResolution.SensorFullHeight);
    ACDK_LOGD("video size   : w(%u),h(%u)", mSensorResolution.SensorVideoWidth, mSensorResolution.SensorVideoHeight);
    ACDK_LOGD("video1 size  : w(%u),h(%u)", mSensorResolution.SensorHighSpeedVideoWidth, mSensorResolution.SensorHighSpeedVideoHeight);
    ACDK_LOGD("video2 size  : w(%u),h(%u)", mSensorResolution.SensorSlimVideoWidth, mSensorResolution.SensorSlimVideoHeight);
    ACDK_LOGD("video3 size  : w(%u),h(%u)", mSensorResolution.SensorCustom1Width, mSensorResolution.SensorCustom1Height);
    ACDK_LOGD("video4 size  : w(%u),h(%u)", mSensorResolution.SensorCustom2Width, mSensorResolution.SensorCustom2Height);
    ACDK_LOGD("video5 size  : w(%u),h(%u)", mSensorResolution.SensorCustom3Width, mSensorResolution.SensorCustom3Height);
    ACDK_LOGD("video6 size  : w(%u),h(%u)", mSensorResolution.SensorCustom4Width, mSensorResolution.SensorCustom4Height);
    ACDK_LOGD("video7 size  : w(%u),h(%u)", mSensorResolution.SensorCustom5Width, mSensorResolution.SensorCustom5Height);
    ACDK_LOGD("bit depth(%u)",mSensorFormatInfo.u4BitDepth);
    ACDK_LOGD("isPacked(%u)",mSensorFormatInfo.u4IsPacked);
    ACDK_LOGD("color order(%u)",mSensorFormatInfo.u4Order);
    //ACDK_LOGD("Checksum value(0x%x)",mGetCheckSumValue);
    ACDK_LOGD("Delay frame(0x%x)",mu4SensorDelay);


getSensorInfoExit:

    ACDK_LOGD("-");

    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::calcPreviewWin(
                   MUINT32 const surfaceWidth,
                   MUINT32 const surfaceHeight,
                   MUINT32 &x,
                   MUINT32 &y,
                   MUINT32 &width,
                   MUINT32 &height)
{
    ACDK_LOGD("+");

    //====== Local Variable ======

    MUINT32 offset = 0;
    MUINT32 tempW = surfaceWidth, tempH = surfaceHeight;
    MUINT32 degree = mOrientation;

    //====== Calculation ======

    // decide preview size && offset
    // the screen scan direction has a angle's shift to camera sensor

    if(degree == 90 || degree== 270) //Sensor need to rotate
    {
        tempW = surfaceHeight;
        tempH = surfaceWidth;
    }

    if(tempW > tempH)
    {
        width  = (tempH / 3 * 4);
        height = (width / 4 * 3);
    }
    else
    {
        height = (tempW / 4 * 3);
        width  = (height / 3 * 4);
    }

    m_pAcdkUtilityObj->queryPrvSize(width,height);

    x = (tempW - width)  / 2;
    y = (tempH - height) / 2;

    if(degree == 90 || degree== 270)    //Sensor need to rotate
    {
        SWAP(x, y);
        SWAP(width, height);
    }

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::createMemBuf(MUINT32 &memSize, MUINT32 bufCnt, IMEM_BUF_INFO *bufInfo)
{
    ACDK_LOGD("bufCnt(%d)", bufCnt);

    MINT32 err = ACDK_RETURN_NO_ERROR;
    MUINT32 alingSize = (memSize + L1_CACHE_BYTES - 1) & ~(L1_CACHE_BYTES - 1);

    ACDK_LOGD("memSize(%u),alingSize(%u)", memSize, alingSize);

    memSize = alingSize;

    if(bufCnt > 1)  // more than one
    {
        for(MUINT32 i = 0; i < bufCnt; ++i)
        {
            bufInfo[i].size = alingSize;

            if(m_pIMemDrv->allocVirtBuf(&bufInfo[i]) < 0)
            {
                ACDK_LOGE("m_pIMemDrv->allocVirtBuf() error, i(%d)",i);
                err = ACDK_RETURN_API_FAIL;
            }

            if(m_pIMemDrv->mapPhyAddr(&bufInfo[i]) < 0)
            {
                ACDK_LOGE("m_pIMemDrv->mapPhyAddr() error, i(%d)",i);
                err = ACDK_RETURN_API_FAIL;
            }
        }
    }
    else
    {
        bufInfo->size = alingSize;

        if(m_pIMemDrv->allocVirtBuf(bufInfo) < 0)
        {
            ACDK_LOGE("m_pIMemDrv->allocVirtBuf() error");
            err = ACDK_RETURN_API_FAIL;
        }

        if(m_pIMemDrv->mapPhyAddr(bufInfo) < 0)
        {
            ACDK_LOGE("m_pIMemDrv->mapPhyAddr() error");
            err = ACDK_RETURN_API_FAIL;
        }
    }

    ACDK_LOGD("-");
    return err;
}


/******************************************************************************
*
*******************************************************************************/
MINT32 AcdkCctMain::destroyMemBuf(MUINT32 bufCnt, IMEM_BUF_INFO *bufInfo)
{
    ACDK_LOGD("bufCnt(%d)", bufCnt);

    MINT32 err = ACDK_RETURN_NO_ERROR;

    if(bufCnt > 1)  // more than one
    {
        for(MUINT32 i = 0; i < bufCnt; ++i)
        {
            if(0 == bufInfo[i].virtAddr)
            {
                ACDK_LOGD("Buffer doesn't exist, i(%d)",i);
                continue;
            }

            if(m_pIMemDrv->unmapPhyAddr(&bufInfo[i]) < 0)
            {
                ACDK_LOGE("m_pIMemDrv->unmapPhyAddr() error, i(%d)",i);
                err = ACDK_RETURN_API_FAIL;
            }

            if (m_pIMemDrv->freeVirtBuf(&bufInfo[i]) < 0)
            {
                ACDK_LOGE("m_pIMemDrv->freeVirtBuf() error, i(%d)",i);
                err = ACDK_RETURN_API_FAIL;
            }
        }
    }
    else
    {
        if(0 == bufInfo->virtAddr)
        {
            ACDK_LOGD("Buffer doesn't exist");
        }

        if(m_pIMemDrv->unmapPhyAddr(bufInfo) < 0)
        {
            ACDK_LOGE("m_pIMemDrv->unmapPhyAddr() error");
            err = ACDK_RETURN_API_FAIL;
        }

        if (m_pIMemDrv->freeVirtBuf(bufInfo) < 0)
        {
            ACDK_LOGE("m_pIMemDrv->freeVirtBuf() error");
            err = ACDK_RETURN_API_FAIL;
        }
    }

    ACDK_LOGD("-");
    return err;
}

/******************************************************************************
*
*******************************************************************************/
MINT32 AcdkCctMain::sensorFormatSetting(MUINT32 mode, MUINT32 &FormatType, MUINT32 &imgFormat, MUINT32 &imgSize, MUINT32 *imgStride)
{
    ACDK_LOGD("+");

    MUINT32 tempRAWPixelByte = 0;
    MUINT32 tempWidth, tempHeight;
    MUINT32 tempSize,tempStride,tempFormat;

    //====== Mode Setting ======
    setDimension(mode, tempWidth, tempHeight);
    tempStride = tempWidth;

    //====== Format Setting ======

    if(mSensorType == SENSOR_TYPE_RAW)  // RAW
    {
        if(FormatType == 0)
        {
            //get RAW image bit depth
            switch(mSensorFormatInfo.u4BitDepth)
            {
                case RAW_SENSOR_8BIT : tempFormat  = eImgFmt_BAYER8;
                    break;
                case RAW_SENSOR_10BIT : tempFormat  = eImgFmt_BAYER10;
                    break;
                case RAW_SENSOR_12BIT : tempFormat  = eImgFmt_BAYER12;
                    break;
                case RAW_SENSOR_14BIT : tempFormat  = eImgFmt_BAYER14; //To do : raw 14 bit
                    break;
                default : tempFormat = eImgFmt_UNKNOWN;
                          ACDK_LOGE("unknown raw image bit depth(%u)",mSensorFormatInfo.u4BitDepth);
                          return ACDK_RETURN_INVALID_PARA;
            }

            // calculate real stride and get byte per pixel. for RAW sensor only
            if(ACDK_RETURN_NO_ERROR != m_pAcdkUtilityObj->queryRAWImgFormatInfo(tempFormat,tempWidth,tempStride,tempRAWPixelByte))
            {
                ACDK_LOGE("queryRAWImgFormatInfo fail");
                return ACDK_RETURN_API_FAIL;
            }
            else
            {
                tempSize = ceil(tempStride * tempHeight * (tempRAWPixelByte / 4.0));
            }

            // set value
            imgSize   = tempSize;
            imgFormat = tempFormat;
            ACDK_LOGD("RAW : imgSize(%u),imgFormat(0x%x)",imgSize,imgFormat);

            if(imgStride != NULL)
            {
                imgStride[0] = tempStride;
                imgStride[1] = 0;
                imgStride[2] = 0;
                ACDK_LOGD("RAW : imgStride[0](%u)",imgStride[0]);
            }

        }
        else
        {
            //get RAW image bit depth
            switch(mSensorFormatInfo.u4BitDepth)
            {
                case RAW_SENSOR_8BIT : tempFormat  = eImgFmt_FG_BAYER8;
                    break;
                case RAW_SENSOR_10BIT : tempFormat  = eImgFmt_FG_BAYER10;
                    break;
                case RAW_SENSOR_12BIT : tempFormat  = eImgFmt_FG_BAYER12;
                    break;
                case RAW_SENSOR_14BIT : tempFormat  = eImgFmt_FG_BAYER14; //To do : raw 14 bit
                    break;
                default : tempFormat = eImgFmt_UNKNOWN;
                          ACDK_LOGE("unknown raw image bit depth(%u)",mSensorFormatInfo.u4BitDepth);
                          return ACDK_RETURN_INVALID_PARA;
            }

            // calculate real stride and get byte per pixel. for RAW sensor only
            if(ACDK_RETURN_NO_ERROR != m_pAcdkUtilityObj->queryRAWImgFullGInfo(tempFormat,tempWidth,tempStride,tempRAWPixelByte))
            {
                ACDK_LOGE("queryRAWImgFormatInfo fail");
                return ACDK_RETURN_API_FAIL;
            }
            else
            {
                tempSize = (tempStride * tempHeight*15)>>3;
            }

            imgSize   = tempSize;
            imgFormat = tempFormat;
            ACDK_LOGD("RAW : imgSize(%u),imgFormat(0x%x)",imgSize,imgFormat);

            if(imgStride != NULL)
            {
                imgStride[0] = tempStride;
                imgStride[1] = 0;
                imgStride[2] = 0;
                ACDK_LOGD("RAW : imgStride[0](%u)",imgStride[0]);
            }
        }
    }
    else if(mSensorType == SENSOR_TYPE_YUV) //YUV
    {
        //mapping YUV format between Sensor_Hal and CamIOPipe
        //Cb = U, Cr = V
        //
        switch(mSensorFormatInfo.u4Order)
        {
            case SENSOR_FORMAT_ORDER_UYVY :
                tempFormat = eImgFmt_UYVY;
                break;
            case SENSOR_FORMAT_ORDER_VYUY :
                tempFormat = eImgFmt_VYUY;
                break;
            case SENSOR_FORMAT_ORDER_YUYV :
                tempFormat = eImgFmt_YUY2;
                break;
            case SENSOR_FORMAT_ORDER_YVYU :
                tempFormat = eImgFmt_YVYU;
                break;
            default : tempFormat = eImgFmt_UNKNOWN;
                      ACDK_LOGE("unknown YUV type(0x%x)",mSensorFormatInfo.u4Order);
                      return ACDK_RETURN_INVALID_PARA;
        }

        // calculate image size
        if(ACDK_RETURN_NO_ERROR != m_pAcdkUtilityObj->queryImageSize(tempFormat,tempWidth,tempHeight,tempSize))
        {
            ACDK_LOGE("YUV - queryImageSize fail");
            return ACDK_RETURN_API_FAIL;
        }

        // calculate image stride

        if(imgStride != NULL)
        {
            if(ACDK_RETURN_NO_ERROR != m_pAcdkUtilityObj->queryImageStride(tempFormat,tempWidth,0,&imgStride[0]))
            {
                ACDK_LOGE("YUV - queryImageStride fail(0) : %d",imgStride[0]);
                return ACDK_RETURN_API_FAIL;
            }

            if(ACDK_RETURN_NO_ERROR != m_pAcdkUtilityObj->queryImageStride(tempFormat,tempWidth,1,&imgStride[1]))
            {
                ACDK_LOGE("YUV - queryImageStride fail(1) : %d",imgStride[1]);
                return ACDK_RETURN_API_FAIL;
            }

            if(ACDK_RETURN_NO_ERROR != m_pAcdkUtilityObj->queryImageStride(tempFormat,tempWidth,2,&imgStride[2]))
            {
                ACDK_LOGE("YUV - queryImageStride fail(2) : %d",imgStride[2]);
                return ACDK_RETURN_API_FAIL;
            }

            ACDK_LOGD("YUV - imgStride[0](%u),imgStride[1](%u),imgStride[2](%u)",imgStride[0],
                                                                                 imgStride[1],
                                                                                 imgStride[2]);
        }
        // set value
        imgSize = tempSize;
        imgFormat = tempFormat;
        ACDK_LOGD("YUV - imgSize(%u),imgFormat(0x%x)",imgSize,imgFormat);

    }
    else
    {
        ACDK_LOGD("sensor type not yet");
    }

    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::startPreview(Func_CB prvCb)
{
    ACDK_LOGD("+");

    MINT32 err = ACDK_RETURN_NO_ERROR;
    MINT32 continuous = 1;
    MUINT32 sensorImgSize = 0;
    MUINT32 RRZOImgSize = 0;
    MUINT32 RRZOWidth = 0;
    MUINT32 FormatType =0;
    MUINT32 displaySize,sensorFormatRaw,sensorFormatFullG;
    //OVL support YUY2/RGB565
    //MUINT32 dispalyFormat = eImgFmt_RGB565;
    MUINT32 dispalyFormat = eImgFmt_YUY2;
    MUINT32 sensorStride[3] = {0}, RRZOStride[3] = {0},dispalyStride[3] = {0};
    MUINT32 sensor_mode = CAPTURE_MODE;
    //MUINT32 sensorArray[1] = {0}, sensorArray2[1] = {1};
    IHalSensor::ConfigParam configParam[1];

    //====== Check and Set State ======

    err = stateMgr(ACDK_CCT_MAIN_PREVIEW);
    if(err == ACDK_RETURN_ERROR_STATE)
    {
        ACDK_LOGD("warning: redundent command. protect!!");
        return ACDK_RETURN_NO_ERROR;
    }
    else if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("stateMgr fail(0x%x)",err);
        return err;
    }

    acdkCctMainSetState(ACDK_CCT_MAIN_PREVIEW);

    if (!m_pSensorHalObj) {
        ACDK_LOGE("IHalSensor createSensor fail");
        err = ACDK_RETURN_API_FAIL;
        goto startPreviewExit;
    }
    // power on sensor
    // Open sensor number decide by sensorArray
    m_pSensorHalObj->powerOn(mSensorCallerName, 1, &msensorArray[0]);

    //get sensor test pattern checksum value
    m_pSensorHalObj->sendCommand(mSensorDev, SENSOR_CMD_GET_TEST_PATTERN_CHECKSUM_VALUE,
                        (MINTPTR)&mGetCheckSumValue, 0, 0);
    ACDK_LOGD("Checksum value(0x%x)",mGetCheckSumValue);

    #ifdef CAM_FULLSIZE_PREVIEW
    sensor_mode = CAPTURE_MODE;
    #elif defined CAM_VIDEO_MODE_TEST
    sensor_mode = VIDEO_MODE;
    #else
    sensor_mode = PREVIEW_MODE;
    #endif

    //====== Calculate Memory Size ======
    // calculate pass1 frame buffer size and stride
    err = sensorFormatSetting(sensor_mode,FormatType,sensorFormatRaw,sensorImgSize,&sensorStride[0]);
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("sensorFormatSetting fail(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto startPreviewExit;
    }
    // For RRZO information
    FormatType = 1;
    err = sensorFormatSetting(sensor_mode,FormatType,sensorFormatFullG,RRZOImgSize,&RRZOStride[0]);
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("sensorFormatSetting fail(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto startPreviewExit;
    }

    ACDK_LOGD("RRZOImgSize(%d),RRZOStride[0](%d)",RRZOImgSize,RRZOStride[0]);

    // FullG shall be 64bit align
    #ifdef CAM_FULLSIZE_PREVIEW
    RRZOWidth = (mSensorResolution.SensorFullWidth>>6)<<6;
    RRZOStride[0] = RRZOWidth;
    RRZOImgSize = (RRZOWidth*mSensorResolution.SensorFullHeight*15)>>3;
    #elif defined CAM_VIDEO_MODE_TEST
    RRZOWidth = (mSensorResolution.SensorVideoWidth>>6)<<6;
    RRZOStride[0] = RRZOWidth;
    RRZOImgSize = (RRZOWidth*mSensorResolution.SensorVideoHeight*15)>>3;
    #else
    RRZOWidth = (mSensorResolution.SensorPreviewWidth>>6)<<6;
    RRZOStride[0] = RRZOWidth;
    RRZOImgSize = (RRZOWidth*mSensorResolution.SensorPreviewHeight*15)>>3;
    #endif
    ACDK_LOGD("RRZOWidth(%d),RRZOStride[0](%d)",RRZOImgSize,RRZOStride[0]);

    //calculate display frame buffer size and stride
    m_pAcdkUtilityObj->queryImageSize(dispalyFormat,mPrvWidth,mPrvHeight,displaySize);
    m_pAcdkUtilityObj->queryImageStride(dispalyFormat,mPrvWidth,0,&dispalyStride[0]);
    m_pAcdkUtilityObj->queryImageStride(dispalyFormat,mPrvWidth,1,&dispalyStride[1]);
    m_pAcdkUtilityObj->queryImageStride(dispalyFormat,mPrvWidth,2,&dispalyStride[2]);

    ACDK_LOGD("sensorImgSize(%u),displaySize(%u)",sensorImgSize,displaySize);

    //====== Allocate Memory ======
    createMemBuf(sensorImgSize, OVERLAY_BUFFER_CNT, mPrvIMemInfo);      // pass1 - IMGO Out
    createMemBuf(RRZOImgSize, OVERLAY_BUFFER_CNT, mPrvIMemInfoRRZO);      // pass1 - RRZO Out
    createMemBuf(displaySize, OVERLAY_BUFFER_CNT, mDispIMemInfo);     // pass2 - video out
    createMemBuf(sensorImgSize, 1, &mCapIMemInfo); // Raw buffer
    if (mSurfaceIMemInfo[0].virtAddr == 0) {
        createMemBuf(displaySize, SURFACE_NUM, mSurfaceIMemInfo);  // surface - frame buffer
    }

    ACDK_LOGD("mPrvIMemInfo     : size(sensorImgSize) = %u",mPrvIMemInfo[0].size);
    ACDK_LOGD("mPrvIMemInfoRRZO : size(sensorImgSize) = %u",mPrvIMemInfoRRZO[0].size);
    ACDK_LOGD("mDispIMemInfo    : size(displaySize)   = %u",mDispIMemInfo[0].size);
    ACDK_LOGD("mSurfaceIMemInfo : size(displaySize) = %u",mSurfaceIMemInfo[0].size);

    for(MINT32 i = 0; i < OVERLAY_BUFFER_CNT; ++i)
    {
        ACDK_LOGD("prvVA[%d]  = 0x%lx, prvPA[%d]  = %p",i,(void*)mPrvIMemInfo[i].virtAddr,i,(void*)mPrvIMemInfo[i].phyAddr);
        ACDK_LOGD("rrzoVA[%d] = 0x%lx, rrzoPA[%d]  = %p",i,(void*)mPrvIMemInfoRRZO[i].virtAddr,i,(void*)mPrvIMemInfoRRZO[i].phyAddr);
        ACDK_LOGD("dispVA[%d] = 0x%lx, dispPA[%d] = %p",i,(void*)mDispIMemInfo[i].virtAddr,i,(void*)mDispIMemInfo[i].phyAddr);
    }

    for(MINT32 i = 0; i < SURFACE_NUM; ++i)
    {
        ACDK_LOGD("surfaceVA[%d] = 0x%lx, surfacePA[%d] = %p",i,(void*)mSurfaceIMemInfo[i].virtAddr,i,(void*)mSurfaceIMemInfo[i].phyAddr);
    }

    // init value
    for(MINT32 i = 0; i < OVERLAY_BUFFER_CNT; ++i)
    {
        if(0 == mPrvIMemInfo[i].virtAddr && 0 == mPrvIMemInfo[i].phyAddr)
        {
            ACDK_LOGE("mPrvIMemInfo[%d] Get the memory fail",i);
            err = ACDK_RETURN_MEMORY_ERROR;
            goto startPreviewExit;
        }
        else
        {
            memset((MUINT8*)mPrvIMemInfo[i].virtAddr, 0, mPrvIMemInfo[i].size);
            memset((MUINT8*)mPrvIMemInfoRRZO[i].virtAddr, 0, mPrvIMemInfoRRZO[i].size);
        }

        if(0== mDispIMemInfo[i].virtAddr && 0 == mDispIMemInfo[i].phyAddr)
        {
            ACDK_LOGE("mDispIMemInfo[%d] Get the memory fail",i);
            err = ACDK_RETURN_MEMORY_ERROR;
            goto startPreviewExit;
        }
        else
        {
            memset((MUINT8*)mDispIMemInfo[i].virtAddr, 0, mDispIMemInfo[i].size);
        }
    }

    for(MINT32 i = 0; i < SURFACE_NUM; ++i)
    {
        if(0 == mSurfaceIMemInfo[i].virtAddr && 0 == mSurfaceIMemInfo[i].phyAddr)
        {
            ACDK_LOGE("mSurfaceIMemInfo[%d] Get the memory fail",i);
            err = ACDK_RETURN_MEMORY_ERROR;
            goto startPreviewExit;
        }
        else
        {
            //for OVL output using rgb565 = 0x0000 -->black color
            //for OVL output using yuv422 = 0x0000 --> Green color , 0x0080 --> Black color
            //only first frame reset
            if(mFrameCnt == 0)
            {
                memset((MUINT8*)mSurfaceIMemInfo[i].virtAddr, 0, mSurfaceIMemInfo[i].size);
            }
         }
    }

    //====== Config Sensor ======
    ACDK_LOGD("Sensor FPS: %d, %d, %d", sensorStaticInfo[0].captureFrameRate, sensorStaticInfo[0].previewFrameRate,
        sensorStaticInfo[0].videoFrameRate);

    // Open sensor number decide by sensorArray
    //m_pSensorHalObj->powerOn(mSensorCallerName, 1, &msensorArray[0]);
    #ifdef CAM_FULLSIZE_PREVIEW
    configParam[0] =
    {
        index               : mSensorIndex, //modify
        crop                : MSize(sensorStaticInfo[0].captureWidth, sensorStaticInfo[0].captureHeight), //FIXME
        scenarioId          : SENSOR_SCENARIO_ID_NORMAL_CAPTURE,
        isBypassScenario    : 0,
        isContinuous        : 1,
        HDRMode             : 0,
        framerate           : sensorStaticInfo[0].captureFrameRate/10,
        twopixelOn          : 0,
        debugMode           : 0,
    };
    #elif defined CAM_VIDEO_MODE_TEST
    configParam[0] =
    {
        index               : mSensorIndex, //modify
        crop                : MSize(sensorStaticInfo[0].videoWidth, sensorStaticInfo[0].videoHeight), //FIXME
        scenarioId          : SENSOR_SCENARIO_ID_NORMAL_VIDEO,
        isBypassScenario    : 0,
        isContinuous        : 1,
        HDRMode             : 0,
        framerate           : sensorStaticInfo[0].videoFrameRate/10,
        twopixelOn          : 0,
        debugMode           : 0,
    };
    #else
    configParam[0] =
    {
        index               : mSensorIndex, //modify
        crop                : MSize(sensorStaticInfo[0].previewWidth, sensorStaticInfo[0].previewHeight), //FIXME
        scenarioId          : SENSOR_SCENARIO_ID_NORMAL_PREVIEW,
        isBypassScenario    : 0,
        isContinuous        : 1,
        HDRMode             : 0,
        framerate           : sensorStaticInfo[0].previewFrameRate/10,
        twopixelOn          : 0,
        debugMode           : 0,
    };
    #endif
    #ifdef ACDK_FAKE_SENSOR
    configParam[0].framerate = 10; // 1 fps, for fpga dbg only
    #endif

    //m_pSensorHalObj->configure(1, &configParam[0]); ==> move to AcdkMhal through NormalPipe
    if(mSensorDev == SENSOR_DEV_MAIN )//main sensor & atv use sensorHalaram[0]
    {
        ACDK_LOGD("main/atv-Continous=%u",configParam[0].isContinuous);
        ACDK_LOGD("main/atv-BypassSensorScenario=%u",configParam[0].isBypassScenario);
    }
    else if(mSensorDev == SENSOR_DEV_SUB || mSensorDev == SENSOR_DEV_MAIN_2)// main2 & sub sensor
    {
        ACDK_LOGD("main2/sub-Continous=%u",configParam[0].isContinuous);
        ACDK_LOGD("main2/sub-BypassSensorScenario=%u",configParam[0].isBypassScenario);
    }

    // Only can get current sensor information.
    //m_pSensorHalObj->querySensorDynamicInfo(mSensorDev, &sensorDynamicInfo[0]);
    //ACDK_LOGD("TgInfo[0:2] = %d/%d/%d", sensorDynamicInfo[0].TgInfo, sensorDynamicInfo[1].TgInfo, sensorDynamicInfo[2].TgInfo);
    //ACDK_LOGD("pixelMode[0:2] = %d/%d/%d", sensorDynamicInfo[0].pixelMode, sensorDynamicInfo[1].pixelMode, sensorDynamicInfo[2].pixelMode);

    if(mTestPatternOut) {
        MINT32 u32Enable = 1;
        err = m_pSensorHalObj->sendCommand(mSensorDev,
                                           SENSOR_CMD_SET_TEST_PATTERN_OUTPUT,
                                           (MUINTPTR)&u32Enable,
                                           0,
                                           0);
    }

    //====== Preview Parameter Setting ======
    memset(&mAcdkCctMhalPrvParam, 0, sizeof(acdkCctMhalPrvParam_t));

    //preview param setting
    //mAcdkCctMhalPrvParam.scenarioHW         = eScenarioID_VSS;
    mAcdkCctMhalPrvParam.sensorID          = mSensorDev; //Main / sub /Main2
    mAcdkCctMhalPrvParam.sensorIndex       = mSensorIndex; //Sensor Index
    mAcdkCctMhalPrvParam.sensorConfigParam[0] = configParam[0];
    //mAcdkCctMhalPrvParam.mSensorPixelMode  = sensorDynamicInfo[0].pixelMode;
    mAcdkCctMhalPrvParam.sensorWidth       = mSensorResolution.SensorFullWidth;
    mAcdkCctMhalPrvParam.sensorHeight      = mSensorResolution.SensorFullHeight;
    mAcdkCctMhalPrvParam.sensorFullWidth   = mSensorResolution.SensorFullWidth;
    mAcdkCctMhalPrvParam.sensorFullHeight  = mSensorResolution.SensorFullHeight;
    mAcdkCctMhalPrvParam.rrzoWidth         = RRZOWidth;
    mAcdkCctMhalPrvParam.sensorStride[0]   = sensorStride[0];
    mAcdkCctMhalPrvParam.sensorStride[1]   = sensorStride[1];
    mAcdkCctMhalPrvParam.sensorStride[2]   = sensorStride[2];
    mAcdkCctMhalPrvParam.rrzoStride[0]     = RRZOStride[0];
    mAcdkCctMhalPrvParam.rrzoStride[1]     = RRZOStride[1];
    mAcdkCctMhalPrvParam.rrzoStride[2]     = RRZOStride[2];
    mAcdkCctMhalPrvParam.sensorType        = mSensorType;
    mAcdkCctMhalPrvParam.sensorFormat      = sensorFormatRaw;
    mAcdkCctMhalPrvParam.sensorColorOrder  = mSensorFormatInfo.u4Order;
    mAcdkCctMhalPrvParam.mu4SensorDelay    = mu4SensorDelay;
    mAcdkCctMhalPrvParam.imgImemBuf        = mPrvIMemInfo;
    mAcdkCctMhalPrvParam.rrzoImemBuf       = mPrvIMemInfoRRZO;
    mAcdkCctMhalPrvParam.dispImemBuf       = mDispIMemInfo;
    mAcdkCctMhalPrvParam.frmParam.w        = mPrvWidth;
    mAcdkCctMhalPrvParam.frmParam.h        = mPrvHeight;
    mAcdkCctMhalPrvParam.frmParam.flip     = mSensorHFlip ? MTRUE : MFALSE;
    mAcdkCctMhalPrvParam.frmParam.stride[0] = dispalyStride[0];
    mAcdkCctMhalPrvParam.frmParam.stride[1] = dispalyStride[1];
    mAcdkCctMhalPrvParam.frmParam.stride[2] = dispalyStride[2];
    mAcdkCctMhalPrvParam.frmParam.frmFormat = dispalyFormat;
    mAcdkCctMhalPrvParam.acdkMainObserver  = acdkObserver(cameraCallback, this);
    mAcdkCctMhalPrvParam.IsFactoryMode = !!mIsFacotory;

    switch(mOrientation) { /* TBD fixme */
        case 0 : mAcdkCctMhalPrvParam.frmParam.orientation = 0;
            break;
        case 90 : mAcdkCctMhalPrvParam.frmParam.orientation = NSCam::eTransform_ROT_90;
            break;
        case 180 : mAcdkCctMhalPrvParam.frmParam.orientation = NSCam::eTransform_ROT_180;
            break;
        case 270 : mAcdkCctMhalPrvParam.frmParam.orientation = NSCam::eTransform_ROT_270;
            break;
    }

    ACDK_LOGD("scenarioHW      = %d", (MINT32)mAcdkCctMhalPrvParam.scenarioHW);
    ACDK_LOGD("sensorID        = %d", (MINT32)mAcdkCctMhalPrvParam.sensorID);
    //ACDK_LOGD("mSensorPixelMode= %d", (MINT32)mAcdkCctMhalPrvParam.mSensorPixelMode);
    ACDK_LOGD("sensorWidth     = %u", mAcdkCctMhalPrvParam.sensorWidth);
    ACDK_LOGD("sensorHeight    = %u", mAcdkCctMhalPrvParam.sensorHeight);
    ACDK_LOGD("rrzoWidth       = %u", mAcdkCctMhalPrvParam.rrzoWidth);
    ACDK_LOGD("sensorStride[0] = %u", mAcdkCctMhalPrvParam.sensorStride[0]);
    ACDK_LOGD("sensorStride[1] = %u", mAcdkCctMhalPrvParam.sensorStride[1]);
    ACDK_LOGD("sensorStride[2] = %u", mAcdkCctMhalPrvParam.sensorStride[2]);
    ACDK_LOGD("rrzoStride[0]   = %u", mAcdkCctMhalPrvParam.rrzoStride[0]);
    ACDK_LOGD("rrzoStride[1]   = %u", mAcdkCctMhalPrvParam.rrzoStride[1]);
    ACDK_LOGD("rrzoStride[2]   = %u", mAcdkCctMhalPrvParam.rrzoStride[2]);
    ACDK_LOGD("sensorType      = %u", mAcdkCctMhalPrvParam.sensorType);
    ACDK_LOGD("sensorFormat    = 0x%x", mAcdkCctMhalPrvParam.sensorFormat);
    ACDK_LOGD("colorOrder      = %u", mAcdkCctMhalPrvParam.sensorColorOrder);
    ACDK_LOGD("frmParam.w      = %u", mAcdkCctMhalPrvParam.frmParam.w);
    ACDK_LOGD("frmParam.h      = %u", mAcdkCctMhalPrvParam.frmParam.h);
    ACDK_LOGD("frmParam.orientation = %d", mAcdkCctMhalPrvParam.frmParam.orientation);
    ACDK_LOGD("frmParam.flip   = %d", mAcdkCctMhalPrvParam.frmParam.flip);
    ACDK_LOGD("frmParam.stride[0] = %u", mAcdkCctMhalPrvParam.frmParam.stride[0]);
    ACDK_LOGD("frmParam.stride[1] = %u", mAcdkCctMhalPrvParam.frmParam.stride[1]);
    ACDK_LOGD("frmParam.stride[2] = %u", mAcdkCctMhalPrvParam.frmParam.stride[2]);
    ACDK_LOGD("frmParam.frmFormat = 0x%x", mAcdkCctMhalPrvParam.frmParam.frmFormat);

    //====== AcdkMhal PreviewStart ======

    err = m_pAcdkCctMhalObj->acdkCctMhalPreviewStart(&mAcdkCctMhalPrvParam);
    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("preview start fail(err=0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
    }

startPreviewExit:

    ACDK_LOGD("-");

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::stopPreview()
{
    ACDK_LOGD("+");

    MINT32 err = ACDK_RETURN_NO_ERROR;

    //====== AcdkMhal PreviewStop ======

    err = m_pAcdkCctMhalObj->acdkCctMhalPreviewStop();
    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("stopPreview Fail(err=0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
    }

    //====== Surface Uninit ======

    #if 0   //cotta--Zaikuo : ION no need to do this
    ACDK_LOGD("unregisterBuffer addr = 0x%x", mSurfaceIMemInfo.virtAddr);
    err = m_pAcdkSurfaceViewObj->unRegisterBuffer( mSurfaceIMemInfo.virtAddr);
    if (err != 0)
    {
        ACDK_LOGE("m_pAcdkSurfaceViewObj->registerBuffer() fail err = 0x%x ", err);
    }
    #endif

    //====== Free Memory ======
    if(mPrvIMemInfo[0].virtAddr != 0)
    {
        destroyMemBuf(OVERLAY_BUFFER_CNT, mPrvIMemInfo);
    }
    if(mPrvIMemInfoRRZO[0].virtAddr != 0)
    {
        destroyMemBuf(OVERLAY_BUFFER_CNT, mPrvIMemInfoRRZO);
    }

    if(mDispIMemInfo[0].virtAddr != 0)
    {
        destroyMemBuf(OVERLAY_BUFFER_CNT, mDispIMemInfo);
    }

    for(int i = 0; i < OVERLAY_BUFFER_CNT; ++i)
    {
       mPrvIMemInfo[i].size = mPrvIMemInfo[i].virtAddr = mPrvIMemInfo[i].phyAddr = 0;
       mPrvIMemInfo[i].memID = -5;

       mPrvIMemInfoRRZO[i].size = mPrvIMemInfoRRZO[i].virtAddr = mPrvIMemInfoRRZO[i].phyAddr = 0;
       mPrvIMemInfoRRZO[i].memID = -5;

       mDispIMemInfo[i].size = mDispIMemInfo[i].virtAddr = mDispIMemInfo[i].phyAddr = 0;
       mDispIMemInfo[i].memID = -5;
    }

    //====== Set State ======

    acdkCctMainSetState(ACDK_CCT_MAIN_IDLE);

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::GetPureRaw(
                MUINT32 const mode,
                MUINT32 const imgType,
                Func_CB const capCb,
                MUINT32 const width,
                MUINT32 const height,
                MUINT32 const captureCnt,
                MINT32  const isSaveImg)
{
    ACDK_LOGD("+");

    //====== Local Variable ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    MUINT32 sensorScenarioId;
    //VIDO output format depend on OVL used  (eImgFmt_RGB565 /eImgFmt_YUY2)
    MUINT32 qvFormat = eImgFmt_YUY2, capFormat = eImgFmt_YUY2;
    MUINT32 rawType = 0;
    MUINT32 rawBit = 10;//mSensorFormatInfo.u4BitDepth

    //====== Check State ======

    err = stateMgr(ACDK_CCT_MAIN_CAPTURE);
    if(err == ACDK_RETURN_ERROR_STATE)
    {
        ACDK_LOGD("warning: redundent command. protect!!");
        return ACDK_RETURN_NO_ERROR;
    }
    else if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("stateMgr fail(0x%x)",err);
        return err;
    }

    ACDK_LOGD("mCapWidth(%u),mCapHeight(%u)",mCapWidth,mCapHeight);

    // sensor scenario
    setScenario(mode, sensorScenarioId);

    ACDK_LOGD("mode(%u),sensorScenarioId(%u)", mode, sensorScenarioId);

    //====== AF ======
    err = m_pAcdkCctMhalObj->acdkCctMhalCaptureStart(NULL);   // do AF
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("m_pAcdkCctMhalObj->acdkCctMhalCaptureStart(err=0x%x)",err);
        return ACDK_RETURN_API_FAIL;
    }

    //====== Precapture Process ======

    // Do pre capture before stop preview, for 3A
    ACDK_LOGD("change to precapture state");

    err = m_pAcdkCctMhalObj->acdkCctMhalPreCapture();
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("m_pAcdkCctMhalObj->acdkCctMhalPreCapture fail err(0x%x)", err);
        return ACDK_RETURN_API_FAIL;
    }

    ACDK_LOGD("wait readyForCap");

    while(m_pAcdkCctMhalObj->acdkCctMhalReadyForCap() == MFALSE)
    {
        usleep(200);
    }

    //====== Stop Preview ======

    ACDK_LOGD("stop preview");


    memcpy((MUINT8*)mCapIMemInfo.virtAddr,PureRaw_BufAddr,mCapIMemInfo.size);

    err = stopPreview();
    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("stopPreview fail, err(0x%x)",err);
        return ACDK_RETURN_API_FAIL;
    }


    // AcdkMhal
    m_pAcdkCctMhalObj->acdkCctMhalSetState(ACDK_CCT_MHAL_CAPTURE);

    // AcdkCctMain
    acdkCctMainSetState(ACDK_CCT_MAIN_CAPTURE);

    //===== Set ISP Tuning Parameter and AE Capture Mode ======
    ACDK_LOGD("mCapWidth(%u),mCapHeight(%u)",mCapWidth,mCapHeight);

    // sensor scenario
   setScenario(mode, sensorScenarioId);

    ACDK_LOGD("mode(%u),sensorScenarioId(%u)",mode,sensorScenarioId);
    //====== Set to Capture State =====

    mCapType = imgType;
    ACDK_LOGD("mCapType(0x%x)", mCapType);

    if(1)
    {
        ImageBufInfo acdkCapInfo;
        acdkCapInfo.eImgType = (eACDK_CAP_FORMAT)mCapType;
        // PURE_RAW8_TYPE =0x04, PURE_RAW10_TYPE = 0x08
        // PURE_RAW10_TYPE = 0x10, PROCESSED_RAW10_TYPE = 0x20
        if(mCapType & 0x3C)
        {
            if(mSensorType == SENSOR_TYPE_YUV) //YUV
            {
                acdkCapInfo.eImgType             = YUV_TYPE;
                acdkCapInfo.imgBufInfo.bufAddr  = (MUINT8 *)mCapIMemInfo.virtAddr;
                //acdkCapInfo.imgBufInfo.imgSize   = mRawIMemInfo.size;
            setDimension(mode, acdkCapInfo.imgBufInfo.imgWidth, acdkCapInfo.imgBufInfo.imgHeight);

                switch(mSensorFormatInfo.u4Order)
                {
                    case SENSOR_OUTPUT_FORMAT_UYVY :
                    case SENSOR_OUTPUT_FORMAT_CbYCrY :
                            acdkCapInfo.imgBufInfo.imgFmt= YUVFmt_UYVY;
                        break;
                    case SENSOR_OUTPUT_FORMAT_VYUY :
                    case SENSOR_OUTPUT_FORMAT_CrYCbY :
                            acdkCapInfo.imgBufInfo.imgFmt= YUVFmt_VYUY;
                        break;
                    case SENSOR_OUTPUT_FORMAT_YUYV :
                    case SENSOR_OUTPUT_FORMAT_YCbYCr :
                            acdkCapInfo.imgBufInfo.imgFmt= YUVFmt_YUY2;
                        break;
                    case SENSOR_OUTPUT_FORMAT_YVYU :
                    case SENSOR_OUTPUT_FORMAT_YCrYCb :
                            acdkCapInfo.imgBufInfo.imgFmt= YUVFmt_YVYU;
                        break;
                    default : acdkCapInfo.imgBufInfo.imgFmt = YUVFmt_Unknown;
                              ACDK_LOGE("unknown YUV type(0x%x)",mSensorFormatInfo.u4Order);
                }
                acdkCapInfo.imgBufInfo.imgSize = acdkCapInfo.imgBufInfo.imgWidth*acdkCapInfo.imgBufInfo.imgHeight*2;
                ACDK_LOGD("YUVImg.bufAddr     = %p", acdkCapInfo.imgBufInfo.bufAddr);
                ACDK_LOGD("YUVImg.imgWidth    = %u", acdkCapInfo.imgBufInfo.imgWidth);
                ACDK_LOGD("YUVImg.imgHeight   = %u", acdkCapInfo.imgBufInfo.imgHeight);
                ACDK_LOGD("YUVImg.imgSize     = %u", acdkCapInfo.imgBufInfo.imgSize);
                ACDK_LOGD("YUVImg.imgFmt      = %d", acdkCapInfo.imgBufInfo.imgFmt);
            }
            else
            {
            setDimension(mode,acdkCapInfo.RAWImgBufInfo.imgWidth,acdkCapInfo.RAWImgBufInfo.imgHeight);

                acdkCapInfo.RAWImgBufInfo.bufAddr   = (MUINT8 *)mCapIMemInfo.virtAddr;
                acdkCapInfo.RAWImgBufInfo.imgSize   = acdkCapInfo.RAWImgBufInfo.imgWidth*acdkCapInfo.RAWImgBufInfo.imgHeight*10/8;
                acdkCapInfo.RAWImgBufInfo.isPacked  = MTRUE;

                acdkCapInfo.RAWImgBufInfo.bitDepth  = mSensorFormatInfo.u4BitDepth;
                acdkCapInfo.RAWImgBufInfo.eColorOrder = (eRAW_ColorOrder)mSensorFormatInfo.u4Order;

                ACDK_LOGD("RAWImg.bufAddr     = %p", acdkCapInfo.RAWImgBufInfo.bufAddr);
                ACDK_LOGD("RAWImg.bitDepth    = %u", acdkCapInfo.RAWImgBufInfo.bitDepth);
                ACDK_LOGD("RAWImg.imgWidth    = %u", acdkCapInfo.RAWImgBufInfo.imgWidth);
                ACDK_LOGD("RAWImg.imgHeight   = %u", acdkCapInfo.RAWImgBufInfo.imgHeight);
                ACDK_LOGD("RAWImg.imgSize     = %u", acdkCapInfo.RAWImgBufInfo.imgSize);
                ACDK_LOGD("RAWImg.isPacked    = %u", acdkCapInfo.RAWImgBufInfo.isPacked);
                ACDK_LOGD("RAWImg.eColorOrder = %u", acdkCapInfo.RAWImgBufInfo.eColorOrder);
            }

            char value[PROPERTY_VALUE_MAX] = {'\0'};
            property_get("vendor.camera.acdkdump.enable", value, "0");
            MINT32 dumpEnable = atoi(value);
            if(dumpEnable == 4)
            {
                char szFileName[256];
                MINT32 i4WriteCnt = 0;
                MINT32 cnt = 0;
                if(mSensorType == SENSOR_TYPE_YUV) //YUV
                    sprintf(szFileName, "%s/acdkCapPureRaw.yuv",MEDIA_PATH);
                else
                    sprintf(szFileName, "%s/acdkCapPureRaw.raw",MEDIA_PATH);

                //====== Write RAW Data ======

                FILE *pFp = fopen(szFileName, "wb");

                if(NULL == pFp )
                {
                    ACDK_LOGE("Can't open file to save image");
                    return ACDK_RETURN_NULL_OBJ;
                }
                if(mSensorType == SENSOR_TYPE_YUV) //YUV
                    i4WriteCnt = fwrite((void *)acdkCapInfo.imgBufInfo.bufAddr, 1, acdkCapInfo.imgBufInfo.imgSize, pFp);
                else
                    i4WriteCnt = fwrite((void *)acdkCapInfo.RAWImgBufInfo.bufAddr, 1, acdkCapInfo.RAWImgBufInfo.imgSize, pFp);

                fflush(pFp);

                if(0 != fsync(fileno(pFp)))
                {
                    ACDK_LOGE("fync fail");
                    fclose(pFp);
                    return ACDK_RETURN_API_FAIL;
                }

                ACDK_LOGD("Save image file name:%s, w(%d), h(%d)", szFileName, mPrvWidth, mPrvHeight);
                fclose(pFp);
            }



        }
        capCb(&acdkCapInfo);
    }

    //====== Uninit and Release ======
    // free memory
    if(mRawIMemInfo.virtAddr != 0)
    {
        destroyMemBuf(1, &mRawIMemInfo);
        mRawIMemInfo.size = mRawIMemInfo.virtAddr = mRawIMemInfo.phyAddr = 0;
        mRawIMemInfo.memID = -5;
    }

    if(mJpgIMemInfo.virtAddr != 0)
    {
        destroyMemBuf(1, &mJpgIMemInfo);

        mJpgIMemInfo.size = mJpgIMemInfo.virtAddr = mJpgIMemInfo.phyAddr = 0;
        mJpgIMemInfo.memID = -5;
    }

    if(mQvIMemInfo.virtAddr != 0)
    {
        destroyMemBuf(1, &mQvIMemInfo);

        mQvIMemInfo.size = mQvIMemInfo.virtAddr = mQvIMemInfo.phyAddr = 0;
        mQvIMemInfo.memID = -5;
    }
    if(mCapIMemInfo.virtAddr != 0)
    {
        destroyMemBuf(1, &mCapIMemInfo);

        mCapIMemInfo.size = mCapIMemInfo.virtAddr = mCapIMemInfo.phyAddr = 0;
        mCapIMemInfo.memID = -5;
    }

    //====== Stop Capture ======

    err = m_pAcdkCctMhalObj->acdkCctMhalCaptureStop();
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("m_pAcdkCctMhalObj->acdkCctMhalCaptureStop(err=0x%x)",err);
        return ACDK_RETURN_API_FAIL;
    }

    //====== Set to IDLE State ======

    // AcdkMhak
    m_pAcdkCctMhalObj->acdkCctMhalSetState(ACDK_CCT_MHAL_IDLE);

    // AcdkCctMain
    acdkCctMainSetState(ACDK_CCT_MAIN_IDLE);

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::startPreview_CameraDev(Func_CB prvCb,  MUINT32 const mode)
{
    ACDK_LOGD("+");

    MINT32 err = ACDK_RETURN_NO_ERROR;
    MINT32 continuous = 1;
    MUINT32 sensorImgSize = 0;
    MUINT32 RRZOImgSize = 0;
    MUINT32 RRZOWidth = 0;
    MUINT32 FormatType =0;
    MUINT32 displaySize, sensorFormatRaw;
    MUINT32 dispalyFormat = eImgFmt_YUY2;
    MUINT32 sensorStride[3] = {0}, RRZOStride[3] = {0},dispalyStride[3] = {0};
    MUINT32 sensor_mode = CAPTURE_MODE; // refer to startPreview's setting

    //====== Check and Set State ======
    err = stateMgr(ACDK_CCT_MAIN_PREVIEW);
    if(err == ACDK_RETURN_ERROR_STATE)
    {
        ACDK_LOGD("warning: redundent command. protect!!");
        return ACDK_RETURN_NO_ERROR;
    }
    else if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("stateMgr fail(0x%x)",err);
        return err;
    }

    acdkCctMainSetState(ACDK_CCT_MAIN_PREVIEW);

    err = sensorFormatSetting(sensor_mode,FormatType,sensorFormatRaw,sensorImgSize,&sensorStride[0]);

    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("sensorFormatSetting fail(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto startPreviewExit;
    }

    //====== Allocate Memory ======
    //calculate display frame buffer size and stride
    m_pAcdkUtilityObj->queryImageSize(dispalyFormat,mPrvWidth,mPrvHeight,displaySize);

    ACDK_LOGD("sensorImgSize(%u),displaySize(%u)",sensorImgSize,displaySize);

    //video output buffer for preview
    if(mDispIMemInfo[0].virtAddr == 0)
    {
        createMemBuf(displaySize,OVERLAY_BUFFER_CNT, mDispIMemInfo);     // pass2 - video out
    }

    for(MINT32 i = 0; i < OVERLAY_BUFFER_CNT; ++i)
    {
         ACDK_LOGD("dispVA[%d] = %p, dispPA[%d] = %p, dispmemID[%d] = %p",i,(void*)mDispIMemInfo[i].virtAddr,i,(void*)mDispIMemInfo[i].phyAddr,i,mDispIMemInfo[i].memID);
    }

    //====== Allocate Memory ======
    sensorImgSize = sensorStaticInfo[0].captureWidth*sensorStaticInfo[0].captureHeight*2;
     if(mCapIMemInfo.virtAddr == 0)
    {
        createMemBuf(sensorImgSize, 1, &mCapIMemInfo); // Raw buffer
     }
    ACDK_LOGD("mCapIMemInfo : size(capturesize) = %u",mCapIMemInfo.size);

    if(mSurfaceIMemInfo[0].virtAddr == 0)
    {
        createMemBuf(displaySize, SURFACE_NUM, mSurfaceIMemInfo);  // surface - frame buffer
    }
    ACDK_LOGD("mSurfaceIMemInfo : size(displaySize) = %u",mSurfaceIMemInfo[0].size);
    for(MINT32 i = 0; i < SURFACE_NUM; ++i)
    {
        ACDK_LOGD("surfaceVA[%d] = %p, surfacePA[%d] = %p",i,mSurfaceIMemInfo[i].virtAddr,i,mSurfaceIMemInfo[i].phyAddr);
    }

    // init value
    for(MINT32 i = 0; i < SURFACE_NUM; ++i)
    {
        if(0 == mSurfaceIMemInfo[i].virtAddr && 0 == mSurfaceIMemInfo[i].phyAddr)
        {
            ACDK_LOGE("mSurfaceIMemInfo[%d] Get the memory fail",i);
            err = ACDK_RETURN_MEMORY_ERROR;
            goto startPreviewExit;
        }
        else
        {
            //for OVL output using rgb565 = 0x0000 -->black color
            //for OVL output using yuv422 = 0x0000 --> Green color , 0x0080 --> Black color
            //only first frame reset
            if(mFrameCnt == 0)
            {
                memset((MVOID *)mSurfaceIMemInfo[i].virtAddr, 0, mSurfaceIMemInfo[i].size);
            }
         }
    }

    //====== Preview Parameter Setting ======
    memset(&mAcdkCctMhalPrvParam, 0, sizeof(acdkCctMhalPrvParam_t));

    //preview param setting
    //mAcdkCctMhalPrvParam.scenarioHW         = eScenarioID_VSS;
    mAcdkCctMhalPrvParam.sensorID          = mSensorDev; //Main / sub /Main2
    mAcdkCctMhalPrvParam.sensorIndex       = mSensorIndex; //Sensor Index
    //mAcdkCctMhalPrvParam.mSensorPixelMode  = sensorDynamicInfo[0].pixelMode;
    mAcdkCctMhalPrvParam.sensorWidth       = mSensorResolution.SensorFullWidth;
    mAcdkCctMhalPrvParam.sensorHeight      = mSensorResolution.SensorFullHeight;
    mAcdkCctMhalPrvParam.sensorFullWidth   = mSensorResolution.SensorFullWidth;
    mAcdkCctMhalPrvParam.sensorFullHeight  = mSensorResolution.SensorFullHeight;
    mAcdkCctMhalPrvParam.rrzoWidth         = RRZOWidth;
    mAcdkCctMhalPrvParam.sensorStride[0]   = sensorStride[0];
    mAcdkCctMhalPrvParam.sensorStride[1]   = sensorStride[1];
    mAcdkCctMhalPrvParam.sensorStride[2]   = sensorStride[2];
    mAcdkCctMhalPrvParam.rrzoStride[0]     = RRZOStride[0];
    mAcdkCctMhalPrvParam.rrzoStride[1]     = RRZOStride[1];
    mAcdkCctMhalPrvParam.rrzoStride[2]     = RRZOStride[2];
    mAcdkCctMhalPrvParam.sensorType        = mSensorType;
    mAcdkCctMhalPrvParam.sensorFormat      = sensorFormatRaw;
    mAcdkCctMhalPrvParam.sensorColorOrder  = mSensorFormatInfo.u4Order;
    mAcdkCctMhalPrvParam.mu4SensorDelay    = mu4SensorDelay;
    mAcdkCctMhalPrvParam.imgImemBuf        = mPrvIMemInfo;
    mAcdkCctMhalPrvParam.rrzoImemBuf       = mPrvIMemInfoRRZO;
    mAcdkCctMhalPrvParam.dispImemBuf       = mDispIMemInfo;
    mAcdkCctMhalPrvParam.frmParam.w        = mPrvWidth;
    mAcdkCctMhalPrvParam.frmParam.h        = mPrvHeight;
    mAcdkCctMhalPrvParam.frmParam.flip     = mSensorHFlip ? MTRUE : MFALSE;
    mAcdkCctMhalPrvParam.frmParam.stride[0] = dispalyStride[0];
    mAcdkCctMhalPrvParam.frmParam.stride[1] = dispalyStride[1];
    mAcdkCctMhalPrvParam.frmParam.stride[2] = dispalyStride[2];
    mAcdkCctMhalPrvParam.frmParam.frmFormat = dispalyFormat;
    mAcdkCctMhalPrvParam.acdkMainObserver  = acdkObserver(cameraCallback, this);
    mAcdkCctMhalPrvParam.mHDR_EN           = (MUINT32)mHDR_EN;
    setScenario( mode, mAcdkCctMhalPrvParam.PreviewSensorScenario);
    mAcdkCctMhalPrvParam.IsFactoryMode = !!mIsFacotory;

    switch(mOrientation) {   /* TBD fixme */
        case 0 : mAcdkCctMhalPrvParam.frmParam.orientation = 0;
            break;
        case 90 : mAcdkCctMhalPrvParam.frmParam.orientation = NSCam::eTransform_ROT_90;
            break;
        case 180 : mAcdkCctMhalPrvParam.frmParam.orientation = NSCam::eTransform_ROT_180;
            break;
        case 270 : mAcdkCctMhalPrvParam.frmParam.orientation = NSCam::eTransform_ROT_270;
            break;
    }

    ACDK_LOGD("scenarioHW      = %d", (MINT32)mAcdkCctMhalPrvParam.scenarioHW);
    ACDK_LOGD("sensorID        = %d", (MINT32)mAcdkCctMhalPrvParam.sensorID);
    //ACDK_LOGD("mSensorPixelMode= %d", (MINT32)mAcdkCctMhalPrvParam.mSensorPixelMode);
    ACDK_LOGD("sensorWidth     = %u", mAcdkCctMhalPrvParam.sensorWidth);
    ACDK_LOGD("sensorHeight    = %u", mAcdkCctMhalPrvParam.sensorHeight);
    ACDK_LOGD("rrzoWidth       = %u", mAcdkCctMhalPrvParam.rrzoWidth);
    ACDK_LOGD("sensorStride[0] = %u", mAcdkCctMhalPrvParam.sensorStride[0]);
    ACDK_LOGD("sensorStride[1] = %u", mAcdkCctMhalPrvParam.sensorStride[1]);
    ACDK_LOGD("sensorStride[2] = %u", mAcdkCctMhalPrvParam.sensorStride[2]);
    ACDK_LOGD("rrzoStride[0]   = %u", mAcdkCctMhalPrvParam.rrzoStride[0]);
    ACDK_LOGD("rrzoStride[1]   = %u", mAcdkCctMhalPrvParam.rrzoStride[1]);
    ACDK_LOGD("rrzoStride[2]   = %u", mAcdkCctMhalPrvParam.rrzoStride[2]);
    ACDK_LOGD("sensorType      = %u", mAcdkCctMhalPrvParam.sensorType);
    ACDK_LOGD("sensorFormat    = 0x%x", mAcdkCctMhalPrvParam.sensorFormat);
    ACDK_LOGD("colorOrder      = %u", mAcdkCctMhalPrvParam.sensorColorOrder);
    ACDK_LOGD("frmParam.w      = %u", mAcdkCctMhalPrvParam.frmParam.w);
    ACDK_LOGD("frmParam.h      = %u", mAcdkCctMhalPrvParam.frmParam.h);
    ACDK_LOGD("frmParam.orientation = %d", mAcdkCctMhalPrvParam.frmParam.orientation);
    ACDK_LOGD("frmParam.flip   = %d", mAcdkCctMhalPrvParam.frmParam.flip);
    ACDK_LOGD("frmParam.stride[0] = %u", mAcdkCctMhalPrvParam.frmParam.stride[0]);
    ACDK_LOGD("frmParam.stride[1] = %u", mAcdkCctMhalPrvParam.frmParam.stride[1]);
    ACDK_LOGD("frmParam.stride[2] = %u", mAcdkCctMhalPrvParam.frmParam.stride[2]);
    ACDK_LOGD("frmParam.frmFormat = 0x%x", mAcdkCctMhalPrvParam.frmParam.frmFormat);
    ACDK_LOGD("PreviewSensorScenario = %d", mAcdkCctMhalPrvParam.PreviewSensorScenario);
    //====== AcdkMhal PreviewStart ======

    err = m_pAcdkCctMhalObjEng->acdkCctMhalPreviewStart(&mAcdkCctMhalPrvParam);
    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("preview start fail(err=0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
    }

startPreviewExit:

    ACDK_LOGD("-");

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::stopPreview_CameraDev()
{
    ACDK_LOGD("+");

    MINT32 err = ACDK_RETURN_NO_ERROR;

    //====== AcdkMhal PreviewStop ======

    err = m_pAcdkCctMhalObjEng->acdkCctMhalPreviewStop();
    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("stopPreview Fail(err=0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
    }

    //====== Surface Uninit ======

    //====== Free Memory ======
    if(mPrvIMemInfo[0].virtAddr != 0)
    {
        destroyMemBuf(OVERLAY_BUFFER_CNT, mPrvIMemInfo);
    }
    if(mPrvIMemInfoRRZO[0].virtAddr != 0)
    {
        destroyMemBuf(OVERLAY_BUFFER_CNT, mPrvIMemInfoRRZO);
    }

    if(mDispIMemInfo[0].virtAddr != 0)
    {
        destroyMemBuf(OVERLAY_BUFFER_CNT, mDispIMemInfo);
    }

    for(int i = 0; i < OVERLAY_BUFFER_CNT; ++i)
    {
       mPrvIMemInfo[i].size = mPrvIMemInfo[i].virtAddr = mPrvIMemInfo[i].phyAddr = 0;
       mPrvIMemInfo[i].memID = -5;

       mPrvIMemInfoRRZO[i].size = mPrvIMemInfoRRZO[i].virtAddr = mPrvIMemInfoRRZO[i].phyAddr = 0;
       mPrvIMemInfoRRZO[i].memID = -5;

       mDispIMemInfo[i].size = mDispIMemInfo[i].virtAddr = mDispIMemInfo[i].phyAddr = 0;
       mDispIMemInfo[i].memID = -5;
    }

    //====== Set State ======

    acdkCctMainSetState(ACDK_CCT_MAIN_IDLE);

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::takePicture_CameraDev(
                MUINT32 const mode,
                MUINT32 const imgType,
                Func_CB const capCb,
                MUINT32 const width,
                MUINT32 const height,
                MUINT32 const captureCnt,
                MINT32  const isSaveImg)
{
    ACDK_LOGD("+");

    //====== Local Variable ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    MUINT32 sensorScenarioId;
    //VIDO output format depend on OVL used  (eImgFmt_RGB565 /eImgFmt_YUY2)
    MUINT32 qvFormat = eImgFmt_YUY2, capFormat = eImgFmt_YUY2;
    MUINT32 rawType = 0;
    MUINT32 rawBit = 10;//mSensorFormatInfo.u4BitDepth

    //====== Check State ======

    //err = stateMgr(ACDK_CCT_MAIN_CAPTURE);
    if(err == ACDK_RETURN_ERROR_STATE)
    {
        ACDK_LOGD("warning: redundent command. protect!!");
        return ACDK_RETURN_NO_ERROR;
    }
    else if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("stateMgr fail(0x%x)",err);
        return err;
    }

#if CAPTURE_WAIT_AF_DONE == 1
    cam3aParam.u4AfMode = 0;
    mpIHal3A->send3ACtrl(E3ACtrl_Enable3ASetParams, MTRUE, 0);
    mpIHal3A->setParams(cam3aParam);
    mpIHal3A->autoFocus();
    ACDK_LOGD("doing AF");
    usleep(4000000); //wait for AF done.
    mpIHal3A->send3ACtrl(E3ACtrl_Enable3ASetParams, MFALSE, 0);
#endif

    //====== Parameter Setting ======
    // width and height
    if (width != 0 && height != 0)
    {
        mCapWidth  = width;
        mCapHeight = height;

        if((mCapWidth % 16) != 0)
        {
            mCapWidth = mCapWidth & (~0xF);
        }

        if((mCapHeight % 16) != 0)
        {
            mCapHeight = mCapHeight & (~0xF);
        }
    }
    else
    {
        setDimension(mode, mCapWidth, mCapHeight);
    }

    ACDK_LOGD("mCapWidth(%u),mCapHeight(%u)",mCapWidth,mCapHeight);
    if((mCapWidth == 0) || (mCapHeight == 0))
    {
        mCapWidth = 640;mCapHeight = 480;
        ACDK_LOGE("Warning Input w/h is zero!!! Set to default. mCapWidth(%u),mCapHeight(%u)",mCapWidth,mCapHeight);
    }

    // sensor scenario
    setScenario(mode, sensorScenarioId);

    ACDK_LOGD("mode(%u),sensorScenarioId(%u)",mode,sensorScenarioId);

    //====== Precapture Process ======
    // Do pre capture before stop preview, for 3A
    ACDK_LOGD("change to precapture state");

    //err = m_pAcdkCctMhalObj->acdkCctMhalPreCapture();
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("m_pAcdkCctMhalObj->acdkCctMhalPreCapture fail err(0x%x)", err);
        return ACDK_RETURN_API_FAIL;
    }
    ACDK_LOGD("wait readyForCap");

    //while(m_pAcdkCctMhalObj->acdkCctMhalReadyForCap() == MFALSE)
    {
        usleep(200);
    }

    //====== Set to Capture State =====

    // AcdkMhal
    //m_pAcdkCctMhalObj->acdkCctMhalSetState(ACDK_CCT_MHAL_CAPTURE);

    // AcdkCctMain
    //acdkCctMainSetState(ACDK_CCT_MAIN_CAPTURE);
    //====== Capture start ======
    //====== Preview Parameter Setting ======
    memset(&mAcdkCctMhalCapParam, 0, sizeof(acdkCctMhalCapParam_t));

    //Capture param setting
    mAcdkCctMhalCapParam.sensorID           = mSensorDev; //Main / sub /Main2
    mAcdkCctMhalCapParam.sensorIndex        = mSensorIndex; //Sensor Index
    mAcdkCctMhalCapParam.CaptureMode        = sensorScenarioId;
    mAcdkCctMhalCapParam.mCapType           = imgType;
    mAcdkCctMhalCapParam.mCapWidth          = mCapWidth;
    mAcdkCctMhalCapParam.mCapHeight         = mCapHeight;
    mAcdkCctMhalCapParam.sensorFullWidth    = mSensorResolution.SensorFullWidth;
    mAcdkCctMhalCapParam.sensorFullHeight   = mSensorResolution.SensorFullHeight;
    mAcdkCctMhalCapParam.acdkMainObserver   = acdkObserver(cameraCallback, this);
    mAcdkCctMhalCapParam.mMFLL_EN           = (MUINT32)mMFLL_EN;
    mAcdkCctMhalCapParam.mHDR_EN           = (MUINT32)mHDR_EN;
    mAcdkCctMhalCapParam.mEMultiNR          = (MUINT32)mEMultiNR;
    err = m_pAcdkCctMhalObjEng->acdkCctMhalCaptureStart(&mAcdkCctMhalCapParam);   // do AF
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("m_pAcdkCctMhalObjEng->acdkCctMhalCaptureStart(err=0x%x)",err);
        return ACDK_RETURN_API_FAIL;
    }

    //===== Set ISP Tuning Parameter and AE Capture Mode ======

    ACDK_LOGD("mOperaMode(%u)", mOperaMode);

    //====== Single Shot Process ======

    mCapType = imgType;
    ACDK_LOGD("mCapType(0x%x)", mCapType);

    //====== Stop Capture ======
    err = m_pAcdkCctMhalObjEng->acdkCctMhalCaptureStop();
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("m_pAcdkCctMhalObjEng->acdkCctMhalCaptureStop(err=0x%x)",err);
        return ACDK_RETURN_API_FAIL;
    }
    //====== Capture Callback ======
    if(capCb != NULL)
    {
        ImageBufInfo acdkCapInfo;
        acdkCapInfo.eImgType = (eACDK_CAP_FORMAT)mCapType;
        // PURE_RAW8_TYPE =0x04, PURE_RAW10_TYPE = 0x08
        // PURE_RAW10_TYPE = 0x10, PROCESSED_RAW10_TYPE = 0x20
        if(mCapType & 0x3C)
        {
            setDimension(mode,acdkCapInfo.RAWImgBufInfo.imgWidth,acdkCapInfo.RAWImgBufInfo.imgHeight);

            char szFileName[256];
            MINT32 i4WriteCnt = 0;
            MINT32 captureSize =0;
            //Unpack pure raw size = width*height*2
            captureSize = acdkCapInfo.RAWImgBufInfo.imgWidth*acdkCapInfo.RAWImgBufInfo.imgHeight*2;
            if((mCapType & PURE_RAW8_TYPE) || (mCapType & PURE_RAW10_TYPE))
            {
                sprintf(szFileName, "%s/pure__%dx%d_10_%d",MEDIA_PATH,
                    acdkCapInfo.RAWImgBufInfo.imgWidth,acdkCapInfo.RAWImgBufInfo.imgHeight,sensorStaticInfo[0].sensorFormatOrder);

                DIR *dir;
                if ((dir=opendir("/mnt/vendor/cct")) == NULL)
                {
                    ACDK_LOGE("Open dir failed: /mnt/vendor/cct");
                }
                else
                {
                    struct dirent *ptr;
                    int result;
                    while ((ptr=readdir(dir)) != NULL)
                    {
                        char filename[256];
                        char pathname[300];
                        strncpy(filename, ptr->d_name, 255);
                        filename[255] = 0;
                        sprintf(pathname, "%s/%s", "/mnt/vendor/cct", filename);

                        if( filename == strstr(filename, "fcell_dst_") && 0 != strstr(filename, ".raw") ) // start with "fcell_dst_" + contains ".raw"
                        {
                            ACDK_LOGD("copy %s to %s", pathname, szFileName);

                            //read
                            FILE *pFp = fopen(pathname, "rb");
                            int tmpSize, tmpSize2, tmpSize3;
                            if(NULL == pFp )
                            {
                                ACDK_LOGE("Can't open file: %d", pathname);
                                return ACDK_RETURN_NULL_OBJ;
                            }
                            fseek(pFp, 0L, SEEK_END);
                            tmpSize = ftell(pFp);
                            fseek(pFp, 0L, SEEK_SET);
                            void *tmpData = (void*)malloc(tmpSize);
                            if(NULL == tmpData )
                            {
                                ACDK_LOGE("Can't allocate memory");
                                fclose(pFp);
                                return ACDK_RETURN_NULL_OBJ;
                            }
                            tmpSize2 = fread(tmpData, 1, tmpSize, pFp);
                            fclose(pFp);

                            //write
                            FILE *pFp_cct = fopen(szFileName, "wb");
                            if(NULL == pFp_cct )
                            {
                                ACDK_LOGE("Can't open file: %s", szFileName);
                                free(tmpData);
                                return ACDK_RETURN_NULL_OBJ;
                            }
                            tmpSize3 = fwrite( tmpData, 1, tmpSize2, pFp_cct );
                            free(tmpData);

                            //remove
                            result = remove( pathname );
                            ACDK_LOGD("copy done: size1=%d, size2=%d, size3=%d, result=%d", tmpSize, tmpSize2, tmpSize3, result);
                        }
                    }
                    closedir(dir);
                }
            }
            else
            {
                sprintf(szFileName, "%s/proc__%dx%d_10_%d",MEDIA_PATH,
                    acdkCapInfo.RAWImgBufInfo.imgWidth,acdkCapInfo.RAWImgBufInfo.imgHeight,sensorStaticInfo[0].sensorFormatOrder);
            }

            ACDK_LOGD("Save Raw image file name:%s\n", szFileName);
            //====== Read RAW Data ======
            FILE *pFp = fopen(szFileName, "rb");

            if(NULL == pFp )
            {
                ACDK_LOGE("Can't open file to save image");
                //fclose(pFp);
                return ACDK_RETURN_NULL_OBJ;
            }
            i4WriteCnt = fread((void *)mCapIMemInfo.virtAddr, 1, captureSize, pFp);

            fflush(pFp);

            if(0 != fsync(fileno(pFp)))
            {
                ACDK_LOGE("fync fail");
                fclose(pFp);
                return ACDK_RETURN_API_FAIL;
            }
            fclose(pFp);
            // Engineer mode product unpack pure raw data
            if(mUnPack == MTRUE) //for specail mode use : unpack raw image to normal package
            {
                MUINT32 imgStride = mSensorResolution.SensorFullWidth, unPackSize = 0;
                MUINT32 tempFmt, tempRAWPixelByte;
                IMEM_BUF_INFO unPackIMem;

                //calculate stride
                switch(mSensorFormatInfo.u4BitDepth)
                {
                    case 8 : tempFmt  = eImgFmt_BAYER8;
                        break;
                    case 10 : tempFmt = eImgFmt_BAYER10;
                        break;
                    case 12 : tempFmt = eImgFmt_BAYER12;
                        break;
                    default : tempFmt = eImgFmt_UNKNOWN;
                              ACDK_LOGE("unknown raw image bit depth(%u)",mSensorFormatInfo.u4BitDepth);
                              break;
                }

                // calculate real stride and get byte per pixel. for RAW sensor only
                if(ACDK_RETURN_NO_ERROR != m_pAcdkUtilityObj->queryRAWImgFormatInfo(tempFmt,acdkCapInfo.RAWImgBufInfo.imgWidth,imgStride,tempRAWPixelByte))
                {
                    ACDK_LOGE("queryRAWImgFormatInfo fail");
                }

                //prepare memory
                unPackSize = acdkCapInfo.RAWImgBufInfo.imgWidth * acdkCapInfo.RAWImgBufInfo.imgHeight * 2;
                ACDK_LOGD("unPackSize(%u),imgStride(%u)",unPackSize,imgStride);

                createMemBuf(unPackSize, 1, &unPackIMem);

                if(unPackIMem.virtAddr == 0)
                {
                    memset((MVOID *)unPackIMem.virtAddr, 0, unPackIMem.size);
                    ACDK_LOGE("unPackIMem is NULL");
                }
                else
                {
                    m_pAcdkUtilityObj->rawImgUnpack(mRawIMemInfo,
                                                    unPackIMem,
                                                    acdkCapInfo.RAWImgBufInfo.imgWidth,
                                                    acdkCapInfo.RAWImgBufInfo.imgHeight,
                                                    mSensorFormatInfo.u4BitDepth,
                                                    imgStride);

                    acdkCapInfo.RAWImgBufInfo.bufAddr   =  (MUINT8 *)unPackIMem.virtAddr;
                    acdkCapInfo.RAWImgBufInfo.imgSize   = unPackIMem.size;
                    acdkCapInfo.RAWImgBufInfo.isPacked  = MFALSE;
                }


            }
            else
            {
                acdkCapInfo.RAWImgBufInfo.bufAddr   =  (MUINT8 *)mCapIMemInfo.virtAddr;
                acdkCapInfo.RAWImgBufInfo.imgSize   = captureSize;
                acdkCapInfo.RAWImgBufInfo.isPacked  = MFALSE;
            }

            acdkCapInfo.RAWImgBufInfo.bitDepth  = mSensorFormatInfo.u4BitDepth;
            acdkCapInfo.RAWImgBufInfo.eColorOrder = (eRAW_ColorOrder)mSensorFormatInfo.u4Order;

            ACDK_LOGD("RAWImg.bufAddr     = %p", acdkCapInfo.RAWImgBufInfo.bufAddr);
            ACDK_LOGD("RAWImg.bitDepth    = %u", acdkCapInfo.RAWImgBufInfo.bitDepth);
            ACDK_LOGD("RAWImg.imgWidth    = %u", acdkCapInfo.RAWImgBufInfo.imgWidth);
            ACDK_LOGD("RAWImg.imgHeight   = %u", acdkCapInfo.RAWImgBufInfo.imgHeight);
            ACDK_LOGD("RAWImg.imgSize     = %u", acdkCapInfo.RAWImgBufInfo.imgSize);
            ACDK_LOGD("RAWImg.isPacked    = %u", acdkCapInfo.RAWImgBufInfo.isPacked);
            ACDK_LOGD("RAWImg.eColorOrder = %u", acdkCapInfo.RAWImgBufInfo.eColorOrder);
        }
        else if(mCapType == JPEG_TYPE)
        {
            acdkCapInfo.imgBufInfo.bufAddr   = Capture_BufAddr;
            acdkCapInfo.imgBufInfo.imgWidth  = mCapWidth;
            acdkCapInfo.imgBufInfo.imgHeight = mCapHeight;
            acdkCapInfo.imgBufInfo.imgSize   = Capture_Size;

            ACDK_LOGD("imgBufInfo.bufAddr   = %p", acdkCapInfo.imgBufInfo.bufAddr);
            ACDK_LOGD("imgBufInfo.imgWidth  = %u", acdkCapInfo.imgBufInfo.imgWidth);
            ACDK_LOGD("imgBufInfo.imgHeight = %u", acdkCapInfo.imgBufInfo.imgHeight);
            ACDK_LOGD("imgBufInfo.imgSize   = %u", acdkCapInfo.imgBufInfo.imgSize);
        }

        capCb(&acdkCapInfo);
    }

    //====== Stop Preview ======
    ACDK_LOGD("stop preview");
    err = stopPreview_CameraDev();
    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("stopPreview fail, err(0x%x)",err);
       return ACDK_RETURN_API_FAIL;
    }

    //====== Set to IDLE State ======

    // AcdkMhak
    m_pAcdkCctMhalObjEng->acdkCctMhalSetState(ACDK_CCT_MHAL_IDLE);

    // AcdkCctMain
    acdkCctMainSetState(ACDK_CCT_MAIN_IDLE);

    //====== Uninit and Release ======
    // free memory
    if(mRawIMemInfo.virtAddr != 0)
    {
        destroyMemBuf(1, &mRawIMemInfo);
        mRawIMemInfo.size = mRawIMemInfo.virtAddr = mRawIMemInfo.phyAddr = 0;
        mRawIMemInfo.memID = -5;
    }

    if(mJpgIMemInfo.virtAddr != 0)
    {
        destroyMemBuf(1, &mJpgIMemInfo);

        mJpgIMemInfo.size = mJpgIMemInfo.virtAddr = mJpgIMemInfo.phyAddr = 0;
        mJpgIMemInfo.memID = -5;
    }

    if(mQvIMemInfo.virtAddr != 0)
    {
        destroyMemBuf(1, &mQvIMemInfo);

        mQvIMemInfo.size = mQvIMemInfo.virtAddr = mQvIMemInfo.phyAddr = 0;
        mQvIMemInfo.memID = -5;
    }


    ACDK_LOGD("-");

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::getFrameCnt(MUINT32 &frameCnt)
{
    frameCnt = mFrameCnt;
    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::quickViewImg(MUINT32 qvFormat)
{
    //====== Check Input Argument ======
    if(mQvIMemInfo.virtAddr == 0)
    {
        ACDK_LOGE("mQvIMemInfo is empty");
        return ACDK_RETURN_NULL_OBJ;
    }

    //====== Local Variable Setting ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    MUINT8 *pQVBufIn = (MUINT8 *)mQvIMemInfo.virtAddr;

    //====== Dump QV Image (Debug) ======

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.camera.acdkdump.enable", value, "0");
    MINT32 dumpEnable = atoi(value);

    //if(dumpEnable == 5)
    {
        MINT32 i4WriteCnt = 0;
        char szFileName[256];
        sprintf(szFileName, "%s/acdkQV1.yuv" , MEDIA_PATH);

        FILE *pFp = fopen(szFileName, "wb");

        if(NULL == pFp)
        {
            ACDK_LOGE("Can't open file to save image");
            return ACDK_RETURN_NULL_OBJ;
        }

        i4WriteCnt = fwrite(pQVBufIn, 1, mQvIMemInfo.size, pFp);

        fflush(pFp);

        if(0 != fsync(fileno(pFp)))
        {
            ACDK_LOGE("fync fail");
            fclose(pFp);
            return ACDK_RETURN_API_FAIL;
        }

        ACDK_LOGD("Save image file name:%s, w(%d), h(%d)", szFileName, mQVWidth, mQVHeight);

        fclose(pFp);
    }

    //====== Process QV Image ======

    IMEM_BUF_INFO procQvMemInfo;
    createMemBuf(mQvIMemInfo.size, 1, &procQvMemInfo);

    ACDK_LOGD("procQvMemInfo : vir(%p),phy(%p)",(void*)procQvMemInfo.virtAddr,(void*)procQvMemInfo.phyAddr);

    m_pAcdkUtilityObj->imageProcess(qvFormat,
                                    qvFormat,
                                    mQVWidth,
                                    mQVHeight,
                                    mOrientation,
                                    mSensorHFlip,
                                    mQvIMemInfo,
                                    procQvMemInfo);

    //====== Set Surface Parameter ======

    MUINT8 *pQVBufOut =  (MUINT8 *)procQvMemInfo.virtAddr;

    if(dumpEnable == 5)
    {
        MINT32 i4WriteCnt = 0;
        char szFileName[256];
        sprintf(szFileName, "%s/acdkQV2.rgb" , MEDIA_PATH);

        FILE *pFp = fopen(szFileName, "wb");

        if(NULL == pFp)
        {
            ACDK_LOGE("Can't open file to save image");
            return ACDK_RETURN_NULL_OBJ;
        }

        i4WriteCnt = fwrite(pQVBufOut, 1, procQvMemInfo.size, pFp);

        fflush(pFp);

        if(0 != fsync(fileno(pFp)))
        {
            ACDK_LOGE("fync fail");
            fclose(pFp);
            return ACDK_RETURN_API_FAIL;
        }

        ACDK_LOGD("Save image file name:%s, w(%d), h(%d)", szFileName, mQVWidth, mQVHeight);

        fclose(pFp);
    }

    ACDK_LOGD("QV VA(%p)",pQVBufOut);
    ACDK_LOGD("surfaceIndex[%u],VA(%p),PA(%p)",mSurfaceIndex,(void*)mSurfaceIMemInfo[mSurfaceIndex].virtAddr,(void*)mSurfaceIMemInfo[mSurfaceIndex].phyAddr);

    // show frame by overlay
    err = m_pAcdkSurfaceViewObj->setOverlayInfo(0,
                                                mPrvStartX,
                                                mPrvStartY,
                                                mQVWidth,
                                                mQVHeight,
                                                mSurfaceIMemInfo[mSurfaceIndex].phyAddr,
                                                mSurfaceIMemInfo[mSurfaceIndex].virtAddr,
                                                mOrientation);

    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("setOverlayInfo fail err(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto quickViewImgExit;
    }

    ACDK_LOGD("qv size(%u)", procQvMemInfo.size);
    ACDK_LOGD("qv size(%u)", mQvIMemInfo.size);
    //memcpy((MVOID *)mSurfaceIMemInfo[mSurfaceIndex].virtAddr, pQVBufOut, procQvMemInfo.size);
    memcpy( (MUINT8 *)mSurfaceIMemInfo[mSurfaceIndex].virtAddr, (MUINT8 *)mQvIMemInfo.virtAddr, mQvIMemInfo.size);


    //m_pIMemDrv->cacheFlushAll();
    m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &mSurfaceIMemInfo[mSurfaceIndex]);

    err = m_pAcdkSurfaceViewObj->refresh();
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("refresh fail err(0x%x)", err);
        err = ACDK_RETURN_API_FAIL;
    }

    //update surface index
    mSurfaceIndex = (mSurfaceIndex + 1) % SURFACE_NUM;

quickViewImgExit:

    if(procQvMemInfo.virtAddr != 0 )
    {
        destroyMemBuf(1, &procQvMemInfo);
    }

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::setSrcDev(MINT32 srcDev)
{
    ACDK_LOGD("srcDev(%d)",srcDev);

    MINT32 err = ACDK_RETURN_NO_ERROR;

    //====== Switch sensor ======

    switch(srcDev)
    {
        case SENSOR_DEV_MAIN : //main
            mSensorDev = SENSOR_DEV_MAIN;
            mSensorVFlip = 0;
            mSensorHFlip = 0;
            break;
        case SENSOR_DEV_SUB : //sub
            mSensorDev = SENSOR_DEV_SUB;
            mSensorVFlip = 0;
            mSensorHFlip = 1;
            break;
        //case SENSOR_DEV_ATV : //atv
        //    mSensorDev = SENSOR_DEV_ATV;
        //    mSensorVFlip = 0;
        //    mSensorHFlip = 0;
        //    break;
        case SENSOR_DEV_MAIN_2 : //main2
            mSensorDev = SENSOR_DEV_MAIN_2;
            mSensorVFlip = 0;
            mSensorHFlip = 0;
            break;
        default:
            ACDK_LOGE("wrong mSensorDev = %d", srcDev);
            err = ACDK_RETURN_INVALID_SENSOR;
    }

    //====== Get Sensro Info ======

    if(mSensorInit == MTRUE)
    {
        err = getSensorInfo();
        if(err != ACDK_RETURN_NO_ERROR)
        {
            ACDK_LOGE("getSensorInfo error(0x%x)",err);
            err = ACDK_RETURN_API_FAIL;
        }
    }

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::setOperMode(eACDK_OPERA_MODE eOperMode)
{
    ACDK_LOGD("eOperMode(%d)", eOperMode);
    mOperaMode = eOperMode;
    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::setShutterTime(MUINT32 a_Time)
{
    ACDK_LOGD("time(%u)",a_Time);

    mSetShutTime = a_Time;

    return ACDK_RETURN_NO_ERROR;
}
/*******************************************************************************
*
********************************************************************************/

MBOOL AcdkCctMain::makeExifHeader(MUINT32 const u4CamMode,
                                    MUINT8* const puThumbBuf,
                                    MUINT32 const u4ThumbSize,
                                    MUINT8* puExifBuf,
                                    MUINT32 &u4FinalExifSize,
                                    MUINT32 u4ImgIndex,
                                    MUINT32 u4GroupId)

{

   ACDK_LOGD("+ (u4CamMode, puThumbBuf, u4ThumbSize, puExifBuf) = (%d, %p, %d, %p)",
                           u4CamMode,  puThumbBuf, u4ThumbSize, puExifBuf);

   if (u4ThumbSize > 63 * 1024)
   {
       ACDK_LOGD("The thumbnail size is large than 63K, the exif header will be broken");
   }
   bool ret = true;
#if 0

   uint32_t u4App1HeaderSize = 0;
   uint32_t u4AppnHeaderSize = 0;

   uint32_t exifHeaderSize = 0;
   CamExif rCamExif;
   CamExifParam rExifParam;
   CamDbgParam rDbgParam;
   MUINT32 capFormat = eImgFmt_YUY2;

   // shot param
   ShotParam rShotParam((EImageFormat)capFormat,   // yuv format
                         mCapWidth,                // picutre width
                         mCapHeight,               // picture height
                         0,                        // picture rotation (mOrientation)
                         0,                        // picture flip => single shot not support
                         eImgFmt_YUY2,             // postview format
                         mQVWidth,                 // postview width
                         mQVHeight,                // postview height
                         0,                        // postview rotation => no use. acdkMain should handle rotation by itself
                         0,                        // postview flip => single shot not support
                         100);                     // u4ZoomRatio

   // the bitstream already rotated. rotation should be 0
   rExifParam.u4Orientation = 0;
   rExifParam.u4ZoomRatio = 100;
   //
   //camera_info rCameraInfo = MtkCamUtils::DevMetaInfo::queryCameraInfo(getOpenId());
   rExifParam.u4Facing = mSensorDev;//Main/Sub Camera
   //
   rExifParam.u4ImgIndex = u4ImgIndex;
   rExifParam.u4GroupId = u4GroupId;
   //
   rExifParam.u4FocusH = 0;
   rExifParam.u4FocusL = 0;
   //
   //! CamDbgParam (for camMode, shotMode)
   rDbgParam.u4CamMode = u4CamMode;
   rDbgParam.u4ShotMode = 0;//getShotMode();
   //
   rCamExif.init(rExifParam,  rDbgParam);

   Hal3AIf* p3AHal = Hal3AIf::createInstance((halSensorDev_e)mSensorDev);
   SensorHal* pSensorHal = SensorHal::createInstance();

   p3AHal->set3AEXIFInfo(&rCamExif);
   // the bitstream already rotated. it need to swap the width/height
   if (90 == rShotParam.u4PictureRotation || 270 == rShotParam.u4PictureRotation)
   {
       rCamExif.makeExifApp1(rShotParam.u4PictureHeight,  rShotParam.u4PictureWidth, u4ThumbSize, puExifBuf,  &u4App1HeaderSize);
   }
   else
   {
       rCamExif.makeExifApp1(rShotParam.u4PictureWidth, rShotParam.u4PictureHeight, u4ThumbSize, puExifBuf,  &u4App1HeaderSize);
   }
   // copy thumbnail image after APP1
   MUINT8 *pdest = puExifBuf + u4App1HeaderSize;
   ::memcpy(pdest, puThumbBuf, u4ThumbSize) ;
   //
   // 3A Debug Info
   p3AHal->setDebugInfo(&rCamExif);
   //
   // Sensor Debug Info
   pSensorHal->setDebugInfo(&rCamExif);
   pdest = puExifBuf + u4App1HeaderSize + u4ThumbSize;
   //
   rCamExif.appendDebugExif(pdest, &u4AppnHeaderSize);
   rCamExif.uninit();

   u4FinalExifSize = u4App1HeaderSize + u4ThumbSize + u4AppnHeaderSize;
   p3AHal->destroyInstance();
   pSensorHal->destroyInstance();

   ACDK_LOGD("- (app1Size, appnSize, exifSize) = (%d, %d, %d)",
                         u4App1HeaderSize, u4AppnHeaderSize, u4FinalExifSize);
#endif
   return ret;
}


/******************************************************************************
*
*******************************************************************************/
void AcdkCctMain::cameraCallback(void* param)
{
    ACDK_LOGD_DYN(g_acdkMainDebug,"+");

    acdkCBInfo * const pCBInfo = reinterpret_cast<acdkCBInfo*>(param);
    if(!pCBInfo)
    {
        ACDK_LOGE("NULL pCBInfo");
        return;
    }

    AcdkCctMain * const _This = reinterpret_cast<AcdkCctMain*>(pCBInfo->mCookie);
    if(!_This )
    {
        ACDK_LOGE("NULL AcdkCctMain");
        return;
    }

    acdkCallbackParam_t acdkCbParam;
    acdkCbParam.type     = pCBInfo->mType;
    acdkCbParam.addr1    = pCBInfo->mAddr1;
    acdkCbParam.addr2    = pCBInfo->mAddr2;
    acdkCbParam.imgSize  = pCBInfo->mDataSize1;
    acdkCbParam.thubSize = pCBInfo->mDataSize2;

    _This->dispatchCallback(&acdkCbParam);

     ACDK_LOGD_DYN(g_acdkMainDebug,"-");
}


/*******************************************************************************
*
********************************************************************************/
void AcdkCctMain::dispatchCallback(void *param)
{
    ACDK_LOGD_DYN(g_acdkMainDebug,"+");

    MINT32 err = ACDK_RETURN_NO_ERROR;
    acdkCallbackParam_t *pcbParam = (acdkCallbackParam_t *)param;

    switch(pcbParam->type)
    {
        case ACDK_CB_PREVIEW :  //0x1
            ACDK_LOGD_DYN(g_acdkMainDebug,"Preview Callback");
            err = handlePreviewCB(pcbParam);
            break;
        case ACDK_CB_RAW :  //0x2
            ACDK_LOGD_DYN(g_acdkMainDebug,"Raw Callback");
            err = handleRAWCB(pcbParam);
            break;
        case ACDK_CB_JPEG :  //0x3
            ACDK_LOGD_DYN(g_acdkMainDebug,"JPEG Callback");
            err =  handleJPEGCB(pcbParam);
            break;
        case ACDK_CB_QV :    //0x4
            ACDK_LOGD_DYN(g_acdkMainDebug,"QV Callback");
            err = handleQVCB(pcbParam);
            break;
        case ACDK_CB_CAPTURE :    //0x6
            ACDK_LOGD_DYN(g_acdkMainDebug,"Capture Callback");
            err = handleCapCB(pcbParam);
            break;
        default :
            err =  ACDK_RETURN_INVALID_PARA;
            break;
    }

    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("CB(%d),err(0x%x)",(MINT32)pcbParam->type, err);
    }

    ACDK_LOGD_DYN(g_acdkMainDebug,"-");
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::handlePreviewCB(MVOID *param)
{
    ACDK_LOGD_DYN(g_acdkMainDebug,"+");

    acdkCallbackParam_t *pCBParam = (acdkCallbackParam_t *) param;

    MINT32 err = ACDK_RETURN_NO_ERROR;
    MUINT8 *pVirBufin  = NULL;
    MUINT8 *displayAddr = pCBParam->addr1;

    mFrameCnt++;
    pVirBufin = displayAddr;
    PureRaw_BufAddr = pCBParam->addr2;

    ACDK_LOGD_DYN(g_acdkMainDebug,"mPrvStartX(%u),mPrvStartY(%u)",mPrvStartX,mPrvStartY);
    ACDK_LOGD_DYN(g_acdkMainDebug,"mPrvWidth(%u),mPrvHeight(%u)",mPrvWidth,mPrvHeight);
    ACDK_LOGD_DYN(g_acdkMainDebug,"index(%u),VA(%p),PA(0x%x),Size(%p)",mSurfaceIndex,(void*)mSurfaceIMemInfo[mSurfaceIndex].virtAddr,(void*)mSurfaceIMemInfo[mSurfaceIndex].phyAddr,mSurfaceIMemInfo[mSurfaceIndex].size);
    ACDK_LOGD_DYN(g_acdkMainDebug,"mOrientation(%u)",mOrientation);
    ACDK_LOGD_DYN(g_acdkMainDebug,"addr1(%p)addr2(%p)imgsize(0x%x)thumbsize(0x%x)",pCBParam->addr1,pCBParam->addr2,pCBParam->imgSize,pCBParam->thubSize);

    // show frame by overlay
    if(mOrientation == 90 || mOrientation == 270)
    {
        err = m_pAcdkSurfaceViewObj->setOverlayInfo(0,
                                                    mPrvStartX,
                                                    mPrvStartY,
                                                    mPrvHeight,
                                                    mPrvWidth,
                                                    mSurfaceIMemInfo[mSurfaceIndex].phyAddr,
                                                    mSurfaceIMemInfo[mSurfaceIndex].virtAddr,
                                                    mOrientation);
    }
    else
    {
        err = m_pAcdkSurfaceViewObj->setOverlayInfo(0,
                                                    mPrvStartX,
                                                    mPrvStartY,
                                                    mPrvWidth,
                                                    mPrvHeight,
                                                    mSurfaceIMemInfo[mSurfaceIndex].phyAddr,
                                                    mSurfaceIMemInfo[mSurfaceIndex].virtAddr,
                                                    mOrientation);
    }

    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("setOverlayInfo fail err(0x%x)", err);
        return ACDK_RETURN_API_FAIL;
    }

#if 0

    ACDK_LOGD("m_pAcdkSurfaceViewObj->setOverlayBuf");
    ACDK_LOGD("pPhybufin  = 0x%x",(MUINT32)pVirBufin);
    ACDK_LOGD("srcFormat  = %u",(MUINT32)mAcdkCctMhalPrvParam.frmParam.frmFormat);
    ACDK_LOGD("srcWidth   = %u",mAcdkCctMhalPrvParam.frmParam.w);
    ACDK_LOGD("srcHeight  = %u",mAcdkCctMhalPrvParam.frmParam.h);
    ACDK_LOGD("srcOrientation = %u",mOrientation);
    ACDK_LOGD("mSensorHFlip = %u",mSensorHFlip);
    ACDK_LOGD("mSensorVFlip = %u",mSensorVFlip);

    //! FIXME, the mOrientation should depend on screen and sensor

    err = m_pAcdkSurfaceViewObj->setOverlayBuf(
                               0,
                               pPhybufin,
                               11,   //MHAL_FORMAT_YUY2
                               mAcdkCctMhalPrvParam.frmParam.w,
                               mAcdkCctMhalPrvParam.frmParam.h,
                               mOrientation,
                               mSensorHFlip,
                               mSensorVFlip
                               );

    if (err != 0)
    {
        ACDK_LOGE("setOverlayBuf fail err = 0x%x", err);
        return ACDK_RETURN_API_FAIL;
    }
#endif

    //====== Copy Image to Surface Memory =======

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.camera.acdkdump.enable", value, "0");
    MINT32 dumpEnable = atoi(value);

    if(dumpEnable == 2 || dumpEnable == 3)
    {
        ACDK_LOGD("dump");

        char szFileName[256];
        MINT32 i4WriteCnt = 0;
        MINT32 cnt = 0;

        ACDK_LOGD("prv VA(%p)",displayAddr);

        sprintf(szFileName, "%s/acdkPrv2.rgb",MEDIA_PATH);

        //====== Write RAW Data ======

        FILE *pFp = fopen(szFileName, "wb");

        if(NULL == pFp )
        {
            ACDK_LOGE("Can't open file to save image");
            return ACDK_RETURN_NULL_OBJ;
        }

        i4WriteCnt = fwrite(pVirBufin, 1, mSurfaceIMemInfo[mSurfaceIndex].size, pFp);

        fflush(pFp);

        if(0 != fsync(fileno(pFp)))
        {
            ACDK_LOGE("fync fail");
            fclose(pFp);
            return ACDK_RETURN_API_FAIL;
        }

        ACDK_LOGD("Save image file name:%s, w(%d), h(%d)", szFileName, mPrvWidth, mPrvHeight);

        fclose(pFp);

    }
    //======= Factory mode SMT used ======
    if(mIsFacotory == 1)
    {
        memcpy((MUINT8 *)mSurfaceIMemInfo[mSurfaceIndex].virtAddr, pVirBufin, mSurfaceIMemInfo[mSurfaceIndex].size);
    }
    else
    {
        //====== Color format transfer YV12 to YUY2 ======
        MUINT32 displaySize = mPrvWidth*mPrvHeight*3/2;
        IMEM_BUF_INFO ISrcMemInfo;
        MUINT32 tempW, tempH;

        memcpy((MUINT8 *)(mDispIMemInfo[2].virtAddr), pVirBufin, displaySize);
        ISrcMemInfo.memID = mDispIMemInfo[2].memID;
        ISrcMemInfo.virtAddr = mDispIMemInfo[2].virtAddr;
        ISrcMemInfo.phyAddr = mDispIMemInfo[2].phyAddr;
        ISrcMemInfo.size = displaySize;
        if(mOrientation == 90 || mOrientation == 270)
        {
            tempW = mPrvHeight;
            tempH = mPrvWidth;
        }
        else
        {
            tempW = mPrvWidth;
            tempH = mPrvHeight;
        }
        ACDK_LOGD("procQvMemInfo : vir(%p),phy(%p)",(void*)ISrcMemInfo.virtAddr,(void*)ISrcMemInfo.phyAddr);

        m_pAcdkUtilityObj->imageProcess(eImgFmt_YV12,
                                        eImgFmt_YUY2,
                                        tempW,
                                        tempH,
                                        mOrientation,
                                        mSensorHFlip,
                                        ISrcMemInfo,
                                        mSurfaceIMemInfo[mSurfaceIndex]);

        ACDK_LOGD("Dst VA(%p), Src VA(%p) size(%p)", (void*)mSurfaceIMemInfo[mSurfaceIndex].virtAddr, (void*)pVirBufin, displaySize);
    }

    //m_pIMemDrv->cacheFlushAll();
    m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &mSurfaceIMemInfo[mSurfaceIndex]);
    //====== Refresh Screen =======
    err = m_pAcdkSurfaceViewObj->refresh();
    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("refresh fail err(0x%x)", err);
        return ACDK_RETURN_API_FAIL;
    }

    //update surface index
    mSurfaceIndex = (mSurfaceIndex + 1) % SURFACE_NUM;

    ACDK_LOGD_DYN(g_acdkMainDebug,"-");
    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/

MINT32 AcdkCctMain::handleRAWCB(MVOID *param)
{
    ACDK_LOGD("+");

    //====== Check Input Argument ======

    if (param == NULL)
    {
        ACDK_LOGE("param is NULL");
        return ACDK_RETURN_NULL_OBJ;
    }

    //====== Local Variable Setting ======

    acdkCallbackParam_t *pCBParam = (acdkCallbackParam_t *)param;
    MUINT8 *pCapBufIn = pCBParam->addr1;

    ACDK_LOGD("RAW src VA(%p)",pCapBufIn);

    //====== Copy RAW Data ======
    if(mRawIMemInfo.virtAddr == 0)
    {
        createMemBuf(pCBParam->imgSize, 1, &mRawIMemInfo);
    }

    if(mRawIMemInfo.virtAddr == 0)
    {
        ACDK_LOGE("mRawIMemInfo is NULL");
        return ACDK_RETURN_NULL_OBJ;
    }
    else
    {
        memcpy((MUINT8 *)mRawIMemInfo.virtAddr, pCapBufIn, mRawIMemInfo.size);
        //m_pIMemDrv->cacheFlushAll();
        m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &mRawIMemInfo);
        ACDK_LOGD("mRawIMemInfo : vir(%p),phy(%p),memID(%d)",(void*)mRawIMemInfo.virtAddr,(void*)mRawIMemInfo.phyAddr,mRawIMemInfo.memID);
    }

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::handleJPEGCB(MVOID *param)
{
    ACDK_LOGD("+");

    //====== Check Input Argument ======

    if(param == NULL)
    {
        ACDK_LOGE("param is NULL");
        return ACDK_RETURN_NULL_OBJ;
    }

    //====== Local Variable Setting ======

    acdkCallbackParam_t *pCBParam = (acdkCallbackParam_t *) param;
    MUINT8 *pJpgBufIn = pCBParam->addr1;
    MUINT32 imgSize;
    ACDK_LOGD("JPEG src VA(%p)",pJpgBufIn);

    if(mIsSOI) // JFIF
        imgSize = pCBParam->imgSize;
    else //EXIF
        imgSize = pCBParam->imgSize + 64*1024;

    //====== Copy RAW Data ======

    if(mJpgIMemInfo.virtAddr == 0)
    {
        createMemBuf(imgSize, 1, &mJpgIMemInfo);
    }

    if(mJpgIMemInfo.virtAddr == 0)
    {
        ACDK_LOGE("mJpgIMemInfo is NULL");
        return ACDK_RETURN_NULL_OBJ;
    }
    else
    {
        if(mIsSOI)
        {
            memcpy((MUINT8 *)mJpgIMemInfo.virtAddr, pJpgBufIn, mJpgIMemInfo.size);
           //m_pIMemDrv->cacheFlushAll();
           m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &mJpgIMemInfo);
        }
        else
        {
            MUINT8 *puExifHeaderBuf = new MUINT8[128 * 1024];
            MUINT32 u4ExifHeaderSize = 0;

            makeExifHeader(eAppMode_FactoryMode, NULL, 0, puExifHeaderBuf, u4ExifHeaderSize,0,0);
            ACDK_LOGD("(exifHeaderBuf, size) = (%p, %d)",puExifHeaderBuf, u4ExifHeaderSize);
            memcpy((MUINT8 *)mJpgIMemInfo.virtAddr, puExifHeaderBuf, u4ExifHeaderSize);
            memcpy((MUINT8 *)(mJpgIMemInfo.virtAddr + u4ExifHeaderSize), pJpgBufIn, pCBParam->imgSize);
            //m_pIMemDrv->cacheFlushAll();
            m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &mJpgIMemInfo);
            mJpgIMemInfo.size = u4ExifHeaderSize + pCBParam->imgSize;

            delete [] puExifHeaderBuf;
        }
        ACDK_LOGD("mJpgIMemInfo : vir(%p),phy(%p),memID(%d)",mJpgIMemInfo.virtAddr,mJpgIMemInfo.phyAddr,mJpgIMemInfo.memID);
    }

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::handleQVCB(MVOID *param)
{
    ACDK_LOGD("+");

    //====== Check Input Argument ======

    if (param == NULL)
    {
        ACDK_LOGE("param is NULL");
        return ACDK_RETURN_NULL_OBJ;
    }

    //====== Local Variable Setting ======

    acdkCallbackParam_t *pCBParam = (acdkCallbackParam_t *)param;
    MUINT8 *pQVBufIn  = pCBParam->addr1;

    ACDK_LOGD("QV src VA(%p)", pQVBufIn);

    //====== Copy RAW Data ======

    if(mQvIMemInfo.virtAddr == 0)
    {
        createMemBuf(pCBParam->imgSize, 1, &mQvIMemInfo);
    }

    if(mQvIMemInfo.virtAddr == 0)
    {
        ACDK_LOGE("mQvIMemInfo is NULL");
        return ACDK_RETURN_NULL_OBJ;
    }
    else
    {
        memcpy((MUINT8 *)mQvIMemInfo.virtAddr, pQVBufIn, mQvIMemInfo.size);
        //m_pIMemDrv->cacheFlushAll();
        m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &mQvIMemInfo);

        ACDK_LOGD(" mQvIMemInfo : vir(%p),phy(%p),memID(%d)",(void*)mQvIMemInfo.virtAddr,(void*)mQvIMemInfo.phyAddr,mQvIMemInfo.memID);
    }

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;
}
/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::handleCapCB(MVOID *param)
{
    ACDK_LOGD_DYN(g_acdkMainDebug,"+");

    acdkCallbackParam_t *pCBParam = (acdkCallbackParam_t *) param;

    MINT32 err = ACDK_RETURN_NO_ERROR;

    MUINT8 *captureAddr = pCBParam->addr1;
    MUINT32 captureSize = pCBParam->imgSize;

    MINT32 i4WriteCnt = 0;
    char szFileName[256];
    //====== Write RAW Data ======

    ACDK_LOGD_DYN(g_acdkMainDebug,"JPEG src VA(0x%x)",(MUINTPTR)captureAddr);

    // Copy jpeg to output buffer
    memcpy((MUINT8 *)mCapIMemInfo.virtAddr, captureAddr,captureSize);
    Capture_BufAddr = (MUINT8 *)mCapIMemInfo.virtAddr;
    Capture_Size = captureSize;

    sprintf(szFileName, "%s/acdkCap.jpg",MEDIA_PATH);

    FILE *pFp = fopen(szFileName, "wb");

    if(NULL == pFp)
    {
        ACDK_LOGE("Can't open file to save image");
        err = ACDK_RETURN_NULL_OBJ;
        goto CapCBsaveJPEGImgExit;
    }

    i4WriteCnt = fwrite((void *)captureAddr, 1, captureSize, pFp);

    fflush(pFp);

    if(0 != fsync(fileno(pFp)))
    {
        ACDK_LOGE("fync fail");
        fclose(pFp);
        err = ACDK_RETURN_API_FAIL;
        goto CapCBsaveJPEGImgExit;
    }

    ACDK_LOGD("Save image file name:%s,w(%u),h(%u)", szFileName, mCapWidth, mCapHeight);

    fclose(pFp);

    ACDK_LOGD_DYN(g_acdkMainDebug,"captureAddr(%p),captureSize(%d)",(void*)captureAddr,captureSize);

    ACDK_LOGD_DYN(g_acdkMainDebug,"-");

CapCBsaveJPEGImgExit:

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::saveJPEGImg()
{
    ACDK_LOGD("+");

    //====== Check Input Argument ======

    if(mJpgIMemInfo.virtAddr == 0)
    {
        ACDK_LOGE("mJpgIMemInfo is empty");
        return ACDK_RETURN_NULL_OBJ;
    }

    //====== Local Variable Setting ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    MUINT8 *pImgBufIn = (MUINT8 *)mJpgIMemInfo.virtAddr;
    MINT32 i4WriteCnt = 0;
    char szFileName[256];

    //====== Write RAW Data ======

    ACDK_LOGD("JPEG src VA(%p)",pImgBufIn);

    sprintf(szFileName, "%s/acdkCap.jpg",MEDIA_PATH);

    FILE *pFp = fopen(szFileName, "wb");

    if(NULL == pFp)
    {
        ACDK_LOGE("Can't open file to save image");
        err = ACDK_RETURN_NULL_OBJ;
        goto saveJPEGImgExit;
    }

    i4WriteCnt = fwrite(pImgBufIn, 1, mJpgIMemInfo.size, pFp);

    fflush(pFp);

    if(0 != fsync(fileno(pFp)))
    {
        ACDK_LOGE("fync fail");
        fclose(pFp);
        err = ACDK_RETURN_API_FAIL;
        goto saveJPEGImgExit;
    }

    ACDK_LOGD("Save image file name:%s,w(%u),h(%u)", szFileName, mCapWidth, mCapHeight);

    fclose(pFp);

saveJPEGImgExit:

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCctMain::saveRAWImg(MINT32 mode)
{
    ACDK_LOGD("+");

    //====== Check Input Argument ======

    if(mRawIMemInfo.virtAddr == 0)
    {
        ACDK_LOGE("mRawIMemInfo is empty");
        return ACDK_RETURN_NULL_OBJ;
    }

    //====== Local Variable Setting ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    MUINT8 *pImgBufIn = (MUINT8 *)mRawIMemInfo.virtAddr;
    MINT32 i4WriteCnt = 0;
    char szFileName[256] = "";

    //====== Width & Height Setting ======

    MUINT32 rawWidth = 0, rawHeight = 0;

    setDimension(mode, rawWidth, rawHeight);

    //====== Write RAW Data ======
    ACDK_LOGD("RAW src VA(%p)",pImgBufIn);
    if(mSensorType == SENSOR_TYPE_YUV) //YUV
    {
        sprintf(szFileName, "%s/YUVImg_YUY2.yuv" , MEDIA_PATH);
    }
    else
    {
        if(mCapType == PURE_RAW8_TYPE || g_dumpRAW == PURE_RAW8_TYPE)
        {
            sprintf(szFileName, "%s/acdkCapPureRaw8.raw" , MEDIA_PATH);
        }
        else if(mCapType == PURE_RAW10_TYPE || g_dumpRAW == PURE_RAW10_TYPE)
        {
            sprintf(szFileName, "%s/acdkCapPureRaw10.raw" , MEDIA_PATH);
        }
        else if(mCapType == PROCESSED_RAW8_TYPE || g_dumpRAW == PROCESSED_RAW8_TYPE)
        {
            sprintf(szFileName, "%s/acdkCapProcRaw8.raw" , MEDIA_PATH);
        }
        else if(mCapType == PROCESSED_RAW10_TYPE || g_dumpRAW == PROCESSED_RAW10_TYPE)
        {
            sprintf(szFileName, "%s/acdkCapProcRaw10.raw" , MEDIA_PATH);
        }
    }

    FILE *pFp = fopen(szFileName, "wb");

    if(NULL == pFp)
    {
        ACDK_LOGE("Can't open file to save image");
        err = ACDK_RETURN_NULL_OBJ;
        goto saveRAWImgExit;
    }

    i4WriteCnt = fwrite(pImgBufIn, 1, mRawIMemInfo.size, pFp);

    fflush(pFp);

    if(0 != fsync(fileno(pFp)))
    {
        ACDK_LOGE("fync fail");
        fclose(pFp);
        err = ACDK_RETURN_API_FAIL;
        goto saveRAWImgExit;
    }

    ACDK_LOGD("Save image file name:%s,w(%u),h(%u)", szFileName, rawWidth, rawHeight);

    fclose(pFp);

saveRAWImgExit :

    ACDK_LOGD("-");
    return err;
}


/*******************************************************************************
*Functionality : take care of command and used as API for upper layer
********************************************************************************/
MINT32 AcdkCctMain::sendcommand(
                MUINT32 const a_u4Ioctl,
                MUINT8        *puParaIn,
                MUINT32 const u4ParaInLen,
                MUINT8        *puParaOut,
                MUINT32 const u4ParaOutLen,
                MUINT32       *pu4RealParaOutLen
)
{
    MINT32 err = ACDK_RETURN_NO_ERROR;

    ACDK_LOGD("cmd(0x%x)", a_u4Ioctl);

    if (a_u4Ioctl == ACDK_CMD_QV_IMAGE)
    {
        //ACDK_LOGD("ACDK_CMD_QV_IMAGE");
        //err = quickViewImg((MUINT8*)puParaIn, (Func_CB)puParaOut);

        ACDK_LOGD("no need to call QV command right now");
    }
    else if (a_u4Ioctl == ACDK_CMD_RESET_LAYER_BUFFER)
    {
        ACDK_LOGD("ACDK_CMD_RESET_LAYER_BUFFER");

        err = m_pAcdkSurfaceViewObj->resetLayer(0);
    }
    else if (a_u4Ioctl == ACDK_CMD_SET_SRC_DEV)
    {
        MINT32 *pSrcDev = (MINT32 *)puParaIn;
        if(*pSrcDev == 8){ // 1: main, 2: sub, 4 :main2
            *pSrcDev = 4;
        }
        err = setSrcDev(*pSrcDev);

        if(err != ACDK_RETURN_NO_ERROR)
        {
            ACDK_LOGE("ACDK_CMD_SET_SRC_DEV fail(0x%x)",err);
        }
    }
    else if(a_u4Ioctl == ACDK_CMD_SET_OPERATION_MODE)
    {
        eACDK_OPERA_MODE eOpMode = ACDK_OPT_NONE_MODE;
        MINT32 *pOperMode = (MINT32 *)puParaIn;
        if (*pOperMode == 1)
        {
            eOpMode = ACDK_OPT_META_MODE;
        }
        else if (*pOperMode == 2)
        {
            eOpMode = ACDK_OPT_FACTORY_MODE;
        }

        err = setOperMode(eOpMode);
        if (err != ACDK_RETURN_NO_ERROR)
        {
            ACDK_LOGE("ACDK_CMD_SET_OPERATION_MODE fail(0x%x)",err);
        }
    }
    else if(a_u4Ioctl == ACDK_CMD_SET_SHUTTER_TIME)
    {
        MINT32 *pShutterTime = (MINT32 *)puParaIn;
        err = setShutterTime(*pShutterTime);

        if(err != ACDK_RETURN_NO_ERROR)
        {
            ACDK_LOGE("ACDK_CMD_SET_SHUTTER_TIME fail(0x%x)",err);
        }
    }
    else if(a_u4Ioctl == ACDK_CMD_GET_SHUTTER_TIME)
    {
        *((MUINT32 *)puParaOut) = mGetShutTime;
        ACDK_LOGD("ACDK_CMD_GET_SHUTTER_TIME(%u)",*((MUINT32 *)puParaOut));
    }
    else if(a_u4Ioctl == ACDK_CMD_GET_CHECKSUM)
    {
        *((MUINT32 *)puParaOut) = mGetCheckSumValue;
        ACDK_LOGD("ACDK_CMD_GET_CHECKSUM(%u)",*((MUINT32 *)puParaOut));
    }
    else if(a_u4Ioctl == ACDK_CMD_GET_AF_INFO)
    {
        mGetAFInfo = m_pAcdkCctMhalObj->acdkCctMhalGetAFInfo();

        *((MUINT32 *)puParaOut) = mGetAFInfo;;
        ACDK_LOGD("ACDK_CMD_GET_AF_INFO(%u)",*((MUINT32 *)puParaOut));
    }
    else if(a_u4Ioctl == ACDK_CMD_GET_SENSOR_MODE_NUM)
    {
        *((MUINT32 *)puParaOut) = mSensorModeNum;
        ACDK_LOGD("ACDK_CMD_GET_SENSOR_MODE_NUM(%u)",*((MUINT32 *)puParaOut));
    }
    else if(a_u4Ioctl == ACDK_CMD_PREVIEW_START)
    {
        PACDK_PREVIEW_STRUCT pCamPreview = (ACDK_PREVIEW_STRUCT *)puParaIn;

        ACDK_LOGD("ACDK_CMD_PREVIEW_START");
        ACDK_LOGD("Preview Width:%d", pCamPreview->u4PrvW);
        ACDK_LOGD("Preview Height:%d", pCamPreview->u4PrvH);
        ACDK_LOGD("Preview HDREn:%d", pCamPreview->u16HDRModeEn);
        ACDK_LOGD("Preview OP Type:%d", pCamPreview->u4OperaType);

        mHDR_EN = pCamPreview->u16HDRModeEn;

        mTestPatternOut = pCamPreview->u16PreviewTestPatEn;
        /* 0: Pure ISP, 1: Eng ISP */
        if(pCamPreview->u4OperaType == 0)
        {
            mIsFacotory = 1;
            mIsISPEng = 0;
            err = startPreview(pCamPreview->fpPrvCB);
        }
        else // CCT mode
        {
            mIsISPEng = 1;
            err = startPreview_CameraDev(pCamPreview->fpPrvCB, pCamPreview->u4SensorMode);
        }

        if(err != ACDK_RETURN_NO_ERROR)
        {
            ACDK_LOGE("ACDK_CMD_PREVIEW_START fail(0x%x)",err);
        }
    }
    else if(a_u4Ioctl == ACDK_CMD_PREVIEW_STOP)
    {
        ACDK_LOGD("ACDK_CMD_PREVIEW_STOP");
        /* 0: Pure ISP, 1: Eng ISP */
        if(mIsISPEng == 0)
        {
            err = stopPreview();
        }
        else
        {
            err = stopPreview_CameraDev();
        }

        if(err != ACDK_RETURN_NO_ERROR)
        {
            ACDK_LOGE("ACDK_CMD_PREVIEW_STOP fail(0x%x)",err);
        }
    }
    else if(a_u4Ioctl == ACDK_CMD_CAPTURE)
    {
        PACDK_CAPTURE_STRUCT pCapConfig = (ACDK_CAPTURE_STRUCT *)puParaIn;
        ACDK_LOGD("ACDK_CMD_CAPTURE");
        ACDK_LOGD("Capture Mode:%d", pCapConfig->eCameraMode);
        ACDK_LOGD("Operation Mode:%d", pCapConfig->eOperaMode);
        ACDK_LOGD("Format:%d", pCapConfig->eOutputFormat);
        ACDK_LOGD("Width:%d", pCapConfig->u2JPEGEncWidth);
        ACDK_LOGD("Height:%d", pCapConfig->u2JPEGEncHeight);
        ACDK_LOGD("CB:0x%x", pCapConfig->fpCapCB);
        ACDK_LOGD("cap cnt:%d", pCapConfig->u4CapCount);
        ACDK_LOGD("isSave:%d", pCapConfig->i4IsSave);
        ACDK_LOGD("MFLL_En:%d", pCapConfig->MFLL_En);
        ACDK_LOGD("mHDR_En:%d", pCapConfig->HDRModeEn);
        ACDK_LOGD("eMultiNR:%d", pCapConfig->eMultiNR);

        // get operation mode
        mOperaMode = pCapConfig->eOperaMode;
        mIsSOI     = MTRUE;  //  MTRUE : JFPIF header , MFALSE : EXIF  header
        mMFLL_EN   = pCapConfig->MFLL_En;
        mEMultiNR  = pCapConfig->eMultiNR;
        mHDR_EN    = pCapConfig->HDRModeEn;

        switch(pCapConfig->eOutputFormat)
        {
            case JPEG_TYPE :
            case PURE_RAW8_TYPE :
            case PURE_RAW10_TYPE :
            case PROCESSED_RAW8_TYPE :
            case PROCESSED_RAW10_TYPE :
                /* 0: Pure ISP, 1: Eng ISP */
                if(mIsISPEng == 0)
                {
                    err = GetPureRaw(pCapConfig->eCameraMode,
                                  pCapConfig->eOutputFormat,
                                  pCapConfig->fpCapCB,
                                  pCapConfig->u2JPEGEncWidth,
                                  pCapConfig->u2JPEGEncHeight,
                                  pCapConfig->u4CapCount,
                                  pCapConfig->i4IsSave);
                }
                else
                {
                    err = takePicture_CameraDev(pCapConfig->eCameraMode,
                                  pCapConfig->eOutputFormat,
                                  pCapConfig->fpCapCB,
                                  pCapConfig->u2JPEGEncWidth,
                                  pCapConfig->u2JPEGEncHeight,
                                  pCapConfig->u4CapCount,
                                  pCapConfig->i4IsSave);
                }

                break;
            default :
                ACDK_LOGE("No Support Format");
                err = ACDK_RETURN_API_FAIL;
                break;
        }

        if(err != ACDK_RETURN_NO_ERROR)
        {
            ACDK_LOGE("ACDK_CMD_CAPTURE fail(0x%x)",err);
        }
    }
    else if(a_u4Ioctl == ACDK_CMD_SET_AF_MODE)
    {
        MBOOL *pIsAfMode = (MBOOL *)puParaIn;
        ACDK_LOGD("ACDK_CMD_SET_AF_MODE : AfMode(%d)", *pIsAfMode);
        m_pAcdkCctMhalObjEng->acdkCctMhalSetAFMode(*pIsAfMode);
    }

   return err;
}

/*******************************************************************************
*Functionality : set DestWidth/Height based on mode and mSensorResolution
********************************************************************************/
inline void AcdkCctMain::setDimension(MUINT32 const mode, MUINT32 &mDestWidth, MUINT32 &mDestHeight)
{
  if(mode == PREVIEW_MODE)
  {
    mDestWidth = mSensorResolution.SensorPreviewWidth;
    mDestHeight = mSensorResolution.SensorPreviewHeight;
  }
  else if(mode == VIDEO_MODE)
  {
    mDestWidth = mSensorResolution.SensorVideoWidth;
    mDestHeight = mSensorResolution.SensorVideoHeight;
  }
  else if(mode == VIDEO1_MODE)
  {
    mDestWidth = mSensorResolution.SensorHighSpeedVideoWidth;
    mDestHeight = mSensorResolution.SensorHighSpeedVideoHeight;
  }
  else if(mode == VIDEO2_MODE)
  {
    mDestWidth = mSensorResolution.SensorSlimVideoWidth;
    mDestHeight = mSensorResolution.SensorSlimVideoHeight;
  }
  else if(mode == VIDEO3_MODE)
  {
    mDestWidth = mSensorResolution.SensorCustom1Width;
    mDestHeight = mSensorResolution.SensorCustom1Height;
  }
  else if(mode == VIDEO4_MODE)
  {
    mDestWidth = mSensorResolution.SensorCustom2Width;
    mDestHeight = mSensorResolution.SensorCustom2Height;
  }
  else if(mode == VIDEO5_MODE)
  {
    mDestWidth = mSensorResolution.SensorCustom3Width;
    mDestHeight = mSensorResolution.SensorCustom3Height;
  }
  else if(mode == VIDEO6_MODE)
  {
    mDestWidth = mSensorResolution.SensorCustom4Width;
    mDestHeight = mSensorResolution.SensorCustom4Height;
  }
  else if(mode == VIDEO7_MODE)
  {
    mDestWidth = mSensorResolution.SensorCustom5Width;
    mDestHeight = mSensorResolution.SensorCustom5Height;
  }
  else
  {
    mDestWidth = mSensorResolution.SensorFullWidth;
    mDestHeight = mSensorResolution.SensorFullHeight;
  }
}

/*******************************************************************************
*Functionality : set DestScenario based on mode
********************************************************************************/
inline void AcdkCctMain::setScenario(MUINT32 const mode, MUINT32 &DestScenario)
{
  if(mode == PREVIEW_MODE)
      {
          DestScenario = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
      }
    else if(mode == VIDEO_MODE)
      {
          DestScenario = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
      }
  else if(mode == VIDEO1_MODE)
      {
          DestScenario = SENSOR_SCENARIO_ID_SLIM_VIDEO1;
      }
  else if(mode == VIDEO2_MODE)
      {
          DestScenario = SENSOR_SCENARIO_ID_SLIM_VIDEO2;
      }
  else if(mode == VIDEO3_MODE)
      {
          DestScenario = SENSOR_SCENARIO_ID_CUSTOM1;
      }
  else if(mode == VIDEO4_MODE)
      {
          DestScenario = SENSOR_SCENARIO_ID_CUSTOM2;
      }
  else if(mode == VIDEO5_MODE)
      {
          DestScenario = SENSOR_SCENARIO_ID_CUSTOM3;
      }
  else if(mode == VIDEO6_MODE)
      {
          DestScenario = SENSOR_SCENARIO_ID_CUSTOM4;
      }
  else if(mode == VIDEO7_MODE)
      {
          DestScenario = SENSOR_SCENARIO_ID_CUSTOM5;
      }
      else
      {
          DestScenario = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
      }
}


