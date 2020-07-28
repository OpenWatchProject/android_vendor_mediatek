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
 * @file IDualFeatureRequest.h
 * @brief Base class for dual feature requests
 */

#ifndef _MTK_CAMERA_FEATURE_INTERFACE_INTERFACE_DUAL_FEATURE_REQUEST_COMMON_H_
#define _MTK_CAMERA_FEATURE_INTERFACE_INTERFACE_DUAL_FEATURE_REQUEST_COMMON_H_

// Standard C header file
#include <chrono>
// Android system/core header file
#include <utils/RefBase.h>
#include <utils/RWLock.h>
#include <utils/KeyedVector.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/feature/effectHalBase/EffectRequest.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/metadata/IMetadata.h>
// Module header file

// Local header file

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature{
namespace NSDualFeature {

typedef MINT32 RequestBufferID;
using namespace std::chrono;

#define DUAL_REQ_META_KEY "MetaKey"
/*******************************************************************************
 * Enum Definition
********************************************************************************/
enum ResultState
{
    eRESULT_DEPTH_NOT_READY,
    eRESULT_COMPLETE,
    eRESULT_ERROR,
    eRESULT_YUV_DONE,
    eRESULT_FLUSH
};

enum BufferIOType{
    eBUFFER_IOTYPE_INPUT,
    eBUFFER_IOTYPE_OUTPUT,
    eBUFFER_IOTYPE_SIZE
};

enum BufferType{
    eBUFFER_TYPE_IMAGE,
    eBUFFER_TYPE_META
};
/*******************************************************************************
* Structure Definition
********************************************************************************/
struct NodeBufferSetting
{
public:
    RequestBufferID bufferID;
    BufferIOType ioType;
};

/*******************************************************************************
* Class Definition
********************************************************************************/

/**
 * @class IDualFeatureRequest
 * @brief request interface for dual feature
 */
class IDualFeatureRequest : public EffectRequest
{
public:
    typedef MVOID (*PFN_IREQ_FINISH_CALLBACK_T)(MVOID* tag, ResultState state, sp<IDualFeatureRequest>& request);
    typedef MINT32 (*PFN_ID_MAPPER_T) (MINT32);

protected:
    IDualFeatureRequest() = delete;
    IDualFeatureRequest(
        MUINT32 _reqNo = 0,
        PFN_IREQ_FINISH_CALLBACK_T _cb = NULL,
        MVOID* _tag = NULL);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IDualFeatureRequest Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief push input/output image stream buffer into the EffectRequest
     * @param [in] reqestID request ID
     * @param [in] setting buffer setting
     * @param [in] pImgBuf image buffer pointer
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL pushRequestImageBuffer(
                const NodeBufferSetting& setting,
                sp<IImageBuffer>& pImgBuf
            );

    /**
     * @brief push input/output metadata into the EffectRequest
     * @param [in] reqestID request ID
     * @param [in] setting buffer setting
     * @param [in] pMetaBuf Metadata pointer
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL pushRequestMetadata(
                const NodeBufferSetting& setting,
                IMetadata* pMetaBuf
            );

    /**
     * @brief get input/output image buffer from the EffectRequest
     * @param [in] ioType input or output
     * @param [in] bufferID image buffer ID
     * @param [out] rpImgBuf image buffer pointer
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL getRequestImageBuffer(
                const NodeBufferSetting& setting,
                IImageBuffer*& rpImgBuf
            );

    /**
     * @brief get input/output metadata from the EffectRequest
     * @param [in] ioType input or output
     * @param [in] bufferID meta buffer ID
     * @param [out] rpMetaBuf Metadata pointer
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL getRequestMetadata(
                const NodeBufferSetting& setting,
                IMetadata*& rpMetaBuf
            );

    /**
     * @brief pop input/output image buffer from the EffectRequest
     * @param [in] ioType input or output
     * @param [in] bufferID image buffer ID
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL popRequestImageBuffer(
                const NodeBufferSetting& setting
            );

    /**
     * @brief pop input/output metadata from the EffectRequest
     * @param [in] ioType input or output
     * @param [in] bufferID meta buffer ID
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL popRequestMetadata(
                const NodeBufferSetting& setting
            );
    /**
     * @brief check the buffer id is inside request
     * @param [in] bufferID buffer id
     * @return
     * - MTRUE indicates yes
     * - MFALSE indicates no
     */
     MBOOL isRequestBuffer(RequestBufferID bufferID);
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
                RequestBufferID bufferID,
                BufferIOType& rIOType);
    /**
     * @brief start the request timer
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL startTimer();

    /**
     * @brief stop the request timer
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL stopTimer();

    /**
     * @brief get elapsed time from timer started
     * @return
     * - elapsed time in milliseconds
     */
    virtual MUINT32 getElapsedTime();
    /**
     * @brief launch finish callback
     * @return
     * - elapsed time in milliseconds
     */
    virtual MVOID launchFinishCallback(ResultState state);
    /**
     * @brief copy current request to input request
     * @param [out] pRequest request to be copied
     * @param [in] idMapper index key mapper,
     *              if return value is negative the buffer copy will skip
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL copyTo(
        IDualFeatureRequest* pRequest,
        PFN_ID_MAPPER_T keyMapper = [](int id){return id;}
        );
    /**
     * @brief get buffer Type
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL getBufferType(
                RequestBufferID bufferID,
                BufferType& bufType);
     /**
     * @brief show all in/out frame data
     */
    MVOID showContent();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IDualFeatureRequest Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief callback for EffectRequest
     * @return
     * - elapsed time in milliseconds
     */
    static MVOID onRequestFinishCallback(
                    MVOID* tag,
                    String8 status,
                    sp<EffectRequest>& request) {};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IDualFeatureRequest Protected member.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    PFN_IREQ_FINISH_CALLBACK_T mpfnCallback;
    // RWLock
    android::RWLock mFrameLock[eBUFFER_IOTYPE_SIZE];
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IDualFeatureRequest Private member.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    // time point
    system_clock::time_point mStart;
    system_clock::time_point mEnd;
    // Use to record the IO map
    KeyedVector<RequestBufferID, BufferIOType> mBufBIDToIOType;
    KeyedVector<RequestBufferID, BufferIOType> mMetaBIDToIOType;
};

inline const char* ResultState2Name(ResultState state)
{
#define MAKE_NAME_CASE(name) \
  case name: return #name;

  switch(state)
  {
    MAKE_NAME_CASE(eRESULT_DEPTH_NOT_READY);
    MAKE_NAME_CASE(eRESULT_COMPLETE);
    MAKE_NAME_CASE(eRESULT_ERROR);
    MAKE_NAME_CASE(eRESULT_YUV_DONE);
    MAKE_NAME_CASE(eRESULT_FLUSH);
  };
  return "UNKNOWN";
#undef MAKE_NAME_CASE
}


};
};
};

#endif
