/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef _MTK_HARDWARE_PIPELINE_EXTENSION_MFNRCONTROLLER_V1_6_H_
#define _MTK_HARDWARE_PIPELINE_EXTENSION_MFNRCONTROLLER_V1_6_H_

#include "MFNRCtrler.h"

// MFNR Core Lib
#include <mtkcam/feature/mfnr/IMfllCore.h>
#include <mtkcam/feature/mfnr/IMfllEventListener.h>

// MTKCAM
#include <mtkcam/utils/std/StlUtils.h> // NSCam::SpinLock

// STL
#include <future> // std::future
#include <map> // std::map

// MFNR early postview is a mechanism to generate postview much earlier,
// we use the first time of stage BFBLD to generate postview. Some chips may
// not support encoding RAW to 3 outputs, hence if early postview has problem,
// disable this flag.
#define MFLL_FLOW_EARLY_POSTVIEW    1

using namespace android;
using namespace mfll;
using namespace NSCam;
using namespace NSCam::plugin;

class MFNRCtrler_v1_6 : public MFNRCtrler
{
//
// Override from MFNRCtrler
//
public:
    virtual void                updateMfllStrategy() override;
    virtual bool                initMfnrCore() override;
    virtual bool                doMfnr() override;
    virtual void                doCancel() override;


//
// Protected Methods
//
protected:
    // After retrieved a RequestFrame, this method will be invoked to add
    // imagebuffers, information that MFNR core library needs.
    //  @param sourceImg        Source image, full size one.
    //  @param mv               Motion vector (GMV).
    //  @param pAppMeta         App metadata from P1 node.
    //  @param pHalMeta         Hal metadata from P1 node.
    //  @param pLcsoBuffer      Lcso raw buffer.
    //  @param pRrzoBuffer      Rrzo raw buffer.
    virtual void                addDataToMfnrCore(
                                    sp<IMfllImageBuffer>&   sourceImg,
                                    MfllMotionVector        mv,
                                    IMetadata*              pAppMeta,
                                    IMetadata*              pHalMeta,
                                    IImageBuffer*           pLcsoBuffer,
                                    IImageBuffer*           pRrzoBuffer
                                    );

    // If there's any error frames, this method will be invoked to add the
    // dummy frames(fake frames, null frame) to MFNR core library. The aim is
    // to make sure MFNR core library can finish his job.
    //  @param dummyFrame       A NULL pointer of IMfllImageBuffer.
    virtual void                addDummyFrameToMfnrCore(
                                    sp<IMfllImageBuffer>&   dummyFrame
                                    );

    // To calculate motion vector.
    //  @param pHalMeta         GMV(Global Motion Vector) information is generated
    //                          from EIS, and the information is stored in HAL
    //                          metadata.
    //  @param index            The index of the index-th GMV.
    //  @return                 Motion vector, unit is pixel, in source image domain.
    virtual MfllMotionVector    calMotionVector(
                                    const IMetadata* pHalMeta,
                                    size_t index
                                    );

    // Release source images, class RequestFrame supports partial release, to
    // release unnecessary source image ASAP for saving resource problem.
    // Furthermore, release all extra output images too, and marks as error, which
    // is to avoid the frames are sent to preview pipeline.
    //  @param index            The i-th RequestFrame to be released.
    //  @note                   The extra output frames is going to be released
    //                          too and with error code.
    virtual void                releaseSourceImages(int index);

    // Process output YUV, after MFNR finished, this method will be invoked to
    // handle the remains jobs, such as debug exif appending, to generate output
    // YUV and thumbnail YUV ... etc.
    virtual void                processMixedYuv();

    // Clean up resource, such MfllCore ... etc.
    virtual void                cleanUp();

    // Check if we need to enable early postview mechanism
    virtual bool                isEnabledEarlyPostview() const;

//
// MFNR event callbacks
//
// To use these virtual functions to implement on and done event from MFNR core
// library, lets this implementation be more flexible.
//
public:

    virtual void onEvent(enum EventType t, MfllEventStatus_t &status, void *mfllCore, void *param1 = NULL, void *param2 = NULL);
    virtual void doneEvent(enum EventType t, MfllEventStatus_t &status, void *mfllCore, void *param1 = NULL, void *param2 = NULL);


//
// Get & Set
//
public:
    // Get dropped frame number.
    //  @return                 Frame dropped amount.
    inline int  getDroppedFrameNum() const { return m_droppedFrameNum; }


//
// Attributes
//
protected:
    // represents frame dropped number
    int m_droppedFrameNum;

    // represents if need debug exif
    int m_bNeedExifInfo;

    // represents if MFNR has been fired or not.
    bool m_bFired;

    // if request effect
    int m_effectMode;

    // queue the RequestFrame dequed from MFNRCtrler
    std::deque< std::shared_ptr<RequestFrame> > m_vUsedRequestFrame;
    NSCam::SpinLock                             m_vUsedRequestFrameMx;

    // saves main request frame
    std::weak_ptr<RequestFrame>                 m_mainRequestFrame;

    // MFNR Core library
    sp<IMfllCore>           m_pCore;
    std::mutex              m_pCoreMx; // protect m_pCore
    sp<IMfllCapturer>       m_pCapturer;
    sp<IMfllMfb>            m_pMfb;
    sp<IMfllNvram>          m_spNvramProvider;

    struct __PostViewReadyPack__ {
        std::mutex                              locker;
        std::condition_variable                 cond;
        bool                                    isReady;
        __PostViewReadyPack__() : isReady(false) {};
        ~__PostViewReadyPack__() = default;
    } m_postViewReadyPack;


    // <isIgnored, Status>
    std::map<mfll::EventType, std::pair<bool, mfll::MfllErr> >
        m_mfnrStageStatus;
    std::mutex
        m_mfnrStageStatusLock;


//
// Constructor & destructor
//
public:
    MFNRCtrler_v1_6(
            int sensorId,
            MfllMode mfllMode,
            MUINT32 sensorMode,
            int realIso = 0,
            int exposureTime = 0,
            bool isFalshOn = false
    );
    virtual ~MFNRCtrler_v1_6();
}; // class MFNRCtrler_v1_6
#endif//_MTK_HARDWARE_PIPELINE_EXTENSION_MFNRCONTROLLER_V1_6_H_
