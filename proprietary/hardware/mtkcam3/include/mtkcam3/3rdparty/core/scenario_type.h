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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_SCENARIOTYPE_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_SCENARIOTYPE_H_
//
#include <mtkcam/def/common.h>
#include <mtkcam3/3rdparty/mtk/mtk_scenario_type.h>
#include <mtkcam3/3rdparty/mtk/mtk_scenario_type_ISP.h>
#include <mtkcam3/3rdparty/mtk/mtk_feature_type.h>
#include <mtkcam3/3rdparty/customer/customer_scenario_type.h>
#include <mtkcam3/3rdparty/customer/customer_scenario_type_ISP.h>
#include <mtkcam3/3rdparty/customer/customer_feature_type.h>
//
#include <vector>
#include <string>
#include <list>

// add feature set by camera scenario
#define CAMERA_SCENARIO_START(SCENARIO_NAME)                              \
{                                                                         \
    SCENARIO_NAME,                                                        \
    {                                                                     \
        #SCENARIO_NAME,                                                   \
        {

#define ADD_CAMERA_FEATURE_SET(FEATURE, FEATURE_COMBINATION)              \
        { #FEATURE, #FEATURE_COMBINATION, FEATURE, FEATURE_COMBINATION },

#define CAMERA_SCENARIO_END                                               \
        }                                                                 \
    }                                                                     \
},

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {
namespace scenariomgr {

enum DualFeatureMode
{
    DualFeatureMode_NONE = 0,
    DualFeatureMode_YUV,
    DualFeatureMode_HW_DEPTH,
    DualFeatureMode_MTK_VSDOF,
    DualFeatureMode_MTK_FOVA,
};

enum IspProcessInOutType
{
    // support list
    eIspProcessInOutType_SingleRawIn_YuvOut = 0,
    eIspProcessInOutType_SingleRawIn_JpegOut,
    eIspProcessInOutType_SingleYuvIn_YuvOut,
    eIspProcessInOutType_SingleYuvIn_JpegOut,
    eIspProcessInOutType_MultipleRawIn_RawOut,
    eIspProcessInOutType_MultipleRawIn_YuvOut,
    eIspProcessInOutType_MultipleRawIn_JpegOut,
    eIspProcessInOutType_MultipleYuvIn_YuvOut,
    eIspProcessInOutType_MultipleYuvIn_JpegOut,
    eIspProcessInOutType_QuerySupportedKeyFeature,
    eIspProcessInOutType_Support_List_Size,

    // error list
    eIspProcessInOutType_Unknow     = 0xFE,
    eIspProcessInOutType_NotSupport = 0xFF,
};

struct ScenarioHint
{
    // pipeline session operation mode
    uint32_t operationMode = 0;
    bool isVideoMode = false;
    bool isSuperNightMode = false;
    bool isIspHidlTuningEnable = false;
    bool isLogical = true;

    bool            isDualCam           = false;
    int32_t         dualDevicePath      = -1;
    int32_t         multiCamFeatureMode = -1;
    int32_t         captureFeatureMode  = -1;

    DualFeatureMode mDualFeatureMode = DualFeatureMode_NONE;

    // hint for capture scenario
    bool    isYuvReprocess = false;
    bool    isRawReprocess = false;
    bool    isYuvRequestForIspHidl = false;
    bool    isRawRequestForIspHidl = false;
    bool    isRawOrDngRequest = false;
    bool    isCShot     = false;
    bool    isMaster    = true;
    int32_t captureScenarioIndex = -1; // force to get indicated scenario by vendor tag
    //
    int32_t ispTuningHint = -1; // hint to request buffer for reprocessing (request single frame for App to use isp hidl)
    //
    bool isIspHidl = false;
    IspProcessInOutType ispProcessInOutType = eIspProcessInOutType_Unknow;

    // hint for streaming scenario
    // TODO: add others hint to choose streaming scenario
    int32_t streamingScenarioIndex = -1; // force to get indicated scenario by vendor tag
};

struct CaptureScenarioConfig
{
    // buffer format requirement
    bool needUnpackRaw = false;

    // buffer count requirement
    uint32_t maxAppRaw16OutputBufferNum = 1;
    uint32_t maxAppJpegStreamNum        = 1;
};

struct StreamingScenarioConfig
{
    // margin requirement
    std::list<MUINT32> fixedMargins; // 25 means plugin need Input = Output * ( 1 + 25%)
};

struct FeatureSet
{
    std::string featureName;            // for debug
    std::string featureCombinationName; // for debug
    MUINT64 feature = 0;
    MUINT64 featureCombination = 0;
};

struct ScenarioFeatures
{
    std::string scenarioName; // for debug
    std::vector<FeatureSet> vFeatureSet;
};

};  //namespace scenariomgr
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_SCENARIOTYPE_H_

