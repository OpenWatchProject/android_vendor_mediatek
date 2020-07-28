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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam3/3rdparty/customer/customer_feature_type.h>
#include <mtkcam3/3rdparty/mtk/mtk_feature_type.h>
// Module header file

// Local header file

// Logging header file
#include "camera_custom_isp_profile_mapping.h"
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>

using namespace NSIspTuning;
using namespace NSCam::NSPipelinePlugin;

#define ISP_PROFILE_MAPPING_TABLE_START()\
const std::vector<ProfileMappingItem> gISPProfileMappingArray =\
{

#define ISP_PROFILE_MAPPING_TABLE_END()\
};

#define ISPHIDL_PROFILE_MAPPING_TABLE_START()\
const std::vector<ProfileMappingItem> gISPProfileMappingArray_ISPHIDL =\
{

#define ISPHIDL_PROFILE_MAPPING_TABLE_END()\
};

#define MAPPING_ITEM_BEGIN(iFeatureID, iScenario)\
{\
    {\
        iFeatureID,\
        iScenario,\
        eSensorComb_Single\
    },{

#define MAPPING_ITEM_BEGIN_MULTI(iFeatureID, iScenario, iSensorComb)\
{\
    {\
        iFeatureID,\
        iScenario,\
        iSensorComb\
    },{

#define ADD_STAGE_PROFILE_MAPPING(iStage, iProfile) {iStage, iProfile},
#define MAPPING_ITEM_END() \
    }\
},


#define CONFIG_DEFAULT_STAGE_PROFILE_MAP_START()\
const std::map<EProfileMappingStages, NSIspTuning::EIspProfile_T> gDefaultProfileMap =\
{

#define CONFIG_DEFAULT_STAGE_PROFILE_MAP_END()\
};

#define ADD_DEFAULT_PROFILE_MAPPING_ITEM(iStage, iProfile) {iStage, iProfile},

// default Stage-Profile mapping table
CONFIG_DEFAULT_STAGE_PROFILE_MAP_START()
    // default YUV Reprocessing profile = EIspProfile_YUV_Reprocess
    ADD_DEFAULT_PROFILE_MAPPING_ITEM(eStage_Y2Y_Main, EIspProfile_YUV_Reprocess)
CONFIG_DEFAULT_STAGE_PROFILE_MAP_END()

// Proprietary flow ISP Profile Mapping table
ISP_PROFILE_MAPPING_TABLE_START()
    // AINR
    MAPPING_ITEM_BEGIN(MTK_FEATURE_AINR, eMappingScenario_Capture)
        ADD_STAGE_PROFILE_MAPPING(eStage_R2Y_Main, EIspProfile_Capture)
        ADD_STAGE_PROFILE_MAPPING(eStage_R2YDCE_1stRun, EIspProfile_AINR_Main)
        ADD_STAGE_PROFILE_MAPPING(eStage_Y2YDCE_2ndRun, EIspProfile_Capture_DCE)
        ADD_STAGE_PROFILE_MAPPING(eStage_AIBC_FE, EIspProfile_AINR_MainYUV)
        ADD_STAGE_PROFILE_MAPPING(eStage_chromaNR, EIspProfile_AINR_Main)
        ADD_STAGE_PROFILE_MAPPING(eStage_MDP, EIspProfile_AINR_Main)
    MAPPING_ITEM_END()
    // AINR + YHDR
    MAPPING_ITEM_BEGIN(MTK_FEATURE_AINR_YHDR, eMappingScenario_Capture)
        ADD_STAGE_PROFILE_MAPPING(eStage_R2Y_Main, EIspProfile_Capture)
        ADD_STAGE_PROFILE_MAPPING(eStage_R2YDCE_1stRun, EIspProfile_AINR_Main)
        ADD_STAGE_PROFILE_MAPPING(eStage_Y2YDCE_2ndRun, EIspProfile_Capture_DCE)
        ADD_STAGE_PROFILE_MAPPING(eStage_AIBC_FE, EIspProfile_AINR_MainYUV)
        ADD_STAGE_PROFILE_MAPPING(eStage_chromaNR, EIspProfile_AINR_Main)
        ADD_STAGE_PROFILE_MAPPING(eStage_MDP, EIspProfile_AINR_Main)
    MAPPING_ITEM_END()
    // MFNR
    MAPPING_ITEM_BEGIN(MTK_FEATURE_MFNR, eMappingScenario_Capture)
        // MFNR DCE not perform in P2A, eStage_R2YDCE_1stRun/2ndRun is used in drop-to-one case
        ADD_STAGE_PROFILE_MAPPING(eStage_R2Y_Main, EIspProfile_MFNR_Before_Blend)
        ADD_STAGE_PROFILE_MAPPING(eStage_R2YDCE_1stRun, EIspProfile_Capture)
        ADD_STAGE_PROFILE_MAPPING(eStage_Y2YDCE_2ndRun, EIspProfile_Capture_DCE)
        ADD_STAGE_PROFILE_MAPPING(eStage_MDP, EIspProfile_Capture)
    MAPPING_ITEM_END()
    // single capture
    MAPPING_ITEM_BEGIN(NO_FEATURE_NORMAL, eMappingScenario_Capture)
        ADD_STAGE_PROFILE_MAPPING(eStage_R2Y_Main, EIspProfile_Capture)
        ADD_STAGE_PROFILE_MAPPING(eStage_R2YDCE_1stRun, EIspProfile_Capture)
        ADD_STAGE_PROFILE_MAPPING(eStage_Y2YDCE_2ndRun, EIspProfile_Capture_DCE)
        ADD_STAGE_PROFILE_MAPPING(eStage_DSDN20_2ndRun, EIspProfile_Capture_DSDN)
        ADD_STAGE_PROFILE_MAPPING(eStage_MDP, EIspProfile_Capture)
    MAPPING_ITEM_END()
    // VSDOF - Bayer+Bayer
    MAPPING_ITEM_BEGIN_MULTI(MTK_FEATURE_VSDOF, eMappingScenario_Capture, eSensorComb_BayerBayer)
        ADD_STAGE_PROFILE_MAPPING(eStage_R2Y_Main, EIspProfile_N3D_Capture)
        ADD_STAGE_PROFILE_MAPPING(eStage_R2Y_Sub, EIspProfile_N3D_Capture_Full)
        ADD_STAGE_PROFILE_MAPPING(eStage_RRZ_R2Y_Main, EIspProfile_N3D_Capture_Depth)
        ADD_STAGE_PROFILE_MAPPING(eStage_RRZ_R2Y_Sub, EIspProfile_N3D_Capture_Depth)
        ADD_STAGE_PROFILE_MAPPING(eStage_MDP, EIspProfile_Capture)
    MAPPING_ITEM_END()
    // VSDOF - Bayer+Mono
    MAPPING_ITEM_BEGIN_MULTI(MTK_FEATURE_VSDOF, eMappingScenario_Capture, eSensorComb_BayerMono)
        ADD_STAGE_PROFILE_MAPPING(eStage_R2Y_Main, EIspProfile_N3D_Capture)
        ADD_STAGE_PROFILE_MAPPING(eStage_R2Y_Sub, EIspProfile_N3D_Capture_Full)
        ADD_STAGE_PROFILE_MAPPING(eStage_RRZ_R2Y_Main, EIspProfile_N3D_Capture_Depth)
        ADD_STAGE_PROFILE_MAPPING(eStage_RRZ_R2Y_Sub, EIspProfile_N3D_Capture_Depth)
        ADD_STAGE_PROFILE_MAPPING(eStage_MDP, EIspProfile_Capture)
    MAPPING_ITEM_END()
ISP_PROFILE_MAPPING_TABLE_END()

// ISP HIDL version table
ISPHIDL_PROFILE_MAPPING_TABLE_START()
    // AINR
    MAPPING_ITEM_BEGIN(MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_AINR, eMappingScenario_Capture)
        ADD_STAGE_PROFILE_MAPPING(eStage_R2Y_Main, EIspProfile_Capture)
        ADD_STAGE_PROFILE_MAPPING(eStage_R2YDCE_1stRun, EIspProfile_AINR_Main)
        ADD_STAGE_PROFILE_MAPPING(eStage_Y2YDCE_2ndRun, EIspProfile_Capture_DCE)
        ADD_STAGE_PROFILE_MAPPING(eStage_AIBC_FE, EIspProfile_AINR_MainYUV)
        ADD_STAGE_PROFILE_MAPPING(eStage_chromaNR, EIspProfile_AINR_Main)
        ADD_STAGE_PROFILE_MAPPING(eStage_MDP, EIspProfile_AINR_Main)
    MAPPING_ITEM_END()
    // MFNR
    MAPPING_ITEM_BEGIN(MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_MFNR, eMappingScenario_Capture)
        ADD_STAGE_PROFILE_MAPPING(eStage_R2Y_Main, EIspProfile_MFNR_Before_Blend)
        // MFNR DCE not perform in P2A, eStage_R2YDCE_1stRun/2ndRun is used in drop-to-one case
        ADD_STAGE_PROFILE_MAPPING(eStage_R2YDCE_1stRun, EIspProfile_Capture)
        ADD_STAGE_PROFILE_MAPPING(eStage_Y2YDCE_2ndRun, EIspProfile_Capture_DCE)
        ADD_STAGE_PROFILE_MAPPING(eStage_MDP, EIspProfile_Capture)
    MAPPING_ITEM_END()
    // single capture
    MAPPING_ITEM_BEGIN(MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_DEFAULT_NONE, eMappingScenario_Capture)
        ADD_STAGE_PROFILE_MAPPING(eStage_R2Y_Main, EIspProfile_Capture)
        ADD_STAGE_PROFILE_MAPPING(eStage_R2YDCE_1stRun, EIspProfile_Capture)
        ADD_STAGE_PROFILE_MAPPING(eStage_Y2YDCE_2ndRun, EIspProfile_Capture_DCE)
        ADD_STAGE_PROFILE_MAPPING(eStage_DSDN20_2ndRun, EIspProfile_Capture_DSDN)
        ADD_STAGE_PROFILE_MAPPING(eStage_MDP, EIspProfile_Capture)
    MAPPING_ITEM_END()
    // YUV reprocessing
    MAPPING_ITEM_BEGIN(MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_YUV_REPROCESS, eMappingScenario_Capture)
        ADD_STAGE_PROFILE_MAPPING(eStage_Y2Y_Main, EIspProfile_YUV_Reprocess)
    MAPPING_ITEM_END()
ISPHIDL_PROFILE_MAPPING_TABLE_END()
