//#define LOG_NDEBUG 0
#define LOG_TAG "[VT][Sink]Renderer"
#include <utils/Log.h>
#include <gui/Surface.h>
#include <media/AudioTrack.h>
#if !(PLATFORM_VERSION == R)
#include <media/ICrypto.h>
#else
#include <mediadrm/ICrypto.h>
#endif
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/MediaCodec.h>
#include <media/MediaCodecBuffer.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>
#include "Renderer.h"
#include <cutils/properties.h>
#include "comutils.h"
#include "VTAVSync.h"
#include <media/stagefright/SurfaceUtils.h>
#define ATRACE_TAG ATRACE_TAG_VIDEO
#include <utils/Trace.h>
#define DECODER_BW_THRESHOLD_LOW     500000  // 500K
#define DECODER_BW_THRESHOLD_HIGH    2000000  // 2Mbyte/s
#define AVSYNC_THRESHOLD    10000000ll  //10
#define ERROR_DELAY_US    3000000ll  //10
#define PAUSE_DELAY_LOOP   3  //3S




namespace android
{

/*
   Drives the decoding process using a MediaCodec instance. Input buffers
   queued by calls to "queueInputBuffer" are fed to the decoder as soon
   as the decoder is ready for them, the client is notified about output
   buffers as the decoder spits them out.
*/
struct Renderer::DecoderContext : public AHandler {
    enum {
        kWhatOutputBufferReady =0,
        //kWhatDecoderBWNotify,
        kWhatOutputFormatChanged,
        kWhatError,
    };
    DecoderContext(int32_t multiId,const sp<AMessage> &notify);

    status_t init(
        const sp<AMessage> &format,
        const sp<Surface> &surface);

    void queueInputBuffer(const sp<ABuffer> &accessUnit);

    status_t renderOutputBufferAndRelease(size_t index);
    status_t releaseOutputBuffer(size_t index);

//    status_t setAvpfParamters(const sp<AMessage> &params);

protected:
    virtual ~DecoderContext();

    virtual void onMessageReceived(const sp<AMessage> &msg);

private:
    enum {
        kWhatDecoderNotify = 10,
        //kWhatCheckDecoderBW,
    };
    /*
    struct BandwidthEntry {
        int64_t mAddSysTimeUs;
        int32_t mNumBytes;
    };*/
    int32_t mMultiInstanceID;
    int32_t mIsVideo;
    sp<AMessage> mNotify;
    sp<ALooper> mDecoderLooper;
    sp<MediaCodec> mDecoder;
    Vector<sp<MediaCodecBuffer> > mDecoderInputBuffers;
    Vector<sp<MediaCodecBuffer> > mDecoderOutputBuffers;
    List<size_t> mDecoderInputBuffersAvailable;
    bool mDecoderNotificationPending;
    //bool mPushBlank;

    List<sp<ABuffer> > mAccessUnits;
    //List<BandwidthEntry> mBandwidthHistory;
    //int32_t mTotalBandwidthEntrySize;
    //int32_t mBandwidthkBps;
    //int64_t mLastBandwidthCalcTimeUs;
    List<int64_t> mDecodingTimeQueue; // decoding time (us) for video
    KeyedVector<int64_t,int32_t> mTimeUsTokenMaps;


    void onDecoderNotify();
    void scheduleDecoderNotification();
    void queueDecoderInputBuffers();
    void scheduleCheckDecoderBW();

    void queueOutputBuffer(
        size_t index, int64_t timeUs, const sp<MediaCodecBuffer> &buffer);

    void notifyError(int32_t err);
    // calc the decoder bandwidth in period
    //void addBandwidthMeasurement(size_t numBytes);
    //bool  checkAndGetBandwidthKbps(int32_t* kbps) ;


    DISALLOW_EVIL_CONSTRUCTORS(DecoderContext);
};

////////////////////////////////////////////////////////////////////////////////

/*
   A "push" audio renderer. The primary function of this renderer is to use
   an AudioTrack in push mode and making sure not to block the event loop
   be ensuring that calls to AudioTrack::write never block. This is done by
   estimating an upper bound of data that can be written to the AudioTrack
   buffer without delay.
*/
struct Renderer::AudioRenderer : public AHandler {
    AudioRenderer(const sp<DecoderContext> &DecoderContext,sp<AMessage> notify);

    void queueInputBuffer(size_t index, int64_t timeUs, const sp<MediaCodecBuffer> &buffer);
    int64_t  getAudioPlayOutTimeUs();
    void dequeueTrackAccessUnit();

protected:
    virtual ~AudioRenderer();
    virtual void onMessageReceived(const sp<AMessage> &msg);

private:
    enum {
        kWhatPushAudio =10,
    };

    struct BufferInfo {
        size_t mIndex;
        int64_t mTimeUs;
        sp<MediaCodecBuffer> mBuffer;
    };

    sp<DecoderContext> mDecoderContext;
    sp<AudioTrack> mAudioTrack;
    sp<AMessage>  mNotify;
    bool mStartFeedBackPCM;

    List<BufferInfo> mInputBuffers;
    bool mPushPending;

    size_t mNumFramesWritten;
    int64_t mLatencyUs;

    void schedulePushIfNecessary();
    void onPushAudio();

    ssize_t writeNonBlocking(const uint8_t *data, size_t size);

    DISALLOW_EVIL_CONSTRUCTORS(AudioRenderer);
};

////////////////////////////////////////////////////////////////////////////////

Renderer::DecoderContext::DecoderContext(int32_t multiId,const sp<AMessage> &notify)
    : mMultiInstanceID(multiId),
      mIsVideo(0) ,
      mNotify(notify),
      mDecoderNotificationPending(false)
      //mTotalBandwidthEntrySize(0),
      //mBandwidthkBps(0),
      // mLastBandwidthCalcTimeUs(-1),
{
    VT_LOGD("[ID=%d]create decoder",mMultiInstanceID);
/*
    //push blank property map
    char value[PROPERTY_VALUE_MAX];
    int MAMap = 0;
    int TestMode = 0;
    mPushBlank = true;

    if(property_get("persist.vendor.vt.OPTest_MM", value, NULL)) {
        TestMode = atoi(value);
    }

    memset(value, 0, sizeof(value));
    if(property_get("persist.vendor.vt.OPTest_MA", value, NULL)) {
        MAMap = strtol(value,NULL,16);
        if(TestMode){
            mPushBlank = (MAMap & 0x2) ? false : true;
        }
    }

    VT_LOGI("OPTest.MA TestMode=%d MAMap=0x%x 0x2=%d mPushBlank=%d", TestMode, MAMap, (MAMap & 0x2), mPushBlank);*/
}

Renderer::DecoderContext::~DecoderContext()
{
    if(mDecoder.get() != NULL) {
        mDecoder->release();
        mDecoder.clear();
        mDecoder = NULL;

        mDecoderLooper->stop();
        mDecoderLooper.clear();
        mDecoderLooper = NULL;
    }
}

status_t Renderer::DecoderContext::init(
    const sp<AMessage> &format,
    const sp<Surface> &surface)
{
    CHECK(mDecoder == NULL);
    ATRACE_CALL();
    AString mime;
    CHECK(format->findString("mime", &mime));

    mDecoderLooper = new ALooper;

    if(!strncasecmp("video/", mime.c_str(), 6)) {
        mDecoderLooper->setName("video codec looper");
    } else {
        mDecoderLooper->setName("audio codec looper");
    }

    mDecoderLooper->start(
        false /* runOnCallingThread */,
        false /* canCallJava */,
        PRIORITY_AUDIO);

    mDecoder = MediaCodec::CreateByType(
                   mDecoderLooper, mime.c_str(), false /* encoder */);

    CHECK(mDecoder.get() != NULL);

    if(!strncasecmp("video/", mime.c_str(), 6)) {
        //format->setInt32("vdec-no-record", 1);
        format->setInt32("vdec-lowlatency", 1);
        //format->setInt32("max-queue-buffer", 1);
        //format->setInt32("input-buffer-number", 4);
        //format->setInt32("vilte-mode", 1);
/*
        char value[PROPERTY_VALUE_MAX];
        int blank_disable = (mPushBlank == true ? 0 : 1);

        if(property_get("persist.vendor.vt.blank.disable", value, NULL)) {
            blank_disable = atoi(value);
        }

        VT_LOGD("[ID=%d] blank_disable=%d", mMultiInstanceID, blank_disable);

        if(!blank_disable) {
            format->setInt32("push-blank-buffers-on-shutdown", 1);
        }*/
    }

    VT_LOGD("[ID=%d]configure %s",mMultiInstanceID,format->debugString(0).c_str());
    status_t err = mDecoder->configure(
                       format,
                       surface == NULL? NULL : surface ,
                       NULL /* crypto */,
                       0 /* flags */);

    if(err != OK) {
        VT_LOGE("[ID=%d]mDecoder configure  fail  %d ",mMultiInstanceID,err);
        return err;
    }

    err = mDecoder->start();

    if(err != OK) {
        VT_LOGE("[ID=%d]mDecoder start  fail  %d ",mMultiInstanceID,err);
        return err;
    }

/*
    //only for avpf==>
    sp < AMessage >  params = new AMessage();
    sp < AMessage >  avpfNotify = NULL;
    CHECK(format->findMessage("avpf-notify", &avpfNotify));
    params->setMessage("avpf-notify",avpfNotify);
    mDecoder->setParameters(params);
    //<===  only for avpf
*/

    err = mDecoder->getInputBuffers(&mDecoderInputBuffers);

    if(err != OK) {
        VT_LOGE("[ID=%d]mDecoder getInputBuffers  fail  %d ",mMultiInstanceID,err);
        return err;
    }

    err = mDecoder->getOutputBuffers(&mDecoderOutputBuffers);

    if(err != OK) {
        VT_LOGE("[ID=%d]mDecoder getOutputBuffers  fail  %d ",mMultiInstanceID,err);
        return err;
    }

    scheduleDecoderNotification();

    //scheduleCheckDecoderBW();
    if(surface.get() != NULL) {

        mIsVideo =1;
    }


    return OK;
}

void Renderer::DecoderContext::queueInputBuffer(const sp<ABuffer> &accessUnit)
{
    CHECK(mDecoder.get() != NULL);
    ATRACE_CALL();
    int32_t token =0;

    if(accessUnit->meta()->findInt32("token",&token)) {
        ATRACE_ASYNC_END("RND-DCT", token);
        ATRACE_ASYNC_BEGIN("DCT-DEC", token);
    }

    int64_t timeUs =-1;
    CHECK(accessUnit->meta()->findInt64("timeUs",&timeUs));

    if(mAccessUnits.size() > 20) {
        VT_LOGW("[ID=%d]check decoder perf token %d timeUs %" PRId64 " ms, queue %zu",
                mMultiInstanceID,token,timeUs,mAccessUnits.size());
    }


    mAccessUnits.push_back(accessUnit);
    queueDecoderInputBuffers();

}

status_t Renderer::DecoderContext::renderOutputBufferAndRelease(
    size_t index)
{
    ATRACE_CALL();

    return mDecoder->renderOutputBufferAndRelease(index);
}

status_t Renderer::DecoderContext::releaseOutputBuffer(size_t index)
{
    ATRACE_CALL();
    return mDecoder->releaseOutputBuffer(index);
}

void Renderer::DecoderContext::queueDecoderInputBuffers()
{
    if(mDecoder == NULL) {
        VT_LOGD("[ID=%d]decoder is NULL now, should not queueDecoderInputBuffers",mMultiInstanceID);
        return;
    }


    bool submittedMore = false;

    int32_t needStartCode = 0;

    while(!mAccessUnits.empty()
            && !mDecoderInputBuffersAvailable.empty()) {
        size_t index = *mDecoderInputBuffersAvailable.begin();

        mDecoderInputBuffersAvailable.erase(
            mDecoderInputBuffersAvailable.begin());

        sp<ABuffer> srcBuffer = *mAccessUnits.begin();
        mAccessUnits.erase(mAccessUnits.begin());

        const sp<MediaCodecBuffer> &dstBuffer =
            mDecoderInputBuffers.itemAt(index);
        int nal_length = srcBuffer->size();

        if(srcBuffer->meta()->findInt32("needStartCode",&needStartCode) && needStartCode == 1) {
            memcpy(dstBuffer->data(), "\x00\x00\x00\x01", 4);
            memcpy(dstBuffer->data() +4, srcBuffer->data(), srcBuffer->size());
            nal_length += 4;
            //ALOGD("inputbuffer addStartCode");
        } else {
            memcpy(dstBuffer->data(), srcBuffer->data(), srcBuffer->size());
            //ALOGD("inputbuffer not addStartCode");
        }

        int64_t timeUs =-1;
        CHECK(srcBuffer->meta()->findInt64("timeUs", &timeUs));

/*
        int32_t lostfull = 0;

        if(srcBuffer->meta()->findInt32("lostfull",&lostfull) && lostfull == 1) {
            dstBuffer->meta()->setInt32("lostfull",1);
        } else {
            dstBuffer->meta()->setInt32("lostfull",0);
        }
*/

        mDecodingTimeQueue.push_back(timeUs);
        int32_t token = -1;

        if(srcBuffer->meta()->findInt32("token",&token)) {
            ATRACE_ASYNC_END("DCT-DEC", token);
            ATRACE_ASYNC_BEGIN("DEC-RED", token);
        }

        VT_LOGV("[ID=%d]queueDecoderInputBuffers token %d timeUs %" PRId64 " ----",mMultiInstanceID,token,timeUs);

        mTimeUsTokenMaps.add(timeUs,token);

        ATRACE_INT("queueInputBuffer", index);

        status_t err = mDecoder->queueInputBuffer(
                           index,
                           0 /* offset */,
                           nal_length,
                           timeUs,
                           0 /* flags */);

        if(err != OK) {
            VT_LOGE("[ID=%d]err == %d delete peer surface before stop sink ",mMultiInstanceID,err);
            return ;
        }

        //CHECK_EQ(err, (status_t)OK);
        //if need check BW,we call this function
        //addBandwidthMeasurement(srcBuffer->size());

        submittedMore = true;
    }

    if(submittedMore) {
        scheduleDecoderNotification();
    }
}

void Renderer::DecoderContext::onMessageReceived(
    const sp<AMessage> &msg)
{
    switch(msg->what()) {
        VT_LOGV("[ID=%d]DecoderContext mIsVideo %d what %d ",mMultiInstanceID, mIsVideo,msg->what());
    case kWhatDecoderNotify: {
        onDecoderNotify();
        break;
    }
    /*
    case kWhatCheckDecoderBW:
       {
        int32_t decBW;
           bool decoderIsLowBW = checkAndGetBandwidthKbps(&decBW);
        VT_LOGV("DecoderContext mIsVideo %d  lowBW %d BW %d", mIsVideo, decoderIsLowBW ,decBW);
        if(decoderIsLowBW){

        sp<AMessage> msg = mNotify->dup();
        msg->setInt32("what", kWhatDecoderBWNotify);
        msg->setInt32("isVideo", mIsVideo);
        msg->setInt32("bandwidth", decBW);
            msg->post();
        }
        scheduleCheckDecoderBW();
           break;
       }*/

    default:
        VT_LOGE("[ID=%d]%s",mMultiInstanceID,msg->debugString(0).c_str());
        TRESPASS();
    }
}

void Renderer::DecoderContext::onDecoderNotify()
{
    mDecoderNotificationPending = false;
    ATRACE_CALL();

    for(;;) {
        size_t index = -1;
        status_t err = mDecoder->dequeueInputBuffer(&index);
        ATRACE_INT("dequeueInputBuffer", index);

        if(err == OK) {
            VT_LOGV("[ID=%d][form codec]dequeueInputBuffer index %zu ",mMultiInstanceID, index);
            mDecoderInputBuffersAvailable.push_back(index);
        } else if(err == -EAGAIN) {
            break;
        } else {
            VT_LOGE("[ID=%d]err == %d delete peer surface before stop sink@dequeueInputBuffer ",mMultiInstanceID,err);
            notifyError(err);
            //TRESPASS();
            return;
        }
    }

    queueDecoderInputBuffers();

    for(;;) {
        size_t index = -1;
        size_t offset= -1;
        size_t size= -1;
        int64_t timeUs= -1;
        uint32_t flags = 0;
        status_t err = mDecoder->dequeueOutputBuffer(
                           &index,
                           &offset,
                           &size,
                           &timeUs,
                           &flags);

        if(err == OK) {
            ATRACE_INT("dequeueOutputBuffer", index);
            VT_LOGV("[ID=%d][from codec]dequeueOutputBuffer:timeUs  %" PRId64 " ",mMultiInstanceID,timeUs);
            queueOutputBuffer(
                index, timeUs, mDecoderOutputBuffers.itemAt(index));

        } else if(err == INFO_OUTPUT_BUFFERS_CHANGED) {
            VT_LOGD("[ID=%d][from codec]:err == INFO_OUTPUT_BUFFERS_CHANGED",mMultiInstanceID);
            err = mDecoder->getOutputBuffers(
                      &mDecoderOutputBuffers);
            CHECK_EQ(err, (status_t) OK);
        } else if(err == INFO_FORMAT_CHANGED) {
            VT_LOGD("[ID=%d][from codec]err == INFO_FORMAT_CHANGED",mMultiInstanceID);
            sp<AMessage> format = new AMessage();
            status_t res = mDecoder->getOutputFormat(&format);

            if(res != OK) {
                VT_LOGE("[ID=%d]Failed to get output format for after INFO event (err=%d)", mMultiInstanceID,res);
                CHECK(res == OK);
            }

            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("what", kWhatOutputFormatChanged);
            notify->setMessage("format", format);
            notify->post();
            // We don't care.
        } else if(err == -EAGAIN) {
            VT_LOGV("[ID=%d]err == -EAGAIN",mMultiInstanceID);
            break;
        } else {
            VT_LOGE("[ID=%d]err == %d delete peer surface before stop sink@dequeueOutputBuffer ",mMultiInstanceID,err);
            // TRESPASS();
            notifyError(err);    //just let post error here
            return;

        }
    }

    scheduleDecoderNotification();
}

void Renderer::DecoderContext::scheduleDecoderNotification()
{
    if(mDecoderNotificationPending) {
        return;
    }

    sp<AMessage> notify =
        new AMessage(kWhatDecoderNotify, this);

    mDecoder->requestActivityNotification(notify);
    mDecoderNotificationPending = true;
}

void Renderer::DecoderContext::queueOutputBuffer(
    size_t index, int64_t timeUs, const sp<MediaCodecBuffer> &buffer)
{
    ATRACE_CALL();
    CHECK(!mDecodingTimeQueue.empty());
    //int64_t decodingTimeUs = *(mDecodingTimeQueue.begin());
    mDecodingTimeQueue.erase(mDecodingTimeQueue.begin());

    int32_t token = -1;

    if(mTimeUsTokenMaps.indexOfKey(timeUs) >= 0) {
        token  =  mTimeUsTokenMaps.valueFor(timeUs);
        mTimeUsTokenMaps.removeItem(timeUs);
    }

    ATRACE_ASYNC_END("DEC-RED", token);
    ATRACE_ASYNC_BEGIN("RED-SUR", token);
    buffer->meta()->setInt32("token", token);


    sp<AMessage> msg = mNotify->dup();
    msg->setInt32("what", kWhatOutputBufferReady);
    msg->setSize("index", index);
    msg->setInt64("timeUs", timeUs);
    msg->setObject("buffer", buffer);
    msg->post();
}
/*
void Renderer::DecoderContext::scheduleCheckDecoderBW(){

    sp<AMessage> msg =new AMessage(kWhatCheckDecoderBW,  this);
    msg->post(1000000ll);//check once a second
}

void Renderer::DecoderContext::addBandwidthMeasurement(size_t numBytes) {
     ATRACE_CALL();
    int64_t nowUs=ALooper::GetNowUs();
    BandwidthEntry entry;
    entry.mAddSysTimeUs = nowUs;
    entry.mNumBytes = numBytes;
    mBandwidthHistory.push_back(entry);
    mTotalBandwidthEntrySize =+ numBytes;

    if (mBandwidthHistory.size() > 100)
    {
        BandwidthEntry *entry = &*mBandwidthHistory.begin();
        mTotalBandwidthEntrySize -=entry->mNumBytes;
        mBandwidthHistory.erase(mBandwidthHistory.begin());
    }


    BandwidthEntry *entry1 = &*mBandwidthHistory.begin();
    int64_t addMinUs=entry1->mAddSysTimeUs;


    if(mBandwidthHistory.size() > 30) {//1s calc once
        if(mLastBandwidthCalcTimeUs == -1){
                mLastBandwidthCalcTimeUs = nowUs;
                mBandwidthkBps =    mTotalBandwidthEntrySize*1000/(nowUs - addMinUs) ;
        }else if((nowUs -mLastBandwidthCalcTimeUs) > 1000000ll ) {
                mBandwidthkBps =    mTotalBandwidthEntrySize*1000/(nowUs - addMinUs) ;
                mLastBandwidthCalcTimeUs = nowUs;
        }
    }

}


bool  Renderer::DecoderContext::checkAndGetBandwidthKbps(int32_t* kbps){
    *kbps = mBandwidthkBps;
    if(*kbps < DECODER_BW_THRESHOLD_LOW){
        return true;
    }
    return false;
}

status_t Renderer::DecoderContext::setAvpfParamters(const sp<AMessage> &params)
{
    sp<AMessage> param = new AMessage;

    param->setInt32("avpf", true);
    param->setMessage("avpf-msg", params);
    mDecoder->setParameters(params);
    return OK;
}
*/

void Renderer::DecoderContext::notifyError(int32_t err)
{
    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what", kWhatError);
    notify->setInt32("err", err);
    notify->post();

    /*//temp no need
        //remove some frame before last I
        char value[PROPERTY_VALUE_MAX];
        int enable_drop = 0;

        if(property_get("persist.vendor.vt.render.drop", value, NULL)) {
            enable_drop = atoi(value);
        }

        VT_LOGD("[ID=%d] enable_drop=%d", mMultiInstanceID, enable_drop);

        if(enable_drop == 0) return;

        size_t latestIIndex = 0;
        int32_t IDR = 0;
        int32_t IDRCount = 0;

        if(mAccessUnits.size() > 60) {
            for(size_t i = 0; i < mAccessUnits.size(); i++) {
                IDR = 0;
                (*mAccessUnits.begin())->meta()->findInt32("is-sync-frame",&IDR);

                if(IDR == 1) {
                    IDRCount++;
                    latestIIndex = i;

                }
            }

            if(IDRCount == 0) return;

            size_t left = mAccessUnits.size() - latestIIndex;
            VT_LOGW("total IDR %d,latest one is %zu,left %zu",IDRCount,latestIIndex,left);

            while(mAccessUnits.size() > left) {
                int32_t IDR = 0;
                (*mAccessUnits.begin())->meta()->findInt32("is-sync-frame",&IDR);

                int64_t timeUs =-1;
                CHECK((*mAccessUnits.begin())->meta()->findInt64("timeUs",&timeUs));

                int32_t token =0;
                (*mAccessUnits.begin())->meta()->findInt32("token",&token);

                if(IDR == 0) {
                    mAccessUnits.erase(mAccessUnits.begin());
                }

                VT_LOGW("[ID=%d]remove buffer token %d timeUs %" PRId64 " ms,IDR %d",
                        mMultiInstanceID,token,timeUs,IDR);
            }


        }

    */

}

////////////////////////////////////////////////////////////////////////////////

Renderer::AudioRenderer::AudioRenderer(
    const sp<DecoderContext> &DecoderContext,
    sp<AMessage> notify)
    : mDecoderContext(DecoderContext),
      mNotify(notify),
      mStartFeedBackPCM(false),
      mPushPending(false),
      mNumFramesWritten(0),
      mLatencyUs(-1)
{
    mAudioTrack = new AudioTrack(
        AUDIO_STREAM_DEFAULT,
        48000.0f,
        AUDIO_FORMAT_PCM,
        AUDIO_CHANNEL_OUT_STEREO,
        (int) 0 /* frameCount */);

    CHECK_EQ((status_t) OK, mAudioTrack->initCheck());
    mLatencyUs = (int64_t) mAudioTrack->latency() * 1000;
    //mFrameSize = mAudioTrack->frameSize();
    mAudioTrack->start();
    VT_LOGI("create AudioRenderer:mLatencyUs %lld us ", (long long) mLatencyUs);
}

Renderer::AudioRenderer::~AudioRenderer()
{
}

void Renderer::AudioRenderer::queueInputBuffer(
    size_t index, int64_t timeUs, const sp<MediaCodecBuffer> &buffer)
{
    BufferInfo info;
    info.mIndex = index;
    info.mTimeUs = timeUs;
    info.mBuffer = buffer;

    mInputBuffers.push_back(info);
    schedulePushIfNecessary();
}

void Renderer::AudioRenderer::onMessageReceived(
    const sp<AMessage> &msg)
{
    switch(msg->what()) {
    case kWhatPushAudio: {
        onPushAudio();
        break;
    }

    default:
        break;
    }
}

void Renderer::AudioRenderer::schedulePushIfNecessary()
{
    VT_LOGV("AudioRenderer:mPushPending %d,mInputBuffers.size %zu",mPushPending,mInputBuffers.size());

    if(mPushPending || mInputBuffers.empty()) {
        return;
    }

    mPushPending = true;

    uint32_t numFramesPlayed;
    CHECK_EQ(mAudioTrack->getPosition(&numFramesPlayed),
             (status_t) OK);

    uint32_t numFramesPendingPlayout = mNumFramesWritten - numFramesPlayed;

    // This is how long the audio sink will have data to
    // play back.

    if(mAudioTrack->getSampleRate() != 0) {
        const float msecsPerFrame = 1000.0f / mAudioTrack->getSampleRate();

        int64_t delayUs =
            msecsPerFrame * numFramesPendingPlayout * 1000ll;

        // Let's give it more data after about half that time
        // has elapsed.
        VT_LOGV("audioRender:kWhatPushAudio delay %" PRId64 " us",delayUs /2);
        (new AMessage(kWhatPushAudio, this))->post(delayUs / 2);
    } else {
        ALOGE("schedulePushIfNecessary  getSampleRate == 0");
        return ;
    }

}

int64_t  Renderer::AudioRenderer::getAudioPlayOutTimeUs()
{

    uint32_t numFramesPlayed;
    CHECK_EQ(mAudioTrack->getPosition(&numFramesPlayed), (status_t) OK);
    VT_LOGD("numFramesPlayed %d",numFramesPlayed);
    int64_t result = -1;

    if(mAudioTrack->getSampleRate() != 0) {
        result = -mLatencyUs + (numFramesPlayed * 1000000ll) / mAudioTrack->getSampleRate();
    }

    return result;
}
void Renderer::AudioRenderer::dequeueTrackAccessUnit()
{
    mStartFeedBackPCM = true;

}
void Renderer::AudioRenderer::onPushAudio()
{
    VT_LOGV("onPushAudio +");
    mPushPending = false;

    while(!mInputBuffers.empty()) {
        const BufferInfo &info = *mInputBuffers.begin();

        ssize_t n = writeNonBlocking(
                        info.mBuffer->data(), info.mBuffer->size());

        if(n < (ssize_t) info.mBuffer->size()) {
            CHECK_GE(n, 0);

            info.mBuffer->setRange(
                info.mBuffer->offset() + n, info.mBuffer->size() - n);
            break;
        }

        if(mStartFeedBackPCM) {
            sp<ABuffer> buffer = new ABuffer(info.mBuffer->size());
            buffer->meta()->setInt64("timeUs", info.mTimeUs);
            memcpy(buffer->data(),
                   info.mBuffer->data(),
                   (info.mBuffer->size()));

            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("what",Renderer::kWhatOutPCM);
            notify->setBuffer("accessUnit", buffer);
            notify->setInt32("isVideo", 0);
            notify->post();

        }

        mDecoderContext->releaseOutputBuffer(info.mIndex);

        mInputBuffers.erase(mInputBuffers.begin());
    }

    schedulePushIfNecessary();
    VT_LOGV("onPushAudio -");
}

ssize_t Renderer::AudioRenderer::writeNonBlocking(
    const uint8_t *data, size_t size)
{
    uint32_t numFramesPlayed;
    status_t err = mAudioTrack->getPosition(&numFramesPlayed);

    if(err != OK) {
        VT_LOGE("writeNonBlocking fail err %d",err);
        return err;
    }

    ssize_t numFramesAvailableToWrite =
        mAudioTrack->frameCount() - (mNumFramesWritten - numFramesPlayed);

    size_t numBytesAvailableToWrite =
        numFramesAvailableToWrite * mAudioTrack->frameSize();

    if(size > numBytesAvailableToWrite) {
        size = numBytesAvailableToWrite;
    }

    CHECK_EQ(mAudioTrack->write(data, size), (ssize_t) size);

    size_t numFramesWritten = size / mAudioTrack->frameSize();
    mNumFramesWritten += numFramesWritten;
    VT_LOGE("writeNonBlocking mNumFramesWritten %zu",mNumFramesWritten);

    return size;
}

////////////////////////////////////////////////////////////////////////////////

Renderer::Renderer(
    int32_t multiId,const sp<IGraphicBufferProducer> &bufferProducer, const sp<AMessage> &notify)
    : mMultiInstanceID(multiId),
      mSurface(new Surface(bufferProducer,/*controlledByApp*/ true)),       //async queue
      mNotify(notify),
      mVideoRenderPending(false),
      mFeebBackAudioPCM(false),
      mNumFramesDrop(0),
      mNumFrames(0),
      mAudioRealTimeUs(-1),
      mFirstVideoMediaTimeUs(-1),
      mFirstVideoComeSysTimeUs(-1),
      mFirstVideoTimeBaseValid(false),
      mCurrentVideoHow(VIDEO_SHOW),
      mLastVideoHow(VIDEO_SHOW),
      mContinuouslyDropThreshold(1),
      mContinuouslyDropCnt(0),
      mVideoTrackIndex(-1),
      mRotationDegree(0),
      mRotateInside(false),
      mNTPIsValid(false)
{
    VT_LOGI("[ID=%d]Renderer++",mMultiInstanceID);

    //av sync property map
    char value[PROPERTY_VALUE_MAX];
    int MAMap = 0;
    int TestMode = 0;
    mEnableAvsync = true;
    mPushBlank = 1;

    if(property_get("persist.vendor.vt.OPTest_MM", value, NULL)) {
        TestMode = atoi(value);
    }

    memset(value, 0, sizeof(value));
    if(property_get("persist.vendor.vt.OPTest_MA", value, NULL)) {
        MAMap = strtol(value,NULL,16);;
        if(TestMode){
            mEnableAvsync = (MAMap & 0x1) ? false : true;
	     mPushBlank = (MAMap & 0x2) ? 0 : 1;
        }
    }


    int blank_disable = (mPushBlank == 1 ? 0 : 1);

    if(property_get("persist.radio.blank.disable", value, NULL)) {
        blank_disable = atoi(value);
	 if(blank_disable)  mPushBlank = 0;
   }


    VT_LOGI("OPTest.MA TestMode=%d MAMap=0x%x 0x1=%d mEnableAvsync=%d,mPushBlank=%d",
    			TestMode, MAMap, (MAMap & 0x1),mEnableAvsync,mPushBlank);

}

Renderer::~Renderer()
{
    VT_LOGI("[ID=%d]delete--",mMultiInstanceID);

    if(mDecoderContext[0].get() != NULL) {
        VT_LOGI("[ID=%d]clear decoder audio--",mMultiInstanceID);
        mDecoderContext[0].clear();
        mDecoderContext[0] = NULL;
    }

    if(mDecoderContext[1].get() != NULL) {
        VT_LOGI("[ID=%d]clear decoder video--",mMultiInstanceID);
        mDecoderContext[1].clear();
        mDecoderContext[1] = NULL;
    }


    if(mPushBlank){
       status_t err = native_window_api_connect(mSurface.get(), NATIVE_WINDOW_API_MEDIA);
       if(err != OK) {
           VT_LOGW("[push blank]native_window_api_connect returned an error: %s (%d)", strerror(-err), err);
       }
       pushBlankBuffersToNativeWindow(static_cast<ANativeWindow *>(mSurface.get()));
    }

    VT_LOGI("[ID=%d]mPushBlank %d & clear surface for handle--",mMultiInstanceID, mPushBlank);

    if(mSurface != NULL) {
        status_t err = native_window_api_disconnect(mSurface.get(), NATIVE_WINDOW_API_MEDIA);

        if(err != OK) {
            VT_LOGW("[just handle,not error]native_window_api_disconnect returned an error: %s (%d)", strerror(-err), err);
        }

        mSurface.clear();
    }
}

status_t Renderer::stop(int32_t pushBlank){
	mPushBlank = pushBlank;
    VT_LOGI("[ID=%d] stop-mPushBlank %d+++ mNumFrames=%d",mMultiInstanceID,mPushBlank,mNumFrames);

	return OK;
}

void Renderer::onMessageReceived(const sp<AMessage> &msg)
{
    switch(msg->what()) {
    case kWhatDecoderNotify: {
        onDecoderNotify(msg);
        break;
    }

    case kWhatRenderVideo: {
        onRenderVideo();
        break;
    }

    case kWhatQueueAccessUnit: {
        onQueueAccessUnit(msg);
        break;
    }
    case kWhatSetFormat: {
        onSetFormat(msg);
        break;
    }
/*
    case kWhatSetAvpfParams: {
        sp<AMessage>  params =  NULL;
        int32_t isVideo = 0;
        CHECK(msg->findInt32("isVideo", &isVideo));
        CHECK(msg->findMessage("avpf-params",&params));
        mDecoderContext[isVideo]->setAvpfParamters(params);
        break;
    }
*/

    case kWhatScanDisplayStatus: {
        int32_t lastNum = 0;
        int32_t lastShow = 0;
        int32_t loop = 0;

        CHECK(msg->findInt32("lastNum", &lastNum));
        CHECK(msg->findInt32("lastShow",&lastShow));
        CHECK(msg->findInt32("loop",&loop));

        int32_t nowShow = lastShow;
        int32_t scanDelayPlus = 2;

        if(mNumFrames > lastNum && lastShow == 0) {
            nowShow = 1;
        } else if(mNumFrames <= lastNum && lastShow == 1) {
            if(loop >= PAUSE_DELAY_LOOP) {
                nowShow = 0;
            } else {
                loop++;//only add here
                VT_LOGD("[wait to pause]mNumFrames %d,nowShow %d ~~lastNum %d  lastShow %d  loop %d",mNumFrames,nowShow ,lastNum,lastShow,loop);    //may define different error code in future
            }

            //scanDelayPlus = 1;//no need, maybe future can fine tune
        } else if(mNumFrames > lastNum && lastShow == 1) {    //keep showing
            loop = 0 ;
        } else if(mNumFrames <= lastNum && lastShow == 0) {    //keep no showing
            loop = 0 ;
            VT_LOGD("[keep pause]mNumFrames %d,nowShow %d ~~lastNum %d  lastShow %d  loop %d",mNumFrames,nowShow ,lastNum,lastShow,loop);    //may define different error code in future
        }

        if(nowShow != lastShow) {    //change show state
            VT_LOGD("[change]mNumFrames %d,nowShow %d !=lastNum %d  lastShow %d  loop %d",mNumFrames,nowShow ,lastNum,lastShow,loop);    //may define different error code in future
            sp<AMessage> msg = mNotify->dup();
            msg->setInt32("what", kWhatPeerDisplayStatus);
            msg->setInt32("show", nowShow);
            msg->post();
            loop = 0;
        }

        msg->setInt32("lastNum", mNumFrames);
        msg->setInt32("lastShow", nowShow);
        msg->setInt32("loop", loop);
        msg->post(500000*scanDelayPlus);
        break;
    }

    default:
        VT_LOGE("[ID=%d]%s",mMultiInstanceID,msg->debugString(0).c_str());
        TRESPASS();
    }
}
void Renderer::dequeueTrackAccessUnit(int32_t isVideo)
{
    CHECK(!isVideo);
    mFeebBackAudioPCM = true;
}
status_t Renderer::setFormat(int32_t isVideo, const sp<AMessage> &format)
{
    /*    sp<AMessage> msg = new AMessage(kWhatSetFormat, this);
        msg->setInt32("isVideo", isVideo);
        msg->setMessage("format", format);
        msg->post();*/

    return (internalSetFormat(isVideo, format));
}

status_t Renderer::onSetFormat(const sp<AMessage> &msg)
{
    int32_t isVideo = 0;
    CHECK(msg->findInt32("isVideo", &isVideo));

    sp<AMessage> format;
    CHECK(msg->findMessage("format", &format));

    status_t err = internalSetFormat(isVideo, format);
    return err;
}

status_t Renderer::internalSetFormat(
    int32_t isVideo,const sp<AMessage> &format)
{
    CHECK_LT(isVideo, 2);
    status_t err = OK;

    VT_LOGI("[ID=%d]SetFormat: '%s' format is %s",mMultiInstanceID,isVideo?"video":"audio",format->debugString(0).c_str());
    CHECK(mDecoderContext[isVideo] == NULL);

    sp<AMessage> notify = new AMessage(kWhatDecoderNotify, this);
    notify->setInt32("isVideo", isVideo);

    mDecoderContext[isVideo] = new DecoderContext(mMultiInstanceID,notify);

    looper()->registerHandler(mDecoderContext[isVideo]);

    if(isVideo == 1) {
        mVideoTrackIndex  = isVideo;
    }


    err = mDecoderContext[isVideo]->init(format, isVideo == 1 ? mSurface : NULL) ;

    if(err != OK) {
        VT_LOGE("[ID=%d]mDecoderContext init fail  %d ",mMultiInstanceID,err);
        mDecoderContext[isVideo].clear();
        mDecoderContext[isVideo] = NULL;
        return err;
    }

    if(isVideo != 1) {
        // Audio
        mAudioRenderer = new AudioRenderer(mDecoderContext[isVideo],mNotify);
        looper()->registerHandler(mAudioRenderer);
    }

    /* not real data start point
    sp<AMessage> msg = new AMessage(kWhatScanDisplayStatus, this);
    msg->setInt32("lastNum", 0);
    msg->setInt32("lastShow", 1);
    msg->post(2000000);*/
    return OK;
}


void Renderer::queueAccessUnit(
    int32_t isVideo, const sp<ABuffer> &accessUnit)
{
    ATRACE_CALL();
    sp<AMessage> msg = new AMessage(kWhatQueueAccessUnit, this);
    msg->setInt32("isVideo", isVideo);
    msg->setBuffer("accessUnit", accessUnit);
    msg->post();


    int32_t token =0;

    if(accessUnit->meta()->findInt32("token",&token)) {
        ATRACE_ASYNC_END("SNK-RND", token);
        ATRACE_ASYNC_BEGIN("RND-DCT", token);
    }
}

void Renderer::onQueueAccessUnit(const sp<AMessage> &msg)
{
    ATRACE_CALL();
    int32_t isVideo = 0;
    CHECK(msg->findInt32("isVideo", &isVideo));

    sp<ABuffer> accessUnit;
    CHECK(msg->findBuffer("accessUnit", &accessUnit));

    CHECK_LT(isVideo, 2);
    CHECK(mDecoderContext[isVideo].get() != NULL);

    int32_t degree = 0;

    if(isVideo && accessUnit->meta()->findInt32("rotation-degrees", &degree)) {
        mRotateInside = true;
        resetSurfaceTransformDegree(degree);    //UT  use
    }

    //save timestamp info
    if(isVideo) {
        int64_t ntp = 0;
        int64_t timeUs = 0;
        CHECK(accessUnit->meta()->findInt64("ntp", &ntp));
        CHECK(accessUnit->meta()->findInt64("timeUs", &timeUs));
        CHECK(accessUnit->meta()->findInt32("ntp-valid", &mNTPIsValid));

        if(mNTPIsValid) {
            mVideoTimeUsNTPMaps.add(timeUs, ntp);
        }
    }


    mDecoderContext[isVideo]->queueInputBuffer(accessUnit);
}


void Renderer::onDecoderNotify(const sp<AMessage> &msg)
{
    ATRACE_CALL();
    int32_t  isVideo;
    CHECK(msg->findInt32("isVideo", &isVideo));

    int32_t what;
    CHECK(msg->findInt32("what", &what));
    VT_LOGV("[ID=%d]Renderer::onDecoderNotify:isVideo %d what %d",mMultiInstanceID,isVideo,what);

    switch(what) {
    case DecoderContext::kWhatOutputBufferReady: {
        size_t index;
        CHECK(msg->findSize("index", &index));

        int64_t timeUs;
        CHECK(msg->findInt64("timeUs", &timeUs));

        sp<RefBase> obj;
        CHECK(msg->findObject("buffer", &obj));
        sp<MediaCodecBuffer> buffer = static_cast<MediaCodecBuffer *>(obj.get());


        queueOutputBuffer(isVideo, index, timeUs, buffer);
        break;
    }
    /*case DecoderContext::kWhatDecoderBWNotify:
       {
    int32_t  bandwidth,isVideo;
    CHECK(msg->findInt32("bandwidth", &bandwidth));
    CHECK(msg->findInt32("isVideo", &isVideo));
    VT_LOGD("[ID=%d]mIsVideo=%d,bandwidth=%d",isVideo,bandwidth);

    sp<AMessage> msg = mNotify->dup();
    msg->setInt32("what", kWhatDecodeStatus);
    msg->setInt32("bandwidth", bandwidth);
    msg->setInt32("isVideo", isVideo);
    msg->post();
             break;
       }*/
    case DecoderContext::kWhatOutputFormatChanged: {
        sp<AMessage>   format ;
        CHECK(msg->findMessage("format", &format));
        CHECK(msg->findInt32("isVideo", &isVideo));
        VT_LOGD("[ID=%d]mIsVideo=%d,kWhatOutputFormatChanged",mMultiInstanceID,isVideo);
        int32_t width, height;
        CHECK(format->findInt32("width", &width));
        CHECK(format->findInt32("height", &height));
        resetSurfaceTransformDegree(mRotationDegree, true);
        sp<AMessage> msg = mNotify->dup();
        msg->setInt32("what", kWhatVideoSizeChanged);
        msg->setInt32("width", width);
        msg->setInt32("height", height);
        msg->setInt32("isVideo", isVideo);
        msg->post();
        break;
    }
    case DecoderContext::kWhatError: {
        int32_t err =0;
        CHECK(msg->findInt32("err", &err));
        notifyError(err);
        break;
    }
    default:
        VT_LOGE("[ID=%d] '%s'  is %s",mMultiInstanceID,isVideo?"video":"audio",msg->debugString(0).c_str());
        TRESPASS();
    }
}

void Renderer::queueOutputBuffer(
    int32_t  isVideo,
    size_t index, int64_t timeUs, const sp<MediaCodecBuffer> &buffer)
{
    ATRACE_CALL();
    VT_LOGV("Renderer::queueOutputBuffer:isVideo %d index %zu  timeUs %lld us",isVideo,index, (long long) timeUs);

    if(!isVideo) {
        // Audio
        mAudioRenderer->queueInputBuffer(index, timeUs, buffer);
        return;
    }

    if(!mFirstVideoTimeBaseValid) {

        mFirstVideoMediaTimeUs =timeUs;
        mFirstVideoComeSysTimeUs = ALooper::GetNowUs();
        mFirstVideoTimeBaseValid=true;
        VT_LOGI("[ID=%d]1th video frame is out :mFirstVideoMediaTimeUs %" PRId64 " us,mFirstVideoComeSysTimeUs %" PRId64 " us",
                mMultiInstanceID,mFirstVideoMediaTimeUs,mFirstVideoComeSysTimeUs);

        sp<AMessage> msg = mNotify->dup();
        msg->setInt32("what", kWhatPeerDisplayStatus);
        msg->setInt32("show", 1);
        msg->post();
        //kick off scan now
        sp<AMessage> msg1 = new AMessage(kWhatScanDisplayStatus, this);
        msg1->setInt32("lastNum", mNumFrames);
        msg1->setInt32("lastShow", 1);
        msg1->setInt32("loop", 0);
        msg1->post(1000000);
    }


    OutputInfo info;
    info.mIndex = index;
    info.mTimeUs = timeUs;
    info.mBuffer = buffer;
    mVideoOutputBuffers.push_back(info);
    scheduleVideoRenderIfNecessary();


}


status_t  Renderer::getAudioPlayOutTimeUs(int64_t videotime,int64_t* audioTime,int64_t* audioNTP)
{
    ATRACE_CALL();
    //this is a audio path
    VT_LOGV("[ID=%d]Renderer::getAudioPlayOutTimeUs+ : videotime %" PRId64 " us",mMultiInstanceID,videotime);
    status_t err = OK;

    if(mAudioRenderer.get() != NULL) {
        VT_LOGV("[ID=%d]Renderer::getAudioPlayOutTimeUs from audioRender +",mMultiInstanceID);
        *audioTime =   mAudioRenderer->getAudioPlayOutTimeUs();
        VT_LOGV("[ID=%d]Renderer::getAudioPlayOutTimeUs from audioRender-  audioTime %" PRId64 " us",mMultiInstanceID,*audioTime);

    } else {
        //get audioNTP
        *audioTime =0;
        VTService::VTAVSync::getDLAudioTimeStamp(audioTime);
        *audioNTP = *audioTime;//record audio ntp for trace log

        int64_t videoNTP =0;

        //check VIdeo NTP
        if(mNTPIsValid && mVideoTimeUsNTPMaps.size() != 0) {
            if(mVideoTimeUsNTPMaps.indexOfKey(videotime) < 0) {
                VT_LOGE("[ID=%d]may be NTP is invalid or  lost for videoTimeUs %" PRId64 " us",mMultiInstanceID,videotime);
            } else {
                videoNTP = mVideoTimeUsNTPMaps.valueFor(videotime);
            }
        }

        //check audio NTP

        if(*audioTime == 0 || videoNTP ==0 || llabs(*audioTime - videoNTP) >  AVSYNC_THRESHOLD) {       //audio ntp is not valid, free run
            *audioTime =videotime;
        }
    }

    return err;
}

void Renderer::scheduleVideoRenderIfNecessary(int64_t delayUs)
{
    if(mVideoRenderPending || mVideoOutputBuffers.empty()) {
        return;
    }

    mVideoRenderPending = true;

    (new AMessage(kWhatRenderVideo, this))->post(delayUs);
}
void Renderer::onRenderVideo()
{
    mVideoRenderPending = false;
    ATRACE_CALL();
    //int64_t nowUs = ALooper::GetNowUs();

    status_t err = OK;


    int64_t delayUs =10000ll;//default schedule

    while(!mVideoOutputBuffers.empty()) {
        const OutputInfo &info = *mVideoOutputBuffers.begin();
        int64_t audioTimeUs = -1;
        int64_t  audioNTP = 0;

        //get time info
        int64_t videoTimeUs = info.mTimeUs;
        char value[PROPERTY_VALUE_MAX];
        bool avSyncType = mEnableAvsync;

        if(property_get("persist.vendor.vt.sink.avsync.enable", value, NULL)) {
            avSyncType = atoi(value);
        }

        if(property_get("vendor.vt.sink.avsync.drop.threshold", value, NULL)) {
            mContinuouslyDropThreshold = atoi(value);
        }


        if(avSyncType) {
            VT_LOGV("[ID=%d]getAudioPlayOutTimeUs+: videoTimeUs=%" PRId64 " ms",mMultiInstanceID,videoTimeUs/1000);
            err =getAudioPlayOutTimeUs(videoTimeUs,&audioTimeUs,&audioNTP);    //have the valid ref time
            VT_LOGV("[ID=%d]getAudioPlayOutTimeUs-: audioTimeUs=%" PRId64 " ms",mMultiInstanceID,audioTimeUs/1000);
        }

        if(err == OK && avSyncType) {    //modem from 0 mean valid

            delayUs = videoTimeUs-audioTimeUs;
            VT_LOGV("[ID=%d][AVSync@AudioTimeStamp]:videoTimeUs %" PRId64 " ,audioTimeUs %" PRId64 ",delayUs %" PRId64 "",
                    mMultiInstanceID,videoTimeUs,audioTimeUs,delayUs);


        } else { //free run for UT
            CHECK(mFirstVideoTimeBaseValid);
            //int64_t nowUs = ALooper::GetNowUs();

            //int64_t shouldUs=mFirstVideoComeSysTimeUs + (videoTimeUs-mFirstVideoMediaTimeUs);

            //delayUs = shouldUs - nowUs ;//early time should delay
            //VT_LOGV("[ID=%d][AVSync@systime]:videoTimeUs %" PRId64 " ,shouldUs %" PRId64 ",nowUs %" PRId64 ",delayUs %" PRId64 "",
            //  mMultiInstanceID,videoTimeUs,shouldUs,nowUs,delayUs);
            delayUs = 0;

        }

        //check av sync

        if(delayUs >  20000ll) {    //ealry  more than 10m
            mCurrentVideoHow=VIDEO_DELAY;
	     if(mLastVideoHow != VIDEO_DELAY){
                 VT_LOGD("[ID=%d][AVSync][ms]vNTP %" PRId64 ",aNTP %" PRId64 ", aNTPO %" PRId64 ",video early %" PRId64 " ms,show %d ",
                         mMultiInstanceID, videoTimeUs/1000, audioTimeUs/1000,audioNTP/1000,delayUs/1000,mCurrentVideoHow);
            }
            delayUs = delayUs/2;
        } else if(delayUs < -70000ll) {    //late more than 70ms
            //if(mContinuouslyDropCnt > mContinuouslyDropThreshold){
            mCurrentVideoHow =VIDEO_SHOW;
            //}else{
            //mCurrentVideoHow =VIDEO_DROP;
            //}
            VT_LOGD("[ID=%d][AVSync][ms]vNTP %" PRId64 ",aNTP %" PRId64 ", aNTPO %" PRId64 ",video late %" PRId64 " ms,show %d ",
                    mMultiInstanceID, videoTimeUs/1000, audioTimeUs/1000,audioNTP/1000,delayUs/1000,mCurrentVideoHow);
            delayUs =0 ;

        } else {
            mCurrentVideoHow =VIDEO_SHOW;
	      if(mLastVideoHow != VIDEO_DELAY){
                 VT_LOGD("[ID=%d][AVSync][ms]vNTP %" PRId64 ",aNTP %" PRId64 ",aNTPO %" PRId64 ",video normal %" PRId64 " ms,show %d ",
                        mMultiInstanceID, videoTimeUs/1000, audioTimeUs/1000,audioNTP/1000,delayUs/1000,mCurrentVideoHow);
		}
            delayUs =0 ;
        }

        switch(mCurrentVideoHow) {
        case VIDEO_SHOW: {
            mNumFrames++;
            mNumFramesDrop =0;
            mContinuouslyDropCnt = 0;
            err =mDecoderContext[mVideoTrackIndex]->renderOutputBufferAndRelease(info.mIndex /*maybe add timeNs*/);

            int32_t token =0;

            if(info.mBuffer->meta()->findInt32("token",&token)) {
                ATRACE_ASYNC_END("RED-SUR", token);
            }

            mVideoOutputBuffers.erase(mVideoOutputBuffers.begin());
            break;
        }

        case VIDEO_DELAY: {
            break;
        }

        case VIDEO_DROP: {
            mContinuouslyDropCnt++;
            err =mDecoderContext[mVideoTrackIndex]->releaseOutputBuffer(info.mIndex);

            int32_t token =0;

            if(info.mBuffer->meta()->findInt32("token",&token)) {
                ATRACE_ASYNC_END("RED-SUR", token);
            }

            mVideoOutputBuffers.erase(mVideoOutputBuffers.begin());


            break;
        }
        }

        //CHECK_EQ(err, (status_t)OK);
        if(err != OK) {    //
            VT_LOGE("[ID=%d]err == %d delete peer surface before stop sink@releaseRender",mMultiInstanceID,err);
            notifyError(err);
            delayUs = ERROR_DELAY_US;
            break;
        }

        mLastVideoHow =mCurrentVideoHow;

        if(mCurrentVideoHow == VIDEO_DELAY) {
            break;
        } else {
            mVideoTimeUsNTPMaps.removeItem(videoTimeUs);    //maybe should not use this, just record the 1th valid timestamp
        }
    }

    scheduleVideoRenderIfNecessary(delayUs);
}

status_t Renderer::resetSurfaceTransformDegree(int32_t degree, bool force)
{
    ATRACE_CALL();

    VT_LOGI("[ID=%d]mRotateInside %d ,degree change from %d to %d",mMultiInstanceID,mRotateInside,mRotationDegree,degree);

    if(!mRotateInside) {
        return OK;
    }

    if((!force) && (degree == mRotationDegree)) {
        VT_LOGW("[ID=%d] do not rotate, same degree",mMultiInstanceID);
        return OK;
    }

    status_t err = OK;

    uint32_t transform = 0;

    switch(degree) {
    case 0:
        /*
                        switch(mRotationDegree){
                            case 90:
                                transform = HAL_TRANSFORM_ROT_270;
                            break;
                            case 180:
                                transform = HAL_TRANSFORM_ROT_180;
                            break;
                            case 270:
                                transform = HAL_TRANSFORM_ROT_90;
                            break;
                        }
        */
        transform = 0;
        VT_LOGE("[ID=%d] rotate back to 0",mMultiInstanceID);
        break;
    case 90:
        transform = HAL_TRANSFORM_ROT_90;
        break;
    case 180:
        transform = HAL_TRANSFORM_ROT_180;
        break;
    case 270:
        transform = HAL_TRANSFORM_ROT_270;
        break;
    default:
        transform = 0;
        break;
    }

    mRotationDegree = degree;

    //if (transform >= 0) {
    VT_LOGE("[ID=%d] rotate option %d",mMultiInstanceID, transform);
    err = native_window_set_buffers_transform(
              static_cast<ANativeWindow *>(mSurface.get()), transform);

    if(err != 0) {
        VT_LOGE("[ID=%d]native_window_set_buffers_transform failed: %s (%d)",
                mMultiInstanceID,strerror(-err), -err);
    }

    //}
    return err;
}

/*
//for avpf
status_t Renderer::setAvpfParamters(const sp<AMessage> &params)
{
    sp<AMessage>  msg =  new AMessage(kWhatSetAvpfParams, this) ;
    msg->setInt32("isVideo", 1);
    msg->setMessage("avpf-params",params);
    msg->post();
    return OK;
}
*/

void Renderer::notifyError(int32_t err)
{
    VT_LOGE("err= %d",err);    //may define different error code in future
    sp<AMessage> msg = mNotify->dup();
    msg->setInt32("what", kWhatError);
    msg->setInt32("err", err);
    msg->post();

}
}  // namespace android




