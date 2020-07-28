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
 * MediaTek Inc. (C) 2013. All rights reserved.
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

#ifndef _IMS_MA_H_
#define _IMS_MA_H_

#include <sys/types.h>
#include <RTPBase.h>
#include <utils/Log.h>
#include <utils/Errors.h>
#include <binder/IMemory.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AHandler.h>
#include <gui/Surface.h>

#include "IVcodecCap.h"
#include "Sink.h"

#include <gui/BufferQueue.h>
#include <gui/IGraphicBufferConsumer.h>
#include <gui/IGraphicBufferProducer.h>

#define PARAMETER_SIZE_MAX    256
#define RTCP_PROFILE_PARAMETER_SIZE    32
#define ERROR_BIND_PORT    (-1000)


namespace android
{
struct AMessage;
/**
 *@ IMS Media Adaptor datapath: source, sink, they are independent path
 *@ MA_SOURCE: Media Adaptor send media data(video, audio) to upper layer
 *@ MA_SINK: Media Adaptor get media data(video, audio) from upper layer
 *@ MA_SOURCE_SINK: Media Adaptor send/get media data to/from upper layer
 */
enum ma_datapath_t {
    MA_SOURCE      = 0x1,
    MA_SINK        = 0x2,
    MA_SOURCE_SINK = MA_SOURCE | MA_SINK
};

enum imsma_extra_bitControl_t{
        MA_EXT_BIT_CTRL_NONE = 0,
        MA_STOP_RX_PUSH_BLANK = 0x01,//will add by bit control
        MA_EXT_BIT_CTRL_MAX = MA_STOP_RX_PUSH_BLANK,
};

//for turn off video
enum imsma_turn_off_video_mode_t {
    MA_TURN_OFF_VIDEO_DISABLE,
    MA_TURN_OFF_VIDEO_ENABLE,
};

enum imsma_turn_off_video_direction_t {
    MA_TURN_OFF_VIDEO_BY_LOCAL,// local  turn off video
    MA_TURN_OFF_VIDEO_BY_PEER,//turn off video  by peer
};

struct imsma_turn_off_video_params_t {
    imsma_turn_off_video_direction_t direction;
    //maybe other params, not sure
};

//hold


enum imsma_hold_mode_t {
    MA_HOLD_DISABLE,
    MA_HOLD_ENABLE,
};

enum imsma_hold_direction_t {
    MA_HOLD_BY_LOCAL,// local  hold call
    MA_HOLD_BY_PEER,//hold call  by peer
};

struct imsma_hold_params_t {
    imsma_hold_direction_t direction;
    //maybe other params, not sure
};

enum imsma_pause_resume_mode_t {
    MA_PAUSE_RESUME_NORMAL,//normal pause-resume
    MA_PAUSE_RESUME_HOLD,//pause-resume for hold
    MA_PAUSE_RESUME_TURN_OFF_VIDEO,//pause-resume for turn_off_video
};

struct imsma_pause_resume_params_t {
    imsma_pause_resume_mode_t mode;
    union {
        imsma_turn_off_video_params_t turnOffVideo;
        imsma_hold_params_t  hold;
        //why use union :flexible to add new params
    };
    // add for pause RX ,if bit&MA_STOP_RX_PUSH_BLANK = 0x01,pause RX then pushblank
    // bit for future expanding usage
    // only normal pause-resume is in use now, so only add for normal pause resume

    int32_t normal_pause_resume_extra_bitControl;
};


/**
 *@ Record Mode
 *@ MA_RECMODE_VIDEO_ONLY: Just record downlink video
 *@ MA_RECMODE_VIDEO_AUDIO: Record downlink video, uplink audio mix downlink audio
 */
/*
enum record_mode_t {
    MA_RECMODE_VIDEO_ONLY,
    MA_RECMODE_VIDEO_AUDIO
};
*/

/**
 *@ Snapshot mode
 *@ MA_SNAPSHOT_PEER: Snapshot just for peer side video
 *@ MA_SNAPSHOT_LOCAL: Snapshot just for local side video
 *@ MA_SNAPSHOT_PEER_LOCAL: Snapshot for peer and local side video
 */
enum snapshot_mode_t {
    MA_SNAPSHOT_PEER       = 0x1,
    MA_SNAPSHOT_LOCAL      = 0x2,
    MA_SNAPSHOT_PEER_LOCAL = MA_SNAPSHOT_PEER|MA_SNAPSHOT_LOCAL,
};



/**
 *@ Record Quality
 *@ MA_RECQUALITY_LOW:   TBD
 *@ MA_RECQUALITY_MEDIUM:  TBD
 *@ MA_RECQUALITY_HIGH: TBD
 */
enum record_quality_t {
    MA_RECQUALITY_LOW,
    MA_RECQUALITY_MEDIUM,
    MA_RECQUALITY_HIGH,
};


/**
 *@ kWhatImsMANotify: Indicate message notification is from ImsMA
 */
enum message_type {
    kWhatImsMANotify,
};

/**
 *@ Callback message from MA to VT Service
 *@ kWhatMACodecError: MA notify VT Service about MA codec error
 *@ kWhatMACameraError: MA notify VT Service about camera error
 *@ kWhatMASnapShotDone: MA notify VT Service about snapshot done
 *@ kWhatMARecordNotify: MA Record information notify to VT Service
 */
enum message_info {
    kWhatMACodecError,
    kWhatMASnapShotDone,
    kWhatMARecordNotify,
    kWhatPeerResolutionDegree,
    kWhatLocalResolutionDegree,
    kWhatLocalError_DropBitrate_Fail, //source bitrate can not drop down, should switch to volte
    kWhatPeerError,
    kWhatUpdateMbrDl,
    kWhatUpdateDebugInfo,
    kWhatCoverPicture,  //for Vzw, when vedio can't be successfully sustained, we should cover contact picture on UI
    kWhatPeerDisplayStatus,
    kWhatLocalRestartCamera,

};


/**
 *@ Callback message of record details from MA to VT Service
 *@ MA_REC_INFO_UNKNOWN: Record error type unknown
 *@ MA_REC_INFO_REACH_MAX_DURATION: Record reach the max duration
 *@ MA_REC_INFO_REACH_MAX_FILESIZE: Record reach the max file size
 *@ MA_REC_INFO_NO_I_FRAME: Record with no video I frame
 *@ MA_REC_INFO_COMPLETE: Indicate record is complete
 */
enum ma_rec_info {
    MA_REC_INFO_UNKNOWN,
    MA_REC_INFO_REACH_MAX_DURATION,
    MA_REC_INFO_REACH_MAX_FILESIZE,
    MA_REC_INFO_NO_I_FRAME,
    MA_REC_INFO_COMPLETE,
};


enum VT_Quality {
    QUALITY_DEFAULT = 0,
    QUALITY_FINE = (1 << 0),
    QUALITY_HIGH = (1 << 1),
    QUALITY_MEDIUM = (1 << 2),
    QUALITY_LOW = (1 << 3),
    QUALITY_END = VIDEO_QUALITY_LOW,
    QUALITY_MAX = 0xFFFFFFFF
};

enum ImsMa_Ratio {
    ImsMa_W_H_Ratio_NotSure = -1,
    ImsMa_W_H_Ratio_Horizontal = 0,
    ImsMa_W_H_Ratio_Vertical =1,
};

/**
 *@ Media stream configuration
 */
typedef struct media_config {
    rtp_rtcp_config_t rtp_rtcp_cap; //rtp
    video_codec_fmtp_t codec_param; //H264 format parameter information
} media_config_t;

typedef struct  sensor_info_vilte {
    uint32_t index;
    uint32_t max_width;
    uint32_t max_height;
    uint32_t degree;
    uint32_t facing;
    uint32_t hal;
} sensor_info_vilte_t;


class ImsMa: public RefBase
{
public:
    ImsMa(uint32_t simID = 0,uint32_t operatorID = 0);
    virtual ~ImsMa();
    /**
     *@ Description: IMS MediaAdaptor module init, it will also initialize related modules of below layer
     *@     init module maybe include:
     *@     common: v2olte, mediacontent, mediasession
     *@     MA_SOURCE: mediasender, mediasource, rtpsender
     *@     MA_SINK: ImsMediaReceiver, mediasink, rtpreceiver
     *@ Parameters:
     *@     type: init which media adaptor data path
     *@ Return:
     *@     status_t type, OK indicate successful, otherwise error type will return
     */
    status_t Init(ma_datapath_t type);

    /*
     *@ Description: IMS MediaAdaptor module start, it will also start related modules of below layer
     *@ Parameters:
     *@     type: start which media adaptor data path
     *@ Return:
     *@     status_t type, OK indicate successful, otherwise error type will return
     */
    status_t Start(ma_datapath_t type);

    /**
     *@ Description: IMS MediaAdaptor module stop, it will also stop related modules of below layer
     *@ Parameters:
     *@     type: stop which media adaptor data path
     *@  bitCtrlCmd |= MA_STOP_RX_PUSH_BLANK if want to push blank
     *@ Return:
     *@     status_t type, OK indicate successful, otherwise error type will return
     */
    status_t Stop(ma_datapath_t type,int32_t bitCtrlCmd = MA_STOP_RX_PUSH_BLANK);

    /**
     *@ Description:
     *@ Parameters:
     *@     type: pause which media adaptor data path
     *@ params: special params for hold or turnOffVideo
     *@ Return:
     *@     status_t type, OK indicate successful, otherwise error type will return
     */
    status_t Pause(ma_datapath_t type,imsma_pause_resume_params_t* params =NULL);

    /**
     *@ Description:
     *@ Parameters:
     *@     type: resume which media adaptor data path
     *@ Return:
     *@     status_t type, OK indicate successful, otherwise error type will return
     */
    status_t Resume(ma_datapath_t type,imsma_pause_resume_params_t* params=NULL);

    /**
     *@ Description: IMS MediaAdaptor module reset, it will also reset related modules of below layer
     *@     if want set new configuration to MA, need call reset function
     *@ Parameters:
     *@     type: reset which media adaptor data path
     *@ Return:
     *@     status_t type, OK indicate successful, otherwise error type will return
     *@ should reset add bitCtrlCmd ??
     */
    status_t Reset(ma_datapath_t type,int32_t bitCtrlCmd = MA_STOP_RX_PUSH_BLANK);



    /**
     *@ Description: sensor_resolution_t is not fit for HAL3 change, for the HW ratio
         *@     element need to be calculated based on specific logic, camera module is thought to be common
         *@     one ,just want to be a provider.
     *@ Parameters:
     *@     sensor_resolution_vilte has a new sensor degree, it is used for HW ratio.it is a new structure
         *@     sensorCnt is the number of all sensor
     *@ Return:
     *@     NA
         */
    static status_t setSensorParameters(sensor_info_vilte *sensor, int32_t sensorCnt);



    /**
        tell codec the save power mode to condig EGL to drop fps
    **/
    static void setLowPowerMode(bool enable);

    /**
     *@ Description: This is a static function for upper layer get decoder Param
     *@ Parameters:
     *@     vdec_cap_t: return profile_id and h264 level of decoder
     *@ Return:
     *@     status_t type, OK indicate successful, otherwise error type will return
     */


    static int32_t getCodecCapability(video_codec_fmtp_t **codeccap,int32_t *prefer_WHRatio = NULL,uint32_t operatorID = 0, VT_Quality quality = QUALITY_DEFAULT);

    /**
     *@ Description: This is a static function for upper layer get codec bitrate table
     *@ Parameters:
     *@     pcount: total array size
     *@ pvideo_media_bitrate: each format bitrate table for diff profile / level
     *@
     */
    static void getCodecBitrateTable(uint32_t *tableCnt,
                                     video_media_bitrate_t **bitrateTable,uint32_t operatorID = 0);

    /*get codec sps by format*/
    static void  getCodecParameterSets(
        video_format_t informat,  //in
        uint32_t *levelCapNumbers,  //out
        video_codec_level_fmtp_t **codeclevelcap,uint32_t operatorID = 0, VT_Quality quality = QUALITY_DEFAULT);

    /**
     *@ Description: This is a static function for upper layer get RTP/RTCP capability, before instalization
     *@ Parameters:
     *@     h264_enc_cap_t: return profile_id and h264 level of encoder
     *@ Return:
     *@     status_t type, OK indicate successful, otherwise error type will return
     */

    static int32_t getRtpRtcpCapability(rtp_rtcp_capability_t** pRtpRtcpCap, uint32_t operatorID = 0);

    /**
     *@ Description: Config media stream for MA initialization
     *@ Parameters:
     *@     config: media stream config, include rtp, rtcp, h264 configuration
     *@ Return:
     *@     status_t type, OK indicate successful, otherwise error type will return
     */
    status_t InitMediaConfig(media_config_t *config);

    /**
     *@ Description: Config media stream for MA initialization
     *@ Parameters:
     *@     config: media stream config, include rtp, rtcp, h264 configuration
     *@ Return:
     *@     status_t type, OK indicate successful, otherwise error type will return
     */
    status_t UpdateMediaConfig(media_config_t *config);

    /**
     *@ Description: Set surface of peer side for MA
     *@ Parameters:
     *@     peer_surface: surface for peer side
     *@ Return:
     *@     status_t type, OK indicate successful, otherwise error type will return
     */
    status_t SetPeerSurface(const sp<Surface>& peer_surface);

    /**
     *@ Description: Set notification message to MA==>only notify to VTservice

     *@     status_t type, OK indicate successful, otherwise error type will return
     */
    status_t SetNotify(const sp<AMessage> &msg);


    /**
    *@ Description: Google demo VT Code show change quality interface and value. Currently, IMS MA will do nothing
    *@ Parameters:
    *@  VT_Quality:  high, middle, low, default selection
    *@ Return:
    *@  status_t type, OK indicate successful, otherwise error type will return
    */
    status_t SetVTQuality(VT_Quality quality);

    /**
    *@ Description: In Camera HAL3, bufferqueue is the YUV data patch from App to ImsMa source
           *@              ImsMa source create BQ and pass the producer handler to App, source get the
           *@              consumer client to receive
     *@ Parameters:
     *@   outBufferProducer: output Bufferqueue producer
     *@ Return:
     *@   status_t type, OK indicate successful, otherwise error type will return
     */
    status_t getBufferQueueProducer(sp<IGraphicBufferProducer>* outBufferProducer);

    /*
    *@ Description: set  current camera facing for CVO and  degree for rotate
    *@ Parameters:facing 1 font,0 back; degree 0,90,180,270
    *@
    */
    status_t setCurrentCameraId(uint32_t index);


    /**

    // the following interfaces are special feature related

    **/

    /**
     *@ Description: Indicate MA do snapshot
     *@ Parameters:
     *@     url: url path for snapshot picture save to
     *@     mode:   snapshot peer, local, or peer + local side pictrue
     *@ Return:
     *@     status_t type, OK indicate successful, otherwise error type will return
     */
    status_t SnapShot(const char* url, snapshot_mode_t mode);

    /**
     *@ Description: Indicate MA do snapshot
     *@ Parameters:
     *@     buffer: snapshot picture save to this buffer
     *@   mode: snapshot peer, local, or peer + local side pictrue
     *@ Return:
     *@     status_t type, OK indicate successful, otherwise error type will return
     */
    status_t SnapShot(sp<IMemory>& buffer, snapshot_mode_t mode);


    /**
     *@ Description: Set record parameters
     *@ Parameters:
     *@     quality: low, medium, high will use different video/audio codecs
     *@     file_name: record save file path and name
     *@ Return:
     *@     status_t type, OK indicate successful, otherwise error type will return
     */
    status_t SetRecordParameters(record_quality_t quality,  char* file_name);

    /**
     *@ Description: Indicate MA to start record
     *@ Parameters:
     *@     mode: record video only, or video and downlink audio plus uplink audio
     *@ Return:
     *@     status_t type, OK indicate successful, otherwise error type will return
     */
    status_t StartRecord(record_mode_t mode);

    /**
     *@ Description: Indicate MA stop recording
     *@ Parameters:
     *@     None
     *@ Return:
     *@     status_t type, OK indicate successful, otherwise error type will return
     */
    status_t StopRecord(void);


    /**
    *@ Description: VT query the local turnOffVideo status of MA
    *@ Parameters:
    *@ Return:
    *@  imsma_turn_off_video_mode_t : currenly only MA_TURN_OFF_VIDEO_ENABLE, MA_TURN_OFF_VIDEO_DISABLE is used
    **/
    imsma_turn_off_video_mode_t GetTurnOffVideoByLocalState();


    /**
    *@ Description: VT query the peer turn off  status of MA
    *@ Parameters:
    *@ Return:
    *@  imsma_turn_off_video_mode_t : currenly only MA_TURN_OFF_VIDEO_ENABLE, MA_TURN_OFF_VIDEO_DISABLE is used
    **/
    imsma_turn_off_video_mode_t GetTurnOffVideoByPeerState();



    /*
    *@ Description: Change sensor degree
    *@ Parameters:
    *@    degree:
    *@ Return:
    *@
    */

    status_t setDeviceRotationDegree(int32_t degree);


    /*
    *@ Description: set handover state for disable downgrade check
    *@ Parameters:
    *@    state: true-start handover false-stop handover
    *@ Return:
    *@
    */
    status_t setHandoverState(bool state);

private:
    enum {
        create           = 0x01,
        downlink_init    = 0x02,
        uplink_init      = 0x04,

        downlink_started = 0x08,
        uplink_started   = 0x10,

        downlink_stopped = 0x20,
        uplink_stopped   = 0x40,

        reseted          = 0x80,

        notifyset        = 0x100,
        peerSurfaceSet   = 0x200,
        localSurfaceSet  = 0x400,
        initParamSet     = 0x800,

        downlink_paused  = 0x1000,
        uplink_paused     = 0x2000,

        downlink_hold  = 0x4000,//by local
        uplink_hold      = 0x8000,  //by local

        downlink_turn_off_video  = 0x10000, // by peer
        uplink_turn_off_video       = 0x20000,     //by local

        uplink_held      = 0x40000,  //by peer hold
    };



    enum FlagMode {
        SET,
        CLEAR,
        ASSIGN
    };

    uint32_t mSimID;
    uint32_t mOperatorID;
    uint32_t mFlags;

    struct MediaSession;
    sp<MediaSession> mMediaSession;
    sp<AMessage> mHandleMsg; // Msg to MediaSession
    const sp<IGraphicBufferProducer> mLocalSurface;
    const sp<IGraphicBufferProducer> mPeerSurface;
    sp<ALooper> mMsLooper;
    mutable Mutex mLock;
    int32_t mQuality;


    //params

    media_config_t mInitMediaConfig;
    rtp_rtcp_config_t mInitRtpRtcpCap;
    video_codec_fmtp_t mInitCodecCap;
    static video_codec_fmtp_t*  mGetCodecCapPtr  ;//declare
    static video_media_bitrate_t* mGetCodecBitrateTablePtr ;
    static video_codec_level_fmtp_t* mGetVideoCodecLevelFmtpPtr;
    //static sensor_resolution_vilte* mSensorParameterPtr;
    static sensor_resolution_t* mSensorResolution;
//Mark off for HAL3
//    String8 g_CameraparamGet;
//  String8 g_CameraparamSet;

    //for debug, to remeber the instance ID
    static int32_t sMultiInstanceID;
    int32_t mMultiInstanceID;
    static int32_t mSensorCnt;
    int32_t mRTPfd;
    int32_t mRTCPfd;

    status_t CheckSetBR(media_config_t *config);
    status_t makeMyselfSocket(rtp_rtcp_config_t* config);
    int bindSock(uint8_t type, uint16_t port, uint8_t *address);
    int ConnectSock(int fd, uint8_t type, uint16_t port, uint8_t *address);
    void printIPv4(const char* name, uint8_t* ip);
    void printIPv6(const char* name, uint8_t* ip);
    void PreConnect(int fd, char *ifname, uint32_t network_id);

    void modifyFlags(unsigned value, FlagMode mode);
    status_t PostAndAwaitResponse(sp<AMessage> *response);
    status_t SetTurnOffVideo_l(ma_datapath_t type,imsma_turn_off_video_mode_t mode,imsma_turn_off_video_direction_t direction);
    status_t SetHold_l(ma_datapath_t type,imsma_hold_mode_t mode,imsma_hold_direction_t direction);
    //for mormal pause and resume
    status_t Pause_l(ma_datapath_t type,int32_t bitCtrlCmd = 0);
    status_t Resume_l(ma_datapath_t type,int32_t bitCtrlCmd = 0);
    status_t Stop_l(ma_datapath_t type,int32_t bitCtrlCmd = 0);




};
}
#endif //_IMS_MEDIA_ADAPTOR_H_
