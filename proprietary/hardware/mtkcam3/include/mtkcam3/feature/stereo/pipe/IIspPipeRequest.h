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
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARES
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/**
 * @file IISPPipeRequest.h
 * @brief Effect Request Interface inside the Zoom ISPPipe
 */

#ifndef _MTK_CAMERA_FEATURE_INTERFACE_ZOOM_ISPPIPE_EFFECT_REQUEST_H_
#define _MTK_CAMERA_FEATURE_INTERFACE_ZOOM_ISPPIPE_EFFECT_REQUEST_H_

// Standard C header file

// Android system/core header file
#include <utils/RefBase.h>
#include <utils/String8.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam3/feature/effectHalBase/EffectRequest.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/metadata/IMetadata.h>
// Module header file

// Local header file
#include "IIspPipe.h"
#include "IStopWatch.h"


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

/*******************************************************************************
* Enum Definition
********************************************************************************/
/**
 * @brief buffer IN/OUT type
 */
typedef enum eISPPipeBufIOType {
    eBUFFER_IOTYPE_INPUT,
    eBUFFER_IOTYPE_OUTPUT
} BufferIOType;

typedef enum ePipeResultState {
    eRESULT_SUCCESS,
    eRESULT_FAILURE,
    eRESULT_FLUSH
} PipeResultState;

const char* fnStateToName(PipeResultState state);
/*******************************************************************************
* Structure Definition
********************************************************************************/
struct BufferSetting
{
public:
    IspPipeBufferID bufferID;
    BufferIOType ioType;
};

/*******************************************************************************
* Class Definition
********************************************************************************/
/**
 * @class IIspPipeRequest
 * @brief Interface of the effect request inside the Zoom ISP Pipe
 */
class IIspPipeRequest : public EffectRequest
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    typedef MVOID (*PFN_PIPE_FINISH_CALLBACK_T)(MVOID* tag, PipeResultState state, sp<IIspPipeRequest>& request);
    /**
     * @brief create a new IIspPipeRequest
     * @param [in] _reqNo request no.
     * @param [in] _cb callback when job finishes
     * @param [in] _tag cookie pointer
     * @return
     * - allocated IIspPipeRequest instance address
     */
    static sp<IIspPipeRequest> createInstance(
                                                MUINT32 _reqNo = 0,
                                                PFN_PIPE_FINISH_CALLBACK_T _cb = NULL,
                                                MVOID* _tag = NULL
                                            );
protected:
    IIspPipeRequest(
                    MUINT32 _reqNo = 0,
                    PFN_CALLBACK_T _cb = NULL,
                    MVOID* _tag = NULL);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IIspPipeRequest Public Operations.
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
                const BufferSetting& setting,
                sp<IImageBuffer>& pImgBuf
            ) = 0;

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
                const BufferSetting& setting,
                IMetadata* pMetaBuf
            ) = 0;

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
                const BufferSetting& setting,
                IImageBuffer*& rpImgBuf
            ) = 0;

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
                const BufferSetting& setting,
                IMetadata*& rpMetaBuf
            ) = 0;

    /**
     * @brief pop input/output image buffer from the EffectRequest
     * @param [in] ioType input or output
     * @param [in] bufferID image buffer ID
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL popRequestImageBuffer(
                const BufferSetting& setting
            ) = 0;

    /**
     * @brief pop input/output metadata from the EffectRequest
     * @param [in] ioType input or output
     * @param [in] bufferID meta buffer ID
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL popRequestMetadata(
                const BufferSetting& setting
            ) = 0;

    /**
     * @brief set stopWatchCollection for performance measurement
     */
    virtual MVOID setStopWatchCollection(
            sp<IStopWatchCollection> stopWatchCollection
            ) = 0;

    /**
     * @brief get stopWatchCollection for performance measurement
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual sp<IStopWatchCollection> getStopWatchCollection() = 0;

    /**
     * @brief get output buffer plus metadata size
     * @return
     * - buffer size
     */
    virtual size_t getOutputSize() = 0;

    /**
     * @brief start the request timer
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL startTimer() = 0;

    /**
     * @brief stop the request timer
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL stopTimer() = 0;

    /**
     * @brief get elapsed time from timer started
     * @return
     * - elapsed time in milliseconds
     */
    virtual MUINT32 getElapsedTime() = 0;


};



}; // DualCamZoom
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

#endif