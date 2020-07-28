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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/feature/3dnr/lmv_state.h>
#include <mtkcam/feature/DualCam/DualCam.Common.h>
#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe_var.h>
//
#include <cutils/properties.h>

#ifdef LOG_TAG
    #undef LOG_TAG
#endif

#define LOG_TAG "MtkCam/DualCamPipe"

#include "DualCamStreamingFeaturePipe.h"

#include <mtkcam/feature/stereo/StereoCamEnum.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>

#if (MTKCAM_HAVE_VSDOF_SUPPORT == 1) && (MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1)
#include "DualCamBokehFeaturePipe.h"
#define DUALCAM_BOKEH 1
#else
#define DUALCAM_BOKEH 0
#endif

#define PIPE_CLASS_TAG LOG_TAG
#define PIPE_TRACE TRACE_STREAMING_FEATURE_PIPE

#undef MY_LOGV
#undef MY_LOGD
#undef MY_LOGI
#undef MY_LOGW
#undef MY_LOGE
#undef MY_LOGA
#undef MY_LOGF

#undef MY_LOGV_IF
#undef MY_LOGD_IF
#undef MY_LOGI_IF
#undef MY_LOGW_IF
#undef MY_LOGE_IF
#undef MY_LOGA_IF
#undef MY_LOGF_IF

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

#define FUNCTION_IN(id)             CAM_LOGD("[%d][%s]+", id, __FUNCTION__)
#define FUNCTION_OUT(id)            CAM_LOGD("[%d][%s]-", id, __FUNCTION__)

namespace NSCam
{
namespace NSCamFeature
{
namespace NSFeaturePipe
{

IDualCamStreamingFeaturePipe*
IDualCamStreamingFeaturePipe::createInstance(
    MUINT32 sensorIndex,
    const UsageHint& usageHint)
{
    IDualCamStreamingFeaturePipe* DualCamSFPipe = nullptr;
#if DUALCAM_BOKEH
    MINT32 featureMode = StereoSettingProvider::getStereoFeatureMode();
    if((v1::Stereo::E_STEREO_FEATURE_CAPTURE | v1::Stereo::E_STEREO_FEATURE_VSDOF) == featureMode
            || (v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP) == featureMode)
    {
        DualCamSFPipe = new DualCamBokehFeaturePipe(sensorIndex, usageHint);
    }
    else
    {
        DualCamSFPipe = new DualCamStreamingFeaturePipe(sensorIndex, usageHint);
    }
#else
    DualCamSFPipe = new DualCamStreamingFeaturePipe(sensorIndex, usageHint);
#endif
    return DualCamSFPipe;
}

/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL getLocalMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T& rVal)
{
    if (pMetadata == NULL)
    {
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if (!entry.isEmpty())
    {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
inline MBOOL getLocalMetadataEntry(
    IMetadata* pMetadata,
    MUINT32 const tag,
    IMetadata::IEntry &entry)
{
    if (pMetadata == NULL)
    {
        return MFALSE;
    }

    entry = pMetadata->entryFor(tag);
    if (!entry.isEmpty())
    {
        return MTRUE;
    }
    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID IDualCamStreamingFeaturePipe::prepareFeatureData(FeaturePipeParam& pipeParam,
        MUINT32 frameNo,
        IMetadata* pMetaInApp, IMetadata* pMetaInHal,
        IMetadata* pMetaOutApp, IMetadata* pMetaOutHal,
        MINT usage)
{
    MINT32 doSync = 0;
    MINT64 timestamp = 0;
    pipeParam.mVarMap.set<MUINT32>(VAR_DUALCAM_FRAME_NO, frameNo);

    if(getLocalMetadata(pMetaInHal, MTK_DUALCAM_TIMESTAMP, timestamp))
    {
        pipeParam.mVarMap.set<MINT64>(VAR_DUALCAM_TIMESTAMP, timestamp);
    }
    else
    {
        if(getLocalMetadata(pMetaInHal, MTK_P1NODE_FRAME_START_TIMESTAMP, timestamp))
        {
            pipeParam.mVarMap.set<MINT64>(VAR_DUALCAM_TIMESTAMP, timestamp);
        }
    }

    getLocalMetadata(pMetaInHal, MTK_DUALZOOM_DO_FRAME_SYNC, doSync);
    pipeParam.mVarMap.set<MINT32>(VAR_DUALCAM_DO_SYNC, doSync);

    MINT32 dropReq = 0;
    getLocalMetadata(pMetaInHal, MTK_DUALZOOM_DROP_REQ, dropReq);
    pipeParam.mVarMap.set<MINT32>(VAR_DUALCAM_DROP_REQ, dropReq);

    MY_LOGD("frameNo:%d doSync:%d timestamp: %lld dropReq:%d usage:%d",
             frameNo,
             doSync,
             timestamp,
             dropReq,
             usage);

    // pack feature params by feature usage.
    switch(usage)
    {
        case NSCam::v1::Stereo::E_DUALCAM_FEATURE_ZOOM:
            prepareDualCamZoomFeatureData(
                                    pipeParam,
                                    pMetaInApp,
                                    pMetaInHal,
                                    pMetaOutApp,
                                    pMetaOutHal);
            break;
        case NSCam::v1::Stereo::E_STEREO_FEATURE_DENOISE:
            prepareDualCamDenoiseFeatureData(
                                    pipeParam,
                                    pMetaInApp,
                                    pMetaInHal,
                                    pMetaOutApp,
                                    pMetaOutHal);
            break;
        case (NSCam::v1::Stereo::E_STEREO_FEATURE_CAPTURE | NSCam::v1::Stereo::E_STEREO_FEATURE_VSDOF):
        case (NSCam::v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP):
            prepareDualCamVSDOFFeatureData(
                                    pipeParam,
                                    pMetaInApp,
                                    pMetaInHal,
                                    pMetaOutApp,
                                    pMetaOutHal);
            break;
        default:
            MY_LOGD("not support usage (%d)", usage);
            break;
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID IDualCamStreamingFeaturePipe::prepareDualCamZoomFeatureData(FeaturePipeParam& pipeParam,
        IMetadata* pMetaInApp, IMetadata* pMetaInHal,
        IMetadata* pMetaOutApp, IMetadata* pMetaOutHal)
{
    MINT32 zoomRatio = 0;
    getLocalMetadata(pMetaInHal, MTK_DUALZOOM_ZOOMRATIO, zoomRatio);
    IMetadata::IEntry entry;
    MRect  P1Crop;
    getLocalMetadata(pMetaInHal, MTK_P1NODE_SCALAR_CROP_REGION, P1Crop);
    if (getLocalMetadataEntry(pMetaInHal, MTK_DUALZOOM_FOV_RECT_INFO, entry))
    {
        MINT32 info[sizeof(DUAL_ZOOM_FOV_INPUT_INFO) / 4];
        DUAL_ZOOM_FOV_INPUT_INFO* pData = reinterpret_cast<DUAL_ZOOM_FOV_INPUT_INFO*>(info);
        MSize  FovMargin;

        getLocalMetadata(pMetaInHal, MTK_DUALZOOM_FOV_MARGIN_PIXEL, FovMargin);
        if (entry.count() < (sizeof(DUAL_ZOOM_FOV_INPUT_INFO) / 4))
        {
            MY_LOGW("FOV info not enough, skip FOV");
        }
        else
        {
            for (unsigned int i = 0; i < sizeof(DUAL_ZOOM_FOV_INPUT_INFO) / 4; i++)
            {
                info[i] = entry.itemAt(i, Type2Type<MINT32>());
            }
            if (P1Crop.s.w != 0 && P1Crop.s.h != 0 && FovMargin.w != 0 && FovMargin.h != 0)
            {
                if (info[4] != P1Crop.s.w || info[5] != P1Crop.s.h)
                {
                    FovMargin.h = FovMargin.w * P1Crop.s.h / P1Crop.s.w;
                    info[4] = P1Crop.s.w;
                    info[5] = P1Crop.s.h;
                    info[6] = P1Crop.s.w - FovMargin.w;
                    info[7] = P1Crop.s.h - FovMargin.h;
                }
            }
            #if (MTKCAM_HAVE_DUAL_ZOOM_VENDOR_FOV_SUPPORT==1)
            ENABLE_VENDOR_FOV(pipeParam.mFeatureMask);
            #else
            ENABLE_FOV(pipeParam.mFeatureMask);
            #endif // MTKCAM_HAVE_DUAL_ZOOM_VENDOR_FOV_SUPPORT

            pipeParam.mVarMap.set<DUAL_ZOOM_FOV_INPUT_INFO>(VAR_DUALCAM_FOV_RECT, *pData);
        }
    }
    if (getLocalMetadataEntry(pMetaInHal, MTK_DUALZOOM_FOV_CALB_INFO, entry))
    {
        MINT32 info[sizeof(DUAL_ZOOM_FOV_INPUT_INFO) / 4];
        DUAL_ZOOM_FOV_FEFM_INFO* pData = reinterpret_cast<DUAL_ZOOM_FOV_FEFM_INFO*>(info);

        if (entry.count() < (sizeof(DUAL_ZOOM_FOV_FEFM_INFO) / 4))
        {
            MY_LOGW("FEFM info not enough, skip FEFM");
            pipeParam.mVarMap.set<MINT32>(VAR_DUALCAM_DO_SYNC, 0);
        }
        else
        {
            for (unsigned int i = 0; i < sizeof(DUAL_ZOOM_FOV_FEFM_INFO) / 4; i++)
            {
                info[i] = entry.itemAt(i, Type2Type<MINT32>());
            }
            if (P1Crop.s.w != 0 && P1Crop.s.h != 0)
            {
                if (info[2] != P1Crop.s.w || info[3] != P1Crop.s.h)
                {
                    info[2] = P1Crop.s.w;
                    info[3] = P1Crop.s.h;
                }
            }

            pipeParam.mVarMap.set<DUAL_ZOOM_FOV_FEFM_INFO>(VAR_DUALCAM_FOV_CALIB_INFO, *pData);
        }
    }
    pipeParam.mVarMap.set<MUINT32>(VAR_DUALCAM_ZOOM_RATIO, (MUINT32)zoomRatio);


    // for 3DNR/LMV
    MINT32 lmv_result = -1;
    getLocalMetadata(pMetaInHal, MTK_LMV_SWITCH_OUT_RESULT, lmv_result);
    pipeParam.mVarMap.set<MINT32>(VAR_LMV_SWITCH_OUT_RST, lmv_result);

    MINT32 lmv_validity = -1;
    getLocalMetadata(pMetaInHal, MTK_LMV_VALIDITY, lmv_validity);
    pipeParam.mVarMap.set<MINT32>(VAR_LMV_VALIDITY, lmv_validity);

    // for dynamic twin
    MINT32 twin_status = -1;
    getLocalMetadata(pMetaInHal, MTK_P1NODE_TWIN_STATUS, twin_status);
    pipeParam.mVarMap.set<MINT32>(VAR_P1RAW_TWIN_STATUS, twin_status);

    MY_LOGD("zoomRatio:%d lmv_result:%d lmvValidity:%d", zoomRatio, lmv_result, lmv_validity);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID IDualCamStreamingFeaturePipe::prepareDualCamDenoiseFeatureData(FeaturePipeParam& pipeParam,
        IMetadata* pMetaInApp, IMetadata* pMetaInHal,
        IMetadata* pMetaOutApp, IMetadata* pMetaOutHal)
{
    MUINT8 afState = 0;
    MUINT8 lenState = 0;
    if(pMetaInHal != nullptr)
    {
        if(!getLocalMetadata(pMetaInHal, MTK_DUALCAM_AF_STATE, afState))
        {
            MY_LOGD("Cannot get af state");
        }
        pipeParam.mVarMap.set<MUINT8>(VAR_DUALCAM_AF_STATE, afState);
        // get lens state
        if(!getLocalMetadata(pMetaInHal, MTK_DUALCAM_LENS_STATE, lenState))
        {
            MY_LOGD("Cannot get lens state");
        }
        pipeParam.mVarMap.set<MUINT8>(VAR_DUALCAM_LENS_STATE, lenState);
        // get iso value from exif
        MINT32 newISO = -1;
        IMetadata exifMeta;
        if(getLocalMetadata<IMetadata>(pMetaInHal, MTK_3A_EXIF_METADATA, exifMeta))
        {
            if(!getLocalMetadata<MINT32>(&exifMeta, MTK_3A_EXIF_AE_ISO_SPEED, newISO))
            {
                MY_LOGW("Get ISO failed");
            }
        }
        pipeParam.mVarMap.set<MINT32>(VAR_DUALCAM_EXIF_ISO, newISO);

        ENABLE_N3D(pipeParam.mFeatureMask);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID IDualCamStreamingFeaturePipe::prepareDualCamVSDOFFeatureData(FeaturePipeParam& pipeParam,
        IMetadata* pMetaInApp, IMetadata* pMetaInHal,
        IMetadata* pMetaOutApp, IMetadata* pMetaOutHal)
{
}

}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam
