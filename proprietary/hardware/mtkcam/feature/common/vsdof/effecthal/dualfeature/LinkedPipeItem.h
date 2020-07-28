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
 * MediaTek Inc. (C) 2017. All rights reserved.
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
* @file LinkedPipeItem.h
* @brief Use for feature sequential execution flow
*/
#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_LINKED_PIPEITEM_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_LINKED_PIPEITEM_H_

// Standard C header file
// Android system/core header file
#include <utils/Mutex.h>
#include <utils/RefBase.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>
// mtkcam custom header file
// mtkcam global header file
#include <mtkcam/feature/stereo/pipe/IBokehPipe.h>
#include <mtkcam/feature/stereo/pipe/IDepthMapPipe.h>
#include <mtkcam/feature/stereo/pipe/IDepthMapEffectRequest.h>
#include <mtkcam/feature/stereo/effecthal/DualFeatureEffectHal.h>
// Module header file

// Local header file
/*******************************************************************************
* Macro Define
********************************************************************************/

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature{
namespace NSDualFeature {

using android::Mutex;
using android::sp;
using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NSCam::NSCamFeature::NSFeaturePipe_DepthMap;

/*******************************************************************************
* Function Define
********************************************************************************/
/**
 * @brief Try to get metadata value
 * @param [in] pMetadata IMetadata instance
 * @param [in] tag the metadata tag to retrieve
 * @param [out] rVal the metadata value to be stored.
 * @return
 * - MTRUE indicates success.
 * - MFALSE indicates failure.
 */
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
);

/**
 * @brief Try to set metadata value
 * @param [in] pMetadata IMetadata instance
 * @param [in] tag the metadata tag to configure
 * @param [in] rVal the metadata value to set
 * @return
 * - MTRUE indicates success.
 * - MFALSE indicates failure.
 */
template <typename T>
inline MVOID
trySetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
);

/**
 * @brief update the metadata entry
 * @param [in] pMetadata IMetadata instance
 * @param [in] tag the metadata tag to update
 * @param [in] rVal the metadata entry value
 * @return
 * - MTRUE indicates success.
 * - MFALSE indicates failure.
 */
template <typename T>
inline MVOID
updateEntry(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
);
/*******************************************************************************
* Class Define
********************************************************************************/

/**
 * @class ILinkedPipeItem
 * @brief interface for base pipe item
 */
class ILinkedPipeItem : public android::LightRefBase<ILinkedPipeItem>
{
public:
    ILinkedPipeItem() {}
    virtual ~ILinkedPipeItem() {mpNext = nullptr; mpHead = nullptr;}
    virtual MBOOL enque(sp<IDualFeatureRequest> pRequest) = 0;
public:
    sp<ILinkedPipeItem> mpNext = nullptr;
    sp<ILinkedPipeItem> mpHead = nullptr;
};

/**
 * @class DepthPipeItem
 * @brief depthmap pipe item
 */
class DepthPipeItem : public ILinkedPipeItem
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Initialization
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    DepthPipeItem(IDepthMapPipe* pPipe);
    virtual ~DepthPipeItem();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ILinkedPipeItem Public Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MBOOL enque(sp<IDualFeatureRequest> pRequest);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthPipeItem Public Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static MVOID onPipeReady(
                        MVOID* tag,
                        ResultState state,
                        sp<IDualFeatureRequest>& request);

    MVOID onHandlePipeReady(
                        ResultState state,
                        sp<IDualFeatureRequest>& request);

    static DepthMapBufferID mapToDepthBufferID(RequestBufferID bufferID);
public:
    Mutex mOpLock;
    IDepthMapPipe* mpPipe;
    KeyedVector<MINT32, sp<IDualFeatureRequest> > mRequestMap;
    MINT32 miLogLevel;
};

/**
 * @class BokehPipeItem
 * @brief bokeh pipe item
 */
class BokehPipeItem : public ILinkedPipeItem
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Initialization
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    BokehPipeItem(sp<IBokehPipe> pPipe);
    virtual ~BokehPipeItem();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ILinkedPipeItem Public Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MBOOL enque(sp<IDualFeatureRequest> pRequest);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BokehPipeItem Public Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    static MVOID onPipeReady(
                        MVOID* tag,
                        String8 status,
                        sp<NSCam::EffectRequest>& request);

    MVOID onHandlePipeReady(
                        ResultState state,
                        sp<NSCam::EffectRequest>& request);

    static BokehEffectRequestBufferType mapToBokehBufferID(RequestBufferID bufferID);
    /**
     * @brief copy IDualFeatureRequest to EffectRequest
     * @param [in] source IDualFeatureRequest
     * @param [out] EffectRequest
     * @return
     * - MTRUE success
     * - MFALSE failure
     */
    MBOOL copyToBokehRequest(
                        sp<IDualFeatureRequest> source,
                        sp<EffectRequest> target);

    MBOOL _copyMetaToFrame(
                        sp<IDualFeatureRequest> pSourceReq,
                        RequestBufferID copyBufID,
                        sp<EffectFrameInfo> rOutFrame
                        );

    MBOOL _copyBufferToFrame(
                        sp<IDualFeatureRequest> pSourceReq,
                        RequestBufferID copyBufID,
                        sp<EffectFrameInfo> rOutFrame
                        );
public:
    Mutex mOpLock;
    sp<IBokehPipe> mpPipe;
    KeyedVector<MINT32, sp<IDualFeatureRequest> > mRequestMap;
    MINT32 miLogLevel;
};


};
};
};
#endif