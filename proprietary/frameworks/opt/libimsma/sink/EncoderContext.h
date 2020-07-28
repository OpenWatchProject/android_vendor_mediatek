#ifndef LTE_ENCODERCONTEXT_H_

#define LTE_ENCODERCONTEXT_H_

#include <media/stagefright/foundation/AHandler.h>

namespace android
{

struct ABuffer;
struct MediaCodec;
class MediaCodecBuffer;

#define ENABLE_SILENCE_DETECTION        0

// Utility class that receives media access units and converts them into
// media access unit of a different format.
struct EncoderContext : public AHandler {
    EncoderContext(
        const sp<AMessage> &notify,
        const sp<ALooper> &codecLooper,
        const sp<AMessage> &inputFormat,
        const sp<AMessage> &outputFormat);

    size_t getInputBufferCount() const;
    sp<AMessage> getOutputFormat() const;
    bool needToManuallyPrependSPSPPS() const;

    void queueAccessUnit(const sp<ABuffer> &accessUnit);
    void signalEOS();

    void requestIDRFrame();

    enum {
        kWhatAccessUnitOut,
        kWhatEOS,
        kWhatError,
    };

    enum {
        kWhatDoMoreWork,
        kWhatRequestIDRFrame,
        kWhatStop,
        kWhatEncoderActivity,
        kWhatAccessUnitIn,
    };

    void stopAsync();

    int32_t getVideoBitrate() const;
    void setVideoBitrate(int32_t bitrate);

    //static int32_t GetInt32Property(const char *propName, int32_t defaultValue);

protected:
    virtual ~EncoderContext();
    virtual void onMessageReceived(const sp<AMessage> &msg);

private:
    sp<AMessage> mNotify;
    sp<ALooper> mCodecLooper;
    sp<AMessage> mInputFormat;
    sp<AMessage> mOutputFormat;
    bool mIsVideo;
    bool mUsePCM;
    bool mIsFakeEncoder;
    bool mNeedToManuallyPrependSPSPPS;

    sp<MediaCodec> mEncoder;
    sp<AMessage> mEncoderActivityNotify;

    Vector<sp<MediaCodecBuffer> > mEncoderInputBuffers;
    Vector<sp<MediaCodecBuffer> > mEncoderOutputBuffers;

    List<size_t> mAvailEncoderInputIndices;
    List<sp<ABuffer> > mInputBufferQueue;

    bool mDoMoreWorkPending;

#if ENABLE_SILENCE_DETECTION
    int64_t mFirstSilentFrameUs;
    bool mInSilentMode;
#endif

    sp<ABuffer> mPartialAudioAU;

    int32_t mPrevVideoBitrate;

    status_t initEncoder();
    void releaseEncoder();

    status_t feedEncoderInputBuffers();

    void scheduleDoMoreWork();
    status_t doMoreWork();

    void notifyError(status_t err);

    // Packetizes raw PCM audio data available in mInputBufferQueue
    // into a format suitable for transport stream inclusion and
    // notifies the observer.
    status_t feedRawAudioInputBuffers();
    status_t feedRawVideoInputBuffers();
    static bool IsSilence(const sp<ABuffer> &accessUnit);

    DISALLOW_EVIL_CONSTRUCTORS(EncoderContext);
};

}  // namespace android

#endif  // LTE_ENCODERCONTEXT_H_
