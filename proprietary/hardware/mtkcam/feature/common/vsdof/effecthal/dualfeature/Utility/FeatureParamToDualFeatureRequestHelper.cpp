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
#define LOG_TAG "MtkCam/RequestConvert"
//
#include <mtkcam/pipeline/stream/IStreamInfo.h>
#include <mtkcam/pipeline/stream/IStreamBuffer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/feature/stereo/effecthal/DualFeatureEffectHal.h>
#include <mtkcam/feature/stereo/effecthal/FeatureParamToDualFeatureRequestHelper.h>
//
#include <utils/RWLock.h>
#include <utils/Thread.h>
#include <utils/String8.h>
#include <cutils/properties.h>
//
#include <mtkcam/utils/std/Misc.h>
#include <mtkcam/utils/std/Log.h>
//
#include <list>
#include <algorithm>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::NSCamFeature;
using namespace NSCam::NSCamFeature::NSFeaturePipe;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

// ----------------------------------------------------------------------------
// function utility
// ----------------------------------------------------------------------------
// function scope
#define __DEBUG
#define __SCOPE_TIMER
#ifdef __DEBUG
#define FUNCTION_SCOPE      auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#include <memory>
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] +",pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}
#else
#define FUNCTION_SCOPE
#endif // function scope
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
FeatureParamToDualFeatureRequestHelper::
FeatureParamToDualFeatureRequestHelper()
{
    FUNCTION_SCOPE;
    mStreamIDMapToDualBID_main1.clear();
    mStreamIDMapToDualBID_main2.clear();
    // for main1 setting
    // input image stream
    mStreamIDMapToDualBID_main1.add(eSTREAMID_IMAGE_PIPE_RAW_RESIZER, NSDualFeature::RBID_IN_RESIZERAW_MAIN1);
    mStreamIDMapToDualBID_main1.add(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, NSDualFeature::RBID_IN_FULLRAW_MAIN1);
    mStreamIDMapToDualBID_main1.add(eSTREAMID_IMAGE_PIPE_RAW_LCSO, NSDualFeature::RBID_IN_LCSO_MAIN1);
    // output image stream
    mStreamIDMapToDualBID_main1.add(eSTREAMID_IMAGE_YUV_FD, NSDualFeature::RBID_OUT_FD);
    mStreamIDMapToDualBID_main1.add(eSTREAMID_IMAGE_PIPE_YUV_00, NSDualFeature::RBID_OUT_PREVIEW);  // preview
    mStreamIDMapToDualBID_main1.add(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DMBGYUV, NSDualFeature::RBID_OUT_BLUR_MAP);  // preview
    mStreamIDMapToDualBID_main1.add(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DEPTHMAPYUV, NSDualFeature::RBID_OUT_DEPTHMAP);  // preview
    mStreamIDMapToDualBID_main1.add(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGEYUV, NSDualFeature::RBID_OUT_MAINIMAGE);  // preview
    mStreamIDMapToDualBID_main1.add(eSTREAMID_IMAGE_PIPE_YUV_01, NSDualFeature::RBID_OUT_PREVIEW_CALLBACK);  // preview callback
    // input metadata stream
    mStreamIDMapToDualBID_main1.add(eSTREAMID_META_APP_CONTROL, NSDualFeature::RBID_IN_APP_META_MAIN1);
    mStreamIDMapToDualBID_main1.add(eSTREAMID_META_APP_DYNAMIC_P1, NSDualFeature::RBID_IN_P1_RETURN_META);  // for main1?
    mStreamIDMapToDualBID_main1.add(eSTREAMID_META_HAL_DYNAMIC_P1, NSDualFeature::RBID_IN_HAL_META_MAIN1);
    mStreamIDMapToDualBID_main1.add(eSTREAMID_META_APP_DYNAMIC_P2, NSDualFeature::RBID_OUT_APP_META);  // for main1?
    mStreamIDMapToDualBID_main1.add(eSTREAMID_META_HAL_DYNAMIC_P2, NSDualFeature::RBID_OUT_HAL_META);  // for main1?
    // for main2 setting
    // input image stream
    mStreamIDMapToDualBID_main2.add(eSTREAMID_IMAGE_PIPE_RAW_RESIZER, NSDualFeature::RBID_IN_RESIZERAW_MAIN2);
    mStreamIDMapToDualBID_main2.add(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, NSDualFeature::RBID_IN_FULLRAW_MAIN2);
    mStreamIDMapToDualBID_main2.add(eSTREAMID_IMAGE_PIPE_RAW_LCSO, NSDualFeature::RBID_IN_LCSO_MAIN2);
    // input metadata stream
    mStreamIDMapToDualBID_main2.add(eSTREAMID_META_APP_CONTROL, NSDualFeature::RBID_IN_APP_META_MAIN2);
    mStreamIDMapToDualBID_main2.add(eSTREAMID_META_HAL_DYNAMIC_P1, NSDualFeature::RBID_IN_HAL_META_MAIN2);
    //
    mLogLevel = ::property_get_int32("vendor.debug.STEREO.log.pack", 0);
    MY_LOGD("log level(%d)", mLogLevel);
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
FeatureParamToDualFeatureRequestHelper::
~FeatureParamToDualFeatureRequestHelper()
{
    FUNCTION_SCOPE;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
MERROR
FeatureParamToDualFeatureRequestHelper::
convertToEffectRequest(
    const std::shared_ptr<NSCam::v3::DualFeature::process_frame> pFrame_main1,
    const std::shared_ptr<NSCam::v3::DualFeature::process_frame> pFrame_main2,
    sp<NSDualFeature::DualFeatureRequest>& pRequest
)
{
    if(pFrame_main1 == nullptr)
    {
        return INVALID_OPERATION;
    }
    //
    MERROR ret = INVALID_OPERATION;
    NSDualFeature::RequestBufferID bufID;
    NSDualFeature::NodeBufferSetting setting;
    String8 parse_result = String8("");
    MBOOL search_result = MFALSE;
#define PARSE_DATA_AND_PUSH_TO_EFFECT_REQUEST_IMG(CONTAINER, ISMAIN1, IO_TYPE)\
do{\
    for(ssize_t i=0;i<CONTAINER.size();++i)\
    {\
        search_result = mapToDepthBufID(ISMAIN1, CONTAINER.keyAt(i), bufID);\
        if(search_result == OK)\
        {\
            setting.bufferID = bufID;\
            setting.ioType = IO_TYPE;\
            android::sp<IImageBuffer> img = CONTAINER.valueAt(i);\
            pRequest->pushRequestImageBuffer(setting, img);\
            parse_result += String8::format("M[%d] IO[%d] S[%llx] B[%x] P[%p] \n", ISMAIN1, IO_TYPE, CONTAINER.keyAt(i), bufID, img.get());\
        }\
    }\
}while(0)
#define PARSE_DATA_AND_PUSH_TO_EFFECT_REQUEST_META(CONTAINER, ISMAIN1, IO_TYPE)\
do{\
    for(ssize_t i=0;i<CONTAINER.size();++i)\
    {\
        search_result = mapToDepthBufID(ISMAIN1, CONTAINER.keyAt(i), bufID);\
        if(search_result == OK)\
        {\
            setting.bufferID = bufID;\
            setting.ioType = IO_TYPE;\
            IMetadata* meta = CONTAINER.valueAt(i);\
            pRequest->pushRequestMetadata(setting, meta);\
            parse_result += String8::format("M[%d] IO[%d] S[%llx] B[%x] P[%p]\n", ISMAIN1, IO_TYPE, CONTAINER.keyAt(i), bufID, meta);\
        }\
    }\
}while(0)
    // main1
    {
        // set main1 input buffer
        PARSE_DATA_AND_PUSH_TO_EFFECT_REQUEST_IMG(
                            pFrame_main1->mvInImgBufContainer,
                            MTRUE, // is main1?
                            NSDualFeature::BufferIOType::eBUFFER_IOTYPE_INPUT);
        // set main1 output buffer
        PARSE_DATA_AND_PUSH_TO_EFFECT_REQUEST_IMG(
                            pFrame_main1->mvOutImgBufContainer,
                            MTRUE, // is main1?
                            NSDualFeature::BufferIOType::eBUFFER_IOTYPE_OUTPUT);
        // set main1 input metadata
        PARSE_DATA_AND_PUSH_TO_EFFECT_REQUEST_META(
                            pFrame_main1->mvInMetaBufferContainer,
                            MTRUE, // is main1?
                            NSDualFeature::BufferIOType::eBUFFER_IOTYPE_INPUT);
        // set main1 output metadata
        PARSE_DATA_AND_PUSH_TO_EFFECT_REQUEST_META(
                            pFrame_main1->mvOutMetaBufferContainer,
                            MTRUE, // is main1?
                            NSDualFeature::BufferIOType::eBUFFER_IOTYPE_OUTPUT);
    }
    // main2
    if(pFrame_main2 != nullptr)
    {
        // set main2 input buffer
        PARSE_DATA_AND_PUSH_TO_EFFECT_REQUEST_IMG(
                            pFrame_main2->mvInImgBufContainer,
                            MFALSE, // is main1?
                            NSDualFeature::BufferIOType::eBUFFER_IOTYPE_INPUT);
        // set main2 output buffer
        PARSE_DATA_AND_PUSH_TO_EFFECT_REQUEST_IMG(
                            pFrame_main2->mvOutImgBufContainer,
                            MFALSE, // is main1?
                            NSDualFeature::BufferIOType::eBUFFER_IOTYPE_OUTPUT);
        // set main2 input metadata
        PARSE_DATA_AND_PUSH_TO_EFFECT_REQUEST_META(
                            pFrame_main2->mvInMetaBufferContainer,
                            MFALSE, // is main1?
                            NSDualFeature::BufferIOType::eBUFFER_IOTYPE_INPUT);
        // set main2 output metadata
        PARSE_DATA_AND_PUSH_TO_EFFECT_REQUEST_META(
                            pFrame_main2->mvOutMetaBufferContainer,
                            MFALSE, // is main1?
                            NSDualFeature::BufferIOType::eBUFFER_IOTYPE_OUTPUT);
    }
#undef PARSE_DATA_AND_PUSH_TO_EFFECT_REQUEST_META
#undef PARSE_DATA_AND_PUSH_TO_EFFECT_REQUEST_IMG
    MY_LOGD_IF(mLogLevel > 0, "%s", parse_result.string());
    ret = OK;
lbExit:
    return ret;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
MERROR
FeatureParamToDualFeatureRequestHelper::
mapToDepthBufID(
    MBOOL bIsMain1,
    const StreamId_T streamId,
    NSDualFeature::RequestBufferID &bufID
)
{
    MERROR ret = INVALID_OPERATION;
    if(bIsMain1)
    {
        ssize_t index = mStreamIDMapToDualBID_main1.indexOfKey(streamId);
        if(index >= 0)
        {
            bufID = mStreamIDMapToDualBID_main1.valueAt(index);
            ret = OK;
        }
    }
    else
    {
        ssize_t index = mStreamIDMapToDualBID_main2.indexOfKey(streamId);
        if(index >= 0)
        {
            bufID = mStreamIDMapToDualBID_main2.valueAt(index);
            ret = OK;
        }
    }
    return ret;
}