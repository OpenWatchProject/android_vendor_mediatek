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

#define LOG_TAG "MtkCam/Shot"
//
#include <unistd.h>
#include <dlfcn.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
using namespace NSCam;
#include <mtkcam/middleware/v1/IParamsManager.h>
//
#include <mtkcam/middleware/v1/IShot.h>
#include <cutils/properties.h>
//
using namespace android;
using namespace NSShot;

#if (MTK_CAM_STEREO_DENOISE_SUPPORT == 1)
#define DUALCAM_DENOISE (1)
#else
#define DUALCAM_DENOISE (0)
#endif

#if (MTKCAM_HAVE_MTKSTEREO_SUPPORT == 1) && (MTKCAM_HAVE_VSDOF_MODE == 2)
#define DCMF_SUPPORT (1)
#else
#define DCMF_SUPPORT (0)
#endif
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


////////////////////////////////////////////////////////////////////////////////
//  Function Prototype.
////////////////////////////////////////////////////////////////////////////////

/******************************************************************************
 *  Normal Shot
 ******************************************************************************/
extern
sp<IShot>
createInstance_NormalShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
);

/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createInstance_VendorShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
);

/******************************************************************************
 *  Continuous Shot
 ******************************************************************************/
extern
sp<IShot>
createInstance_ContinuousShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
);


/******************************************************************************
 *  Stereo Shot
 ******************************************************************************/
extern
sp<IShot>
createInstance_StereoShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
);

/******************************************************************************
 *  FB Shot
 ******************************************************************************/
extern
sp<IShot>
createFBShotInstance(
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId,
    sp<IParamsManager>  pParamsMgr
);

/******************************************************************************
 *  HDR Shot
 ******************************************************************************/
extern
sp<IShot>
createHdrShotInstance(
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId,
    sp<IParamsManager>  pParamsMgr,
    bool                isZsd
);
/******************************************************************************
 *  MFLL Shot
 ******************************************************************************/
#if MTKCAM_HAVE_MFB_SUPPORT
extern
sp<IShot>
createInstance_MfllShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
);
#else
#define createInstance_MfllShot     createInstance_NormalShot
#endif

#if 1
/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createEngShotInstance(
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId,
    sp<IParamsManager>  pParamsMgr
);
#endif

/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createVSSShotInstance(
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId,
    sp<IParamsManager>  pParamsMgr
);

/******************************************************************************
 *  CapBuf Shot
 ******************************************************************************/
extern
sp<IShot>
createInstance_ZsdShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
);

/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createInstance_ZsdVendorShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
);

/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createInstance_SmartShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
);

/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createInstance_ZsdSmartShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
);

#if DUALCAM_DENOISE
/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createInstance_BMDNShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
);
#endif

#if DCMF_SUPPORT
/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createInstance_DCMFShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
);
#endif

#if (MTKCAM_HAVE_DUAL_ZOOM_FUSION_SUPPORT == 1)
/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createInstance_FusionShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
);
#endif

////////////////////////////////////////////////////////////////////////////////
//  Shot Factory
////////////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 * @brief Create a shot instance.
 *
 * @details
 *
 * @note
 *
 * @param[out] rpShot: Reference to a pointer to a newly-created IShot instance.
 *
 * @param[in] u4ShotMode: Shot Mode (defined in EShotMode)
 *
 * @param[in] i4OpenId: Open id: 0, 1, 2, and so on.
 *
 * @param[in] pParamsMgr: Pointer to IParamsManager instance.
 *
 * @return
 *      true indicates success; false indicates failure.
 *
 ******************************************************************************/
bool
createShotInstance(
    sp<IShot>&          rpShot,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId,
    sp<IParamsManager>  pParamsMgr
)
{
    // if refocus mode or vsdof mode is on, using stereo shot.
   MBOOL const isStereoShot = ( pParamsMgr->getStr(MtkCameraParameters::KEY_STEREO_REFOCUS_MODE) == MtkCameraParameters::ON ||  
                                pParamsMgr->getStr(MtkCameraParameters::KEY_STEREO_VSDOF_MODE) == MtkCameraParameters::ON ? MTRUE : MFALSE);
    if  ( rpShot != 0 )
    {
        if  ( rpShot->getShotMode() == u4ShotMode )
        {
            MY_LOGI("No change ShotMode: <%#x>", u4ShotMode);
            return  true;
        }
        else
        {
            MY_LOGI("Change ShotMode: <%#x> -> <%#x>", rpShot->getShotMode(), u4ShotMode);
            rpShot = NULL;
        }
    }
    //
    //
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    switch  ( u4ShotMode )
    {
    //  Normal Shot
    case eShotMode_NormalShot:
        rpShot = createInstance_NormalShot("NormalShot", u4ShotMode, i4OpenId);
        break;
    case eShotMode_4CellRemosaicShot:
        rpShot = createInstance_NormalShot("4CellRemosaicShot", u4ShotMode, i4OpenId);
        break;
    //
    // vendor shot
    case eShotMode_VendorShot:
        rpShot = createInstance_VendorShot("Vendor", u4ShotMode, i4OpenId);
        break;
    //
    //  AIS/MFNR shot
#if MTKCAM_HAVE_IVENDOR_SUPPORT
    case eShotMode_MfllShot:
        rpShot = createInstance_SmartShot("MfllShot", u4ShotMode, i4OpenId);
        break;
    case eShotMode_ZsdMfllShot:
        rpShot = createInstance_ZsdSmartShot("MfllShot", u4ShotMode, i4OpenId);
        break;
#if DUALCAM_DENOISE
    case eShotMode_BMDNShot:
        rpShot = createInstance_BMDNShot("BMDNShot", u4ShotMode, i4OpenId);
        break;
    case eShotMode_MFHRShot:
        rpShot = createInstance_BMDNShot("MFHRShot", u4ShotMode, i4OpenId);
        break;
#endif
#if DCMF_SUPPORT
        case eShotMode_DCMFShot:
        case eShotMode_DCMFHdrShot:
        rpShot = createInstance_DCMFShot("DCMFShot", u4ShotMode, i4OpenId);
        break;
#endif
#if (MTKCAM_HAVE_DUAL_ZOOM_FUSION_SUPPORT == 1)
    case eShotMode_FusionShot:
        rpShot = createInstance_FusionShot("FusionShot", u4ShotMode, i4OpenId);
        break;
#endif

#else
    case eShotMode_MfllShot:
    case eShotMode_ZsdMfllShot:
        rpShot = createInstance_MfllShot("MfllShot", u4ShotMode, i4OpenId);
        break;
#endif
    //
    //  Smile-detection Shot
    case eShotMode_SmileShot:
        rpShot = createInstance_NormalShot("SmileShot", u4ShotMode, i4OpenId);
        break;
    //
    //  Auto-scene-detection Shot
    case eShotMode_AsdShot:
        rpShot = createInstance_NormalShot("AsdShot", u4ShotMode, i4OpenId);
        break;
    //
    //  Engineer mode Shot
    case eShotMode_EngShot:
        rpShot = createEngShotInstance(u4ShotMode, i4OpenId, pParamsMgr);
        break;
    //
    //  Continuous Shot Ncc
    case eShotMode_ContinuousShot:
        rpShot = createInstance_ContinuousShot("ContinuousShot", u4ShotMode, i4OpenId);
        break;

    //  Continuous Shot Cc
    case eShotMode_ContinuousShotCc:
        rpShot = createInstance_ContinuousShot("ContinuousShotCc", u4ShotMode, i4OpenId);
        break;
    //
    //  High-dynamic-range Shot
    case eShotMode_ZsdHdrShot:
            //rpShot = createInstance_ZsdSmartShot("HDRShot", u4ShotMode, i4OpenId);
            rpShot = createHdrShotInstance(u4ShotMode, i4OpenId, pParamsMgr, true);
        break;
    case eShotMode_HdrShot:
            //rpShot = createInstance_SmartShot("HDRShot", u4ShotMode, i4OpenId);
            rpShot = createHdrShotInstance(u4ShotMode, i4OpenId, pParamsMgr, false);
        break;
    //
    //  Zero-shutter-delay Shot
    case eShotMode_ZsdShot:
#if MTKCAM_HAVE_MTKSTEREO
        MY_LOGD("isStereoShot(%d)", isStereoShot);
        if(isStereoShot)
            rpShot = createInstance_StereoShot("SShot", u4ShotMode, i4OpenId);
        else
#endif
            rpShot = createInstance_ZsdShot("ZSD", u4ShotMode, i4OpenId);
        break;
    //
    // zsd vendor shot
    case eShotMode_ZsdVendorShot:
        rpShot = createInstance_ZsdVendorShot("ZSDVendor", u4ShotMode, i4OpenId);
        break;
    //
    //  Video Snap Shot
    case eShotMode_VideoSnapShot:
#if 1
        rpShot = createVSSShotInstance(u4ShotMode, i4OpenId, pParamsMgr);
#endif
        break;
    //
    //  Face-beautifier Shot
    case eShotMode_FaceBeautyShot:
        rpShot = createFBShotInstance(u4ShotMode, i4OpenId, pParamsMgr);
        break;
    //
    //  Multi-motion Shot
//    case eShotMode_MultiMotionShot:
//        break;
    //
    default: {
            MY_LOGW("Unsupported ShotMode<%#x>", u4ShotMode);
            char const szShotName[] = "TestShot";
            String8 const s8ShotInstFactory = String8::format("createInstance_%s", szShotName);
            //
            void* pCreateInstance = ::dlsym(RTLD_DEFAULT, s8ShotInstFactory.string());
            if  ( ! pCreateInstance ) {
                MY_LOGE("Not exist: %s", s8ShotInstFactory.string());
            }
            else {
                rpShot =reinterpret_cast<sp<IShot> (*)(char const*const, uint32_t const, int32_t const)>
                            (pCreateInstance)(szShotName, u4ShotMode, i4OpenId)
                            ;
            }
        }break;
    }
    //
    return  (rpShot != 0);
}

