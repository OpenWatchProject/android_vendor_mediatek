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

#define LOG_TAG "MtkCam/HwPipeline"
//
#include "PipelineDefaultImp.h"
#include "PipelineUtility.h"
//
#include <mtkcam/drv/IHalSensor.h>
//
#include <mtkcam/pipeline/hwnode/P1Node.h>
#include <mtkcam/pipeline/hwnode/P2Node.h>
#include <mtkcam/pipeline/hwnode/FDNode.h>
#include <mtkcam/pipeline/hwnode/JpegNode.h>
#include <mtkcam/pipeline/hwnode/RAW16Node.h>
#include <mtkcam/pipeline/extension/MFNR.h>
//
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam/utils/hw/IScenarioControl.h>
#include <mtkcam/utils/hw/HwTransform.h>
//
#include <cutils/compiler.h>
//

using namespace android;
using namespace NSCam;
using namespace NSCam::EIS;
using namespace NSCam::v3;
using namespace NSCam::Utils;
using namespace NSCam::v3::Utils;
using namespace NSCam::v3::NSPipelineContext;


/******************************************************************************
 *
 ******************************************************************************/
PipelineModel_Default*
PipelineModel_Default::
create(MINT32 const openId, wp<IPipelineModelMgr::IAppCallback> pAppCallback)
{
    String8 const name = String8::format("%s:%d", magicName(), openId);
    PipelineModel_Default* pPipelineModel = new PipelineDefaultImp(openId, name, pAppCallback);
    if  ( ! pPipelineModel ) {
        MY_LOGE("fail to new an instance");
        return NULL;
    }
    //
    return pPipelineModel;
}

/******************************************************************************
 *
 ******************************************************************************/
PipelineDefaultImp::
PipelineDefaultImp(
    MINT32 const openId,
    android::String8 const& name,
    wp<IPipelineModelMgr::IAppCallback> pAppCallback
)
    : mCommonInfo(std::make_shared<CommonInfo>(openId, name))
    , mParams(std::make_shared<MyProcessedParams>(openId))
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("jpeg.rotation.enable", value, "1");
    int32_t enable = atoi(value);
    mCommonInfo->mJpegRotationEnable = (enable & 0x1)? MTRUE : MFALSE;
    MY_LOGD("Jpeg Rotation enable: %d", mCommonInfo->mJpegRotationEnable);
    //

    mCommonInfo->mCameraSetting = std::make_shared<CameraSettingMgr_Imp>(openId);

    mCommonInfo->mLogLevel = ::property_get_int32("debug.camera.log", 0);
    if ( mCommonInfo->mLogLevel == 0 ) {
        mCommonInfo->mLogLevel = ::property_get_int32("debug.camera.log.hwpipeline", 0);
    }
    mParams->mSupportJpegRot = mCommonInfo->mJpegRotationEnable;

    mConfigHandler  = new ConfigHandler(mCommonInfo, mParams);
    if  ( mConfigHandler == nullptr ) {
        MY_LOGE("Bad mConfigHandler");
    }

    mRequestHandler = new RequestHandler(mCommonInfo, mParams, pAppCallback, mConfigHandler);
    if  ( mRequestHandler == nullptr ) {
        MY_LOGE("Bad mRequestHandler");
    }
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineDefaultImp::
~PipelineDefaultImp()
{
    FUNC_START;
    //
#if MTKCAM_HAVE_IVENDOR_SUPPORT
    plugin::NSVendorManager::remove(getOpenId());
#endif
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
void
PipelineDefaultImp::
onLastStrongRef(const void* /*id*/)
{
    FUNC_START;
    FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
/**
 * Given:
 *      App input meta streams
 *      App in/out image streams
 *
 * Action:
 *      Determine CONFIG stream set
 *      Determine I/O streams of each node
 *      Prepare Hal stream pools
 *      Configure each node (with their streams)
 *
 */
MERROR
PipelineDefaultImp::
configure(
    PipeConfigParams const& rConfigParams,
    android::sp<IPipelineModel> pOldPipeline
)
{
    RWLock::AutoRLock _l(mRWLock);
    if( mConfigHandler==nullptr )
        return DEAD_OBJECT;

    return mConfigHandler->configureLocked(rConfigParams, pOldPipeline);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
submitRequest(AppRequest& request)
{
    RWLock::AutoRLock _l(mRWLock);
    if( mRequestHandler==nullptr )
        return DEAD_OBJECT;

    return mRequestHandler->submitRequestLocked(request);
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
beginFlush()
{
    FUNC_START;
    //
    if( mCommonInfo->mpPipelineContext.get() )
        mCommonInfo->mpPipelineContext->flush();
    else
        MY_LOGW("no context");
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
PipelineDefaultImp::
endFlush()
{
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
PipelineDefaultImp::
waitDrained()
{
    FUNC_START;
    if( mCommonInfo->mpPipelineContext.get() )
        mCommonInfo->mpPipelineContext->waitUntilDrained();
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<PipelineContext>
PipelineDefaultImp::
getContext()
{
    FUNC_START;
    if( mCommonInfo->mpPipelineContext.get() )
        return mCommonInfo->mpPipelineContext;
    FUNC_END;
    return NULL;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
PipelineDefaultImp::
skipStream(
    MBOOL skipJpeg,
    IImageStreamInfo* pStreamInfo
) const
{
    if  (
            skipJpeg
        &&  pStreamInfo->getImgFormat() == HAL_PIXEL_FORMAT_BLOB
        &&  pStreamInfo->getImgSize().size() >= 1920*1080
        )
    {
 //&& limited mode
        return MTRUE;
    }

    return MFALSE;
}
