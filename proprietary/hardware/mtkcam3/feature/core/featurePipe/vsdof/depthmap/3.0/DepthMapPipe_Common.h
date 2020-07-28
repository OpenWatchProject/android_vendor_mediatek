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

#ifndef _MTK_CAMERA_INCLUDE_STEREO_DEPTHMAP_FEATUREPIPE_COMMON_H_
#define _MTK_CAMERA_INCLUDE_STEREO_DEPTHMAP_FEATUREPIPE_COMMON_H_

// Standard C header file
#include <ctime>
#include <chrono>
#include <bitset>
#include <aee.h>
// Android system/core header file
#include <utils/RefBase.h>
#include <utils/KeyedVector.h>

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/def/common.h>
#include <mtkcam3/feature/stereo/pipe/vsdof_common.h>
#include <mtkcam3/feature/stereo/pipe/IDepthMapPipe.h>
#include <mtkcam3/feature/stereo/pipe/IDualFeatureRequest.h>

// Module header file
#include <mtkcam/drv/iopipe/Port.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/drv/def/Dip_Notify_datatype.h>
// Local header file

// Logging log header
#define PIPE_MODULE_TAG "DepthMapPipe"
#define PIPE_CLASS_TAG "Common"
#include <featurePipe/core/include/PipeLog.h>

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using android::String8;
using android::KeyedVector;
using namespace android;
using namespace NSCam::NSCamFeature::NSDualFeature;

/*******************************************************************************
* Const Definition
********************************************************************************/
const int VSDOF_CONST_FE_EXEC_TIMES = 2;
const int VSDOF_DEPTH_P2FRAME_SIZE = 10 + 2; // P2A + P2ABayer
const int VSDOF_WORKING_BUF_SET = 3;
const int VSDOF_WORKING_EXTRA_BUF_SET = 4;  // For N3D two-stage related
const int VSDOF_FRAME_DELAY_SIZE = 6;

/*******************************************************************************
* Macro Definition
********************************************************************************/
#undef MY_LOGV_IF
#undef MY_LOGD_IF
#undef MY_LOGI_IF
#undef MY_LOGW_IF
#undef MY_LOGE_IF

#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)

#define AEE_ASSERT(fmt, arg...) \
    do { \
        android::String8 const str = android::String8::format(fmt, ##arg); \
        VSDOF_MDEPTH_LOGE("ASSERT(%s) fail", str.string()); \
        CAM_ULOG_FATAL(\
            NSCam::Utils::ULog::MOD_SFPIPE_DEPTH,\
            "mtkcam/DepthMapPipe", \
            str.string()); \
    } while(0)

#define DEFINE_CONTAINER_LOGGING(funcIdx, containerMap)\
    std::string ids="";\
    auto fn##funcIdx = [&]() -> const char*{\
                    for(auto index=0;index<containerMap.size();index++)\
                        ids += std::to_string(containerMap.keyAt(index)) + "|";\
                    return ids.c_str();};

#define CONT_LOGGING(funcIdx)\
    fn##funcIdx()

#define NODE_SIGNAL_ERROR_GUARD()\
    if(mpNodeSignal->getStatus(NodeSignal::STATUS_IN_ERROR))\
    {\
        MY_LOGE("%s : Node Signal error occur!!!", getName());\
        return MFALSE;\
    }\

/******************************************************************************
* Enum Definition
********************************************************************************
/**
  * @brief Buffer ID inside DepthMapPipe
 */
typedef enum DepthMapNode_BufferDataTypes{
    BID_INVALID = PUBLIC_PBID_START,

// ======== public id section start ==========

    // input buffer
    BID_P2A_IN_FSRAW1 = PBID_IN_FSRAW1,
    BID_P2A_IN_FSRAW2 = PBID_IN_FSRAW2,
    BID_P2A_IN_RSRAW1 = PBID_IN_RSRAW1,
    BID_P2A_IN_RSRAW2 = PBID_IN_RSRAW2,
    BID_P2A_IN_LCSO1 = PBID_IN_LCSO1,
    BID_P2A_IN_LCSO2 = PBID_IN_LCSO2,
    // output buffer
    BID_P2A_OUT_MV_F = PBID_OUT_MV_F,
    BID_P2A_OUT_MV_F_CAP = PBID_OUT_MV_F_CAP,
    BID_P2A_OUT_FDIMG = PBID_OUT_FDIMG,
    BID_N3D_OUT_JPS_MAIN1 = PBID_OUT_MV_F_JPS_MAIN1,
    BID_N3D_OUT_JPS_MAIN2 = PBID_OUT_MV_F_JPS_MAIN2,
    BID_N3D_OUT_LDC = PBID_OUT_LDC, //10
    BID_N3D_OUT_WARPING_MATRIX = PBID_OUT_WARPING_MATRIX,
    BID_N3D_OUT_SCENE_INFO = PBID_OUT_SCENEINFO,
    BID_DPE_OUT_DMP_L = PBID_OUT_DMP_L,
    BID_DPE_OUT_DMP_R = PBID_OUT_DMP_R,
    BID_GF_OUT_DMBG = PBID_OUT_DMBG,
    BID_GF_OUT_DEPTH = PBID_OUT_DEPTHMAP,
    BID_FD_OUT_EXTRADATA = PBID_OUT_EXTRADATA,

    BID_GF_OUT_DEPTH_WRAPPER = PBID_OUT_DEPTH_WRAPPER,

    BID_N3D_OUT_DEPTH_DBG = PBID_OUT_N3D_DBG,
    BID_P2A_OUT_POSTVIEW = PBID_OUT_POSTVIEW,

    // metadata
    BID_META_IN_APP = PBID_IN_APP_META,
    BID_META_IN_HAL_MAIN1 = PBID_IN_HAL_META_MAIN1,
    BID_META_IN_HAL_MAIN2 = PBID_IN_HAL_META_MAIN2,
    BID_META_IN_P1_RETURN = PBID_IN_P1_RETURN_META,
    BID_META_OUT_APP = PBID_OUT_APP_META,
    BID_META_OUT_HAL = PBID_OUT_HAL_META,

// ======== public id section end ==========

    // queued metadata (used in QUEUED_DEPTH flow)
    BID_META_IN_APP_QUEUED = PUBLIC_PBID_END,  //100
    BID_META_IN_HAL_MAIN1_QUEUED,
    BID_META_IN_HAL_MAIN2_QUEUED,
    BID_META_OUT_APP_QUEUED,
    BID_META_OUT_HAL_QUEUED,
    BID_META_IN_P1_RETURN_QUEUED,

    // internal P2A buffers
    BID_P2A_FE1B_INPUT,
    BID_P2A_FE2B_INPUT,
    BID_P2A_FE1C_INPUT,
    BID_P2A_FE2C_INPUT,
    BID_P2A_TUNING,
    // P2A output
    BID_P2A_OUT_FE1AO,
    BID_P2A_OUT_FE2AO,
    BID_P2A_OUT_FE1BO,
    BID_P2A_OUT_FE2BO,
    BID_P2A_OUT_FE1CO,
    BID_P2A_OUT_FE2CO,
    BID_P2A_OUT_RECT_IN1,
    BID_P2A_OUT_RECT_IN2,
    BID_P2A_OUT_CC_IN1,
    BID_P2A_OUT_CC_IN2,
    BID_P2A_OUT_FMAO_LR, //20
    BID_P2A_OUT_FMAO_RL,
    BID_P2A_OUT_FMBO_LR,
    BID_P2A_OUT_FMBO_RL,
    BID_P2A_OUT_FMCO_LR,
    BID_P2A_OUT_FMCO_RL,
    BID_P2A_OUT_MY_S,
    BID_P2A_OUT_INTERNAL_FD,
    BID_P2A_INTERNAL_IMG3O,
    // N3D output
    BID_N3D_OUT_MV_Y,
    BID_N3D_OUT_MASK_M,
    BID_N3D_OUT_SV_Y,
    BID_N3D_OUT_MASK_S,
    // DPE output
    BID_DPE_INTERNAL_DMP,
    BID_DPE_OUT_CFM_L,
    BID_DPE_OUT_CFM_R,
    BID_DPE_OUT_RESPO_L,
    BID_DPE_OUT_RESPO_R,
    // Last DMP
    BID_DPE_INTERNAL_LAST_DMP,
    //
    BID_OCC_OUT_OCC,
    BID_OCC_OUT_NOC,
    BID_OCC_OUT_DS_MVY,
    // WMF
    BID_WMF_HOLEFILL_INTERNAL,
    BID_WMF_DMW_INTERNAL,
    BID_WMF_OUT_DMW,
    // GF
    BID_GF_INTERNAL_DMBG,
    BID_GF_INTERNAL_DEPTHMAP,
    //
    BID_PQ_PARAM,
    BID_DP_PQ_PARAM,

#ifdef GTEST
    // UT output
    BID_FE2_HWIN_MAIN1,
    BID_FE2_HWIN_MAIN2,
    BID_FE3_HWIN_MAIN1,
    BID_FE3_HWIN_MAIN2,
#endif

} InternalDepthMapBufferID;

/**
  * @brief Node ID inside the DepthMapPipe
 */
typedef enum eDepthMapPipeNodeID {
    eDPETHMAP_PIPE_NODEID_P2A,
    eDPETHMAP_PIPE_NODEID_P2ABAYER,
    eDPETHMAP_PIPE_NODEID_N3D,
    eDPETHMAP_PIPE_NODEID_DPE,
    eDPETHMAP_PIPE_NODEID_WMF,
    eDPETHMAP_PIPE_NODEID_OCC,
    eDPETHMAP_PIPE_NODEID_HWOCC,
    eDPETHMAP_PIPE_NODEID_FD,
    eDPETHMAP_PIPE_NODEID_GF,
    // any node need to put upon this line
    eDPETHMAP_PIPE_NODE_SIZE,
} DepthMapPipeNodeID;

typedef std::bitset<eDPETHMAP_PIPE_NODE_SIZE> PipeNodeBitSet;

/**
  * @brief Data ID used in handleData inside the DepthMapPipe
 */
enum DepthMapDataID {
    ID_INVALID,
    ROOT_ENQUE,
    BAYER_ENQUE,

    P2A_TO_N3D_FEFM_CCin,
    P2A_TO_N3D_NOFEFM_RECT1,
    P2A_TO_N3D_NOFEFM_RECT2,
    P2A_TO_FD_IMG,
    N3D_TO_DPE_MVSV_MASK,
    N3D_TO_FD_EXTDATA_MASK,
    DPE_TO_OCC_MVSV_DMP_CFM,
    DPE_TO_HWOCC_MVSV_DMP,  //10
    OCC_TO_WMF_OMYSN,
    HWOCC_TO_WMF_NOC,
    WMF_TO_GF_DMW_MY_S,
    //OCC/NOC/DMW
    WMF_TO_GF_OND,
    GF_OUT_DEPTHMAP,
    // DepthMap output
    P2A_OUT_MV_F,
    P2A_OUT_FD,
    P2A_OUT_MV_F_CAP,
    GF_OUT_DMBG,
    GF_OUT_INTERNAL_DMBG,
    FD_OUT_EXTRADATA,
    N3D_OUT_JPS_WARPMTX,
    DPE_OUT_DISPARITY,
    DEPTHMAP_META_OUT,
    GF_OUT_DEPTH_WRAPPER,
    // notify YUV done
    P2A_OUT_YUV_DONE,
    HWOCC_OUT_3RD_NOC,

    // Bayer-run specific
    P2A_TO_OCC_MY_S,
    P2A_TO_HWOCC_MY_S,
    // eDEPTH_FLOW_TYPE_QUEUED_DEPTH specific
    P2A_OUT_DMBG,
    QUEUED_FLOW_DONE,
    // Notify error occur underlying the nodes
    ERROR_OCCUR_NOTIFY,
    // Notify the request's depthmap is not ready
    REQUEST_DEPTH_NOT_READY,
    // use to dump p1 raw buffers
    TO_DUMP_RAWS,
    TO_DUMP_MAPPINGS,
    // use to dump buffers
    TO_DUMP_BUFFERS,
    TO_DUMP_IMG3O,
    #ifdef GTEST
    // For UT
    UT_OUT_FE
    #endif
};

/**
  * @brief Data ID used in handleData inside the DepthMapPipe
 */
typedef enum eStereoP2Path
{
    eP2APATH_MAIN1,
    eP2APATH_MAIN2,
    eP2APATH_MAIN1_BAYER

} StereoP2Path;

typedef enum eINPUT_RAW_TYPE
{
    eRESIZE_RAW,
    eFULLSIZE_RAW
} INPUT_RAW_TYPE;

/*******************************************************************************
* Structure Definition
********************************************************************************/

/*******************************************************************************
* Class Definition
********************************************************************************/
/**
 * @class DepthPipeLoggingSetup
 * @brief Control the logging enable of the depthmap pipe
 */

class DepthPipeLoggingSetup
{
public:
    static MBOOL mbProfileLog;
    static MBOOL mbDebugLog;
};

#undef VSDOF_LOGD
#undef VSDOF_PRFLOG
// logging macro
#define VSDOF_LOGD(fmt, arg...) \
    do { if(DepthPipeLoggingSetup::mbDebugLog) {MY_LOGD("%d: " fmt, __LINE__, ##arg);} } while(0)

#define VSDOF_MDEPTH_LOGD(fmt, arg...) \
    do { if(DepthPipeLoggingSetup::mbDebugLog) {CAM_ULOGD(NSCam::Utils::ULog::MOD_SFPIPE_DEPTH, \
                                                        "%d: " fmt, __LINE__, ##arg);} } while(0)
#define VSDOF_MDEPTH_LOGE(fmt, arg...) \
    do { if(DepthPipeLoggingSetup::mbDebugLog) {CAM_ULOGE(NSCam::Utils::ULog::MOD_SFPIPE_DEPTH, \
                                                        "%d: " fmt, __LINE__, ##arg);} } while(0)
#define VSDOF_MDEPTH_LOGW(fmt, arg...) \
    do { if(DepthPipeLoggingSetup::mbDebugLog) {CAM_ULOGW(NSCam::Utils::ULog::MOD_SFPIPE_DEPTH, \
                                                        "%d: " fmt, __LINE__, ##arg);} } while(0)

#define VSDOF_PRFLOG(fmt, arg...) \
    do { if(DepthPipeLoggingSetup::mbProfileLog) {MY_LOGD("[VSDOF_Profile] %d: " fmt, __LINE__, ##arg);} } while(0)

// user load use MY_LOGE to logging
#ifdef USER_LOAD_PROFILE
    #define VSDOF_PRFTIME_LOG(fmt, arg...) \
        MY_LOGE("[VSDOF_Profile] %d: " fmt, __LINE__, ##arg);
    #define VSDOF_INIT_LOG(fmt, arg...) \
        MY_LOGE("[VSDOF_INIT] %d: " fmt, __LINE__, ##arg);
#else
    #define VSDOF_PRFTIME_LOG(fmt, arg...) \
        do { if(DepthPipeLoggingSetup::mbProfileLog) {MY_LOGD("[VSDOF_Profile] %d: " fmt, __LINE__, ##arg);} } while(0)
    #define VSDOF_INIT_LOG(fmt, arg...) \
        MY_LOGD("[VSDOF_INIT] %d: " fmt, __LINE__, ##arg);
#endif

/******************************************************************************
* Type Definition
********************************************************************************/
class DepthMapPipeNode;
typedef KeyedVector<DepthMapPipeNodeID, DepthMapPipeNode*> DepthMapPipeNodeMap;

const DepthMapBufferID INPUT_METADATA_PBID_LIST[] = {PBID_IN_APP_META, PBID_IN_HAL_META_MAIN1, PBID_IN_HAL_META_MAIN2, PBID_IN_P1_RETURN_META};
const DepthMapBufferID OUTPUT_METADATA_PBID_LIST[] = {PBID_OUT_APP_META, PBID_OUT_HAL_META};

}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam

#endif // _MTK_CAMERA_INCLUDE_STEREO_DEPTHMAP_FEATUREPIPE_COMMON_H_
