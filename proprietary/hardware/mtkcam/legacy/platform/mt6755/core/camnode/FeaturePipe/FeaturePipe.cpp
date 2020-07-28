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
 * MediaTek Inc. (C) 2010. All rights reserved.
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
//#define LOG_TAG "MtkCam/PostProc.PipeWrapper"
/*************************************************************************************
* Log Utility
*************************************************************************************/
#undef   LOG_TAG    // Decide a Log TAG for current file.
#define  LOG_TAG    "MtkCam/FeaturePipe"

#include <mtkcam/Log.h>

// mkdbg: get property
#include <cutils/properties.h>  // For property_get().

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define LOG_VRB(fmt, arg...)        CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define LOG_INF(fmt, arg...)        CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define LOG_WRN(fmt, arg...)        CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define LOG_ERR(fmt, arg...)        CAM_LOGE("[%s] " fmt, __func__, ##arg)
#define LOG_AST(cond, fmt, arg...)  do{ if(!(cond)) CAM_LOGA("[%s] " fmt, __func__, ##arg); } while(0)



#define FUNCTION_LOG_START      LOG_DBG("+");
#define FUNCTION_LOG_END        LOG_DBG("-");
#define ERROR_LOG               LOG_ERR("Error");
//

#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/IPostProcPipe.h>
#include <mtkcam/ispio_sw_scenario.h>
#include <mtkcam/iopipe/PostProc/INormalStream.h>
#include <vector>
#include <cutils/atomic.h>
#include "FeaturePipe.BufHandler.h"
#include "FeaturePipe.h"
#include "PortMap.h"
#include <mtkcam/utils/fwk/MtkCamera.h>

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSPostProc;


// mkdbg: get frame no: start
static inline MUINT32 getMagicNo(QParams const& rParams){
    if (rParams.mvMagicNo.size())
        return rParams.mvMagicNo[0];
    else
        return rParams.mFrameNo;
}


/******************************************************************************
 *
 ******************************************************************************/
IFeaturePipe*
IFeaturePipe::createInstance(char const* szCallerName, MUINT32 openedSensorIndex)
{
    FUNCTION_LOG_START;
    return new FeaturePipe(openedSensorIndex);
    FUNCTION_LOG_END;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
FeaturePipe::destroyInstance(char const* szCallerName)
{
    FUNCTION_LOG_START;
    delete this;
    FUNCTION_LOG_END;
}

/*******************************************************************************
*
********************************************************************************/
FeaturePipe::FeaturePipe(MUINT32 openedSensorIndex)
            : mOpenedSensorIndex(openedSensorIndex)
            , mInited(MFALSE)
            , mFeatureOnMask(0)
            , mpCamMsgCbInfo(NULL)
            , mEnqueCount(0)
#ifdef USING_MTK_CPUCTRL
            , mpCpuCtrl(NULL)
            , misCpuCtrl(MFALSE)
            , misCpuCtrl_count(0)
#endif
{
   //
    mpCamGraph = ICamGraph::createInstance(openedSensorIndex,"FeaturePipe");
}


/*******************************************************************************
*
********************************************************************************/
FeaturePipe::~FeaturePipe()
{
    mpCamGraph->destroyInstance();

}



/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FeaturePipe::init()
{
    Mutex::Autolock autoLock(mModuleMtx);
    FUNCTION_LOG_START;
    bool ret = true;
    ret = initFeatureGraph();
    mInited = MTRUE;
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FeaturePipe::init(UINT32 iFeatureMask)
{
    Mutex::Autolock autoLock(mModuleMtx);
    FUNCTION_LOG_START;
    bool ret = true;	
    ret = setFeature(iFeatureMask);
    mInited = MTRUE;
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FeaturePipe::uninit()
{
    Mutex::Autolock autoLock(mModuleMtx);
    FUNCTION_LOG_START;
    bool ret = MFALSE;
    if (!mInited)
        return ret;
    if(mpVFBCBThread != NULL)
        mpVFBCBThread->requestExit();
    ret = uninitFeatureGraph();
    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FeaturePipe::enque(QParams const& rParams)
{
    LOG_DBG("FrameNo(%d)", getMagicNo(rParams));

    Mutex::Autolock autoLock(mModuleMtx);
    //FUNCTION_LOG_START;
    bool ret = MFALSE;
    if (!mInited)
        return ret;

    char mkDebugFlag[PROPERTY_VALUE_MAX] = "0";
    property_get("vendor.debug.fpipe.p2a.mk", mkDebugFlag, "0");
#ifdef FORCE_P2A_MKDBG
    mkDebugFlag[0] = 1;
#endif
    if( mkDebugFlag[0] != '0' ) { LOG_DBG("mkdbg_fp-enque_001: frm: %d", getMagicNo(rParams)); }

    /* Post to graph nodes */
    ret = enqueFeatureGraph(rParams);

    if( mkDebugFlag[0] != '0' ) { LOG_DBG("mkdbg_fp-enque_002: frm: %d", getMagicNo(rParams)); }

    //FUNCTION_LOG_END;
    return ret;

}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FeaturePipe::deque(QParams& rParams,MINT64 i8TimeoutNs)
{
    //Mutex::Autolock autoLock(mModuleMtx);
    FUNCTION_LOG_START;
    bool ret = MFALSE;
    if (!mInited)
        return ret;

    /* Deque from graph nodes */
    ret = dequeFeatureGraph(rParams, i8TimeoutNs);

    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 * Set camera message-callback information.
 ******************************************************************************/
void
FeaturePipe::setCallbacks(sp<CamMsgCbInfo> const& rpCamMsgCbInfo)
{
    Mutex::Autolock _l(mModuleMtx);
    //
    //  value copy
    mpCamMsgCbInfo = rpCamMsgCbInfo;
}


/******************************************************************************
 *
 ******************************************************************************/
void
FeaturePipe::enableMsgType(int32_t msgType)
{
    if (mpCamMsgCbInfo != NULL)
    {
        ::android_atomic_or(msgType, &mpCamMsgCbInfo->mMsgEnabled);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
FeaturePipe::disableMsgType(int32_t msgType)
{
    if (mpCamMsgCbInfo != NULL)
    {
        ::android_atomic_and(~msgType, &mpCamMsgCbInfo->mMsgEnabled);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FeaturePipe::performCallback(int32_t msgType, camera_frame_metadata_t msgData)
{
    (void)msgType;
    bool ret = MFALSE;

#if 0
    //single thread version
    if ((mpCamMsgCbInfo != NULL) &&
            (msgType == (msgType & ::android_atomic_release_load(&mpCamMsgCbInfo->mMsgEnabled))))
    {
        camera_memory_t* dummyBuffer = mpCamMsgCbInfo->mRequestMemory(-1, 1, 1, mpCamMsgCbInfo->mCbCookie);
        if  ( dummyBuffer )
        {
            LOG_DBG("Callback with msgType(%d)", msgType);
            mpCamMsgCbInfo->mDataCb(
                msgType,
                dummyBuffer,
                0,
                &msgData,
                mpCamMsgCbInfo->mCbCookie
            );
            dummyBuffer->release(dummyBuffer);
            ret = MTRUE;
        }
    }
#else
    CMD_Q_T cmd;
    cmd.isFDCB = true;
    cmd.isSDCB = false;
    cmd.isGSCB = false;
    cmd.isStopCmd = false;
    //cmd.FrameMeta.number_of_faces = msgData.number_of_faces;
    //cmd.FrameMeta.faces = msgData.faces;
    cmd.FrameMeta = msgData;
    memcpy(cmd.Faces, msgData.faces, sizeof(camera_face_t)*cmd.FrameMeta.number_of_faces);

    int isPost = false;
    isPost = mpVFBCBThread->postCommand(cmd);
    if(!isPost) {
        LOG_WRN("Post command fail!");
    }
    else {
        ret = MTRUE;
    }
#endif

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FeaturePipe::performCallback(int32_t msgType, int32_t msgExt)
{
    (void)msgType;
    bool ret = MFALSE;

#if 0
    //single thread version
    if ((mpCamMsgCbInfo != NULL) &&
            (msgType == (msgType & ::android_atomic_release_load(&mpCamMsgCbInfo->mMsgEnabled))))
    {
        if ((msgExt == MTK_CAMERA_MSG_EXT_NOTIFY_GESTURE_DETECT) || (msgExt == MTK_CAMERA_MSG_EXT_NOTIFY_SMILE_DETECT))
        {
            LOG_DBG("Notify MTK_CAMERA_MSG_EXT_NOTIFY shot...");
            mpCamMsgCbInfo->mNotifyCb(
                MTK_CAMERA_MSG_EXT_NOTIFY,
                msgExt,
                0,
                mpCamMsgCbInfo->mCbCookie
            );

            ret = MTRUE;
        }
    }
#else
    CMD_Q_T cmd;
    cmd.isFDCB = false;
    cmd.isSDCB = false;
    cmd.isGSCB = false;
    cmd.isStopCmd = false;
    if (msgExt == MTK_CAMERA_MSG_EXT_NOTIFY_SMILE_DETECT)
    {
        cmd.isSDCB = true;
    }
    else if(msgExt == MTK_CAMERA_MSG_EXT_NOTIFY_GESTURE_DETECT)
    {
        cmd.isGSCB = true;
    }

    int isPost = false;
    isPost = mpVFBCBThread->postCommand(cmd);
    if(!isPost) {
        LOG_WRN("Post command fail!");
    }
    else {
        ret = MTRUE;
    }
#endif

    return ret;
}
