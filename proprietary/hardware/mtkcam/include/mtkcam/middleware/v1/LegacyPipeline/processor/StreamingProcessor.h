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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_STREAMINGPROCESSOR_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_STREAMINGPROCESSOR_H_
//
#include <utils/RefBase.h>
#include <utils/Vector.h>
#include "ResultProcessor.h"
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/request/RequestSettingBuilder.h>

using namespace android;
//
/******************************************************************************
 *
 ******************************************************************************/
//
namespace NSCam {
namespace v1 {


class INotifyCallback
    : public virtual RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual                         ~INotifyCallback() {};

    virtual void                    doNotifyCallback(
                                        int32_t _msgType,
                                        int32_t _ext1,
                                        int32_t _ext2
                                    )                                       = 0;

    virtual void                    doDataCallback(
                                        int32_t  _msgType,
                                        void*    _data,
                                        uint32_t _size
                                    )                                       = 0;

    virtual void                    doExtCallback(
                                        int32_t _msgType,
                                        int32_t _ext1,
                                        int32_t _ext2
        )                                       = 0;


    virtual bool                    msgTypeEnabled( int32_t msgType )       = 0;
};


class StreamingProcessor
    : public virtual RefBase
    , public ResultProcessor::IListener
    , public ResultProcessor::IDataListener
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual                                ~StreamingProcessor() {};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    /**
     *
     * Auto focus.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual status_t                        startAutoFocus()                       = 0;

    /**
     *
     * Cancel auto focus.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual status_t                        cancelAutoFocus()                      = 0;

    /**
     *
     * Do pre-capture. Blocking function.
     *
     * @return
     *      0 indicates success; otherwise failure.
     *
     */
    virtual status_t                        preCapture(
                                                int& flashRequired,
                                                nsecs_t tTimeout
                                            )                                      = 0;

    /**
     *
     * Start smooth zoom.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual status_t                        startSmoothZoom(int value)             = 0;

    /**
     *
     * Stop smooth zoom.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual status_t                        stopSmoothZoom()                       = 0;

public:

    virtual status_t                        sendCommand(
                                                int32_t cmd,
                                                int32_t arg1, int32_t arg2
                                            )                                      = 0;

    virtual void                            dump()                                 = 0;

public:
    // for sendCommand
    enum eStreamingProcessorExtCmd
    {
        eSP_extCmd_define_start = 0x50000000,
        eSP_extCmd_resume_focus,
        eSP_extCmd_pause_focus,
        eSP_extCmd_burst_num,
    };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static sp< StreamingProcessor >     createInstance(
                                            MINT32                            openId,
                                            sp<INotifyCallback>         const &rpCamMsgCbInfo,
                                            wp< RequestSettingBuilder > const &rpRequestSettingBuilder,
                                            sp< IParamsManagerV3 >      const &rpParamsManagerV3,
                                            MINT32                            aStartRequestNumber,
                                            MINT32                            aEndRequestNumber
                                        );

};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v1
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_STREAMINGPROCESSOR_H_

