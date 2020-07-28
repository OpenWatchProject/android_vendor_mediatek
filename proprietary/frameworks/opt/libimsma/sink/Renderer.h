#ifndef DIRECT_RENDERER_H_

#define DIRECT_RENDERER_H_

#include <media/stagefright/foundation/AHandler.h>

namespace android
{

struct ABuffer;
class  AudioTrack;
class  IGraphicBufferProducer;
struct MediaCodec;
class MediaCodecBuffer;
// Renders audio and video data queued by calls to "queueAccessUnit".
struct Renderer : public AHandler {

    /*@ render feed back information key*/
    enum {
        kWhatError = 0,
        kWhatOutPCM, //if need renderer pcm, the pcm should be post to sink to let recorder to record
        kWhatGetAudioPlayOutTimeUs,  //get audio playout time
        //kWhatDecodeStatus ,//post decoer statutus to MA, for RR
        kWhatVideoSizeChanged,
        kWhatPeerDisplayStatus,
    };

    /**
    *@ bufferProducer: video renderer surface
    *@ notify: sink module set notify to Renderer, Renderer post msg by this notify
    */
    Renderer(int32_t multiId,const sp<IGraphicBufferProducer> &bufferProducer,const sp<AMessage> &notify);

    /**pass the decoder config data
    *@ isVideo: distinct video or audio
    *@ format: the codec config info wrapped in msg
    */
    status_t  setFormat(int32_t isVideo, const sp<AMessage> &format);
    /**send the video/aduio data to render moder
    *@ isVideo: distinct video or audio
    *@ accessUnit: video or audio frame in ABuffer
    */
    void queueAccessUnit(int32_t isVideo, const sp<ABuffer> &accessUnit);

    /**
    @: tell render to output which track data

    **/
    void dequeueTrackAccessUnit(int32_t isVideo);
    //for avpf
//    status_t setAvpfParamters(const sp<AMessage> &params);

    status_t stop(int32_t pushBlank = 1);

protected:
    virtual void onMessageReceived(const sp<AMessage> &msg);
    virtual ~Renderer();

private:
    struct DecoderContext;
    struct AudioRenderer;

    enum {
        kWhatDecoderNotify = 10,
        kWhatRenderVideo,
        kWhatQueueAccessUnit,
        kWhatSetFormat,
        kWhatSetAvpfParams ,
        kWhatScanDisplayStatus,
    };

    struct OutputInfo {
        size_t mIndex;
        int64_t mTimeUs;
        sp<MediaCodecBuffer> mBuffer;
    };
    enum VideoHow {
        VIDEO_SHOW = 1,
        VIDEO_DELAY = 2,
        VIDEO_DROP  = 3,
    };


    int32_t mMultiInstanceID;
    sp<Surface> mSurface;
    sp<AMessage> mNotify;

    sp<DecoderContext> mDecoderContext[2];
    List<OutputInfo> mVideoOutputBuffers;
    bool mVideoRenderPending;
    bool mFeebBackAudioPCM;

    sp<AudioRenderer> mAudioRenderer;
    int32_t mNumFramesDrop;
    int32_t mNumFrames;

    int64_t mAudioRealTimeUs;
    int64_t mFirstVideoMediaTimeUs;
    int64_t mFirstVideoComeSysTimeUs;
    bool  mFirstVideoTimeBaseValid;
    VideoHow mCurrentVideoHow;
    VideoHow mLastVideoHow;
    int32_t mContinuouslyDropThreshold;
    int32_t mContinuouslyDropCnt;
    int32_t mVideoTrackIndex;
    int32_t mRotationDegree;
    bool mRotateInside;
    bool mEnableAvsync;

    KeyedVector<int64_t,int64_t> mVideoTimeUsNTPMaps;
    int32_t mNTPIsValid;
    int32_t mPushBlank;//bit for expand?


    void onDecoderNotify(const sp<AMessage> &msg);

    void queueOutputBuffer(
        int32_t isVideo,
        size_t index, int64_t timeUs, const sp<MediaCodecBuffer> &buffer);

    void scheduleVideoRenderIfNecessary(int64_t delayUs = 0);
    void onRenderVideo();

    status_t  onSetFormat(const sp<AMessage> &msg);
    void onQueueAccessUnit(const sp<AMessage> &msg);

    status_t  internalSetFormat(int32_t isVideo, const sp<AMessage> &format);

    status_t  getAudioPlayOutTimeUs(int64_t videotime,int64_t* audioTime,int64_t* audioNTP);    //post sync msg to sink to get the auditime

    status_t resetSurfaceTransformDegree(int32_t degree, bool force = false);
    void notifyError(int32_t err);

    DISALLOW_EVIL_CONSTRUCTORS(Renderer);
};

}  // namespace android

#endif  // _DIRECT_RENDERER_H_
