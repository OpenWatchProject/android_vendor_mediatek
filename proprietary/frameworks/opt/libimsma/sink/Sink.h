
#ifndef _SINK_H_

#define _SINK_H_


#include <gui/Surface.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/MediaDefs.h>
#include <media/mediarecorder.h>

#include "Recorder.h"
namespace android
{

struct AMessage;
struct Renderer;
struct Recorder;


struct video_dec_fmt {
    int32_t bitrate;
    int32_t framerate;
    int32_t width;
    int32_t height;
    int32_t sarWidth;
    int32_t sarHeight;
    int32_t profile;
    int32_t level;
    sp<ABuffer> csd;
    const char* mimetype;
} ;
struct audio_dec_fmt {
    int32_t sampleRate;
    int32_t channelCount;
    int32_t bitWidth;
    sp<ABuffer> csd;
    const char* mimetype;
} ;
enum record_mode_t {
    RECORD_DLVIDEO_MIXADUIO,  //dl video + VTservice sent mix audio, now use this
    RECORD_DLVIDEO_DLAUDIO_ULAUDIO, //dl video + ul audio + dl audio ,3 path data
    RECORD_MIXAUDIO_ONLY,   //VTservice sent mix audio
    RECORD_DLVIDEO_ONLY,     //dl video
};
/**
 *@ record_config: recorder related configuration parameter
 *@ ve: video encoder parameters  if need , if record mode is RECORD_MIXAUDIO_ONLY,set =NULL
 *@ ve: audio encoder parameters  if need
 *@ parameters: other record parameters which are not codec related ,such as rotation/max-length/duration...
 */
struct record_config {
    bool useFd;     // whether use fd as the wirter Fd
    const char *path;// dir path from app, is useFd= true, this path should be NULL
    int fd;  //fd
    int64_t offset;  //offset with fd,
    int64_t length ; //length with fd,
    record_mode_t mode;//tell me the source
    output_format outf; //output file format    -->in mediaRecord.h
    audio_encoder ae;  //audio encoder   -->in mediaRecord.h, aac or amr, now should be amr
    video_encoder ve;//video encoder   -->in mediaRecord.h-->should be H264
    sp<AMessage> params;//"latitude","longtitude","degree";"max-size";"max-duration"... 'channel-count' 'audio-bitrate' 'video-bitrate' 'sample-rate'
};


/**
 *@ Sink: sink class,interface to IMS MA, render and record tracks data
 */
struct Sink : public AHandler {

    enum Flags {
        FLAG_SPECIAL_MODE = 1,//modem handle the audio display
        FLAG_NORMAL_MODE = 2,//modem is not handle the audio path
    };

    // to indicate data content
    enum TrackIndex {
        AUDIO_UL=0x01,
        AUDIO_DL=0x02,
        VIDEO_DL=0x04,
        VIDEO_UL=0x08,
        AUDIO_DL_UL_MIX = AUDIO_DL | AUDIO_UL,//for record
    };

    /*@  msg post to  IMS MA by sink/render/record*/

    enum {
        //basic
        kWhatError = 0,
        kWhatKeyFrameRequest ,
        kWhatResolutionNotify ,

        //record feedback info
        /* 1)kWhatRecordInfo,feedback record infor to MA,msg will return with 1 int32_t value:
        "info" -->tell the info value
        */
        kWhatRecordInfo,
        kWhatAvpfFeedBack,

        //render playback feedback info
        /* 1)kWhatPLY_DecorderStatus,decoder bandwidth for RR
                  will return with 2 int32_t value:
                      "isVideo" -->tell is video or audio track
                      'bandwidth'-->tell the bw value
             */
        kWhatPeerDisplayStatus,
    };

    /**
    *@ flags: special or normal, now shoule be =FLAG_SPECIAL_MODE
    *@ bufferProducer: surface form app
    *@ notify: IMS MA set notify to sink, sink post msg by this notify
    */
    Sink(int32_t multiId,uint32_t flags ,uint32_t simID,uint32_t operatorID);
    void SetSurface(const sp<IGraphicBufferProducer> &bufferProducer);
    void SetNotify(const sp<AMessage> &notify);

    /**@ start sink module, parameter describe how much tracks are enabled and the enabled track info;
        for video: mime/width/height/bitrate/framerate/profile/level/rtp-time-scale rate
        for audio: mime/samplerate/channel count/stereo/bitrate/rtp-time-scale rate
    *@ audioUL: if the   audioUp  ==  NULL -->AUDIO_UPLINK is not enable;
    *@ audioDL: if the   audioDown  ==  NULL. -->AUDIO_DOWNLIK is not enable,
    *@ videoDL: if the   videoDown  ==  NULL. -->VIDEO_DOWNLIK is not enable,
    */
    status_t setSinkCongfig(audio_dec_fmt* audioFormat,video_dec_fmt * videoFormat);
    status_t start(bool startVideo,bool startAudio);
    /*@stop sink module*/
    status_t stop(int32_t pushBlank = 1);
    /*@confi record function,set the record target parameters*/
    status_t initRecorder(record_config* RecCfg);
    /*@start  recording:AMessage wrap each track info, such as the start(...)
    *@ audioUL: if the   audioUp  !=  NULL ,will record the audioUL,should set the parameter
    *@ audioDL: if the   audioDown  !=  NULL ,will record the audioDL,should set the parameter
    *@ videoDL: if the   videoDown   !=  NULL ,will record the videoDL,should set the parameter
    */
    status_t startRecord();

    /*@stop  recording*/
    status_t stopRecord();
    /**@ queueAcessUnit : send data to sink
    *@ trackIndex: identify track
    *@ accessUnit: AU with meta info
      meta info:
        setInt64("ntp-time",  ntpTime);//must put 64 bit NTP  ,if no ntp, put 0 in this meta to tell sink no ntp to use
        setInt32("timeUs",  1); //rtp time change to timeUs
        setInt32("damage",  1); //damage buffer for video
    */
    status_t queueAccessUnit(TrackIndex trackIndex,const sp<ABuffer> &accessUnit) ;    //

    //for avpf
//    status_t setAvpfParamters(const sp<AMessage> &params);

protected:
    virtual ~Sink();
    virtual void onMessageReceived(const sp<AMessage> &msg);

private:

    enum {
        kWhatStart = 10,
        kWhatStop,

        kWhatStartRecord,
        kWhatStopRecord,

        kWhatAccessUnit,

        kWhatRecorderNotify,
        kWhatRenderNotify,

        kWhatMixPCM,

        kWhatFeedRender,
        kWhatFeedRecorder,
        kWhatCodecAvpfFeedBack ,
        kWhatSetAvpfParams ,

    };
    //time key
    enum {
        kTimeKeyNtpBase = 'ntpb',
        kTimeKeyRtpBase = 'rtpb',
        kTimeKeyNtp = 'cntp',
        kTimeKeyRtp = 'crtp',
        kTimeKeyTimeUs = 'tsus',
        kTimeKeySysTimeUs = 'stus',
    };
    enum State {
        IDLE            = 0x01,
        INITED          = 0x02,
        STARTED         = 0x04,
        SURFACE_SET     = 0x08,
        RENDERING       = 0x10,
        RECORDING       = 0x20,
        PAUSED          = 0x40,
        STOPPED         = 0x80,
        UPDATING        = 0x100, //update config
        RENDER_ERROR     = 0x200, //RENDER_ERROR
    };

    enum  {
        Update_Fps              = 0x01,//only some parameter changes:bitrate,fps...
        Update_Bitrate          = 0x02 ,//need reconfig camera preview : W,H change
        //whether we should change the preview size during video call?
        Update_Resolution       = 0x04 ,//need reconfig encoder,W H
        Update_IFrameInterval   = 0x08,
        Update_CodecType        = 0x10,
        Update_ProfileLevel     = 0x20,
    };

    struct TimeInfo {
        int64_t ntp;//audio,video use ntp time
        int64_t timeUs;//rtp module will add timeUs for rtp time
        int64_t systimeUs;
        int64_t refineTimeUs;
    };
    struct TrackInfo {
        TrackIndex  trackIndex;
        AString mime;
        sp<AMessage> mTrackMeta;
        bool mCsdParsed;
        //internal calc
        bool mFirstAccessUnitIsValid;
        bool mTrackNTPIsValid;
        TimeInfo mFirstFrame;
        TimeInfo mCurrentFrame;
        TimeInfo mLastFrame;
        TimeInfo mFirstRecordFrame;
        uint32_t  mFrameCount;
        bool TrackNTPIsValid() {
            return mTrackNTPIsValid;
        }

        bool TrackFirstAUIsValid() {
            return mFirstAccessUnitIsValid;
        }
        int64_t TrackNTPBase() {
            return mFirstFrame.ntp;
        }
        int32_t TrackSysTimeBaseUs() {
            return mFirstFrame.systimeUs;
        }
    };

    int32_t mMultiInstanceID;
    uint32_t mSimID;
    uint32_t mOperatorID;
    uint32_t mState;
    uint32_t mFlags;
    sp<IGraphicBufferProducer> mSurface;
    sp<AMessage> mNotify;
    sp<AMessage> mTimeReqNotify;
    sp<ALooper> mLooper;
    sp<ALooper> mRenderLooper;
    sp<ALooper> mRecorderLooper;
    sp<Renderer> mRenderer;
    sp<Recorder> mRecorder;

    video_dec_fmt mVideoConfigSettings;
    audio_dec_fmt mAudioConfigSettings;
    record_config *mRecordCfg;

    KeyedVector<TrackIndex,TrackInfo*> mTracks;

    List<sp<ABuffer> > mAudioDLPCMQueue;
    List<sp<ABuffer> > mAudioULPCMQueue;
    List<sp<ABuffer> > mMixPCMQueue;


    List<sp<ABuffer> > mVideoDLQueue;

    bool mIDRFrameRequestPending;
    uint32_t  mHaveTracks;
    uint32_t  mRenderTracks;
    bool mRecordInputAudioHas2Tracks;

    //int64_t mAudioLatency;
    //int64_t mAudioPlayOutTimeUs;
    //int64_t mAudioMediaTimeUs;
    //int64_t mAudioRealTimeUs;
    int64_t mStartTimeUs;

    int32_t mRotationDegree;
    int32_t mFlip;
    int32_t mFacing;
    int32_t mVideoWidth;
    int32_t mVideoHeight;
    bool mNotifyClientSize;

    bool mDump;
    bool mRotateInside;
    bool mSupportSliceNAL;
    status_t initLooper_l();//internal initialize
    status_t stop_l(int32_t pushBlank = 1);
    bool  checkAllTracksInfo();
    void updateTrackInfo(TrackIndex trackIndex, sp<ABuffer> &accessUnit);
    void dispatchInputAccessUnit(TrackIndex trackIndex, sp<ABuffer> &accessUnit);
    void handlePCMMix();
    void addAccessUnitToPCMMixer(TrackIndex trackIndex,const sp<ABuffer> &accessUnit);
    void postKeyFrameRequest();
    bool convertToMeta(bool isVideo, sp<AMessage> meta);
    bool findAndParseCsdNAL(sp<ABuffer> &accessUnit,bool withStartCode,int32_t* profile,int32_t* level,int32_t* width,int32_t* height,int32_t* sarWidth,int32_t* sarHeight);
    bool findAndParseCsdNALHEVC(sp<ABuffer> &accessUnit,bool withStartCode,int32_t* width,int32_t* height);
    status_t postAsynAndWaitReturnError(const sp<AMessage> &msg) ;

    //for update
    int32_t compareSettingChange(audio_dec_fmt* audioFormat,video_dec_fmt * videoFormat);
    //for slice data
    bool waitForNextNal(sp<ABuffer> &accessUnit);

    DISALLOW_EVIL_CONSTRUCTORS(Sink);
};

}  // namespace android

#endif  // _SINK_H_
