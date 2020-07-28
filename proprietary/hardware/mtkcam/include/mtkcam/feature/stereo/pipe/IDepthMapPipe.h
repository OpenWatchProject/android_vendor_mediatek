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

/**
 * @file IDepthMapPipe.h
 * @brief DepthMap feature pipe interfaces, classes and settings
 */

#ifndef _MTK_CAMERA_FEATURE_PIPE_INTERFACE_DEPTH_MAP_PIPE_H_
#define _MTK_CAMERA_FEATURE_PIPE_INTERFACE_DEPTH_MAP_PIPE_H_

// Standard C header file

// Android system/core header file
#include <utils/RefBase.h>

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/def/common.h>
#include <mtkcam/feature/stereo/StereoCamEnum.h>

// Module header file

// Local header file

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using NSCam::v1::Stereo::SeneorModuleType;

// use for v2.0/v1.0
#define DEPTHMAP_META_KEY_STRING "Metadata"
#define DEPTHMAP_COMPLETE_KEY "onComplete"
#define DEPTHMAP_FLUSH_KEY "onFlush"
#define DEPTHMAP_ERROR_KEY "onError"
#define DEPTHMAP_NOT_READY_KEY "onDepthNotReady"
#define DEPTHMAP_YUV_DONE_KEY "onYUVDone"

/*******************************************************************************
* Enum Definition
********************************************************************************/
typedef MINT32 DepthMapBufferID;

typedef enum DepthMapNode_Public_BufferDataTypes{
    PBID_INVALID = -1,
    PUBLIC_PBID_START = 0,
    // input buffer
    PBID_IN_FSRAW1,
    PBID_IN_FSRAW2,
    PBID_IN_RSRAW1,
    PBID_IN_RSRAW2,
    PBID_IN_LCSO1,
    PBID_IN_LCSO2,
    // output buffer
    PBID_OUT_MV_F,
    PBID_OUT_MV_F_CAP,
    PBID_OUT_FDIMG,
    PBID_OUT_MV_F_JPS_MAIN1,
    PBID_OUT_MV_F_JPS_MAIN2,
    PBID_OUT_LDC,
    PBID_OUT_WARPING_MATRIX,
    PBID_OUT_SCENEINFO,
    PBID_OUT_DMP_L,
    PBID_OUT_DMP_R,
    PBID_OUT_DMBG,
    PBID_OUT_DEPTHMAP,
    PBID_OUT_HAL_DEPTHMAP,
    PBID_OUT_EXTRADATA,
    PBID_OUT_N3D_DBG,
    PBID_OUT_POSTVIEW,
    PBID_OUT_DEPTH_WRAPPER,
    PBID_OUT_TUNING_BUF,
    // input meta
    PBID_IN_APP_META,
    PBID_IN_HAL_META_MAIN1,
    PBID_IN_HAL_META_MAIN2,
    PBID_IN_P1_RETURN_META,
    // output meta
    PBID_OUT_APP_META,
    PBID_OUT_HAL_META,
    // end
    PUBLIC_PBID_END = 100
} PublicDepthBufferID;

/**
 * @brief Operation Status of EffectRequest
 */
typedef enum eDepthMapPipeOpState {
    eSTATE_STANDALONE = 0,
    eSTATE_NORMAL = 1,
    eSTATE_CAPTURE = 2
} DepthMapPipeOpState;

/**
 * @brief DepthMapNode supoorted feature flow/mode
 */
typedef enum eDepthNodeFeatureMode {
    eDEPTHNODE_MODE_VSDOF,
    eDEPTHNODE_MODE_MTK_DEPTH,
    eDEPTHNODE_MODE_MTK_UNPROCESS_DEPTH,
    eDEPTHNODE_MODE_DENOISE
} DepthNodeFeatureMode;

/**
 * @brief DepthMapNode supoorted flow type
 */

typedef enum eDepthFlowType {
    eDEPTH_FLOW_TYPE_STANDARD,
    eDEPTH_FLOW_TYPE_QUEUED_DEPTH  /*!< PV/VR use the queued depth of nearest frame*/
} DepthMapFlowType;

/*******************************************************************************
* Struct Definition
********************************************************************************/
/**
 * @brief DepthMapPipe functional option.
 */

struct DepthMapPipeOption : public android::LightRefBase<DepthMapPipeOption>
{
public:
    DepthMapPipeOption() {}

    DepthMapPipeOption(
        SeneorModuleType type,
        DepthNodeFeatureMode mode,
        DepthMapFlowType flow)
    : mSensorType(type)
    , mFeatureMode(mode)
    , mFlowType(flow) {}

    MVOID setEnableLCE(MBOOL bEnable) {mbEnableLCE = bEnable;}
    MVOID setIsRecordMode(MBOOL bIsRecMode) {mbIsRecordMode = bIsRecMode;}
     /**
     * @brief Enable depth generation control
     * @param [in] bEnable enable or not
     * @param [in] iSkipFrame the frame number between two depth
                               generation
     */
    MVOID setEnableDepthGenControl(MBOOL bEnable, MUINT8 iDepthFrameGap)
    {
        mbDepthGenControl = bEnable;
        miDepthFrameGap = ( iDepthFrameGap <= 0 ) ? 0 : iDepthFrameGap;
    }
public:
    SeneorModuleType mSensorType;      /*!< Indicate the current sensor config*/
    DepthNodeFeatureMode mFeatureMode;   /*!< Indicate the feature mode*/
    DepthMapFlowType mFlowType;
    MBOOL mbEnableLCE = MTRUE;    // LCE Default on
    MBOOL mbDepthGenControl = MFALSE;
    MUINT8 miDepthFrameGap = 0;
    MBOOL mbIsRecordMode = MFALSE;
};

/**
 * @brief DepthMapPipe config setting
 */

struct DepthMapPipeSetting : public android::LightRefBase<DepthMapPipeSetting>
{
public:
    DepthMapPipeSetting()
    : miSensorIdx_Main1(-1), miSensorIdx_Main2(-1) {}

    DepthMapPipeSetting(
        MUINT32 senIdx_main1,
        MUINT32 senIdx_main2,
        MSize szRRZO_main1)
    : miSensorIdx_Main1(senIdx_main1)
    , miSensorIdx_Main2(senIdx_main2)
    , mszRRZO_Main1(szRRZO_main1) {}
public:
    MUINT32 miSensorIdx_Main1;
    MUINT32 miSensorIdx_Main2;
    MSize mszRRZO_Main1;
};

/*******************************************************************************
* Class Definition
********************************************************************************/
class IDepthMapEffectRequest;
/**
 * @class IDepthMapPipe
 * @brief DepthMapPipe interface
 */
class IDepthMapPipe
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief create a instance of DepthMapPipe
     * @param [in] pSetting DepthMapPipe control setting
     * @param [in] pPipeOption the configurable settings of DepthMapPipe
     * @return
     * - instance of DepthMapPipe
     */
    static IDepthMapPipe* createInstance(
                                android::sp<DepthMapPipeSetting> pSetting,
                                android::sp<DepthMapPipeOption> pPipeOption);
    MBOOL destroyInstance();
    /**
     * @brief DepthMapPipe destructor
     */
    virtual ~IDepthMapPipe() {};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapPipe Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief perform initial operation
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL init() = 0;
    /**
     * @brief perform uninitial operation
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL uninit() = 0;
    /**
     * @brief perform enque operation
     * @param [in] request to-be-performed EffectRequest
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL enque(android::sp<IDepthMapEffectRequest>& request) = 0;
    /**
     * @brief Flush all the operations inside the feature pipe
     */
    virtual MVOID flush() = 0;
    /**
     * @brief Sync all the operations inside the feature pipe
     */
    virtual MVOID sync() = 0;
};

}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

#endif