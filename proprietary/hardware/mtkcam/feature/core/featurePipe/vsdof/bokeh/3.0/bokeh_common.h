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
#ifndef _MTK_CAMERA_VSDOF_BOKEH_FEATURE_PIPE_BOKEH_COMMON_H_
#define _MTK_CAMERA_VSDOF_BOKEH_FEATURE_PIPE_BOKEH_COMMON_H_

#include <mtkcam/def/BuiltinTypes.h>
#include <mtkcam/drv/iopipe/PortMap.h>
#include <mtkcam/feature/stereo/pipe/IDepthMapPipe.h>
#include <mtkcam/feature/stereo/pipe/IBokehPipe.h>
//
#include <stdint.h>
#define GF_PIPELINE_FRAME_DMG_BUFFER_SIZE 3
#define GF_PIPELINE_FRAME_DMBG_BUFFER_SIZE 3
#define BOKEH_PIPELINE_FRAME_3DNR_BUFFER_SIZE 2
#define BOKEH_PIPELINE_FRAME_TUNING_BUFFER_SIZE 5
#define BOKEH_PIPELINE_FRAME_WDMA_BUFFER_SIZE 1
#define BOKEH_PIPELINE_FRAME_SCENARIO_ID "BOKEH_SCENARIO_ID"
//
#include <mtkcam/def/common.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <isp_tuning/isp_tuning.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/aaa/aaa_hal_common.h>
#include <mtkcam/feature/stereo/pipe/vsdof_common.h>
#include <mtkcam/feature/effectHalBase/EffectRequest.h>
//
// Module header file.
#define PIPE_MODULE_TAG "BokehPipe"
#define PIPE_CLASS_TAG "Common"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG
#include <featurePipe/core/include/PipeLog.h>
#include <featurePipe/vsdof/util/TuningBufferPool.h>

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

//
typedef android::sp<EffectRequest> EffectRequestPtr;
typedef android::sp<EffectFrameInfo> FrameInfoPtr;
//

/*******************************************************************************
* Enum Define
********************************************************************************/
enum eFrameIOType
{
    FRAME_INPUT,
    FRAME_OUTPUT
};

enum BokehPipeDataID
{
    ID_INVALID,
    ID_ROOT_ENQUE,
    GF_BOKEH_REQUEST,
    GF_ERROR,
    P2B_ERROR,
    GF_BOKEH_DMG,
    GF_BOKEH_DMBG,
    P2B_MDP_REQUEST,
    P2B_MDP_WDMA,
    GF_OUT,
    P2B_OUT,
    MDP_OUT,
    VENDOR_OUT,
    VENDOR_TO_MDP,
    SWFILTER_MDP_REQUEST,
};
enum GFNodeMode : MINT8
{
    GF_NODE_MODE_NONE,
    GF_NODE_MODE_NORMAL,
    GF_NODE_MODE_ADVANCE,
};
enum BOKEH_ERROR_MESSAGE
{
    PROCESS_SUCCESS,
    //
    GF_ALGO_BUF_MYS_NULL,
    GF_ALGO_BUF_DMW_NULL,
    GF_ALGO_BUF_MYS_NOT_EXIST,
    GF_ALGO_BUF_DMW_NOT_EXIST,
    GF_ALGO_BUF_MYS_FORMAT_INVALID, // 5
    //
    GF_ALGO_BUF_DMW_FORMAT_INVALID,
    GF_ALGO_BUF_MYS_SIZE_INVALID,
    GF_ALGO_BUF_DMW_SIZE_INVALID,
    P2B_BUF_MAIN_IMG_NOT_EXIST,
    P2B_BUF_DISPLAY_IMG_NOT_EXIST, // 10
    //
    P2B_BUF_RECORD_IMG_NOT_EXIST,
    P2B_BUF_VSDOF_IMG_NOT_EXIST,
    P2B_BUF_MAIN_IMG_SET_PORT_FAILED,
};

/*******************************************************************************
* Structure Definition
********************************************************************************/
struct BokehNodeBufferSetting
{
public:
    BokehEffectRequestBufferType bufferID;
    eFrameIOType ioType;
};

// port description
// (portIndex, portType, in/out(0/1))
// input port

using NSCam::NSIoPipe::PORT_DEPI;
using NSCam::NSIoPipe::PORT_DMGI;
using NSCam::NSIoPipe::PORT_IMGI;
using NSCam::NSIoPipe::PORT_IMG2O;
using NSCam::NSIoPipe::PORT_IMG3O;
using NSCam::NSIoPipe::PORT_WDMAO;
using NSCam::NSIoPipe::PORT_WROTO;
using NSCam::NSIoPipe::PORT_FEO;
using NSCam::NSIoPipe::PORT_MFBO;

/*******************************************************************************
*
********************************************************************************/
/**
 * Try to get metadata value
 *
 * @param[in]  pMetadata: IMetadata instance
 * @param[in]  tag: the metadata tag to retrieve
 * @param[out]  rVal: the metadata value
 *
 *
 * @return
 *  -  true if successful; otherwise false.
 */
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return MFALSE;
    }
    //
    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

/**
 * Try to set metadata value
 *
 * @param[in]  pMetadata: IMetadata instance
 * @param[in]  tag: the metadata tag to set
 * @param[in]  val: the metadata value to be configured
 *
 *
 * @return
 *  -  true if successful; otherwise false.
 */
template <typename T>
inline MVOID
trySetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return;
    }
    //
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}

template <typename T>
MBOOL
tryGetMetadataInRequest(
    const EffectRequestPtr request,
    eFrameIOType type,
    BokehEffectRequestBufferType bid,
    MUINT32 const tag,
    T & rVal)
{

    ssize_t keyIndex = -1;
    sp<EffectFrameInfo> frameInfo = nullptr;
    sp<EffectParameter> pEffParam = nullptr;
    IMetadata* pMeta = nullptr;

    auto& vFrameInfo = (type == FRAME_INPUT) ? request->vInputFrameInfo : request->vOutputFrameInfo;

    keyIndex = vFrameInfo.indexOfKey(bid);
    if(keyIndex>=0)
    {
        frameInfo = vFrameInfo.valueAt(keyIndex);
        pEffParam = frameInfo->getFrameParameter();
        pMeta = reinterpret_cast<IMetadata*>(pEffParam->getPtr(BOKEH_META_KEY_STRING));
    }
    else
    {
        MY_LOGE("Get output metadata fail. bufferType(%d)", type);
        return MFALSE;
    }

    if(pMeta == nullptr){
        MY_LOGE("no such metadata!");
        return MFALSE;
    }

    if( !tryGetMetadata<T>(pMeta, tag, rVal) ) {
        MY_LOGE("no such metadata tag!");
        return MFALSE;
    }

    return MTRUE;
}

template <typename T>
inline MVOID
updateEntry(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}

/*******************************************************************************
* Namespace end.
********************************************************************************/
};
};
};

#endif //_MTK_CAMERA_VSDOF_BOKEH_FEATURE_PIPE_BOKEH_COMMON_H_
