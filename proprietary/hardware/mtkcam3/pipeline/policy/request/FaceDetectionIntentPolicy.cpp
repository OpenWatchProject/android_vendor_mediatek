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

#define LOG_TAG "mtkcam-FDIntentPolicy"

#include <mtkcam3/pipeline/policy/IFaceDetectionIntentPolicy.h>
#include <mtkcam/utils/std/ULog.h>

//
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);


/******************************************************************************
 *
 ******************************************************************************/
using namespace android;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)


/******************************************************************************
 *
 ******************************************************************************/
static auto isDisableFdForAppReprocess() -> bool
{
    return ::property_get_bool("persist.vendor.debug.camera3.disableFaceDetectionForAppReprocess", true);
}
static bool gIsDisableFdForAppReprocess = isDisableFdForAppReprocess();

static auto IsForceFDOn() -> bool
{
    return ::property_get_bool("persist.vendor.debug.camera3.ForceFDOn", false);
}

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {


/**
 * Make a function target as a policy - default version
 */
FunctionType_FaceDetectionIntentPolicy makePolicy_FDIntent_Default()
{
    using namespace fdintent;

    return [](
        RequestOutputParams& out __unused,
        RequestInputParams const& in __unused
    ) -> int
    {
        if ( ! in.hasFDNodeConfigured ) {
            out.isFdEnabled = false;
            return OK;
        }

        auto const pMetadata = in.pRequest_AppControl;
        if ( CC_UNLIKELY( !pMetadata ) ) {
            MY_LOGE("null app control input params");
            return -EINVAL;
        }

        IMetadata::IEntry const& entryFdMode    = pMetadata->entryFor(MTK_STATISTICS_FACE_DETECT_MODE);
        IMetadata::IEntry const& entryfaceScene = pMetadata->entryFor(MTK_CONTROL_SCENE_MODE);
        IMetadata::IEntry const& entryGdMode    = pMetadata->entryFor(MTK_FACE_FEATURE_GESTURE_MODE);
        IMetadata::IEntry const& entrySdMode    = pMetadata->entryFor(MTK_FACE_FEATURE_SMILE_DETECT_MODE);
        IMetadata::IEntry const& entryAsdMode   = pMetadata->entryFor(MTK_FACE_FEATURE_ASD_MODE);

        bool FDMetaEn;
        //
        FDMetaEn =   //(0 != mDebugFdMode) ||
             ( !entryFdMode.isEmpty() && MTK_STATISTICS_FACE_DETECT_MODE_OFF != entryFdMode.itemAt(0, Type2Type<MUINT8>())) ||
             ( !entryfaceScene.isEmpty() && MTK_CONTROL_SCENE_MODE_FACE_PRIORITY == entryfaceScene.itemAt(0, Type2Type<MUINT8>())) ||
             ( !entryGdMode.isEmpty() && MTK_FACE_FEATURE_GESTURE_MODE_OFF != entryGdMode.itemAt(0, Type2Type<MINT32>())) ||
             ( !entrySdMode.isEmpty() && MTK_FACE_FEATURE_SMILE_DETECT_MODE_OFF != entrySdMode.itemAt(0, Type2Type<MINT32>())) ||
             ( !entryAsdMode.isEmpty() && MTK_FACE_FEATURE_ASD_MODE_OFF != entryAsdMode.itemAt(0, Type2Type<MINT32>()));
        static const bool gIsForceFDOn = IsForceFDOn();
        if (gIsForceFDOn)
        {
            FDMetaEn = true;
        }
        if ( FDMetaEn && in.pRequest_AppImageStreamInfo != nullptr ) {
            bool isAppReprocess =
                        in.pRequest_AppImageStreamInfo->pAppImage_Input_Yuv != nullptr
                    ||  in.pRequest_AppImageStreamInfo->pAppImage_Input_Priv != nullptr
                    ||  in.pRequest_AppImageStreamInfo->pAppImage_Input_RAW16 != nullptr
                        ;
            if ( isAppReprocess && gIsDisableFdForAppReprocess ) {
                FDMetaEn = false;
            }
        }
        bool isFDScene = ( !entryfaceScene.isEmpty() && MTK_CONTROL_SCENE_MODE_FACE_PRIORITY == entryfaceScene.itemAt(0, Type2Type<MUINT8>()));
        out.hasFDMeta = !entryFdMode.isEmpty() || isFDScene;
        if (out.hasFDMeta)
        {
            out.isFdEnabled = FDMetaEn;
            out.isFDMetaEn = FDMetaEn;
        }
        else
        {
            out.isFdEnabled = false;
            out.isFDMetaEn = false;
        }
        return OK;
    };
}


};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

