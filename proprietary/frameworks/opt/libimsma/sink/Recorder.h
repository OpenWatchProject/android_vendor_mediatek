#ifndef _RECORDER_H_

#define _RECORDER_H_

#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/AudioSource.h>

#include "MediaMuxer.h"
#include "EncoderContext.h"
#include "Sink.h"

namespace android
{

struct ABuffer;
class  AudioTrack;
class  IGraphicBufferProducer;
struct MediaCodec;
struct EncoderContext;


// Renders audio and video data queued by calls to "queueAccessUnit".
struct Recorder : public AHandler {

    enum record_mode_t {
        RECORD_DLVIDEO_MIXADUIO,  //dl video + VTservice sent mix audio, now use this
        RECORD_DLVIDEO_DLAUDIO_ULAUDIO, //dl video + ul audio + dl audio ,3 path data
        RECORD_MIXAUDIO_ONLY,   //VTservice sent mix audio
        RECORD_DLVIDEO_ONLY,     //dl video
    };

    enum {
        kWhatError = 'erro',
        kWhatREC_ReachMaxDuration = 'recd',  // reach the max duration limit of file
        kWhatREC_ReachMaxFileSize = 'recs',  //reach the max size limitation of file
        kWhatREC_ReachNoStorageSpace = 'outm',  //reach the storage size when recording
        kWhatREC_RequsetIFrame='rqif',//no I frame comes, request an I frame
        kWhatREC_WriteComplete='comp',//wrte all into sdcard
        //other info should feed back to AP
    };

    Recorder(record_mode_t mode,const sp<AMessage> &notify);

    status_t    setVideoSource(int32_t  vs,sp<AMessage> & vsFomart);
    status_t    setAudioSource(int32_t  as,sp<AMessage> & asFomart);
    status_t    setOutputFormat(int32_t  of);
    status_t    setVideoEncoder(int32_t  ve);
    status_t    setAudioEncoder(int32_t  ae);
    status_t    setOutputFile(const char* path);
    status_t    setOutputFile(int32_t  fd, int64_t offset, int64_t length);
    /**
    *@ set the outpt file other parameters,such as rotaion info or latitude and longtitude info
    */
    status_t setParameters(const sp<AMessage> &parameters);
    /**
    *@ prepare recorder: init encoder and writer
    */
    status_t  prepare();
    /**
    *@ start record
    */
    status_t start();
    status_t stop();
    /**send the video/aduio data to recorder
    *@ isVideo: distinct video or audio
    *@ accessUnit: video or audio frame in ABuffer
    */
    void queueAccessUnit(int32_t isVideo, const sp<ABuffer> &accessUnit);
    void setAVTimeStampMap(int64_t videoTimeUs, int64_t audioTimeUs);


protected:
    virtual void onMessageReceived(const sp<AMessage> &msg);
    virtual ~Recorder();

private:
    enum State {
        UNDEFINED,
        INITED,
        PAUSED,
        RECORDING,
        STOPPED,
    };

    enum {
        kWhatInit,
        kWhatStart,
        kWhatStop,
        kWhatPause,
        kWhatResume,
        kWhatEncoderNotify,//rev encoder encoder done data then push to muxeter
        kWhatWriteSample,
        kWhatAccessUnit,
    };

    record_mode_t mRecordMode;
    sp<AMessage> mAudioSourceFmt;
    sp<AMessage> mVideoSourceFmt;
    output_format   mOutputFormat;
    sp<AMessage>  mAudioEncoderFmt;
    sp<AMessage>  mVideoEncoderFmt;

    sp<AMessage>  mParams;
    sp<AudioSource> mAudioSource;

    bool mUse64BitFileOffset;
    int32_t mVideoWidth, mVideoHeight;
    int mOutputFd;
    const char* mPath;
    bool mUseFd;

    State mState;
    int64_t mVideoBaseTimeUs;
    int64_t mAudioBaseTimeUs;


    sp<AMessage> mNotify;
    sp<MediaMuxer> mMediaMuxer;

    KeyedVector<int32_t,sp<EncoderContext> >mEncoderContexts;  //trackIndex<->encoder context

    List<sp<ABuffer> > mAudioInputQueue;
    List<sp<ABuffer> > mVideoInputQueue;
    List<sp<ABuffer> > mAudioOutputQueue;
    List<sp<ABuffer> > mVideoOutputQueue;

    int32_t mNumFrames;
    sp<AReplyToken> mReplyID;
    int32_t mRotationDegree;

    int32_t mVideoTrackIndex,mAudioTrackIndex;
    bool  mNeedVideoEcoder,mNeedAudioEcoder;
    int64_t mVideoLastTimeStamp;
    bool mHasWriteCsdBuffer;

    class MediaRecordListner : public BnMediaRecorderClient
    {
    public:
        MediaRecordListner(Recorder* handler) {
            mHandler = handler;
        }
        ~MediaRecordListner() {}

        virtual void  notify(int msg, int ext1, int ext2) {
            mHandler->HandleEventNotify(msg,ext1,ext2);
        }


    private:
        Recorder* mHandler;
    };

    sp<IMediaRecorderClient> mRecordListner;


    status_t addAudioSource(bool usePCMAudio) ;
    status_t addVideoSource(bool useBitstream);
    void HandleEventNotify(int msg, int params1,int params2);

    void notifyError();
    void preWriteCsdBuffer();
    status_t writeSamplesToMuxter(int32_t trackIndex,sp<ABuffer> &buffer);

    status_t postAsynAndWaitReturnError(const sp<AMessage> &msg) ;
    DISALLOW_EVIL_CONSTRUCTORS(Recorder);
};

}  // namespace android

#endif  // _RECORDER_H_

