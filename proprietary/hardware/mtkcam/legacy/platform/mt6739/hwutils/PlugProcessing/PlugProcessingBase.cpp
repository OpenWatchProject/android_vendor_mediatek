/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "MtkCam/HwUtils/PlugProcessingBase"
//
#include "PlugProcessingBase.h"
#include "PlugProcessingUtils.h"
extern "C";

using namespace android;
using namespace NSCam;


/******************************************************************************
 *
 ******************************************************************************/
void
PlugProcessingBase::InitThread::
exit(void)
{
    FUNC_BGN;
    MY_LOGD_IF(mLogLevel > 1, "InitThread exit");
    Thread::requestExit();
    MY_LOGD_IF(mLogLevel > 1, "InitThread join");
    Thread::join();
    MY_LOGD_IF(mLogLevel > 1, "InitThread done");
    FUNC_END;
};


/******************************************************************************
 *
 ******************************************************************************/
void
PlugProcessingBase::DlFunc::
getFunction(char const * funcName, MVOID** ppFn)
{
    FUNC_BGN;
    *ppFn = NULL;
    if (isValid()) {
        android::Mutex::Autolock _l(mLock);
        void * const pFn = ::dlsym(mLibHandle, funcName);
        if (!pFn) {
            char const * str = ::dlerror();
            MY_LOGI_IF(mLogLevel > 0, "dlsym[%s] : %s @(%p)",
                funcName, (str ? str : "UNKNOWN"), mLibHandle);
        } else {
            *ppFn = pFn;
            MY_LOGI_IF(mLogLevel > 1, "dlsym[%s]@(%p)", funcName, pFn);
        }
    } else {
        MY_LOGI("dlopen NOT executed");
    }
    FUNC_END;
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
PlugProcessingBase::DlFunc::
isValid(void)
{
    FUNC_BGN;
    android::Mutex::Autolock _l(mLock);
    MBOOL res = (mLibHandle == NULL) ? MFALSE : MTRUE;
    FUNC_END;
    return res;
};


/******************************************************************************
 *
 ******************************************************************************/
void
PlugProcessingBase::DlFunc::
libOpen(char const * libName)
{
    FUNC_BGN;
    android::Mutex::Autolock _l(mLock);
    if  (mLibHandle) {
        MY_LOGI("dlopen executed");
    } else {
        mLibHandle = ::dlopen(libName, RTLD_NOW);
        if  (!mLibHandle) {
            char const * str = ::dlerror();
            MY_LOGI_IF(mLogLevel > 0, "dlopen[%s] : %s",
                libName, (str ? str : "UNKNOWN"));
        } else {
            MY_LOGI_IF(mLogLevel > 0, "dlopen[%s]@(%p)", libName, mLibHandle);
        }
    }
    FUNC_END;
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
void
PlugProcessingBase::DlFunc::
libClose(void)
{
    FUNC_BGN;
    android::Mutex::Autolock _l(mLock);
    MINT res = 0;
    if  (mLibHandle) {
        res = ::dlclose(mLibHandle);
        mLibHandle = NULL;
    }
    MY_LOGI_IF(mLogLevel > 1, "dlclose (%d)", res);
    FUNC_END;
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
status_t
PlugProcessingBase::InitThread::
readyToRun()
{
    FUNC_BGN;
    // set name
    char name[32] = {0};
    snprintf(name, sizeof(name), "PlugProc_x%X", mOpenID);
    ::prctl(PR_SET_NAME, (unsigned long)(name), 0, 0, 0);
    // set normal
    struct sched_param sched_p;
    sched_p.sched_priority = 0;
    ::sched_setscheduler(0, (SCHED_OTHER), &sched_p);
    ::setpriority(PRIO_PROCESS, 0, (ANDROID_PRIORITY_FOREGROUND - 2));
    //  Note: "priority" is nice value.
    //
    ::sched_getparam(0, &sched_p);
    MY_LOGD(
        "Tid: %d, policy: %d, priority: %d [%s]"
        , ::gettid(), ::sched_getscheduler(0)
        , sched_p.sched_priority, name
    );
    //
    FUNC_END;
    return OK;
};


/******************************************************************************
 *
 ******************************************************************************/
bool
PlugProcessingBase::InitThread::
threadLoop()
{
    FUNC_BGN;
    sp<PlugProcessingBase> spPlugProcessing = mwpPlugProcessing.promote();
    if (spPlugProcessing != NULL) {
        {
            Mutex::Autolock _ll(spPlugProcessing->mInitLock);
            spPlugProcessing->mInitBegan = MTRUE;
            spPlugProcessing->mInitCond.broadcast();
        }
        //
        CAM_TRACE_FMT_BEGIN("PlugProc_x%X-InitA", mOpenID);
        spPlugProcessing->initImp();
        CAM_TRACE_FMT_END();
    } else {
        MY_LOGI("PlugProcessing NOT exist");
    }
    FUNC_END;
    return MFALSE;
};


/******************************************************************************
 *
 ******************************************************************************/
PlugProcessingBase::
PlugProcessingBase()
    : mOpenID(0xFFFFFFFF)
    , mLock()
    , mLogLevel(0)
    , mOpMode(IPlugProcessing::OP_MODE_SYNC)
    , mReadyLock()
    , mInited(MFALSE)
    , mspInitThread(NULL)
    , mInitLock()
    , mInitCond()
    , mInitBegan(MFALSE)
    , mspDlFunc(NULL)
{
    // check log level first
    mLogLevel = ::property_get_int32("debug.camera.log.plugproc", 0);
    FUNC_BGN;
    //
    FUNC_END;
};


/******************************************************************************
 *
 ******************************************************************************/
PlugProcessingBase::
~PlugProcessingBase()
{
    FUNC_BGN;
    uninit();
    if (mspDlFunc != NULL) {
        mspDlFunc = NULL;
    }
    FUNC_END;
};


/******************************************************************************
 *
 ******************************************************************************/
void
PlugProcessingBase::
onLastStrongRef(const void* /*id*/)
{
    FUNC_BGN;
    uninit();
    FUNC_END;
};


/******************************************************************************
 *
 ******************************************************************************/
void
PlugProcessingBase::
waitInitExit(void)
{
    FUNC_BGN;
    if (mspInitThread != NULL) {
        mspInitThread->exit();
        mspInitThread = NULL;
    }
    FUNC_END;
};


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PlugProcessingBase::
waitInitDone(void)
{
    FUNC_BGN;
    Mutex::Autolock _l(mLock);
    waitInitExit();
    FUNC_END;
    return OK;
};


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PlugProcessingBase::
init(OP_MODE const opMode)
{
    FUNC_BGN;
    MERROR res = OK;
    Mutex::Autolock _ll(mReadyLock);
    MY_LOGI_IF(mLogLevel > 1, "mode(%d:%d) init(%d)", opMode, mOpMode, mInited);
    if (!mInited) {
        Mutex::Autolock _l(mLock);
        mOpMode = (IPlugProcessing::OP_MODE)opMode;
        if (mOpMode == IPlugProcessing::OP_MODE_ASYNC) {
            waitInitExit();
            mspInitThread = new InitThread(this, mOpenID, mLogLevel);
            if (mspInitThread == NULL) {
                MY_LOGE("Create InitThread Fail");
                return NO_MEMORY;
            }
            {
                Mutex::Autolock _ll(mInitLock);
                mInitBegan = MFALSE;
            }
            mspInitThread->run("PlugProcessing");
            {
                Mutex::Autolock _ll(mInitLock);
                if (!mInitBegan) {
                    mInitCond.wait(mInitLock);
                }
            }
        } else {
            CAM_TRACE_FMT_BEGIN("PlugProc_x%X-InitS", mOpenID);
            res = initImp();
            CAM_TRACE_FMT_END();
        }
        mInited = MTRUE;
    } else {
        MY_LOGI_IF(mLogLevel > 0, "init already mode(%d:%d)", opMode, mOpMode);
        res = ALREADY_EXISTS;
    }
    FUNC_END;
    return res;
};


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PlugProcessingBase::
uninit(void)
{
    FUNC_BGN;
    Mutex::Autolock _ll(mReadyLock);
    Mutex::Autolock _l(mLock);
    MERROR res = OK;
    //
    waitInitExit();
    //
    if (mInited) {
        CAM_TRACE_FMT_BEGIN("PlugProc_x%X-Uninit", mOpenID);
        res = uninitImp();
        CAM_TRACE_FMT_END();
        mInited = MFALSE;
    } else {
        MY_LOGD_IF(mLogLevel > 0, "uninit already mode(%d)", mOpMode);
        res = ALREADY_EXISTS;
    }
    FUNC_END;
    return res;
};


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PlugProcessingBase::
sendCommand(MINT32 const cmd,
    MINTPTR arg1, MINTPTR arg2, MINTPTR arg3, MINTPTR arg4)
{
    FUNC_BGN;
    Mutex::Autolock _l(mLock);
    MY_LOGD_IF(mLogLevel > 0, "sendCommand[%d]"
        "(%" PRIxPTR ", %" PRIxPTR ", %" PRIxPTR ", %" PRIxPTR ")",
        cmd, arg1, arg2, arg3, arg4);
    //
    CAM_TRACE_FMT_BEGIN("PlugProc_x%X-Cmd[%d]", mOpenID, cmd);
    MERROR res = sendCommandImp(cmd, arg1, arg2, arg3, arg4);
    CAM_TRACE_FMT_END();
    FUNC_END;
    return res;
};

/******************************************************************************
 *
 ******************************************************************************/
#if 0
void
PlugStaticInfo::dump() const{
     CAM_LOGI("PlugStaticInfo:");
     CAM_LOGI("\teep_buf_size:%zu",eep_buf_size);
} 

void
PlugInitParam::dump() const{
     CAM_LOGI("PlugInitParam:");
     CAM_LOGI("\topenId: %d",openId);
     CAM_LOGI("\timg_w: %zu",img_w);
     CAM_LOGI("\timg_h: %zu",img_h);
     CAM_LOGI("\teep_buf_size: %zu eep_buf_addr: %p",eep_buf_size, eep_buf_addr);
}

void
PlugProcessingParam::dump() const{
     CAM_LOGI("PlugProcessingParam:");
     CAM_LOGI("\tpattern: %d",pattern);
     CAM_LOGI("\tbayer_order: %d",bayer_order);
     CAM_LOGI("\tsrc_buf_fd: %d",src_buf_fd);
     CAM_LOGI("\tdst_buf_fd: %d",dst_buf_fd);
     CAM_LOGI("\timg_w: %zu",img_w);
     CAM_LOGI("\timg_h: %zu",img_h);
     CAM_LOGI("\tsrc_buf_size: %d",src_buf_size);
     CAM_LOGI("\tdst_buf_size: %d",dst_buf_size);
}
#endif
