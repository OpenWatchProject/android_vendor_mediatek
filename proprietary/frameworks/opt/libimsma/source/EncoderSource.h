
#ifndef EncoderSource_H_
#define EncoderSource_H_

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AHandlerReflector.h>
#include <media/MediaSource.h>
#include "comutils.h"

namespace android
{

struct ALooper;
struct AMessage;
struct MediaCodec;
class MediaCodecBuffer;
class MetaData;



struct EncoderSource : public AHandler {

    enum {
        kWhatError =0,
        kWhatAccessUnit,
    };
    enum FlagBits {
        FLAG_USE_SURFACE_INPUT      = 1,
        FLAG_USE_METADATA_INPUT     = 2,
    };
    enum ConfigBits {
        Config_Fps              = 1,
        Config_Bitrate              = 2,
        Config_IframeInterval   = 3,
    };

    EncoderSource(
        const sp<ALooper> &looper,
        const sp<AMessage> &outputFormat,
        const sp<MediaSource> &source,
        const sp<AMessage> & notify,
        int32_t multiId);


    bool isVideo() const {
        return mIsVideo;
    }
    sp<AMessage> getFormat();
    status_t start();
    status_t stop();
    //pause input processing:1)add generation to stop getting data from camera 2)let rotate release any data from camera
    status_t pause();
    status_t resume();

    // for AHandlerReflector
    void onMessageReceived(const sp<AMessage> &msg);

    //for avpf
    status_t setAvpfParamters(const sp<AMessage> &params);
    //update parameters
    status_t setCodecParameters(const sp<AMessage> &params);
    size_t getInputBufferCount() const;

protected:
    virtual ~EncoderSource();

private:
    struct Puller;
    struct Rotator;

    enum {
        kWhatRotatorNotify = 10,
        kWhatEncoderActivity,
        kWhatStart,
        kWhatStop,
        kWhatPause,
        kWhatResume,
        kWhatFlush,
        kWhatSwitchSource,
        kWhatResetEncoder,
        kWhatSetParameters,        
		kWhatPullerDirectNotify ,
    };



    status_t onStart();
    status_t initEncoder();//find rotate info and make sure the wh ratio in this function
    void releaseEncoder();
    void scheduleDoMoreWork();
    void flush_l(int32_t input,int32_t output);
    status_t doMoreWork(int32_t numInput, int32_t numOutput);
    status_t postSynchronouslyAndReturnError(const sp<AMessage> &msg);
    status_t feedEncoderInputBuffersAfterRotate();
    //void extractRotationInfoByFormat(RotateInfo* info);
    //status_t switchSource_l(const sp<MediaSource> &source);
    void setFramerate();
    status_t requestIDRFrame();
    status_t requestFullIDRFrame();
    status_t resetEncoder(const sp<AMessage> &newOutputFormat);

    void flush(int32_t input =1,int32_t output =0);
	void releaseABuffer(sp<MediaCodecBuffer> &accessUnit);

    //for debug multi instance
    int32_t mMultiInstanceID;

    sp<ALooper> mLooper;
    sp<ALooper> mCodecLooper;
    sp<ALooper> mRotatorLooper;
    sp<ALooper> mPullerLooper;
    sp<AMessage> mOutputFormat;

    sp<Puller> mPuller;
    sp<MediaCodec> mEncoder;

    bool mIsVideo;
    bool mStarted;
    bool mStopping;
    bool mPausing;
    bool mDoMoreWorkPending;
	bool mEncoderInputBufferStoreInMeta;
    sp<AMessage> mEncoderActivityNotify;
    sp<AMessage> mNotify;
    sp<MediaSource>  mSource;

    Vector<sp<MediaCodecBuffer> > mEncoderInputBuffers;
    Vector<sp<MediaCodecBuffer> > mEncoderOutputBuffers;
    KeyedVector<int64_t ,int32_t> mTimeUsTokenMaps;

    sp<AMessage> mNofityEncoderInputReadyToRotator;
    sp<Rotator> mRotator;
    List<sp<MediaCodecBuffer> > mInputABufferQueue;
    KeyedVector<sp<MediaCodecBuffer>,size_t> mAvailEncoderInputABufferIndices;//can be user input buffers to rorate

    DISALLOW_EVIL_CONSTRUCTORS(EncoderSource);


};

} // namespace android

#endif /* MediaCodecSource_H_ */
