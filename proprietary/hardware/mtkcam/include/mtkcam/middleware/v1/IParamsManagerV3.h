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

#ifndef _MTK_CAMERA_INC_COMMON_IPARAMSMANAGER_V3_H_
#define _MTK_CAMERA_INC_COMMON_IPARAMSMANAGER_V3_H_

//#include <vector>
#include <utils/String8.h>

#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/fwk/MtkCameraParameters.h>

namespace android
{
/******************************************************************************
 *
 ******************************************************************************/
class IParamsManagerV3
        : public virtual RefBase
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Creation.
    static sp<IParamsManagerV3>    	    createInstance(
                                            String8 const &rName,
                                            MINT32 const cameraId,
                                            sp<IParamsManager> pParamsMgr
                                        );
    //
    virtual ~IParamsManagerV3() {}
    //
    virtual bool                       init() = 0;
    //
    virtual bool                       uninit() = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes .
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual char const*                getName()    const = 0;
    virtual int32_t                    getOpenId()   const = 0;
    virtual sp<IParamsManager>         getParamsMgr()   const = 0;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////     Operations .
    /**
     * Update request with a template base.
     *
     * @param[out]  request
     *
     * @param[in]   sensorMode
     *
     * @param[in]   templateId
     *                 enum camera3_request_template
     *                     CAMERA3_TEMPLATE_PREVIEW = 1
     *                         Standard camera preview operation with 3A on auto.
     *                     CAMERA3_TEMPLATE_STILL_CAPTURE = 2
     *                         Standard camera high-quality still capture with 3A and flash on auto.
     *                     CAMERA3_TEMPLATE_VIDEO_RECORD = 3
     *                         Standard video recording plus preview with 3A on auto, torch off.
     *                     CAMERA3_TEMPLATE_VIDEO_SNAPSHOT = 4
     *                         High-quality still capture while recording video. Application will
     *                         include preview, video record, and full-resolution YUV or JPEG streams in
     *                         request. Must not cause stuttering on video stream. 3A on auto.
     *                     CAMERA3_TEMPLATE_ZERO_SHUTTER_LAG = 5
     *                         Zero-shutter-lag mode. Application will request preview and
     *                         full-resolution data for each frame, and reprocess it to JPEG when a
     *                         still image is requested by user. Settings should provide highest-quality
     *                         full-resolution images without compromising preview frame rate. 3A on
     *                         auto.
     *                     CAMERA3_TEMPLATE_MANUAL = 6
     *                         A basic template for direct application control of capture
     *                         parameters. All automatic control is disabled (auto-exposure, auto-white
     *                         balance, auto-focus), and post-processing parameters are set to preview
     *                         quality. The manual capture parameters (exposure, sensitivity, etc.)
     *                         are set to reasonable defaults, but should be overridden by the
     *                         application depending on the intended use case.
     *
     * @return 0 indicates success; non-zero indicates an error code.
     */
    virtual status_t                updateRequest(
                                        NSCam::IMetadata *request,
                                        MINT32     sensorMode,
                                        MINT32     templateId = -1
                                    ) = 0;

    virtual status_t                updateRequestHal(NSCam::IMetadata *request, MINT32 sensorMode, MBOOL isCapture = MFALSE) = 0;
    virtual status_t                updateRequestJpeg(NSCam::IMetadata *request)   const = 0;
    virtual status_t                updateRequestPreview(NSCam::IMetadata *request)   const = 0;
    virtual status_t                updateRequestRecord(NSCam::IMetadata *request)   const = 0;
    virtual status_t                updateRequestVSS(NSCam::IMetadata *request)   const = 0;
    virtual status_t                updateRequestEng(NSCam::IMetadata *request)    const = 0;

    virtual status_t                setStaticData() = 0;
    virtual status_t                restoreStaticData() = 0;
    virtual void                    setPreviewMaxFps(MUINT32 fps) = 0;
    virtual MUINT32                 getPreviewMaxFps() = 0;
    virtual status_t                calculateCropRegion(MINT32 sensorMode = -1) = 0;
    virtual status_t                getCropRegion(
                                        NSCam::MSize& sensorSize,
                                        NSCam::MRect& activeArrayCrop,
                                        NSCam::MRect& sensorSizeCrop,
                                        NSCam::MRect& activeArrayPreviewCrop,
                                        NSCam::MRect& sensorSizePreviewCrop
                                    ) const = 0;
    virtual status_t                getCropRegion(
                                        NSCam::MSize& sensorSize,
                                        NSCam::MRect& activeArrayCrop,
                                        NSCam::MRect& sensorSizeCrop,
                                        NSCam::MRect& activeArrayPreviewCrop,
                                        NSCam::MRect& sensorSizePreviewCrop,
                                        NSCam::MSize& margin
                                    ) const = 0;

    virtual NSCam::MRect            getAAAPreviewCropRegion() const = 0;

    virtual status_t                getCapCropRegion(
                                        NSCam::MRect& activeArrayCrop,
                                        NSCam::MRect& sensorSizeCrop,
                                        NSCam::MRect& sensorSizePreviewCrop
                                    ) const = 0;

    virtual int                     getZoomRatio() = 0;
    virtual void                    setCancelAF(bool bCancelAF) = 0;
    virtual bool                    getCancelAF() const = 0;
    virtual void                    setAfTriggered(bool bAfTriggered) = 0;
    virtual bool                    getAfTriggered() const = 0;
    virtual void                    setForceTorch(bool bForceTorch) = 0;
    virtual void                    setControlCALTM(bool bEnableControlCALTM, bool bEnableCALTM) = 0;

public:     //// scenario related control
    /**
     * Modify flash mode based on different scenario.
     * eScenario_Video_Preview:
     *     condition: CameraParameters::FLASH_MODE_ON
     *     result   : set flash mode to MTK_FLASH_MODE_TORCH
     * eScenario_Video_Record_Start:
     *     condition: CameraParameters::FLASH_MODE_AUTO &
     *                MTK_CONTROL_AE_STATE_FLASH_REQUIRED
     *     result   : set flash mode to MTK_FLASH_MODE_TORCH
     * eScenario_Video_Record_Stop:
     *     reset flash mode according to CameraParameters
     */
    enum eScenario
    {
        eScenario_define_start = 0x30000000,
        eScenario_Camera_Preview,
        eScenario_Video_Preview,
        eScenario_Video_Preview_Repeat,
        eScenario_Video_Record_Start,
        eScenario_Video_Record_Start_Repeat,
        eScenario_Video_Record_Stop,
    };
    virtual void                    setScenario(int32_t scenario) = 0;
    virtual void                    updateBasedOnScenario(NSCam::IMetadata* meta, MBOOL& bRepeat) = 0;
    virtual void                    getPreviewFpsRange(MINT32& minFps, MINT32& maxFps) = 0;
    virtual void                    setPreviewFpsRange(MINT32& minFps, MINT32& maxFps) = 0;
    //
    virtual void                    getLastUpdateRequest(NSCam::IMetadata& meta) = 0;
};
};
#endif
