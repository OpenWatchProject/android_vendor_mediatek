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

#define LOG_TAG "MtkCam/BuilderBase"

#include "MyUtils.h"
using namespace NSCam;

#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include "PipelineBuilderBase.h"

#if MTKCAM_HAVE_VSDOF_SUPPORT
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#endif // MTKCAM_HAVE_VSDOF_SUPPORT

using namespace NSCam::v1::NSLegacyPipeline;
/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
#define MY_LOGD1(...)               MY_LOGD_IF((mLogLevel>=1),__VA_ARGS__)
#define MY_LOGD2(...)               MY_LOGD_IF((mLogLevel>=2),__VA_ARGS__)
#define MY_LOGD3(...)               MY_LOGD_IF((mLogLevel>=3),__VA_ARGS__)
//
#define FUNC_START                  MY_LOGD1("+")
#define FUNC_END                    MY_LOGD1("-")
//
/******************************************************************************
*
*******************************************************************************/
PipelineBuilderBase::
PipelineBuilderBase(
    MINT32 const openId,
    char const* pipeName,
    ConfigParams const & rParam
)
    : mLogLevel(1)
    , mConfigParams(rParam)
    , mFps(30)
    , mBurstNum(1)
    , mOpenId(openId)
    , mPipeName(pipeName)
    , mbIsSensor(MFALSE)
    , mpDispatcher(NULL)
    , mpControl(NULL)
{
    MY_LOGD("Mode(%d),EIS(%d),LCS(%d),RSS(%d),FSC(%d),DualPD(%d) EisMode(%d), BurstNum(%d), support4cellsensor(%d), enableRrzoCapture(%d), initRequest(%d)",
            mConfigParams.mode,
            mConfigParams.enableEIS,
            mConfigParams.enableLCS,
            mConfigParams.enableRSS,
            mConfigParams.enableFSC,
            mConfigParams.enableDualPD,
            mConfigParams.eisMode,
            mConfigParams.enableBurstNum,
            mConfigParams.support4cellsensor,
            mConfigParams.enableRrzoCapture,
            mConfigParams.initRequest);
}


/******************************************************************************
*
*******************************************************************************/
MERROR
PipelineBuilderBase::
setDst(Vector<PipelineImageParam> const& vParams)
{
    MERROR ret = OK;
    int featureMode = -1;
    for (size_t i = 0; i < vParams.size(); i++) {
        if  ( IImageStreamInfo* pStreamInfo = vParams[i].pInfo.get() )
        {
            switch  (pStreamInfo->getImgFormat())
            {
                case eImgFmt_BAYER8:
                case eImgFmt_BAYER10:
                case eImgFmt_BAYER10_UNPAK:
                case eImgFmt_FG_BAYER10:
                case eImgFmt_BAYER12:
                case eImgFmt_FG_BAYER12:
                case eImgFmt_BAYER14:
                case eImgFmt_FG_BAYER14:
                case eImgFmt_RAW16:
                case eImgFmt_STA_2BYTE:
                case eImgFmt_UFO_BAYER8:
                case eImgFmt_UFO_BAYER10:
                case eImgFmt_UFO_BAYER12:
                case eImgFmt_UFO_BAYER14:
                case eImgFmt_UFO_FG_BAYER8:
                case eImgFmt_UFO_FG_BAYER10:
                case eImgFmt_UFO_FG_BAYER12:
                case eImgFmt_UFO_FG_BAYER14:
                    mDst_Raw.push_back(vParams[i]);
                    break;
                    //
                case eImgFmt_BLOB:
                    mDst_Jpeg.push_back(vParams[i]);
                    break;
                    //
                case eImgFmt_YV12:
                case eImgFmt_NV12:
                case eImgFmt_NV21:
                case eImgFmt_Y8:
                case eImgFmt_Y16:
                case eImgFmt_I422:
                case eImgFmt_I420:
                case eImgFmt_RGBA8888:
                    mDst_Yuv.push_back(vParams[i]);
                    break;
                    //
                case eImgFmt_YUY2:
                    if(    pStreamInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE
                        || pStreamInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_RESIZER )
                    {
                        mDst_Raw.push_back(vParams[i]);
                    }
                    else
                    {
                        mDst_Yuv.push_back(vParams[i]);
                    }
                    break;
                    //
                case eImgFmt_UYVY:
                case eImgFmt_VYUY:
                case eImgFmt_YVYU:
                    if(    pStreamInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE
                        || pStreamInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_RESIZER )
                    {
                        mDst_Raw.push_back(vParams[i]);
                    }
                    else
                    {
                        MY_LOGE("Unsupported format:0x%x, stream:0x%x", pStreamInfo->getImgFormat(), pStreamInfo->getStreamId());
                        ret = BAD_VALUE;
                    }
                    break;
                    //
                case eImgFmt_STA_BYTE:
#if MTKCAM_HAVE_VSDOF_SUPPORT
                    featureMode = StereoSettingProvider::getStereoFeatureMode();
                    if(featureMode == (Stereo::E_STEREO_FEATURE_CAPTURE|Stereo::E_STEREO_FEATURE_VSDOF)
                       || featureMode == (Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP))
                    {
                        mDst_Yuv.push_back(vParams[i]);
                    }
                    else
                    {
                        mDst_Raw.push_back(vParams[i]);
                    }
#else
                    mDst_Raw.push_back(vParams[i]);
#endif // MTKCAM_HAVE_VSDOF_SUPPORT
                    break;
                default:
                    MY_LOGE("Unsupported format:0x%x", pStreamInfo->getImgFormat());
                    ret = BAD_VALUE;
                    break;
            }
        }
    }
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
MVOID
PipelineBuilderBase::
update3AConfigMeta(IMetadata *appMeta, IMetadata *halMeta)
{
    if(appMeta == NULL || halMeta == NULL)
    {
        MY_LOGE("3A init config Meta is NULL! app(%p), hal(%p)", appMeta, halMeta);
        return;
    }
    Hal3AParams &para = mConfigParams.hal3AParams;
    // -- Update Hal Meta ----
    if(para.isIspProfileValid())
        IMetadata::setEntry<MUINT8>(halMeta, MTK_3A_ISP_PROFILE, para.getIspProfile());
    if(para.isHdrModeValid())
        IMetadata::setEntry<MUINT8>(halMeta, MTK_3A_HDR_MODE, para.getHdrMode());

    // -- Update App Meta ---
    if(para.isMinFpsValid() && para.isMaxFpsValid())
    {
        IMetadata::IEntry entry(MTK_CONTROL_AE_TARGET_FPS_RANGE);
        entry.push_back(para.getMinFps(), Type2Type< MINT32 >());
        entry.push_back(para.getMaxFps(), Type2Type< MINT32 >());
        appMeta->update(entry.tag(), entry);
    }

    MY_LOGD("Hal: prof(%d/%u), hdrMode(%d/%u)",
            para.isIspProfileValid(), para.getIspProfile(), para.isHdrModeValid(), para.getHdrMode());
    MY_LOGD("App: minFps(%d/%d), maxFps(%d/%d)",
            para.isMinFpsValid(), para.getMinFps(), para.isMaxFpsValid(), para.getMaxFps());
}
