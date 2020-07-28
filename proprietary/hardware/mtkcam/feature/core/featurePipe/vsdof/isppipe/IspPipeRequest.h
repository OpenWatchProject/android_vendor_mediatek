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
 * @file ISPPipeRequest.h
 * @brief Request inside the ThirdParty ISPPipe
 */

#ifndef _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISPPIPE_REQUEST_H_
#define _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISPPIPE_REQUEST_H_

// Standard C header file
#include <bitset>
#include <memory>
// Android system/core header file
#include <utils/RefBase.h>
#include <utils/String8.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/feature/stereo/pipe/IIspPipeRequest.h>
#include <mtkcam/feature/stereo/pipe/IIspPipe.h>
#include <mtkcam/feature/stereo/pipe/IStopWatch.h>
// Module header file
// Local header file
#include "IspPipeTimer.h"
#include "PipeBufferHandler.h"
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

using std::bitset;

/*******************************************************************************
* Enum Define
********************************************************************************/
enum IspPipeEISOption
{
    PIPE_EIS_OFF,
    PIPE_EIS_ON
};

enum PipeRequestType
{
    PIPE_REQ_META, // no buffer output, only meta output
    PIPE_REQ_FD,    // only FD output, but no preview buffers
    PIPE_REQ_PREVIEW,
    PIPE_REQ_CAPTURE
};

/*******************************************************************************
* Structure Define
********************************************************************************/
struct FDResultInfo
{
    static constexpr MUINT32 MAX_DETECT_FACE_NUM = 15;

    MINT32  miFaceNum = 0;
    MRect   mFaceRegion[FDResultInfo::MAX_DETECT_FACE_NUM];
    MINT32  mRIPDegree[FDResultInfo::MAX_DETECT_FACE_NUM]; //counter-clockwise
};
using FDResultInfoPtr = std::unique_ptr<FDResultInfo>;
/**
 * @struct PipeRequestAttrs
 * @brief IspPipeRequest attributes
 */
struct PipeRequestAttrs
{
    // Default flow
    IspPipeEISOption eisOption          = PIPE_EIS_OFF; // indicate the EIS is on or not
    PipeRequestType reqType             = PIPE_REQ_PREVIEW;
    // DCMF flow
    MBOOL isPureBokeh                   = MFALSE;   // the pure bokeh means only execute bokeh
    FDResultInfoPtr fdResult            = nullptr;  // mean the request include the FD result
    MBOOL isDepthExecuteFailed          = MFALSE;   // failed to execute depth
    MBOOL isDisableDepth                = MFALSE;   // disable depth
    MBOOL isDisableBokeh                = MFALSE;   // disable bokeh
    MBOOL isFDToHDRFlow                 = MFALSE;   // determine whether is the FDNode to HDRNode flow
    MBOOL isFDToDepthFlow               = MFALSE;   // determine whether is the FDNode to Depth flow
    //
};

/*******************************************************************************
* Class Definition
********************************************************************************/
/**
 * @class ISPPipeRequest.h
 * @brief Request used in ThirdParty ISPPipe
 */

class IspPipeRequest : public IIspPipeRequest
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    typedef MVOID (*PFN_PIPE_FINISH_CALLBACK_T)(MVOID* tag, PipeResultState state, sp<IIspPipeRequest>& request);
    IspPipeRequest(
                    MUINT32 _reqNo = 0,
                    PFN_PIPE_FINISH_CALLBACK_T _cb = NULL,
                    MVOID* _tag = NULL);

    virtual ~IspPipeRequest() { mpBufferHandler = nullptr; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IIspPipeRequest Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MBOOL pushRequestImageBuffer(
                const BufferSetting& setting,
                sp<IImageBuffer>& pImgBuf
            );

    virtual MBOOL pushRequestMetadata(
                const BufferSetting& setting,
                IMetadata* pMetaBuf
            ) ;

     virtual MBOOL getRequestImageBuffer(
                const BufferSetting& setting,
                IImageBuffer*& rpImgBuf
            );

    virtual MBOOL getRequestMetadata(
                const BufferSetting& setting,
                IMetadata*& rpMetaBuf
            );

    virtual MBOOL popRequestImageBuffer(
                const BufferSetting& setting
            );

   virtual MBOOL popRequestMetadata(
                const BufferSetting& setting
            );

   virtual MVOID setStopWatchCollection(
                sp<IStopWatchCollection> stopWatchCollection
                );

    virtual sp<IStopWatchCollection> getStopWatchCollection();

    virtual size_t getOutputSize();

    virtual MBOOL startTimer();

    virtual MBOOL stopTimer();

    virtual MUINT32 getElapsedTime();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IspPipeRequest Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief init the effect request
     * @param [in] pHandler current buffer handler address
     * @param [in] pPipeOption isp pipe option
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL init(
            sp<PipeBufferHandler> pHandler,
            sp<IspPipeOption> pPipeOption);
    /**
     * @brief check the output buffer inside the
     *        request is ready or not
     * @return
     * - MTRUE indicates ready
     * - MFALSE indicates not ready
     */
    MBOOL checkAllOutputReady();
    /**
     * @brief set the buffer ready for use
     * @param [in] bufferID buffer id
     * @return
     * - MTRUE indicates success
     * - MFALSE indicates the buffer is not Request buffer.
     */
    MBOOL setOutputBufferReady(const IspPipeBufferID& bufferID);
    /**
     * @brief check the buffer ready or not
     * @param [in] bufferID buffer id
     * @return
     * - MTRUE indicates success
     * - MFALSE indicates the buffer is not Request buffer.
     */
    MBOOL getOutputBufferReady(const IspPipeBufferID& bufferID);
    /**
     * @brief launch the finish callback with states
     * @param [in] state result state
     */
    MVOID launchFinishCallback(PipeResultState state);
    /**
     * @brief get BufferHandler
     * @return
     * - buffer handler pointer
     */
    sp<PipeBufferHandler> getBufferHandler() {return mpBufferHandler;}
    /**
     * @brief get request attributes
     * @return
     * - request attributes
     */
    PipeRequestAttrs& getRequestAttr() {return mReqAttrs;}
    /**
     * @brief check the buffer id is inside request
     * @param [in] bufferID buffer id
     * @return
     * - MTRUE indicates yes
     * - MFALSE indicates no
     */
     MBOOL isRequestBuffer(IspPipeBufferID bufferID);
     /**
     * @brief get the io type of the request buffer
     * @param [in] bufferID buffer id
     * @param [out] ioType io type of this buffer
     * @return
     * - MTRUE indicates success
     * - MFALSE indicates cannot find the iotype,
     * -        that is this buffer is not inside the request
     */
     MBOOL getBufferIOType(
                IspPipeBufferID bufferID,
                BufferIOType& rIOType);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IspPipeRequest Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief Finish callback for EffectRequest
     */
    static MVOID onRequestFinishCallback(
                                MVOID* tag,
                                String8 status,
                                sp<EffectRequest>& request);
    /**
     * @brief parse the request attributes
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL parseReqAttrs(sp<IspPipeOption> pPipeOption);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IspPipeRequest Public Data Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    // timer for this request
    IspPipeTimer mTimer;
    // use for temporary data
    MVOID *mpData;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IspPipeRequest Private Data Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    PipeRequestAttrs mReqAttrs;
    // Request finish callback
    PFN_PIPE_FINISH_CALLBACK_T mpfFinishCallback;
    // buffer handler
    sp<PipeBufferHandler> mpBufferHandler = nullptr;
    // Use to record the IO map
    KeyedVector<IspPipeBufferID, BufferIOType> mBufBIDToIOType;
    KeyedVector<IspPipeBufferID, BufferIOType> mMetaBIDToIOType;
    // stop watch for performance measurement
    sp<IStopWatchCollection> mStopWatchCollection = nullptr;
};

typedef sp<IspPipeRequest> IspPipeRequestPtr;



}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam


#endif