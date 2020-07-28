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

#include <cstddef>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <mtkcam/utils/std/ULogDef.h>
#include "ULogTable.h"


// #define ULOG_EVALUATE_HASH


namespace NSCam {
namespace Utils {
namespace ULog {

alignas(256)
constexpr ModuleNameItem MODULE_NAME_TABLE[] =
{
    // Module name must be match regular expression [A-Za-z0-9_]+
    // Please group by layers

    { MOD_CAMERA_HAL_SERVER,        "CameraHalServer" },
    { MOD_ISP_HAL_SERVER,           "ISPHalServer" },
    { MOD_CAMDEV3_IMP,              "CameraDevice3" },
    { MOD_APP_STREAM_MGR,           "AppStreamMgr" },
    { MOD_CAMERA_DEVICE,            "CameraDevice" },

    { MOD_PIPELINE_PRERELEASE,      "PreRelease" },
    { MOD_PIPELINE_MODEL_CAPTURE,   "PipelineModelCapture" },
    { MOD_PIPELINE_MODEL,           "PipelineModel" },
    { MOD_DEFAULT_PIPELINE_MODEL,   "DefaultPipelineModel" },
    { MOD_4CELL_PIPELINE_MODEL,     "4-CellPipelineModel" },
    { MOD_MULTICAM_PIPELINE_MODEL,  "MultiCamPipelineModel" },
    { MOD_SMVR_PIPELINE_MODEL,      "SMVRPipelineModel" },
    { MOD_STREAM_PIPELINE_MODEL,    "StreamPipelineModel" },
    { MOD_PIPELINE_POLICY,          "PipelinePolicy" },
    { MOD_FEATURE_SETTING_POLICY,   "FeatureSettingPolicy" },
    { MOD_PIPELINE_CONTEXT,         "PipelineContext" },
    { MOD_PIPELINE_UTILITY,         "PipelineUtility" },
    { MOD_ZSL,                      "ZSL" },

    { MOD_HW_NODE,                  "HwNode_unknown" },
    { MOD_P1_NODE,                  "P1Node" },
    { MOD_P1_NODE_2,                "P1Node_2" },
    { MOD_P1_NODE_3,                "P1Node_3" },
    { MOD_P2_CAP_NODE,              "P2CaptureNode" },
    { MOD_JPEG_NODE,                "JpegNode" },
    { MOD_P2_STR_NODE,              "P2StreamingNode" },
    { MOD_FD_NODE,                  "FDNode" },
    { MOD_BASE_NODE,                "BaseNode" },
    { MOD_P2N_COMMON,               "P2Node" },

    { MOD_3A_FRAMEWORK,             "Hal3Av3" },
    { MOD_3A_FRAMEWORK_THREAD,      "Hal3AFlowCtrl_thread" },
    { MOD_3A_FRAMEWORK_FLOW,        "Hal3AFlowCtrl" },
    { MOD_3A_FRAMEWORK_RESULT,      "ResultPool" },
    { MOD_3A_FRAMEWORK_IP_BASE,     "Hal3ARaw" },
    { MOD_ISP_MGR,                  "IspMgr" },
    { MOD_P1_SYNCHELPER,            "SyncHelper" },
    { MOD_P2_CAP_PROC,              "P2_CaptureProcessor" },
    { MOD_FPIPE_CAPTURE,            "CaptureFeaturePipe" },
    { MOD_P2_STR_PROC,              "P2_StreamingProcessor" },
    { MOD_P2_PROC_COMMON,           "P2_Processor" },
    { MOD_P2_BASIC_PROC,            "P2_BasicProcessor" },
    { MOD_FPIPE_COMMON,             "FeaturePipe" },

    { MOD_CAPTURE_ROOT,             "C_Root" },
    { MOD_CAPTURE_RAW,              "C_RAW" },
    { MOD_CAPTURE_BSS,              "C_BSS" },
    { MOD_CAPTURE_P2A,              "C_P2A" },
    { MOD_CAPTURE_MULTIFRAME,       "C_MultiFrame" },
    { MOD_CAPTURE_DEPTH,            "C_Depth" },
    { MOD_CAPTURE_BOKEH,            "C_Bokeh" },
    { MOD_CAPTURE_FUSION,           "C_Fusion" },
    { MOD_CAPTURE_MDP,              "C_MDP" },
    { MOD_CAPTURE_FD,               "C_FD" },
    { MOD_CAPTURE_YUV,              "C_YUV" },
    { MOD_CAPTURE_YUV_1,            "C_YUV_1" },
    { MOD_CAPTURE_YUV_2,            "C_YUV_2" },
    { MOD_CAPTURE_YUV_3,            "C_YUV_3" },

    { MOD_FPIPE_STREAMING,          "StreamingFeaturePipe" },
    { MOD_STREAMING_ROOT,           "S_Root" },
    { MOD_STREAMING_P2A,            "S_P2A" },
    { MOD_STREAMING_P2NR,           "S_P2NR" },
    { MOD_STREAMING_VNR,            "S_VNR" },
    { MOD_STREAMING_P2A_MDP,        "S_P2A_MDP" },
    { MOD_STREAMING_RSC,            "S_RSC" },
    { MOD_STREAMING_DEPTH,          "S_Depth" },
    { MOD_STREAMING_BOKEH,          "S_Bokeh" },
    { MOD_STREAMING_VENDOR_MDP,     "S_VendorMDP" },
    { MOD_STREAMING_FOV,            "S_FOV" },
    { MOD_STREAMING_FOV_FEFM,       "S_FOVFEFM" },
    { MOD_STREAMING_FOV_WARP,       "S_FOVWarp" },
    { MOD_STREAMING_EIS,            "S_EISNode" },
    { MOD_STREAMING_EIS_WARP,       "S_EISWarp" },
    { MOD_STREAMING_MDP,            "S_MDP" },
    { MOD_STREAMING_HELPER,         "S_Helper" },
    { MOD_STREAMING_TPI,            "S_TPI" },
    { MOD_STREAMING_TPI_DISP,       "S_TPIDisp" },
    { MOD_STREAMING_TPI_ASYNC,      "S_TPIAsync" },
    { MOD_STREAMING_TPI_PLUGIN,     "S_TPIPlugin" },

    { MOD_FPNODE_UNKNOWN,           "FPNode_unknown" },
    { MOD_FPNODE_SEQ_QUEUE,         "SeqUtil" },

    { MOD_SFPIPE_DEPTH_P2A,         "D_P2A" },
    { MOD_SFPIPE_DEPTH_P2A_BAYER,   "D_P2ABayer" },
    { MOD_SFPIPE_DEPTH_N3D,         "D_N3D" },
    { MOD_SFPIPE_DEPTH_NR3D,        "D_NR3D" },
    { MOD_SFPIPE_DEPTH_GF,          "D_GF" },
    { MOD_SFPIPE_DEPTH_FD,          "D_FD" },
    { MOD_SFPIPE_DEPTH_WPE,         "D_WPE" },
    { MOD_SFPIPE_DEPTH_DPE,         "D_DPE" },
    { MOD_SFPIPE_DEPTH_OCC,         "D_OCC" },
    { MOD_SFPIPE_DEPTH_HW_OCC,      "D_HW_OCC" },
    { MOD_SFPIPE_DEPTH_WMF,         "D_WMF" },
    { MOD_SFPIPE_DEPTH_DLDEPTH,     "D_DLDEPTH" },

    { MOD_PD_HAL,                   "PD_HAL" },
    { MOD_LMV_HAL,                  "LMV" },
    { MOD_VHDR_HAL,                 "vHDR" },
    { MOD_FD_HAL,                   "FD" },
    { MOD_ASD_HAL,                  "ASD" },
    { MOD_LIB_MFNR,                 "Mfll" },
    { MOD_LIB_AINR,                 "Ainr" },
    { MOD_LIB_SWNR,                 "Swnr" },
    { MOD_LIB_HDR,                  "HDR" },
    { MOD_LIB_DEPTH,                "Depth" },
    { MOD_LIB_BOKEH,                "Bokeh" },
    { MOD_LIB_FUSION,               "Fusion" },
    { MOD_LIB_PUREBOKEH,            "PureBokeh" },
    { MOD_ABF_ADAPT,                "AbfAdapt" },
    { MOD_LIB_FB,                   "FaceBeautify" },
    { MOD_EIS_HAL,                  "EisHal" },
    { MOD_3DNR_HAL,                 "NR3D" },
    { MOD_FSC_HAL,                  "FSC" },
    { MOD_RSS_HAL,                  "RSS" },
    { MOD_VSDOF_HAL,                "VSDoF_HAL" },
    { MOD_VENDOR_LIB_COMMON,        "VendorLib" },

    { MOD_DRV_NORMAL_PIPE,          "NormalPipe" },
    { MOD_DRV_SENSOR,               "Sensor" },
    { MOD_DRV_CAMCAL,               "DrvCamCal" },
    { MOD_DRV_DIP,                  "DrvDIP" },
    { MOD_DRV_RSC,                  "DrvRSC" },
    { MOD_DRV_FD,                   "DrvFD" },
    { MOD_DRV_DPE,                  "DrvDPE" },
    { MOD_DRV_WPE,                  "DrvWPE" },
    { MOD_DRV_OWE,                  "DrvOWE" },
    { MOD_DRV_MFB,                  "DrvMFB" },
    { MOD_DRV_MSS,                  "DrvMSS" },
    { MOD_DRV_MSF,                  "DrvMSF" },
    { MOD_DRV_LPCNR,                "DrvLPCNR" },
    { MOD_DRV_COMMON,               "Driver" },
    { MOD_IOPIPE_EVENT,             "IoPipeEvent" },

    { MOD_UTILITY,                  "Utils" },
    { MOD_IMAGE_BUFFER,             "ImageBuffer" },
    { MOD_METADATA,                 "Metadata" },
    { MOD_MULTICAM_PROVIDER,        "MulticamProvider" },
    { MOD_MODULE_REGISTRY,          "ModuleRegistry" },
    { MOD_SIMAGER,                  "SImager"},
    { MOD_ISP_PROFILE_MAPPER,       "ISPProfileMapper" },
    { MOD_UNIFIED_LOG,              "ULog" },
};


constexpr RequestTypeItem REQUEST_TYPE_NAME_TABLE[] =
{
    { REQ_INVALID_ID,               "INVALID" },
    { REQ_APP_REQUEST,              "AppRequest" },
    { REQ_PIPELINE_FRAME,           "PipelineFrame" },
    { REQ_P2_CAP_REQUEST,           "C_P2Request" },
    { REQ_P2_STR_REQUEST,           "S_P2Request" },
    { REQ_CAP_FPIPE_REQUEST,        "CaptureRequest" },
    { REQ_STR_FPIPE_REQUEST,        "StreamingFeatureRequest" },
    { REQ_DEPTH_REQUEST,            "DepthMapRequest" },
    { REQ_QBUF_INFO,                "QBufInfo" },
    { REQ_QPARAMS,                  "QParams" },

    // Please add before here and in order
    { REQUEST_TYPE_END,             "END" },
};


constexpr size_t MODULE_NAME_TABLE_SIZE = sizeof(MODULE_NAME_TABLE) / sizeof(MODULE_NAME_TABLE[0]);

template <int index>
struct VerifyModNameTable
{
    static constexpr bool isSorted =
        (MODULE_NAME_TABLE[index].moduleId > MODULE_NAME_TABLE[index - 1].moduleId) &&
        VerifyModNameTable<index - 1>::isSorted;
};

template <>
struct VerifyModNameTable<0>
{
    static constexpr bool isSorted = true;
};

// MODULE_NAME_TABLE[] MUST be sorted when fill
static_assert(VerifyModNameTable<MODULE_NAME_TABLE_SIZE - 1>::isSorted,
    "MODULE_NAME_TABLE[] was not filled in monotonic increasing (sorted)");


static_assert(REQUEST_TYPE_NAME_TABLE[REQUEST_TYPE_END].requestTypeId == REQUEST_TYPE_END,
    "REQUEST_TYPE_NAME_TABLE[] must be filled with the names of all request types");

template <int index>
struct VerifyReqNameTable
{
    static constexpr bool allFilled = (REQUEST_TYPE_NAME_TABLE[index].requestTypeId == index) && VerifyReqNameTable<index - 1>::allFilled;
};

template <>
struct VerifyReqNameTable<-1>
{
    static constexpr bool allFilled = true;
};

static_assert(VerifyReqNameTable<REQUEST_TYPE_END>::allFilled, "REQUEST_TYPE_NAME_TABLE[] was not filled in order or entry lost");


inline unsigned int ULogTable::getCacheSlot(ModuleId moduleId)
{
    unsigned int hash = (moduleId ^ (moduleId >> 7) ^ ((moduleId >> 16) | (moduleId >> 20)));
    return (hash & (CACHE_SIZE - 1));
}


void ULogTable::init()
{
#ifdef ULOG_EVALUATE_HASH
    for (size_t i = 0; i < MODULE_NAME_TABLE_SIZE; i++) {
        ModuleId moduleId = MODULE_NAME_TABLE[i].moduleId;
        printf("%08x -> %u\n", moduleId, getCacheSlot(moduleId));
    }
#endif
}


const char *ULogTable::getModuleName(ModuleId moduleId) const
{
    const ModuleNameItem *moduleNameTable = MODULE_NAME_TABLE;

    // Hash cache to accelerate name look-up
    unsigned int slotIndex = getCacheSlot(moduleId);
    // mModuleNameIndexCache is not atomic, but we don't care
    unsigned int tableIndex = static_cast<unsigned int>(mModuleNameIndexCache[slotIndex]);
    if (tableIndex < MODULE_NAME_TABLE_SIZE &&
        moduleNameTable[tableIndex].moduleId == moduleId)
    {
        return moduleNameTable[tableIndex].moduleName;
    }

    // Standard library is not suitable here, we implemented our binary search.
    // We have maken sure MODULE_NAME_TABLE[] is sorted by VerifyModNameTable<>
    int lower = 0, upper = MODULE_NAME_TABLE_SIZE - 1;

    while (lower <= upper) {
        if (lower == upper) {
            if (moduleNameTable[lower].moduleId == moduleId) {
                mModuleNameIndexCache[slotIndex] = static_cast<std::int16_t>(lower);
                return moduleNameTable[lower].moduleName;
            } else {
                return nullptr;
            }
        }

        int mid = (lower + upper) / 2;
        const ModuleNameItem *midEntry = &moduleNameTable[mid];
        if (midEntry->moduleId < moduleId)
            lower = mid + 1;
        else if (midEntry->moduleId > moduleId)
            upper = mid - 1;
        else {
            mModuleNameIndexCache[slotIndex] = static_cast<std::int16_t>(mid);
            return midEntry->moduleName;
        }
    }

    return nullptr;
}


const char *ULogTable::getRequestTypeName(RequestTypeId requestTypeId) const
{
    if (requestTypeId < REQUEST_TYPE_END) {
        // We have verified the table by VerifyReqNameTable<>
        return REQUEST_TYPE_NAME_TABLE[requestTypeId].requestName;
    }

    return nullptr;
}


// ULogTable is invisiable to modules, we add a query API
const char *getULogReqNameById(RequestTypeId req)
{
    const char *requestName = ULogTable::getSingleton().getRequestTypeName(req);
    return (requestName != nullptr) ? requestName : "?";
}


}
}
}

