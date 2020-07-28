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


#ifndef _VILTE_CAMERA_SOURCE_H_
#define _VILTE_CAMERA_SOURCE_H_


#include <ui/GraphicBuffer.h>
#include <gui/Surface.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MetaData.h>
#include <media/MediaSource.h>
#include <utils/String16.h>
#include <media/mediarecorder.h>
#include "EncoderSource.h"
#include "VTCameraSource.h"
/*
*1) video and audio NTP should use the audio video buffer generate time(calc from device reboot)
*2) video NTP is using the video timestamp,so video timestamp should use the camera generated timestamp directly
*    2.1) not set camerasource mStartTimeUs in start(meta)
*    2.2) set vilte mode to camerasource to use camera generated timestamp directly
*3)
*
*
*/


namespace android
{
using android::status_t;


class MediaProfiles;
struct ALooper;


typedef struct video_enc_fmt_t {
    int32_t maxBitrate;
    int32_t bitrate;
    int32_t framerate;
    const char* mimetype;
    int32_t width;
    int32_t height;
    int32_t sarWidth;
    int32_t sarHeight;
    int32_t IFrameIntervalSec;
    int32_t profile;
    int32_t level;
    int32_t widthAdapt;
    int32_t heightAdapt;
    int32_t interfaceType;
} video_enc_fmt;


struct Source : public AHandler {
public:

    //for uplayer interact
    enum {
        kWhatError = 0,
        kWhatResolutionNotify = 1,
        kWhatAccessUnit = 2,
    };

    enum {
        Error_Bitrate_Drop_Down_Fail = 1,
	  Error_Camera_Restart = 2,
    };



    //for top level flow control
    Source(int32_t multiId,uint32_t simID,uint32_t operatorID);
    void     SetNotify(const sp<AMessage> &notify);    //first call after new Source
    status_t Start();
    status_t Stop();
    //for turn off video
    status_t Pause();
    status_t Resume();
    status_t setSourceConfig(video_enc_fmt * settings);


    //for cvo and rotate support
    status_t setDeviceRoateDegree(int32_t degree);

    status_t setCurrentCameraInfo(int32_t facing, int32_t degree,int32_t hal);
    status_t disableCVO(bool disable);

    //for avpf
    status_t setAvpfParamters(const sp<AMessage> &params);
    //tmmbr ajust bitrate
    status_t adjustEncBitRate(int32_t expect_bitrate);

    status_t getBufferQueueProducer(sp<IGraphicBufferProducer>* outBufferProducer);

protected:
    virtual ~Source();
    virtual void onMessageReceived(const sp<AMessage> &msg);

private:
    enum  {
        IDLE                = 0x01,
        INITED              = 0x02,
        STARTED         = 0x04 ,
        PAUSED              = 0x08,        //pause encoder for turn off video
        STOPPED             = 0x10,     //stop
        UPDATING              = 0x20, //update config
    };
    enum  {
        Update_Fps              = 0x01,//only some parameter changes:bitrate,fps...
        Update_Bitrate          = 0x02 ,//need reconfig camera preview : W,H change
        //whether we should change the preview size during video call?
        Update_IFrameInterval   = 0x04,
        Update_CodecType        = 0x08,
        Update_ProfileLevel     = 0x10,

        Update_Resolution_Up     = 0x100 ,//need reconfig encoder,W H
        Update_Resolution_Down  =0x200 ,
        Update_Resolution              = Update_Resolution_Up|Update_Resolution_Down,
        Update_RotationDegree   = 0x400,
    };


    enum {
        //internal use
        kWhatStart  = 10,
        kWhatStop   ,
        kWhatPause  ,
        kWhatResume  ,
        kWhatInitUpdate,

        kWhatSetAvpfParams,
        kWhatAdjustEncBitrate,

        kWhatSourceNotify,
        kWhatCodecAvpfFeedBack ,
        kWhatClearBufferQueue,
    };

    enum ROT_TARGET {
        ROTATE_NONE = 0,
        ROTATE_CAMERA = 0x01,
        ROTATE_DEVICE = 0x02,
        ROTATE_ALL = ROTATE_CAMERA | ROTATE_DEVICE,
    };

    /*
        0: Front-facing camera, facing the user. If camera direction is unknown then this value is used.
        1: Back-facing camera, facing away from the user.

    *///opposite to hal1
    enum {
        CAMERA_FACING_BACK = 1,
        CAMERA_FACING_FRONT = 0,
    };

    //for debug multi instance
    int32_t mMultiInstanceID;
    uint32_t mSimID;
    uint32_t mOperatorID;
    int32_t  mState;
    sp<AMessage> mNotify;
    Mutex mLock;

    //basic

    sp<EncoderSource> mEncoderSource;
    sp<VTCameraSource> mCameraSource;
    int32_t mChooseCameraFacing;
    int32_t mChooseCameraDegree;//for encoder
    int32_t mChooseCameraHal;
    int32_t mDeviceDegree;//for record device dree
    int32_t mRotateTarget;  //for encoder
    int32_t mRotateDegree;//for encoder
    int32_t mRotateMethod;//for encoder

    //encoder
    video_enc_fmt_t mVideoConfigSettings;
    int32_t mColorFormat;
    sp<AMessage> mEncoderOutputFormat;

    //owner looper
    sp<ALooper> mLooper;
    sp<ALooper> mEncoderLooper;

    enum Resolution_Change_Algorithm {
        Encode_DYNAMIC_CHANGE = 1,//send follow the sensor setting
        Encode_CAMERA_RESET     = 2,
    };
    int32_t mResChangeAlgorithm;  //for mResChangeAlgorithm
    //for resolution change

    bool mDisbaleResolutionChange;
    int32_t mResolutionChangeIntervalS;
    int64_t mResolutionChangeTimeUs;
    video_encoder mVideoEncoder;

    static int32_t gInstanceCount   ;

    status_t initLooper_l();//internal initialize
    status_t stop_l();
    status_t setupVideoEncoder(sp<MediaSource> cameraSource);
    status_t  postAsynAndWaitReturnError(const sp<AMessage> &msg);

    //for update
    status_t checkSaveSourceConfig(video_enc_fmt * settings);
    int32_t compareSettingChange(video_enc_fmt * settings);
    //for app
    //native handle the preview,not need notify app the preview displayer orientation degree
    void notifyResolutionDegree(int32_t width,int32_t height,int32_t degree = 0);
    status_t startEncoderSource_l();
    status_t stopEncoderSource_l();
    //notify error
    void notifyError(int32_t errorType);
    void refineWhByRotationDegree(int32_t* width, int32_t* height,int32_t degree);
    status_t setSourceResolutionDegree();
    status_t  resetRotateDegree();
    //for change bitrate
    /*
    1) return changeType , let resetByUpdateConfig to handle change logic
    */
    int32_t evaluateBitrateAdaptive(int32_t expBitrate);
    status_t resetByUpdateConfig(int32_t changeType);
    status_t setCodecParameters(int32_t changeType);
    status_t adjustEncBitRate_l(int32_t expect_bitrate);
    //for signal
    int32_t  adjustEncBySignal();
    int32_t  mBeginSignal;
};
}
#endif  // _VILTE_CAMERA_SOURCE_H_

