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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/**
 * @file AdvCamSettingMgr.h
 *
 * Manage advanced camera setting from app.
 *
 */

#ifndef _MTKCAM_CAMEMRA_SETTING_MGR_IMP_H_
#define _MTKCAM_CAMEMRA_SETTING_MGR_IMP_H_

#include <utils/RefBase.h>
#include <mtkcam/def/common.h>
#include <vector>
#include "../PipelineModel.typedef.h"

using namespace android;
using namespace NSCam::v3;

namespace NSCam {
namespace v3{

/* Manage advanced camera setting */
//class CameraSettingMgr_Imp : public CameraSettingMgr
class CameraSettingMgr_Imp
{
public:


    CameraSettingMgr_Imp(const MUINT32 DevIdx);
    virtual ~CameraSettingMgr_Imp();

    /**
     * This function should be called in Hal3 configure().
     * When this function is called, it will generate AdvCamSetting from Metadata which app previously set,
     * and then clear the Metadata to avoid load repeatly.
     * @param[in] rConfigParams : 
     * @param[out] CamConfig : some advcam setting need to reference pipeline param, such as isRecording or is4K for EIS.
     * @return : error code
     */
    virtual MINT32 generateCamSetting(PipeConfigParams const& rConfigParams, std::shared_ptr<MyProcessedParams> pParams);

    /**
     * Before new request submitted to pipeline, it may need to modify metadata for feature.
     * Such as Isp Profile.
     *@param[in/out] halMetadata : it will update some value in hal metadata
     *@param[in/out] appMetadata : it may update some value in app metadata for Hal1. In Hal3, this maybe NULL.
     *@param[in] pipelineParam : it's some pipeline param to decide metadata setting
     *@param[in] reqParam : per-frame request related parameter, such as has record buf in this req or not.
     *@param[in] inpuParam : Additional pipeline params for generate AdvCamSetting
     */
    virtual MBOOL evluateRequest( parsedAppRequest const& request,
                                  std::shared_ptr<MyProcessedParams> pParams,
                                  std::vector<evaluateRequestResult> &vReqResult
                                );
private:
    MERROR evaluateRequestLocked_InitRequestSetting(
                                                    parsedAppRequest const& request,
                                                    std::shared_ptr<MyProcessedParams> pParams,
                                                    evaluateRequestResult &ReqResult
                                                    );

    MERROR evaluateRequestLocked_Jpeg(
                                                    parsedAppRequest const& request,
                                                    std::shared_ptr<MyProcessedParams> pParams,
                                                    evaluateRequestResult &ReqResult
                                                    );

    MERROR evaluateRequestLocked_FD(
                                                    parsedAppRequest const& request,
                                                    std::shared_ptr<MyProcessedParams> pParams,
                                                    evaluateRequestResult &ReqResult
                                                    );

    MERROR evaluateRequestLocked_Raw16(
                                                    parsedAppRequest const& request,
                                                    std::shared_ptr<MyProcessedParams> pParams,
                                                    evaluateRequestResult &ReqResult
                                                    );

    MERROR evaluateRequestLocked_Streaming(
                                                    parsedAppRequest const& request,
                                                    std::shared_ptr<MyProcessedParams> pParams,
                                                    evaluateRequestResult &ReqResult
                                                    );
                                                    
    MERROR evaluateRequestLocked_Capture(
                                                    parsedAppRequest const& request,
                                                    std::shared_ptr<MyProcessedParams> pParams,
                                                    evaluateRequestResult &ReqResult
                                                    );
                                                    
    MERROR evaluateRequestLocked_Pass1(
                                                    parsedAppRequest const& request,
                                                    std::shared_ptr<MyProcessedParams> pParams,
                                                    evaluateRequestResult &ReqResult
                                                    );
                                                    
    MERROR createStreamInfoLocked_Jpeg_YUV(
                                                    IMetadata const* pMetadata,
                                                    std::shared_ptr<MyProcessedParams> pParams,
                                                    android::sp<IImageStreamInfo>& rpStreamInfo
                                                );
    MERROR createStreamInfoLocked_Thumbnail_YUV(
                                                    IMetadata const* pMetadata,
                                                    std::shared_ptr<MyProcessedParams> pParams,
                                                    android::sp<IImageStreamInfo>& rpStreamInfo
                                                );
    MSize calcThumbnailYuvSize(
                                                    MSize const rPicSize,
                                                    MSize const rThumbnailsize
                                                );
    MBOOL isFdEnable(
                                                    parsedAppRequest const& request,
                                                    std::shared_ptr<MyProcessedParams> pParams,
                                                    evaluateRequestResult& result
                                                );
    MBOOL isFdEnable(
                                                    IMetadata const* pMetadata
                                                );

private:

    // common part
    MUINT32                     mDeviceIdx = -1;
    std::vector<MUINT32>        mSensorIdxs;
    MINT32                      mLogLevel;
    MINT32                      mIsTkApp;
    MINT32                      mPrevFDEn;

};
}
}; // NSCam namespace
#endif // _MTKCAM_CAMEMRA_SETTING_MGR_IMP_H_

